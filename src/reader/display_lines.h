#ifndef DISPLAY_LINES_H_
#define DISPLAY_LINES_H_

#include "doc_api/doc_token.h"

#include <functional>
#include <string>
#include <vector>

struct Line
{
    std::string text;
    DocAddr address;

    Line(std::string text, DocAddr address);
};

void get_display_lines(
    const std::vector<DocToken> &tokens,
    const std::function<bool(const char *, uint32_t)> &fits_on_line,
    std::vector<Line> &out_lines
);

#endif
