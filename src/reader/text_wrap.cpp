#include "./text_wrap.h"
#include <cstring>

namespace {

bool is_whitespace(char c)
{
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

unsigned int non_white_len(const char *line)
{
    unsigned int len = strlen(line);
    while (len && is_whitespace(line[len - 1])) {
        --len;
    }
    return len;
}

int find_last_space(const char *line, unsigned int max_len)
{
    int best_i = -1;
    const char *cur = line;
    while (1) 
    {
        const char *next_char = strchr(cur, ' ');
        if (!next_char)
        {
            break;
        }

        unsigned int i = next_char - line;
        if (i > max_len)
        {
            break;
        }

        best_i = i;
        cur = next_char + 1;
    }
    return best_i;
}

} // namespace

std::vector<std::string> wrap_lines(const char *line, unsigned int max_len)
{
    auto len = non_white_len(line);

    std::vector<std::string> wrapped_lines;
    while (len > max_len)
    {
        const char* next_break = strchr(line, '\n');
        if (next_break) {
            unsigned int break_i = next_break - line;
            if (break_i < max_len) {
                wrapped_lines.push_back(std::string(line, break_i));
                line += break_i + 1;
                len -= break_i + 1;
                continue;
            }
        }

        int split_i = find_last_space(line, max_len);
        if (split_i < 0) {
            // no space found to split on
            wrapped_lines.push_back(
                std::string(line, max_len)
            );
            line += max_len;
            len -= max_len;
        } else {
            // split on space
            wrapped_lines.push_back(
                std::string(line, split_i)
            );
            line += split_i + 1;
            len -= split_i - 1;
        }
    }

    wrapped_lines.push_back(std::string(line, len));

    return wrapped_lines;
}

