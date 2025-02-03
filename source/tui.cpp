#include <iostream>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

#include "tui.h"
#include "machine.h"

using namespace ftxui;

bool input = false;

std::string input_string;

int TUI() {
    auto screen = ScreenInteractive::Fullscreen();

    std::string registers[REGISTER_COUNT];
    std::string io_console = "User Input Here";

    // Scroll state variables
    int pm_scroll_offset = 0;
    int ds_scroll_offset = 0;
    int cs_scroll_offset = 0;

    auto input_component = Input(&input_string, "");

    auto make_box = [](const std::string& title, const std::string* content, int size) {
        std::vector<Element> elements;
        for (int i = 0; i < size; ++i) {
            std::stringstream ss(content[i]);
            std::string line;
            while (std::getline(ss, line)) {
                elements.push_back(text(line));
            }
        }
        return window(text(title), vbox(elements));
    };

    auto make_scroll_box = [&](const std::string& title, const std::string* content, int size, int highlight_index, int scroll_offset) {
        std::vector<Element> elements;
        int terminal_height = Terminal::Size().dimy;
        for (int i = scroll_offset; i < size && i < scroll_offset + terminal_height; ++i) {
            if (i == highlight_index) {
                elements.push_back(text(content[i]) | bgcolor(Color::Yellow));
            } else {
                elements.push_back(text(content[i]));
            }
        }
        return window(text(title), vbox(elements));
    };

    auto make_scroll_int_box = [&](const std::string& title, const int* content, int size, int highlight_index, int scroll_offset) {
        std::vector<Element> elements;
        int terminal_height = Terminal::Size().dimy;
        for (int i = scroll_offset; i < size && i < scroll_offset + terminal_height; ++i) {
            if (i == highlight_index) {
                elements.push_back(text(std::to_string(content[i])) | bgcolor(Color::Yellow));
            }
            else {
                elements.push_back(text(std::to_string(content[i])));
            }
        }
        return window(text(title), vbox(elements));
    };

    auto layout = [&] {
        registers[0] = "Program Counter: " + std::to_string(pc);
        registers[1] = "General Purpose Register: " + std::to_string(gpreg);
        registers[2] = "Data Stack Top: " + std::to_string(top);
        registers[3] = "Data Stack Pointer: " + std::to_string(dataStackPointer);
        registers[4] = "Call Stack Pointer: " + std::to_string(callStackPointer);
        registers[4] = "Stack Pointer: " + std::to_string(stackPointer);

        int midpoint = 19;
        int height = 39;

        //Program memory scrolling
        if (pc <= midpoint) {
            pm_scroll_offset = 0;
        } else if (pc < instructionCount - midpoint) {
            pm_scroll_offset = pc - midpoint;
        } else {
            pm_scroll_offset = instructionCount - height;
        }

        //Data stack scrolling
        if (dataStackPointer < height) {
            ds_scroll_offset = 0;
        }
        else {
            ds_scroll_offset = dataStackPointer - height;
        }

        //Call stack scrolling
        if (callStackPointer < height) {
            cs_scroll_offset = 0;
        }
        else {
            cs_scroll_offset = callStackPointer - height;
        }

        return vbox({
            hbox({
                make_scroll_box("──────────Program Memory", programMemory, PROGRAM_MEMORY_SIZE, pc, pm_scroll_offset) | flex,
                make_scroll_int_box("───────────Data Stack──", dataStack, dataStackPointer, stackPointer, ds_scroll_offset) | flex,
                make_scroll_int_box("───────────Call Stack──", callStack, callStackPointer, callStackPointer - 1, cs_scroll_offset) | flex,
                vbox({
                    make_box("────────────Registers──", registers, REGISTER_COUNT),
                    window(text("────────────Input──"), vbox({input_component->Render()})),
                    make_box("────────────Output──", &outputString, 1) | flex,
                }) | flex,
            }) | flex,
        });
    };

    auto renderer = Renderer([&] { return layout(); });

    int status = 0;

    auto event_handler = CatchEvent(renderer, [&](Event event) {

        if (input) {
            input_component->OnEvent(event);
        }
        else {
            input_string.clear();
        }
        if (event == Event::Return) {
            if ((status = executionStep()) != 0) {
                screen.ExitLoopClosure()();
                status++;
                return false;
            }
            screen.PostEvent(Event::Custom);
        }
        return true;
    });

    screen.Loop(event_handler);

    // std::cout << outputString << std::flush;

    return status;
}
