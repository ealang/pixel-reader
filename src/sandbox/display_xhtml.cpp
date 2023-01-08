#include "filetypes/epub/xhtml_parser.h"
#include "./cli_render_lines.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void display_xhtml(std::string path)
{
    std::ifstream fp(path);
    std::stringstream buffer;
    buffer << fp.rdbuf();

    std::vector<DocToken> tokens;
    std::unordered_map<std::string, DocAddr> ids;
    parse_xhtml_tokens(buffer.str().c_str(), path, 0, tokens, ids);

    std::vector<Line> display_lines = cli_render_tokens(tokens, 80);

    std::cout << path << std::endl;
    for (const auto &line: display_lines)
    {
        std::cout << to_string(line.address) << " | " << line.text << std::endl;
    }
}
