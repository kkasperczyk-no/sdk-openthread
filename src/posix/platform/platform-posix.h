/*
 *  Copyright (c) 2016, The OpenThread Authors.
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
 * @brief
 *   This file includes the platform-specific initializers.
 */

#ifndef PLATFORM_POSIX_H_
#define PLATFORM_POSIX_H_

#include "openthread-posix-config.h"

#include <errno.h>
#include <net/if.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>

#include <openthread/error.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/openthread-system.h>
#include <openthread/platform/time.h>

#include "common/logging.hpp"

#include "radio_url.hpp"
#include "lib/platform/exit_code.h"

/**
 * @def OPENTHREAD_POSIX_VIRTUAL_TIME
 *
 * This setting configures whether to use virtual time.
 *
 */
#ifndef OPENTHREAD_POSIX_VIRTUAL_TIME
#define OPENTHREAD_POSIX_VIRTUAL_TIME 0
#endif

/**
 * This is the socket name used by daemon mode.
 *
 */
#define OPENTHREAD_POSIX_DAEMON_SOCKET_NAME OPENTHREAD_POSIX_CONFIG_DAEMON_SOCKET_BASENAME ".sock"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    OT_SIM_EVENT_ALARM_FIRED        = 0,
    OT_SIM_EVENT_RADIO_RECEIVED     = 1,
    OT_SIM_EVENT_UART_WRITE         = 2,
    OT_SIM_EVENT_RADIO_SPINEL_WRITE = 3,
    OT_EVENT_DATA_MAX_SIZE          = 1024,
};

OT_TOOL_PACKED_BEGIN
struct VirtualTimeEvent
{
    uint64_t mDelay;
    uint8_t  mEvent;
    uint16_t mDataLength;
    uint8_t  mData[OT_EVENT_DATA_MAX_SIZE];
} OT_TOOL_PACKED_END;

struct RadioProcessContext
{
    const fd_set *mReadFdSet;
    const fd_set *mWriteFdSet;
};

/**
 * This function initializes the alarm service used by OpenThread.
 *
 * @param[in]  aSpeedUpFactor   The speed up factor.
 * @param[in]  aRealTimeSignal  The real time signal for microsecond alarms.
 *
 */
void platformAlarmInit(uint32_t aSpeedUpFactor, int aRealTimeSignal);

/**
 * This function retrieves the time remaining until the alarm fires.
 *
 * @param[out]  aTimeval  A pointer to the timeval struct.
 *
 */
void platformAlarmUpdateTimeout(struct timeval *tv);

/**
 * This function performs alarm driver processing.
 *
 * @param[in]  aInstance  The OpenThread instance structure.
 *
 */
void platformAlarmProcess(otInstance *aInstance);

/**
 * This function returns the next alarm event time.
 *
 * @returns The next alarm fire time.
 *
 */
int32_t platformAlarmGetNext(void);

#ifndef MS_PER_S
#define MS_PER_S 1000
#endif
#ifndef US_PER_MS
#define US_PER_MS 1000
#endif
#ifndef US_PER_S
#define US_PER_S (MS_PER_S * US_PER_MS)
#endif
#ifndef NS_PER_US
#define NS_PER_US 1000
#endif

/**
 * This function advances the alarm time by @p aDelta.
 *
 * @param[in]  aDelta  The amount of time to advance.
 *
 */
void platformAlarmAdvanceNow(uint64_t aDelta);

/**
 * This function initializes the radio service used by OpenThread.
 *
 * @note Even when @p aPlatformConfig->mResetRadio is false, a reset event (i.e. a PROP_LAST_STATUS between
 * [SPINEL_STATUS_RESET__BEGIN, SPINEL_STATUS_RESET__END]) is still expected from RCP.
 *
 * @param[in]  aPlatformConfig  Platform configuration structure.
 *
 */
void platformRadioInit(otUrl *aRadioUrl);

/**
 * This function shuts down the radio service used by OpenThread.
 *
 */
void platformRadioDeinit(void);

/**
 * This function shuts down platform network interface.
 *
 */
void platformNetifDeinit(void);

/**
 * This function inputs a received radio frame.
 *
 * @param[in]  aInstance   A pointer to the OpenThread instance.
 * @param[in]  aBuf        A pointer to the received radio frame.
 * @param[in]  aBufLength  The size of the received radio frame.
 *
 */
void platformRadioReceive(otInstance *aInstance, uint8_t *aBuf, uint16_t aBufLength);

/**
 * This function updates the file descriptor sets with file descriptors used by the radio driver.
 *
 * @param[inout]  aReadFdSet   A pointer to the read file descriptors.
 * @param[inout]  aWriteFdSet  A pointer to the write file descriptors.
 * @param[inout]  aMaxFd       A pointer to the max file descriptor.
 * @param[inout]  aTimeout     A pointer to the timeout.
 *
 */
void platformRadioUpdateFdSet(fd_set *aReadFdSet, fd_set *aWriteFdSet, int *aMaxFd, struct timeval *aTimeout);

/**
 * This function performs radio driver processing.
 *
 * @param[in]   aInstance       A pointer to the OpenThread instance.
 * @param[in]   aReadFdSet      A pointer to the read file descriptors.
 * @param[in]   aWriteFdSet     A pointer to the write file descriptors.
 *
 */
void platformRadioProcess(otInstance *aInstance, const fd_set *aReadFdSet, const fd_set *aWriteFdSet);

/**
 * This function initializes the random number service used by OpenThread.
 *
 */
void platformRandomInit(void);

/**
 * This function initializes the logging service used by OpenThread.
 *
 * @param[in] aName   A name string which will be prefixed to each log line.
 *
 */
void platformLoggingInit(const char *aName);

/**
 * This function updates the file descriptor sets with file descriptors used by the UART driver.
 *
 * @param[inout]  aReadFdSet   A pointer to the read file descriptors.
 * @param[inout]  aWriteFdSet  A pointer to the write file descriptors.
 * @param[inout]  aMaxFd       A pointer to the max file descriptor.
 *
 */
void platformUartUpdateFdSet(fd_set *aReadFdSet, fd_set *aWriteFdSet, fd_set *aErrorFdSet, int *aMaxFd);

/**
 * This function performs radio driver processing.
 *
 * @param[in]   aReadFdSet      A pointer to the read file descriptors.
 * @param[in]   aWriteFdSet     A pointer to the write file descriptors.
 * @param[in]   aErrorFdSet     A pointer to the error file descriptors.
 *
 */
void platformUartProcess(const fd_set *aReadFdSet, const fd_set *aWriteFdSet, const fd_set *aErrorFdSet);

/**
 * This function initializes platform netif.
 *
 * @param[in]   aInstance       A pointer to the OpenThread instance.
 * @param[in]   aInterfaceName  A pointer to Thread network interface name.
 *
 */
void platformNetifInit(otInstance *aInstance, const char *aInterfaceName);

/**
 * This function updates the file descriptor sets with file descriptors used by platform netif module.
 *
 * @param[inout]  aReadFdSet    A pointer to the read file descriptors.
 * @param[inout]  aWriteFdSet   A pointer to the write file descriptors.
 * @param[inout]  aErrorFdSet   A pointer to the error file descriptors.
 * @param[inout]  aMaxFd        A pointer to the max file descriptor.
 *
 */
void platformNetifUpdateFdSet(fd_set *aReadFdSet, fd_set *aWriteFdSet, fd_set *aErrorFdSet, int *aMaxFd);

/**
 * This function performs platform netif processing.
 *
 * @param[in]   aReadFdSet      A pointer to the read file descriptors.
 * @param[in]   aWriteFdSet     A pointer to the write file descriptors.
 * @param[in]   aErrorFdSet     A pointer to the error file descriptors.
 *
 */
void platformNetifProcess(const fd_set *aReadFdSet, const fd_set *aWriteFdSet, const fd_set *aErrorFdSet);

/**
 * This function performs notifies state changes to platform netif.
 *
 * @param[in]   aInstance       A pointer to the OpenThread instance.
 * @param[in]   aFlags          Flags that denote the state change events.
 *
 */
void platformNetifStateChange(otInstance *aInstance, otChangedFlags aFlags);

/**
 * This function initialize virtual time simulation.
 *
 * @params[in]  aNodeId     Node id of this simulated device.
 *
 */
void virtualTimeInit(uint16_t aNodeId);

/**
 * This function deinitialize virtual time simulation.
 *
 */
void virtualTimeDeinit(void);

/**
 * This function performs virtual time simulation processing.
 *
 * @param[in]   aInstance       A pointer to the OpenThread instance.
 * @param[in]   aReadFdSet      A pointer to the read file descriptors.
 * @param[in]   aWriteFdSet     A pointer to the write file descriptors.
 *
 */
void virtualTimeProcess(otInstance *  aInstance,
                        const fd_set *aReadFdSet,
                        const fd_set *aWriteFdSet,
                        const fd_set *aErrorFdSet);

/**
 * This function updates the file descriptor sets with file descriptors
 * used by the virtual time simulation.
 *
 * @param[inout]  aReadFdSet   A pointer to the read file descriptors.
 * @param[inout]  aWriteFdSet  A pointer to the write file descriptors.
 * @param[inout]  aErrorFdSet  A pointer to the error file descriptors.
 * @param[inout]  aMaxFd       A pointer to the max file descriptor.
 * @param[inout]  aTimeout     A pointer to the timeout.
 *
 */
void virtualTimeUpdateFdSet(fd_set *        aReadFdSet,
                            fd_set *        aWriteFdSet,
                            fd_set *        aErrorFdSet,
                            int *           aMaxFd,
                            struct timeval *aTimeout);

/**
 * This function sends radio spinel event of virtual time simulation.
 *
 * @param[in] aData     A pointer to the spinel frame.
 * @param[in] aLength   Length of the spinel frame.
 *
 */
void virtualTimeSendRadioSpinelWriteEvent(const uint8_t *aData, uint16_t aLength);

/**
 * This function receives an event of virtual time simulation.
 *
 * @param[out]  aEvent  A pointer to the event receiving the event.
 *
 */
void virtualTimeReceiveEvent(struct VirtualTimeEvent *aEvent);

/**
 * This function sends sleep event through virtual time simulation.
 *
 * @param[in]   aTimeout    A pointer to the time sleeping.
 *
 */
void virtualTimeSendSleepEvent(const struct timeval *aTimeout);

/**
 * This function performs radio spinel processing of virtual time simulation.
 *
 * @param[in]   aInstance   A pointer to the OpenThread instance.
 * @param[in]   aEvent      A pointer to the current event.
 *
 */
void virtualTimeRadioSpinelProcess(otInstance *aInstance, const struct VirtualTimeEvent *aEvent);

/**
 * This function initializes platform UDP driver.
 *
 * @param[in]   aIfName   The name of Thread's platform network interface.
 *
 */
void platformUdpInit(const char *aIfName);

/**
 * This function performs platform UDP driver processing.
 *
 * @param[in]   aInstance   The OpenThread instance structure.
 * @param[in]   aReadFdSet  A pointer to the read file descriptors.
 *
 */
void platformUdpProcess(otInstance *aInstance, const fd_set *aReadSet);

/**
 * This function updates the file descriptor sets with file descriptors used by the platform UDP driver.
 *
 * @param[in]     aInstance    The OpenThread instance structure.
 * @param[inout]  aReadFdSet   A pointer to the read file descriptors.
 * @param[inout]  aMaxFd       A pointer to the max file descriptor.
 */
void platformUdpUpdateFdSet(otInstance *aInstance, fd_set *aReadFdSet, int *aMaxFd);

enum SocketBlockOption
{
    kSocketBlock,
    kSocketNonBlock,
};

/**
 * This function initializes platform TREL UDP6 driver.
 *
 * @param[in]   aInterfaceName   The name of network interface.
 *
 */
void platformTrelInit(const char *aInterfaceName);

/**
 * This function shuts down the platform TREL UDP6 platform driver.
 *
 */
void platformTrelDeinit(void);

/**
 * This function updates the file descriptor sets with file descriptors used by the TREL driver.
 *
 * @param[inout]  aReadFdSet   A pointer to the read file descriptors.
 * @param[inout]  aWriteFdSet  A pointer to the write file descriptors.
 * @param[inout]  aMaxFd       A pointer to the max file descriptor.
 * @param[inout]  aTimeout     A pointer to the timeout.
 *
 */
void platformTrelUpdateFdSet(fd_set *aReadFdSet, fd_set *aWriteFdSet, int *aMaxFd, struct timeval *aTimeout);

/**
 * This function performs TREL driver processing.
 *
 * @param[in]   aInstance       A pointer to the OpenThread instance.
 * @param[in]   aReadFdSet      A pointer to the read file descriptors.
 * @param[in]   aWriteFdSet     A pointer to the write file descriptors.
 *
 */
void platformTrelProcess(otInstance *aInstance, const fd_set *aReadFdSet, const fd_set *aWriteFdSet);

/**
 * This function creates a socket with SOCK_CLOEXEC flag set.
 *
 * @param[in]   aDomain       The communication domain.
 * @param[in]   aType         The semantics of communication.
 * @param[in]   aProtocol     The protocol to use.
 * @param[in]   aBlockOption  Whether to add nonblock flags.
 *
 * @returns The file descriptor of the created socket.
 *
 * @retval  -1  Failed to create socket.
 *
 */
int SocketWithCloseExec(int aDomain, int aType, int aProtocol, SocketBlockOption aBlockOption);

/**
 * The name of Thread network interface.
 *
 */
extern char gNetifName[IFNAMSIZ];

/**
 * The index of Thread network interface.
 *
 */
extern unsigned int gNetifIndex;

/**
 * This function initializes platform Backbone network.
 *
 * @param[in]   aInstance       A pointer to the OpenThread instance.
 * @param[in]   aInterfaceName  A pointer to Thread network interface name.
 *
 */
void platformBackboneInit(otInstance *aInstance, const char *aInterfaceName);

/**
 * This function updates the file descriptor sets with file descriptors used by the platform Backbone network.
 *
 * @param[inout]  aReadFdSet   A reference to the read file descriptors.
 * @param[inout]  aMaxFd       A reference to the max file descriptor.
 *
 */
void platformBackboneUpdateFdSet(fd_set &aReadFdSet, int &aMaxFd);

/**
 * This function performs platform Backbone network processing.
 *
 * @param[in]   aReadFdSet  A reference to the read file descriptors.
 *
 */
void platformBackboneProcess(const fd_set &aReadSet);

/**
 * This function performs notifies state changes to platform Backbone network.
 *
 * @param[in]   aInstance       A pointer to the OpenThread instance.
 * @param[in]   aFlags          Flags that denote the state change events.
 *
 */
void platformBackboneStateChange(otInstance *aInstance, otChangedFlags aFlags);

/**
 * The name of Backbone network interface.
 *
 */
extern char gBackboneNetifName[IFNAMSIZ];

/**
 * The index of Backbone network interface.
 *
 */
extern unsigned int gBackboneNetifIndex;

/**
 * This function initializes the infrastructure interface.
 *
 * @param[in]  aInstance  The OpenThread instance.
 * @param[in]  aIfName    The name of the infrastructure interface.
 *
 * @returns  The index of the infrastructure interface.
 *
 */
uint32_t platformInfraIfInit(otInstance *aInstance, const char *aIfName);

/**
 * This function deinitializes the infrastructure interface.
 *
 */
void platformInfraIfDeinit(void);

/**
 * This function tells if the infrastructure interface is running.
 *
 * @returns TRUE if the infrastructure interface is running, FALSE if not.
 *
 */
bool platformInfraIfIsRunning(void);

/**
 * This function updates the read fd set.
 *
 * @param[out]  aReadFdSet  The fd set to be updated.
 * @param[out]  aMaxFd      The maximum fd to be updated.
 *
 */
void platformInfraIfUpdateFdSet(fd_set &aReadFdSet, int &aMaxFd);

/**
 * This function processes possible events on the infrastructure interface.
 *
 * @param[in]  aInstance   The OpenThread instance.
 * @param[in]  aReadFdSet  The fd set which may contain read vents.
 *
 */
void platformInfraIfProcess(otInstance *aInstance, const fd_set &aReadFdSet);

/**
 * This function enables daemon.
 *
 * @param[in]       aInstance   The OpenThread instance structure.
 *
 */
void platformDaemonEnable(otInstance *aInstance);

/**
 * This function disables daemon.
 *
 */
void platformDaemonDisable(void);

/**
 * This function updates the file descriptor sets with file descriptors used by daemon.
 *
 * @param[inout]    aMainloop   A pointer to the mainloop context.
 *
 */
void platformDaemonUpdate(otSysMainloopContext *aContext);

/**
 * This function performs daemon processing.
 *
 * @param[in]   aMainloop   A pointer to the mainloop context.
 *
 */
void platformDaemonProcess(const otSysMainloopContext *aContext);

#ifdef __cplusplus
}
#endif
#endif // PLATFORM_POSIX_H_
