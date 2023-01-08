#include "./text_wrap.h"

#include "util/str_utils.h"
#include "util/utf8.h"

#include <cstring>
#include <stdexcept>

namespace {

using str_filter_func = std::function<bool(const char *, uint32_t)>;

const char *find_first_break(const char *pos, int max_search)
{
    while (*pos && !is_whitespace(*pos)) {
        ++pos;
        if (--max_search < 0)
        {
            return nullptr;
        }
    }
    return pos;
}

const char *find_last_whitespace(const char *start_pos, str_filter_func &fits_on_line, uint32_t max_line_search_chars)
{
    const char *best_candidate_pos = nullptr;
    const char *cur_pos = start_pos;

    while(true)
    {
        const char *candidate_pos = find_first_break(cur_pos, max_line_search_chars);
        if (!candidate_pos)
        {
            // no break within max search
            break;
        }

        if (!fits_on_line(start_pos, candidate_pos - start_pos))
        {
            // overshot
            break;
        }

        best_candidate_pos = candidate_pos;
        cur_pos = candidate_pos + 1;

        if (*candidate_pos == 0 || *candidate_pos == '\n')
        {
            // break on literal newlines and end of string
            break;
        }
    }

    return best_candidate_pos;
}

const char *find_last_character(const char *start_pos, str_filter_func &fits_on_line, uint32_t max_line_search_chars)
{
    const char *cur_pos = start_pos;
    while (*cur_pos && max_line_search_chars-- > 0)
    {
        const char *candidate_pos = utf8_step(cur_pos);
        if (!fits_on_line(start_pos, candidate_pos - start_pos))
        {
            break;
        }
        cur_pos = candidate_pos;
    }
    return cur_pos;
}

} // namespace

void wrap_lines(
    const char *str,
    std::function<bool(const char *, uint32_t)> fits_on_line,
    std::function<void(const char *, uint32_t)> on_next_line,
    uint32_t max_line_search_chars
)
{
    uint32_t n = strlen(str);
    if (n == 0)
    {
        on_next_line(str, 0);
        return;
    }

    const char *cur_pos = str;
    const char *string_end_pos = cur_pos + n;
    while (cur_pos < string_end_pos)
    {
        const char *break_pos;
        if ((break_pos = find_last_whitespace(cur_pos, fits_on_line, max_line_search_chars)))
        {
            on_next_line(cur_pos, break_pos - cur_pos);
            cur_pos = break_pos + 1;
        }
        else if ((break_pos = find_last_character(cur_pos, fits_on_line, max_line_search_chars)))
        {
            on_next_line(cur_pos, break_pos - cur_pos);
            cur_pos = break_pos;
        }
        else
        {
            throw std::runtime_error("Failed to wrap line");
        }
    }
}
