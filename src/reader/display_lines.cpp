#include "./display_lines.h"
#include "./text_wrap.h"

void get_display_lines(
    const std::vector<DocToken> &tokens,
    const std::function<bool(const char *, int)> &fits_on_line,
    std::vector<Line> &out_lines
)
{
    for (const auto &token: tokens)
    {
        if (token.type == TokenType::Text)
        {
            for (const auto& line: wrap_lines(token.text.c_str(), fits_on_line))
            {
                out_lines.emplace_back(line);
            }
        }
        else if (token.type == TokenType::Section)
        {
            out_lines.emplace_back("");
        }
    }
}
