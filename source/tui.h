#ifndef TUI_H
#define TUI_H

#include <iostream>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

extern bool input;
extern std::string input_string;

int TUI();

int updateTUI();

#endif