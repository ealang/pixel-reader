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

std::string compact_strings(const std::vector<const char*> &strings)
{
    std::string result;
    {
        uint32_t upper_size = 0;
        for (const char *str : strings)
        {
            upper_size += strlen(str);
        }
        result.reserve(upper_size);
    }

    for (const char *str : strings)
    {
        std::string substring = compact_whitespace(str);

        if (result.empty())
        {
            substring = strip_whitespace_left(substring);
        }

        bool space_left = result.empty() ? false : is_whitespace(result[result.size() - 1]);
        bool space_right = substring.empty() ? false : is_whitespace(substring[0]);
        if (space_left && space_right)
        {
            substring = strip_whitespace_left(substring);
        }

        result.append(substring);
    }

    if (!result.empty() && is_whitespace(result[result.size() - 1]))
    {
        result = strip_whitespace_right(result);
    }
    return result;
}
