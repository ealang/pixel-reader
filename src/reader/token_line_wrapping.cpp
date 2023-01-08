#include "./token_line_wrapping.h"

#include "./text_wrap.h"
#include "doc_api/token_addressing.h"

#include <numeric>

void line_wrap_token(
    const DocToken &token,
    std::function<bool(const char *, uint32_t)> text_fits_on_line,
    std::function<void(const DocToken &)> for_each
)
{
    if (token.type == TokenType::Text || token.type == TokenType::Header)
    {
        DocAddr address = token.address;
        wrap_lines(token.data.c_str(), text_fits_on_line, [&](const char *str, uint32_t len) {
            std::string text_line = std::string(str, len);
            for_each(DocToken(token.type, address, text_line));

            address += get_address_width(text_line.c_str());
        });
    }
    else
    {
        for_each(token);
    }
}
