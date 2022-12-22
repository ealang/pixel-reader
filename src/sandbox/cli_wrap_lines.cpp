#include "./cli_wrap_lines.h"
#include "reader/token_line_wrapping.h"

Line::Line(std::string text, DocAddr address)
    : text(std::move(text)), address(address)
{
}

std::vector<Line> cli_wrap_lines(
    const std::vector<DocToken> &tokens,
    uint32_t max_column_width
)
{
    std::vector<Line> out;

    auto fits_on_line = [max_column_width](const char *, uint32_t strlen) {
        return strlen <= max_column_width;
    };

    line_wrap_tokens(
        tokens,
        fits_on_line,
        [&out](const DocToken &token) {
            if (token.type == TokenType::Text)
            {
                out.emplace_back(token.text, token.address);
            }
            else if (token.type == TokenType::Section)
            {
                out.emplace_back("", token.address);
            }
            else if (token.type == TokenType::Image)
            {
                out.emplace_back("[Image " + token.text + "]", token.address);
            }
        }
    );

    return out;
}
