#include "./cli_render_lines.h"
#include "reader/token_line_wrapping.h"

Line::Line(std::string text, DocAddr address)
    : text(std::move(text)), address(address)
{
}

std::vector<Line> cli_render_tokens(
    const std::vector<DocToken> &tokens,
    uint32_t max_column_width
)
{
    std::vector<Line> out;

    auto fits_on_line = [max_column_width](const char *, uint32_t strlen) {
        return strlen <= max_column_width;
    };

    auto on_token = [max_column_width, &out](const DocToken &token) {
        if (token.type == TokenType::Text)
        {
            out.emplace_back(token.data, token.address);
        }
        else if (token.type == TokenType::Image)
        {
            out.emplace_back("[Image " + token.data + "]", token.address);
        }
    };

    for (const auto &token : tokens) {
        line_wrap_token(
            token,
            fits_on_line,
            on_token
        );
    }

    return out;
}
