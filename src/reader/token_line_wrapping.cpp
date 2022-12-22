#include "./token_line_wrapping.h"

#include "./text_wrap.h"
#include "doc_api/token_addressing.h"

#include <numeric>

void line_wrap_tokens(
    const std::vector<DocToken> &tokens,
    std::function<bool(const char *, uint32_t)> text_fits_on_line,
    std::function<void(const DocToken &)> for_each
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

        // TODO: this is assuming tokens have contiguous address space
        uint32_t combined_size = std::accumulate(
            pending_text_tokens.begin(),
            pending_text_tokens.end(),
            0,
            [](uint32_t acc, const DocToken *token) { return acc + get_address_width(*token); }
        );

        std::string combined_text;
        combined_text.reserve(combined_size);
        for (auto token: pending_text_tokens)
        {
            combined_text += token->text;
        }

        wrap_lines(combined_text.c_str(), text_fits_on_line, [&](const char *str, uint32_t len) {
            std::string line_text = std::string(str, len);
            for_each(DocToken(TokenType::Text, increment_address(start_addr, address_offset), line_text));

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
        else
        {
            emit_pending_text();
            for_each(token);
        }
    }

    emit_pending_text();
}
