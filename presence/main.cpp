#include <iostream>
#include <systemd/sd-event.h>
#include <phosphor-logging/log.hpp>
#include "argument.hpp"
#include "gpio_presence.hpp"

using namespace phosphor::logging;
using namespace phosphor::gpio;
using namespace phosphor::gpio::presence;

int main(int argc, char* argv[])
{
    auto options = ArgumentParser(argc, argv);

    auto inventory = options["inventory"];
    auto key = options["key"];
    auto path = options["path"];
    if (argc < 4)
    {
        std::cerr << "Too few arguments\n";
        options.usage(argv);
    }

    if (inventory == ArgumentParser::emptyString)
    {
        std::cerr << "Inventory argument required\n";
        options.usage(argv);
    }

    if (key == ArgumentParser::emptyString)
    {
        std::cerr << "GPIO key argument required\n";
        options.usage(argv);
    }

    if (path == ArgumentParser::emptyString)
    {
        std::cerr << "Device path argument required\n";
        options.usage(argv);
    }

    auto bus = sdbusplus::bus::new_default();
    auto rc = 0;
    sd_event* event = nullptr;
    rc = sd_event_default(&event);
    if (rc < 0)
    {
        log<level::ERR>("Error creating a default sd_event handler");
        return rc;
    }
    EventPtr eventP{event};
    event = nullptr;

    auto name = options["name"];
    Presence presence(bus, inventory, path, std::stoul(key), name, eventP);

    while (true)
    {
        // -1 denotes wait forever
        rc = sd_event_run(eventP.get(), (uint64_t) - 1);
        if (rc < 0)
        {
            log<level::ERR>("Failure in processing request",
                            entry("ERROR=%s", strerror(-rc)));
            break;
        }
    }
    return rc;
}

