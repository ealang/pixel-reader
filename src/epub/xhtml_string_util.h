#ifndef XHTML_STRING_UTIL_H_
#define XHTML_STRING_UTIL_H_

#include <string>

std::string compact_whitespace(const char *str);
std::string remove_carriage_returns(const char *str);
std::string strip_whitespace(const char *str);

// Advance to non-whitespace
const char *strip_whitespace_left(const char *str);

// Set of characters subject to compaction, etc.
inline bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

#endif
