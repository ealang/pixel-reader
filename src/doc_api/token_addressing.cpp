#include "./token_addressing.h"
#include "util/str_utils.h"
#include "util/utf8.h"

namespace {

// Determine if the character is counted in document text addressing.
// Note: for address backwards compatibility, this function cannot change.
// Also need to ensure any changes to xhtml whitespace compaction and
// text-wrapping whitespace breaking are not altering the address calculations.
inline bool char_has_width(char c)
{
    return !is_whitespace(c);
}

} // namespace

uint32_t get_address_width(const char *str)
{
    if (!str)
    {
        return 0;
    }

    uint32_t count = 0;
    char c;
    while ((c = *str))
    {
        if (char_has_width(c))
        {
            ++count;
        }
        str = utf8_step(str);
    }
    return count;
}

uint32_t get_address_width(const DocToken &token)
{
    if (token.type == TokenType::Text || token.type == TokenType::Header)
    {
        return get_address_width(token.data.c_str());
    }
    else if (token.type == TokenType::Image)
    {
        return 1;
    }
    return 0;
}
