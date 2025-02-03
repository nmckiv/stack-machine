#include <iostream>
#include <fstream>
#include <cxxopts.hpp>
#include <exception>

#include "machine.h"
#include "tui.h"
#include "console.h"

int main(int argc, char **argv) {

    cxxopts::Options options("Stack Machine Flags", "Flags to control the operation of the virtual stack machine");

    options.add_options()
        ("d,debug", "Launch with debug interface", cxxopts::value<bool>()->default_value("false"))
        ("f,file", "Path to target file", cxxopts::value<std::string>()->default_value(""))
        ("h,help", "Print usage");

    try {
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }
        else {
            //Launch machine
            reset();

            if (result.count("file")) {
                if (load_file(result["file"].as<std::string>()) != 0) {
                    std::cout << "Error: " << errorString << std::flush;
                    return 1;
                }
            }
            else {
                std::cout << "Please specify a file" << std::endl;
                return 1;
            }

            if (result.count("debug")) {
                interface = "TUI";
                if (TUI() != 0) {
                    std::cout << "Error: " << errorString << std::flush;
                    return 1;
                }
            }
            else {
                interface = "Console";
                if (Console() != 0) {
                    std::cout << "Error: " << errorString << std::flush;
                    return 1;
                }
            }
        }
    }
    catch (const std::exception& e) {  // Catch all standard exceptions
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}