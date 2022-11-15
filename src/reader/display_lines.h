#ifndef DISPLAY_LINES_H_
#define DISPLAY_LINES_H_

#include "epub/doc_token.h"

#include <functional>
#include <string>
#include <vector>

struct Line
{
    std::string text;

    Line(std::string text) : text(text) {}
};

void get_display_lines(
    const std::vector<DocToken> &tokens,
    const std::function<bool(const char *, int)> &fits_on_line,
    std::vector<Line> &out_lines
);

#endif
