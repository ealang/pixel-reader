#ifndef FONT_CATALOG_H_
#define FONT_CATALOG_H_

#include <string>

std::string get_valid_font_name(const std::string &preferred_font_name);
std::string get_prev_font_name(const std::string &font_name);
std::string get_next_font_name(const std::string &font_name);

#endif
