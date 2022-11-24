#include "./xhtml_string_util.h"

#include "util/utf8.h"
#include <cstring>

namespace {

inline bool _is_carriage_return(char c)
{
    return c == '\r';
}

} // namespace

std::string compact_whitespace(const char *str)
{
    std::string result;
    result.reserve(strlen(str));

    bool last_was_whitespace = false;
    char c;
    for (; (c = *str); ++str)
    {
        if (is_whitespace(c))
        {
            if (last_was_whitespace)
            {
                continue;
            }
            last_was_whitespace = true;
            result.push_back(' ');  // treat all whitespace as ' '
        }
        else
        {
            last_was_whitespace = false;
            result.push_back(c);
        }
    }

    return result;
}

std::string remove_carriage_returns(const char *str)
{
    std::string result;
    result.reserve(strlen(str));

    char c;
    while ((c = *str++))
    {
        if (!_is_carriage_return(c))
        {
            result.push_back(c);
        }
    }

    return result;
}

const char *strip_whitespace_left(const char *str)
{
    while (is_whitespace(*str))
    {
        ++str;
    }
    return str;
}

uint32_t count_non_whitespace_chars(const char *str)
{
    uint32_t count = 0;
    char c;
    while ((c = *str))
    {
        if (!is_whitespace(c))
        {
            ++count;
        }
        str = utf8_step(str);
    }
    return count;
}
