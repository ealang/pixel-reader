#include "./xhtml_string_util.h"

#include "util/str_utils.h"

#include <cstring>

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
