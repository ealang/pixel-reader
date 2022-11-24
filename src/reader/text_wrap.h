#ifndef TEXT_WRAP_H_
#define TEXT_WRAP_H_

#include <functional>
#include <string>
#include <vector>

void wrap_lines(
    const char *str,
    std::function<bool(const char *, uint32_t)> fits_on_line,
    std::function<void(const char *, uint32_t)> on_next_line,
    unsigned int max_line_search_chars = 1024
);

#endif
