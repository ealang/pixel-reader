#include "./token_line_scroller.h"

uint32_t TokenLineScroller::get_more_lines_forward(uint32_t num_lines)
{
    uint32_t starting_line_count = lines_buf.size();

    while (num_lines > 0)
    {
        std::vector<DocToken> tokens;

        const DocToken *token = nullptr;
        while ((token = forward_it.read(1)))
        {
            tokens.push_back(*token);
            // Don't separate text tokens since line wrapper needs full lines.
            if (token->type != TokenType::Text)
            {
                break;
            }
        }

        if (tokens.empty())
        {
            global_last_line = lines_buf.line_end();
            break;
        }

        get_display_lines(
            tokens,
            line_fits,
            [&buf=lines_buf, &num_lines](const std::string &text, const DocAddr &start_addr) {
                buf.append(
                    text,
                    start_addr
                );
                if (num_lines > 0)
                {
                    --num_lines;
                }
            }
        );
    }

    return lines_buf.size() - starting_line_count;
}

uint32_t TokenLineScroller::get_more_lines_backward(uint32_t num_lines)
{
    uint32_t starting_line_count = lines_buf.size();

    while (num_lines > 0)
    {
        std::vector<DocToken> tokens;

        const DocToken *token = nullptr;
        while ((token = backward_it.read(-1)))
        {
            tokens.push_back(*token);
            // Don't separate text tokens since line wrapper needs full lines.
            if (token->type != TokenType::Text)
            {
                break;
            }
        }

        if (tokens.empty())
        {
            global_first_line = lines_buf.line_start();
            break;
        }

        std::reverse(tokens.begin(), tokens.end());

        std::vector<Line> lines_tmp;
        get_display_lines(
            tokens,
            line_fits,
            [&lines_tmp](const std::string &text, const DocAddr &start_addr) {
                lines_tmp.emplace_back(text, start_addr);
            }
        );

        for (auto it = lines_tmp.rbegin(); it != lines_tmp.rend(); ++it)
        {
            lines_buf.prepend(
                it->text,
                it->address
            );
            if (num_lines > 0)
            {
                --num_lines;
            }
        }

    }

    return lines_buf.size() - starting_line_count;
}

void TokenLineScroller::clear_buffer()
{
    lines_buf.clear();
    current_line = 0;
    global_first_line = std::nullopt;
    global_last_line = std::nullopt;
}

void TokenLineScroller::initialize_buffer_at(DocAddr address)
{
    clear_buffer();

    // Find starting point that isn't a text token.
    // Line wrapper is not able to handle partial lines of text.
    {
        EPubTokenIter it(forward_it);
        it.seek(address);

        const DocToken *token = nullptr;
        // Look backward for first non-text token.
        while ((token = it.read(-1)) && token->type != TokenType::Text);

        forward_it = it;
        backward_it = it;
    }

    // Backwards pass
    get_more_lines_backward(num_lines_lookahead);

    // Forward pass until address is covered
    while (true)
    {
        if (!get_more_lines_forward(num_lines_lookahead))
        {
            break;
        }

        auto last_addr = lines_buf.last_line_address();
        if (last_addr && *last_addr >= address)
        {
            break;
        }
    }

    current_line = lines_buf.get_line_for_address(address);
    ensure_lines_around(current_line);
}

TokenLineScroller::TokenLineScroller(
    EPubReader &reader,
    DocAddr address,
    uint32_t num_lines_lookahead,
    std::function<bool(const char *, uint32_t)> line_fits
) : forward_it(reader.get_iter(address)),
    backward_it(reader.get_iter(address)),
    line_fits(line_fits),
    num_lines_lookahead(num_lines_lookahead)
{
    initialize_buffer_at(address);
}

void TokenLineScroller::ensure_lines_around(int line_num)
{
    // forwards
    int forward_extent = num_lines_lookahead + line_num;
    if (global_last_line && forward_extent > *global_last_line)
    {
        forward_extent = *global_last_line;
    }

    int forward_needed = forward_extent - lines_buf.line_end();
    if (forward_needed > 0)
    {
        get_more_lines_forward(forward_needed);
    }

    // backwards
    int backwards_extent = line_num - num_lines_lookahead;
    if (global_first_line && *global_first_line > backwards_extent)
    {
        backwards_extent = *global_first_line;
    }

    int backwards_lines_needed = lines_buf.line_start() - backwards_extent;
    if (backwards_lines_needed > 0)
    {
        get_more_lines_backward(backwards_lines_needed);
    }
}

const Line *TokenLineScroller::get_line_relative(int offset)
{
    return lines_buf.get_line(current_line + offset);
}

int TokenLineScroller::get_line_number() const
{
    return current_line;
}

void TokenLineScroller::seek_lines_relative(int offset)
{
    current_line += offset;
    ensure_lines_around(current_line);
}

void TokenLineScroller::seek_to_address(DocAddr address)
{
    initialize_buffer_at(address);
}

void TokenLineScroller::reset_buffer()
{
    const Line *line = get_line_relative(0);
    if (line)
    {
        DocAddr cur_address = line->address;
        clear_buffer();
        initialize_buffer_at(cur_address);
    }
}

std::optional<int> TokenLineScroller::first_line_number() const
{
    return global_first_line;
}

std::optional<int> TokenLineScroller::last_line_number() const
{
    return global_last_line;
}
