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
 *   This file includes the implementation for the vendor defined radio spinel interface
 *   to radio Co-Processor (RCP).
 */

#include "vendor_interface.hpp"

#if OPENTHREAD_POSIX_CONFIG_RCP_BUS == OT_POSIX_RCP_BUS_VENDOR

#include <linux/gpio.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>
#include <sys/select.h>

#include <memory>

#include "hal_interface.hpp"

namespace ot {
namespace Posix {
using ot::Spinel::SpinelInterface;

HalInterface *sHalInterface = nullptr;

VendorInterface::VendorInterface(const Url::Url &aRadioUrl)
{
    sHalInterface = new HalInterface(aRadioUrl);
    assert(sHalInterface != nullptr);
}

otError VendorInterface::Init(ReceiveFrameCallback aCallback, void *aCallbackContext, RxFrameBuffer &aFrameBuffer)
{
    return sHalInterface->Init(aCallback, aCallbackContext, aFrameBuffer);
}

VendorInterface::~VendorInterface(void)
{
    sHalInterface->Deinit();
    delete sHalInterface;
    sHalInterface = nullptr;
}

void VendorInterface::Deinit(void) { sHalInterface->Deinit(); }

uint32_t VendorInterface::GetBusSpeed(void) const { return sHalInterface->GetBusSpeed(); }

void VendorInterface::UpdateFdSet(void *aMainloopContext) { sHalInterface->UpdateFdSet(aMainloopContext); }

void VendorInterface::Process(const void *aMainloopContext) { sHalInterface->Process(aMainloopContext); }

otError VendorInterface::WaitForFrame(uint64_t aTimeoutUs) { return sHalInterface->WaitForFrame(aTimeoutUs); }

otError VendorInterface::SendFrame(const uint8_t *aFrame, uint16_t aLength)
{
    return sHalInterface->SendFrame(aFrame, aLength);
}

otError VendorInterface::HardwareReset(void) { return sHalInterface->HardwareReset(); }

const otRcpInterfaceMetrics *VendorInterface::GetRcpInterfaceMetrics(void) const
{
    return sHalInterface->GetRcpInterfaceMetrics();
}
} // namespace Posix
} // namespace ot

#endif // OPENTHREAD_POSIX_CONFIG_RCP_BUS == OT_POSIX_RCP_BUS_SPI
