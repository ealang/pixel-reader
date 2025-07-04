#include "./cli_render_lines.h"

#include "doc_api/token_addressing.h"
#include "reader/text_wrap.h"

static const std::string BULLET = "•";

Line::Line(std::string text, DocAddr address)
    : text(std::move(text)), address(address)
{
}

std::vector<Line> cli_render_tokens(
    const std::vector<const DocToken *> &tokens,
    uint32_t max_column_width
)
{
    std::vector<Line> out;

    auto fits_on_line = [max_column_width](const char *, uint32_t strlen) {
        return strlen <= max_column_width;
    };

    auto wrap_text = [&](DocAddr address, const std::string &text, bool centered = false, uint32_t extra_text_width = 0)
    {
        uint32_t line_num = 0;
        wrap_lines(text.c_str(), fits_on_line, [&](const char *str, uint32_t len) {
            std::string line_text(str, len);

            if (centered) {
                int center_spacing = (max_column_width - line_text.size()) / 2;
                line_text = std::string(center_spacing, ' ') + line_text;
            }

            out.emplace_back(line_text, address);
            address += get_address_width(line_text);
            if (line_num++ == 0)
            {
                address -= extra_text_width;
            }
        });
    };

    for (const auto *token : tokens) {
        DocAddr address = token->address;
        switch (token->type) {
            case TokenType::Text:
                wrap_text(address, static_cast<const TextDocToken *>(token)->text);
                break;
            case TokenType::Header:
                wrap_text(address, static_cast<const HeaderDocToken *>(token)->text, true);
                break;
            case TokenType::ListItem:
                {
                    const auto *list_token = static_cast<const ListItemDocToken *>(token);
                    std::string prefix = std::string(
                        (list_token->nest_level > 1 ? list_token->nest_level - 1 : 0) * 2,
                        ' '
                    ) + BULLET + " ";
                    uint32_t extra_text_width = get_address_width(prefix);

                    wrap_text(address, prefix + static_cast<const ListItemDocToken *>(token)->text, false, extra_text_width);
                }
                break;
            case TokenType::Image:
                wrap_text(address, "[Image " + static_cast<const ImageDocToken *>(token)->path.string() + "]");
                break;
            default:
                break;
        }
    }

    return out;
}
