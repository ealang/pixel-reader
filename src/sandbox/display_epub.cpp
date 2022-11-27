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


    const auto &toc = epub.get_table_of_contents();
    for (auto &toc_item : toc)
    {
        std::cout << std::string(toc_item.indent_level * 2, ' ') << toc_item.display_name << std::endl;

    }

    std::optional<DocAddr> address = epub.get_start_address();
    while (address)
    {
        std::cout << "--------------------------------" << std::endl;
        uint32_t toc_index = epub.get_toc_index(*address);
        if (toc_index < toc.size())
        {
            std::cout << "TOC: " << toc[toc_index].display_name << std::endl;
        }

        auto tokens = epub.get_tokenized_document(*address);
        std::vector<Line> display_lines = get_display_lines(
            tokens,
            fits_on_line_by_char
        );

        for (const auto &line: display_lines)
        {
            std::cout << to_string(line.address) << " | " << line.text << std::endl;
        }

        address = epub.get_next_doc_address(*address);
    }
}
