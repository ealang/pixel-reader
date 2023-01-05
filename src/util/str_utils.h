#ifndef STR_UTILS_H_
#define STR_UTILS_H_

#include <string>

std::string to_lower(const std::string &str);

std::string remove_carriage_returns(const std::string &str);

std::string strip_whitespace(const char *str);

std::string strip_whitespace_left(const std::string &str);
std::string strip_whitespace_right(const std::string &str);

inline bool is_whitespace(char c)
{
    // Note: Changing this will affect addressing behavior
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

std::string convert_tabs_to_space(const std::string &str, int tab_width);

#endif
