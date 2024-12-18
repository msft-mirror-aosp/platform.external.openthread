/*
 *  Copyright (c) 2022, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file includes the implementation for the IPC Binder interface to radio Co-Processor (RCP).
 */

#include "hal_interface.hpp"

#if OPENTHREAD_POSIX_CONFIG_RCP_BUS == OT_POSIX_RCP_BUS_VENDOR

#include <linux/gpio.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>
#include <sys/select.h>

#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "common/code_utils.hpp"

namespace ot {
namespace Posix {
using ::aidl::android::hardware::threadnetwork::IThreadChip;
using ::aidl::android::hardware::threadnetwork::IThreadChipCallback;
using ::ndk::ScopedAStatus;

using ot::Spinel::SpinelInterface;

HalInterface::HalInterface(const Url::Url &aRadioUrl)
    : mRxFrameCallback(nullptr)
    , mRxFrameContext(nullptr)
    , mRxFrameBuffer(nullptr)
    , mThreadChip(nullptr)
    , mThreadChipCallback(nullptr)
    , mDeathRecipient(AIBinder_DeathRecipient_new(BinderDeathCallback))
    , mBinderFd(-1)
    , mHalInterfaceId(0)
{
    IgnoreError(aRadioUrl.ParseUint8("id", mHalInterfaceId));
    memset(&mInterfaceMetrics, 0, sizeof(mInterfaceMetrics));
    mInterfaceMetrics.mRcpInterfaceType = kSpinelInterfaceTypeVendor;

    VerifyOrDie(ABinderProcess_setupPolling(&mBinderFd) == ::STATUS_OK, OT_EXIT_FAILURE);
    VerifyOrDie(mBinderFd >= 0, OT_EXIT_FAILURE);
}

otError HalInterface::Init(SpinelInterface::ReceiveFrameCallback aCallback,
                           void                                 *aCallbackContext,
                           SpinelInterface::RxFrameBuffer       &aFrameBuffer)
{
    static const std::string            kServicePrefix = std::string() + IThreadChip::descriptor + "/chip";
    const char                         *value;
    ScopedAStatus                       ndkStatus;
    std::shared_ptr<ThreadChipCallback> callback;
    std::string                         serviceName = kServicePrefix + std::to_string(mHalInterfaceId);

    otLogInfoPlat("[HAL] Wait for getting the service %s ...", serviceName.c_str());
    mThreadChip = IThreadChip::fromBinder(::ndk::SpAIBinder(AServiceManager_waitForService(serviceName.c_str())));
    VerifyOrDie(mThreadChip != nullptr, OT_EXIT_FAILURE);

    callback = ndk::SharedRefBase::make<ThreadChipCallback>(this);
    VerifyOrDie(callback->asBinder().get() != nullptr, OT_EXIT_FAILURE);

    mThreadChipCallback = IThreadChipCallback::fromBinder(callback->asBinder());
    VerifyOrDie(mThreadChipCallback != nullptr, OT_EXIT_FAILURE);

    VerifyOrDie(AIBinder_linkToDeath(mThreadChip->asBinder().get(), mDeathRecipient.get(), this) == STATUS_OK,
                OT_EXIT_FAILURE);

    ndkStatus = mThreadChip->open(mThreadChipCallback);
    if (!ndkStatus.isOk())
    {
        otLogCritPlat("[HAL] Open the HAL interface failed: %s", ndkStatus.getMessage());
        DieNow(OT_EXIT_FAILURE);
    }

    mRxFrameCallback = aCallback;
    mRxFrameContext  = aCallbackContext;
    mRxFrameBuffer   = &aFrameBuffer;

    otLogInfoPlat("[HAL] Successfully got the service %s", serviceName.c_str());

    return OT_ERROR_NONE;
}

void HalInterface::BinderDeathCallback(void *aContext)
{
    OT_UNUSED_VARIABLE(aContext);

    otLogInfoPlat("[HAL] Thread network HAL is dead, exit!");
    DieNow(OT_EXIT_FAILURE);
}

HalInterface::~HalInterface(void)
{
    Deinit();

    if (mBinderFd >= 0)
    {
        close(mBinderFd);
    }
}

void HalInterface::Deinit(void)
{
    if (mThreadChip != nullptr)
    {
        mThreadChip->close();
        AIBinder_unlinkToDeath(mThreadChip->asBinder().get(), mDeathRecipient.get(), this);
        mThreadChip         = nullptr;
        mThreadChipCallback = nullptr;
    }

    mRxFrameCallback = nullptr;
    mRxFrameContext  = nullptr;
    mRxFrameBuffer   = nullptr;
}

uint32_t HalInterface::GetBusSpeed(void) const
{
    static const uint32_t kBusSpeed = 1000000;
    return kBusSpeed;
}

otError HalInterface::HardwareReset(void) { return StatusToError(mThreadChip->hardwareReset()); }

void HalInterface::UpdateFdSet(void *aMainloopContext)
{
    otSysMainloopContext *context = reinterpret_cast<otSysMainloopContext *>(aMainloopContext);

    assert(context != nullptr);

    if (mBinderFd >= 0)
    {
        FD_SET(mBinderFd, &context->mReadFdSet);
        context->mMaxFd = std::max(context->mMaxFd, mBinderFd);
    }
}

void HalInterface::Process(const void *aMainloopContext)
{
    const otSysMainloopContext *context = reinterpret_cast<const otSysMainloopContext *>(aMainloopContext);

    assert(context != nullptr);

    if ((mBinderFd >= 0) && FD_ISSET(mBinderFd, &context->mReadFdSet))
    {
        ABinderProcess_handlePolledCommands();
    }
}

otError HalInterface::WaitForFrame(uint64_t aTimeoutUs)
{
    otError        error = OT_ERROR_NONE;
    struct timeval timeout;
    fd_set         readFdSet;
    int            ret;

    VerifyOrExit(mBinderFd >= 0, error = OT_ERROR_FAILED);

    timeout.tv_sec  = static_cast<time_t>(aTimeoutUs / OT_US_PER_S);
    timeout.tv_usec = static_cast<suseconds_t>(aTimeoutUs % OT_US_PER_S);

    FD_ZERO(&readFdSet);
    FD_SET(mBinderFd, &readFdSet);

    ret = select(mBinderFd + 1, &readFdSet, nullptr, nullptr, &timeout);

    if ((ret > 0) && FD_ISSET(mBinderFd, &readFdSet))
    {
        ABinderProcess_handlePolledCommands();
    }
    else if (ret == 0)
    {
        ExitNow(error = OT_ERROR_RESPONSE_TIMEOUT);
    }
    else if (errno != EINTR)
    {
        DieNow(OT_EXIT_ERROR_ERRNO);
    }

exit:

    if (error != OT_ERROR_NONE)
    {
        otLogWarnPlat("[HAL] Wait for frame failed: %s", otThreadErrorToString(error));
    }

    return error;
}

otError HalInterface::SendFrame(const uint8_t *aFrame, uint16_t aLength)
{
    otError       error = OT_ERROR_NONE;
    ScopedAStatus status;

    VerifyOrExit((aFrame != nullptr) && (aLength <= kMaxFrameSize), error = OT_ERROR_INVALID_ARGS);
    status = mThreadChip->sendSpinelFrame(std::vector<uint8_t>(aFrame, aFrame + aLength));
    VerifyOrExit(!status.isOk(), error = OT_ERROR_NONE);
    error = StatusToError(status);
    otLogWarnPlat("[HAL] Send frame to HAL interface failed: %s", otThreadErrorToString(error));

exit:
    if (error == OT_ERROR_NONE)
    {
        mInterfaceMetrics.mTxFrameCount++;
        mInterfaceMetrics.mTxFrameByteCount += aLength;
    }

    return error;
}

void HalInterface::ReceiveFrameCallback(const std::vector<uint8_t> &aFrame)
{
    otError error = OT_ERROR_NONE;

    VerifyOrExit(mRxFrameBuffer != nullptr, error = OT_ERROR_FAILED);
    VerifyOrExit(aFrame.size() > 0, error = OT_ERROR_FAILED);

    for (uint32_t i = 0; i < aFrame.size(); i++)
    {
        if ((error = mRxFrameBuffer->WriteByte(aFrame[i])) != OT_ERROR_NONE)
        {
            otLogNotePlat("[HAL] Drop the received spinel frame: %s", otThreadErrorToString(error));
            mRxFrameBuffer->DiscardFrame();
            ExitNow(error = OT_ERROR_NO_BUFS);
        }
    }

    if (mRxFrameCallback != nullptr)
    {
        mRxFrameCallback(mRxFrameContext);
    }

exit:
    if (error == OT_ERROR_NONE)
    {
        mInterfaceMetrics.mRxFrameCount++;
        mInterfaceMetrics.mRxFrameByteCount += aFrame.size();
    }

    return;
}

otError HalInterface::StatusToError(const ScopedAStatus &aStatus) const
{
    otError error = OT_ERROR_FAILED;

    VerifyOrExit(!aStatus.isOk(), error = OT_ERROR_NONE);

    if (aStatus.getExceptionCode() == EX_ILLEGAL_STATE)
    {
        error = OT_ERROR_INVALID_STATE;
    }
    else if (aStatus.getExceptionCode() == EX_ILLEGAL_ARGUMENT)
    {
        error = OT_ERROR_INVALID_ARGS;
    }
    else if (aStatus.getExceptionCode() == EX_UNSUPPORTED_OPERATION)
    {
        error = OT_ERROR_NOT_IMPLEMENTED;
    }
    else if (aStatus.getExceptionCode() == EX_SERVICE_SPECIFIC)
    {
        switch (aStatus.getServiceSpecificError())
        {
        case IThreadChip::ERROR_FAILED:
            error = OT_ERROR_FAILED;
            break;
        case IThreadChip::ERROR_BUSY:
            error = OT_ERROR_BUSY;
            break;
        case IThreadChip::ERROR_NO_BUFS:
            error = OT_ERROR_NO_BUFS;
            break;
        default:
            error = OT_ERROR_FAILED;
            break;
        }
    }

exit:
    return error;
}

} // namespace Posix
} // namespace ot

#endif // OPENTHREAD_POSIX_CONFIG_RCP_BUS == OT_POSIX_RCP_BUS_VENDOR
