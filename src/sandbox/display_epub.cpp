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
    std::cout << "================================" << std::endl;
    std::cout << path << std::endl;

    EPubReader epub(path);
    if (!epub.open())
    {
        std::cerr << "Unable to open epub" << std::endl;
        return;
    }
    for (auto &toc_item : epub.get_table_of_contents())
    {
        std::cout << std::string(toc_item.indent_level * 2, ' ') << toc_item.display_name << std::endl;

    }

    for (auto &doc_id: epub.get_document_id_order())
    {
        std::cout << "--------------------------------" << std::endl;
        std::cout << "Doc id: " << doc_id << std::endl;
        auto tokens = epub.get_tokenized_document(doc_id);

        std::vector<Line> display_lines = get_display_lines(
            tokens,
            fits_on_line_by_char
        );

        for (const auto &line: display_lines)
        {
            std::cout << to_string(line.address) << " | " << line.text << std::endl;
        }
    }
}
