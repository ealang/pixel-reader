#ifndef COLOR_THEME_DEF_H_
#define COLOR_THEME_DEF_H_

#include "./color_theme.h"
#include <string>

const ColorTheme& get_color_theme(const std::string &name);
std::string get_prev_theme(const std::string &name);
std::string get_next_theme(const std::string &name);

#endif
