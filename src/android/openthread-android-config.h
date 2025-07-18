/*
 *  Copyright (c) 2021, The OpenThread Authors.
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

#include "openthread-config-android-version.h"

/**
 * @def OPENTHREAD_CONFIG_CLI_TX_BUFFER_SIZE
 *
 *  The size of CLI message buffer in bytes
 *
 */
#define OPENTHREAD_CONFIG_CLI_UART_TX_BUFFER_SIZE 3500

/**
 * @def OPENTHREAD_CONFIG_CLI_UART_RX_BUFFER_SIZE
 *
 * The size of CLI UART RX buffer in bytes.
 *
 */
#define OPENTHREAD_CONFIG_CLI_UART_RX_BUFFER_SIZE 3500

/**
 * Enables the default posix infrastructure interface implementation.
 */
#define OPENTHREAD_POSIX_CONFIG_INFRA_IF_ENABLE 1

/**
 * Disables the default posix TUN interface implementation
 * so that we can can use the Android specific implementation.
 */
#define OPENTHREAD_CONFIG_PLATFORM_NETIF_ENABLE 0

/**
 * Enables platform UDP support.
 */
#define OPENTHREAD_CONFIG_PLATFORM_UDP_ENABLE 1

/**
 * Enables CLI for Thread certification.
 */
#define OPENTHREAD_POSIX_CONFIG_DAEMON_CLI_ENABLE 1

/**
 * @def OPENTHREAD_POSIX_CONFIG_THREAD_NETIF_DEFAULT_NAME
 *
 * Define the Thread default network interface name.
 *
 */
#define OPENTHREAD_POSIX_CONFIG_THREAD_NETIF_DEFAULT_NAME "thread-wpan"

/**
 * Avoids killing the ot-daemon process when the infra link disappears.
 */
#define OPENTHREAD_POSIX_CONFIG_EXIT_ON_INFRA_NETIF_LOST_ENABLE 0

/**
 * Enables the posix platform to support power calibration.
 *
 * This flag is needed by the framework API `setChannelTargetPower()` to set the target power of each channel.
 */
#define OPENTHREAD_CONFIG_PLATFORM_POWER_CALIBRATION_ENABLE 1

// Enable the configuration file support for power calibration.
#define OPENTHREAD_POSIX_CONFIG_CONFIGURATION_FILE_ENABLE 1

// Disable the max power table support, this feature is supported by
// OPENTHREAD_POSIX_CONFIG_CONFIGURATION_FILE_ENABLE and
// OPENTHREAD_CONFIG_PLATFORM_POWER_CALIBRATION_ENABLE.
#define OPENTHREAD_POSIX_CONFIG_MAX_POWER_TABLE_ENABLE 0

// Enable the spinel vendor interface to adopt the HAL-based RCP.
#define OPENTHREAD_POSIX_CONFIG_SPINEL_VENDOR_INTERFACE_ENABLE 1

// Using packet filtering in OT core instead of the iptables-based firewall feature.
#define OPENTHREAD_POSIX_CONFIG_FIREWALL_ENABLE 0

// Disable the feature to add external routes to POSIX kernel when external routes are changed in
// netdata.
#define OPENTHREAD_POSIX_CONFIG_INSTALL_EXTERNAL_ROUTES_ENABLE 0

// Disable backtrace support as Android already has support for dumping backtrace after crash.
#define OPENTHREAD_POSIX_CONFIG_BACKTRACE_ENABLE 0

// Enable for Android platform.
#define OPENTHREAD_POSIX_CONFIG_ANDROID_ENABLE 1

// Bind the upstream DNS socket to infra network interface.
#define OPENTHREAD_POSIX_CONFIG_UPSTREAM_DNS_BIND_TO_INFRA_NETIF 1

// Enable TREL to select infra interface
#define OPENTHREAD_POSIX_CONFIG_TREL_SELECT_INFRA_IF 1
