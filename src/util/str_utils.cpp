#include "./str_utils.h"

#include <algorithm>
#include <cstring>

namespace
{

inline bool _is_carriage_return(char c)
{
    return c == '\r';
}

const char* _last_non_whitespace(const char *str)
{
    uint32_t len = strlen(str);
    if (len == 0)
    {
        return str;
    }

    str += len - 1;

    while (len != 0)
    {
        if (!is_whitespace(*str))
        {
            break;
        }
        --len;
        --str;
    }
    return str + 1;
}

} // namespace

std::string to_lower(const std::string &str)
{
    return to_lower(str.c_str());
}

std::string to_lower(const char *str)
{
    if (str == nullptr)
    {
        return {};
    }

    std::string result;
    result.reserve(strlen(str));

    char c;
    while ((c = *str++))
    {
        result.push_back(std::tolower(c));
    }

    return result;
}

std::string remove_carriage_returns(const char *str)
{
    if (str == nullptr)
    {
        return {};
    }

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

std::string strip_whitespace(const char *str)
{
    if (str == nullptr)
    {
        return {};
    }
    const char *start = strip_whitespace_left(str);
    const char *end = _last_non_whitespace(start);
    return std::string(start, end - start);
}

const char *strip_whitespace_left(const char *str)
{
    if (str == nullptr)
    {
        return nullptr;
    }
    while (is_whitespace(*str))
    {
        ++str;
    }
    return str;
}
