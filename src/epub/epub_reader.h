#ifndef EPUB_READER_H_
#define EPUB_READER_H_

#include "doc_api/doc_token.h"

#include <optional>
#include <string>
#include <memory>
#include <vector>

struct TocItem
{
    std::string display_name;
    uint32_t indent_level;
};

struct TocPosition
{
    uint32_t toc_index;
    uint32_t progress_percent;
};

struct EpubReaderState;

class EPubReader
{
    std::unique_ptr<EpubReaderState> state;

public:
    EPubReader(std::string path);
    EPubReader(const EPubReader &) = delete;
    EPubReader &operator=(const EPubReader &) = delete;

    virtual ~EPubReader();

    bool open();
    bool is_open() const;

    const std::vector<TocItem> &get_table_of_contents() const;
    TocPosition get_toc_position(const DocAddr &address) const;
    DocAddr get_toc_item_address(uint32_t toc_item_index) const;

    const std::vector<DocToken> &load_chapter(const DocAddr &address) const;
    DocAddr get_first_chapter_address() const;
    std::optional<DocAddr> get_prev_chapter_address(const DocAddr &address) const;
    std::optional<DocAddr> get_next_chapter_address(const DocAddr &address) const;
};

#endif
