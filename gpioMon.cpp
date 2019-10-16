/**
 * Copyright © 2019 Facebook
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gpioMon.hpp"

#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>

namespace phosphor
{
namespace gpio
{

/* systemd service to kick start a target. */
constexpr auto SYSTEMD_SERVICE = "org.freedesktop.systemd1";
constexpr auto SYSTEMD_ROOT = "/org/freedesktop/systemd1";
constexpr auto SYSTEMD_INTERFACE = "org.freedesktop.systemd1.Manager";

using namespace phosphor::logging;

void GpioMonitor::gpioEventWait()
{

    gpioEventDescriptor.async_wait(
        boost::asio::posix::stream_descriptor::wait_read,
        [this](const boost::system::error_code& ec) {
            if (ec)
            {
                std::string msg = gpioLineMsg + "event handler error" +
                                  std::string(ec.message());
                log<level::ERR>(msg.c_str());
                return;
            }
            gpioEventHandler();
        });
}

void GpioMonitor::gpioEventHandler()
{
    gpiod_line_event gpioLineEvent;
    std::string logMessage;

    if (gpiod_line_event_read_fd(gpioEventDescriptor.native_handle(),
                                 &gpioLineEvent) < 0)
    {
        logMessage = "Failed to read event from fd for " + gpioLineMsg;
        log<level::ERR>(logMessage.c_str());
        return;
    }

    logMessage =
        gpioLineMsg + (gpioLineEvent.event_type == GPIOD_LINE_EVENT_RISING_EDGE
                           ? " Asserted"
                           : " Deasserted");

    log<level::INFO>(logMessage.c_str());

    /* Execute the target if it is defined. */
    if (!target.empty())
    {
        auto bus = sdbusplus::bus::new_default();
        auto method = bus.new_method_call(SYSTEMD_SERVICE, SYSTEMD_ROOT,
                                          SYSTEMD_INTERFACE, "StartUnit");
        method.append(target);
        method.append("replace");

        bus.call_noreply(method);
    }

    /* if cnot required to continue monitoring then return */
    if (!continueAfterKeyPress)
    {
        return;
    }

    /* Schedule a wait event */
    gpioEventWait();
}

int GpioMonitor::requestGPIOEvents()
{

    std::string logMsg;

    /* Request an event to monitor for respected gpio line */
    if (gpiod_line_request(gpioLine, &gpioConfig, 0) < 0)
    {
        logMsg = "Failed to request events for " + gpioLineMsg;
        log<level::ERR>(logMsg.c_str());
        return -1;
    }

    int gpioLineFd = gpiod_line_event_get_fd(gpioLine);
    if (gpioLineFd < 0)
    {
        logMsg = "Failed to get fd for " + gpioLineMsg;
        log<level::ERR>(logMsg.c_str());
        return -1;
    }

    logMsg = gpioLineMsg + " monitoring started";
    log<level::INFO>(logMsg.c_str());

    /* Assign line fd to descriptor for monitoring */
    gpioEventDescriptor.assign(gpioLineFd);

    /* Schedule a wait event */
    gpioEventWait();

    return 0;
}
} // namespace gpio
} // namespace phosphor
