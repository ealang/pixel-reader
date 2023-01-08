#include "doc_api/token_addressing.h"
#include "filetypes/epub/epub_reader.h"
#include "./cli_render_lines.h"

#include <iostream>
#include <iomanip>
#include <string>

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

    // Read entire book
    std::vector<DocToken> tokens;
    {
        auto it = epub.get_iter();
        const DocToken *token = nullptr;
        while ((token = it->read(1)) != nullptr)
        {
            if (token->type == TokenType::Image)
            {
                if (!epub.load_resource(token->data).size())
                {
                    std::cerr << "Unable to load image: " << token->data << std::endl;
                }
            }
            tokens.push_back(*token);
        }
    }

    // Display toc
    const auto &toc = epub.get_table_of_contents();
    for (uint32_t i = 0; i < toc.size(); ++i)
    {
        const auto &toc_item = toc[i];
        std::cout << std::string(toc_item.indent_level * 2, ' ') << toc_item.display_name <<  std::endl;

        // Make sure toc address is found
        auto toc_addr = epub.get_toc_item_address(i);
        if (get_text_number(toc_addr) > 0)
        {
            bool found_matching_addr = false;
            for (const auto &token : tokens)
            {
                if (toc_addr == token.address)
                {
                    found_matching_addr = true;
                    break;
                }
            }
            if (!found_matching_addr)
            {
                std::cerr << "Exact match for toc " << toc_item.display_name << " with address " << to_string(toc_addr) << " not found" << std::endl;
            }
        }
    }

    // Display book
    TocPosition last_progress {0, 0};
    uint32_t line_count = 0;
    for (const auto &line: cli_render_tokens(tokens, 80))
    {
        auto progress = epub.get_toc_position(line.address);

        // Check for new chapter
        if (progress.toc_index != last_progress.toc_index || line_count == 0)
        {
            std::cout << "--------------------------------" << std::endl;
            std::cout << (progress.toc_index < toc.size() ? toc[progress.toc_index].display_name : "null") << std::endl;
        }

        // Check progress
        if (progress.toc_index < last_progress.toc_index)
        {
            std::cerr << "Toc went backwards for address " << to_string(line.address) << std::endl;
        }
        else if (progress.toc_index == last_progress.toc_index && progress.progress_percent < last_progress.progress_percent)
        {
            std::cerr << "Progress went backwards for address " << to_string(line.address)
                      << " (" << last_progress.progress_percent << " -> " << progress.progress_percent << ")" << std::endl;
        }

        last_progress = progress;

        std::cout << to_string(line.address) << " ";
        std::cout << std::setw(3) << progress.progress_percent;
        std::cout << "% | " << line.text << std::endl;

        ++line_count;
    }
}
