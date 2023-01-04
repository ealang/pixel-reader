#ifndef STR_UTILS_H_
#define STR_UTILS_H_

#include <string>

std::string to_lower(const std::string &str);
std::string to_lower(const char *str);

std::string remove_carriage_returns(const char *str);
std::string strip_whitespace(const char *str);

// Advance to non-whitespace
const char *strip_whitespace_left(const char *str);

inline bool is_whitespace(char c)
{
    // Note: Changing this will affect addressing behavior
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

#endif
