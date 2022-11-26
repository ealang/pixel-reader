#include "epub/epub_reader.h"
#include "reader/display_lines.h"

#include <iostream>
#include <string>

static bool fits_on_line_by_char(const char *, uint32_t strlen)
{
    return strlen <= 80;
}

void display_epub(std::string path)
{
    std::cout << path << std::endl;

    EPubReader epub(path);
    if (!epub.open())
    {
        std::cerr << "Unable to open epub" << std::endl;
        return;
    }
    for (auto &tok : epub.get_tok())
    {
        std::cout << "--------------------------------" << std::endl;
        std::cout << tok.doc_id << " " << tok.name << std::endl;
        auto tokens = epub.get_tokenized_document(tok.doc_id);
        std::cout << "Contains " << tokens.size() << " tokens" << std::endl;

        std::vector<Line> display_lines = get_display_lines(
            tokens,
            fits_on_line_by_char
        );

        std::cout << "Contains " << display_lines.size() << " lines" << std::endl;
        for (const auto &line: display_lines)
        {
            std::cout << to_string(line.address) << " | " << line.text << std::endl;
        }
    }
}
