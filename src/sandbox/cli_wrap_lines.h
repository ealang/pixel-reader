#ifndef CLI_WRAP_LINES_H_
#define CLI_WRAP_LINES_H_

#include "doc_api/doc_token.h"
#include <string>
#include <vector>

struct Line
{
    std::string text;
    DocAddr address;

    Line(std::string text, DocAddr address);
};

// Text-only rendering of tokens.
std::vector<Line> cli_wrap_lines(
    const std::vector<DocToken> &tokens,
    uint32_t max_column_width
);

#endif
