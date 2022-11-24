#ifndef XHTML_STRING_UTIL_H_
#define XHTML_STRING_UTIL_H_

#include <string>

std::string compact_whitespace(const char *str);
std::string remove_carriage_returns(const char *str);

const char *strip_whitespace_left(const char *str);
uint32_t count_non_whitespace_chars(const char *str);

// Set of characters subject to compaction, etc.
inline bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

#endif
