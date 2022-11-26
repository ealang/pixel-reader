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

// Break document tokens into word wrapped lines.
// Emit text and start address for each line.
void get_display_lines(
    const std::vector<DocToken> &tokens,
    const std::function<bool(const char *, uint32_t)> &fits_on_line,
    std::function<void(const std::string &, const DocAddr &)> on_line
);

std::vector<Line> get_display_lines(
    const std::vector<DocToken> &tokens,
    const std::function<bool(const char *, uint32_t)> &fits_on_line
);

#endif
