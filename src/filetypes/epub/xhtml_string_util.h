#ifndef XHTML_STRING_UTIL_H_
#define XHTML_STRING_UTIL_H_

#include <string>
#include <vector>

// Convert all whitespace chars to space and limit consecutive whitespace to 1 char length
std::string compact_whitespace(const char *str);

// Join multiple strings, applying html whitespace rules
std::string compact_strings(const std::vector<const char*> &strings);

#endif
