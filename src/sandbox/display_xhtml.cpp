#include "epub/xhtml_parser.h"
#include "reader/display_lines.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static bool fits_on_line_by_char(const char *, uint32_t strlen)
{
    return strlen <= 80;
}

void display_xhtml(std::string path)
{
    std::ifstream fp(path);
    std::stringstream buffer;
    buffer << fp.rdbuf();

    auto tokens = parse_xhtml_tokens(buffer.str().c_str(), path);

    std::vector<Line> display_lines = get_display_lines(
        tokens,
        fits_on_line_by_char
    );

    std::cout << path << std::endl;
    for (const auto &line: display_lines)
    {
        std::cout << to_string(line.address) << " | " << line.text << std::endl;
    }
}
