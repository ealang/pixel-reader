#ifndef EPUB_COVER_H_
#define EPUB_COVER_H_

#include <filesystem>
#include <string>
#include <vector>

struct EpubCover
{
    std::string title;
    std::string author;
    std::string href_absolute;
    std::string media_type;
    std::vector<char> data;
};

bool epub_load_preview_info(const std::filesystem::path &book_path, EpubCover &out_cover);
bool epub_load_book_metadata(const std::filesystem::path &book_path, EpubCover &out_cover);
bool epub_load_cover(const std::filesystem::path &book_path, EpubCover &out_cover);

#endif
