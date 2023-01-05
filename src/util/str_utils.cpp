#include "./str_utils.h"

#include <algorithm>
#include <cstring>

namespace
{

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

const char *_strip_whitespace_left(const char *str)
{
    while (is_whitespace(*str))
    {
        ++str;
    }
    return str;
}

inline bool _is_carriage_return(char c)
{
    return c == '\r';
}

inline bool _is_tab(char c)
{
    return c == '\t';
}

} // namespace

std::string to_lower(const std::string &str)
{
    std::string result;
    result.reserve(str.size());

    for (char c: str)
    {
        result.push_back(std::tolower(c));
    }

    return result;
}

std::string remove_carriage_returns(const std::string &str)
{
    std::string result;
    result.reserve(str.size());

    for (char c: str)
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
    const char *start = _strip_whitespace_left(str);
    const char *end = _last_non_whitespace(start);
    return std::string(start, end - start);
}

std::string strip_whitespace_left(const std::string &str)
{
    return _strip_whitespace_left(str.c_str());
}

std::string strip_whitespace_right(const std::string &str)
{
    const char *start = str.c_str();
    const char *end = _last_non_whitespace(start);
    return std::string(start, end - start);
}

std::string convert_tabs_to_space(const std::string &str, int tab_width)
{
    std::string result;
    result.reserve(str.size());

    for (char c: str)
    {
        if (_is_tab(c))
        {
            result.append(tab_width, ' ');
        }
        else
        {
            result.push_back(c);
        }
    }

    return result;
}
