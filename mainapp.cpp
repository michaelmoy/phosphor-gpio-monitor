/**
 * Copyright © 2016 IBM Corporation
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

#include <iostream>
#include <string>
#include "argument.hpp"
#include "monitor.hpp"

static void exitWithError(const char* err, char** argv)
{
    phosphor::gpio::ArgumentParser::usage(argv);
    std::cerr << "ERROR: " << err << "\n";
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    // Read arguments.
    auto options = phosphor::gpio::ArgumentParser(argc, argv);

    // Parse out path argument.
    auto path = (options)["path"];
    if (path == phosphor::gpio::ArgumentParser::emptyString)
    {
        exitWithError("path not specified.", argv);
    }

    // Parse out key code that we are interested in
    // Its integer mapping to the GPIO key configured by the kernel
    auto code = (options)["code"];
    if (code == phosphor::gpio::ArgumentParser::emptyString)
    {
        exitWithError("Code not specified.", argv);
    }

    // Parse out key value that we are interested in
    // Its either 1 or 0 for press / release
    auto value = (options)["value"];
    if (value == phosphor::gpio::ArgumentParser::emptyString)
    {
        exitWithError("Value not specified.", argv);
    }

    // Parse out target argument. It is fine if the caller does not
    // pass this if they are not interested in calling into any target
    // on meeting a condition.
    auto target = (options)["target"];

    // Create a GPIO monitor object and let it do all the rest
    phosphor::gpio::Monitor monitor(path, std::stoi(code),
                                    std::stoi(value),target);

    return 0;
}
