#include "./display_lines.h"

#include "./text_wrap.h"
#include "doc_api/token_addressing.h"

#include <numeric>

Line::Line(std::string text, DocAddr address)
    : text(text), address(address)
{
}

void get_display_lines(
    const std::vector<DocToken> &tokens,
    const std::function<bool(const char *, uint32_t)> &fits_on_line,
    std::function<void(const std::string &, const DocAddr &)> on_line
)
{
    std::vector<const DocToken *> pending_text_tokens;

    // Combine and wrap adjacent text tokens.
    auto emit_pending_text = [&]() {
        if (pending_text_tokens.empty())
        {
            return;
        }

        DocAddr start_addr = pending_text_tokens[0]->address;
        uint32_t address_offset = 0;

        uint32_t combined_size = std::accumulate(
            pending_text_tokens.begin(),
            pending_text_tokens.end(),
            0,
            [](uint32_t acc, auto token) { return acc + token->text.size(); }
        );

        std::string combined_text;
        combined_text.reserve(combined_size);

        for (auto token: pending_text_tokens)
        {
            combined_text += token->text;
        }

        wrap_lines(combined_text.c_str(), fits_on_line, [&](const char *str, uint32_t len) {
            std::string line_text = std::string(str, len);
            on_line(line_text, increment_address(start_addr, address_offset));

            address_offset += get_address_width(line_text.c_str());
        });

        pending_text_tokens.clear();
    };

    for (const auto &token: tokens)
    {
        if (token.type == TokenType::Text)
        {
            pending_text_tokens.push_back(&token);
        }
        else if (token.type == TokenType::Section)
        {
            emit_pending_text();
            on_line("", token.address);
        }
        else if (token.type == TokenType::Block)
        {
            emit_pending_text();
        }
    }

    emit_pending_text();
}

std::vector<Line> get_display_lines(
    const std::vector<DocToken> &tokens,
    const std::function<bool(const char *, uint32_t)> &fits_on_line
)
{
    std::vector<Line> lines;

    get_display_lines(tokens, fits_on_line, [&](const std::string &text, const DocAddr &address) {
        lines.push_back(Line(text, address));
    });

    return lines;
}
