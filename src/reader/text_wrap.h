#ifndef TEXT_WRAP_H_
#define TEXT_WRAP_H_

#include <functional>
#include <string>
#include <vector>

using str_filter_func = std::function<bool(const char *, int)>;

std::vector<std::string> wrap_lines(
    const std::string &str,
    const str_filter_func &fits_on_line,
    unsigned int max_line_search_chars = 1024
);

#endif
