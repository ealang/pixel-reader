#ifndef EPUB_READER_H_
#define EPUB_READER_H_

#include "doc_api/doc_token.h"
#include "./epub_token_iter.h"

#include <filesystem>
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

    EPubTokenIter get_iter(DocAddr address = make_address()) const;

    std::vector<char> load_resource(std::filesystem::path path) const;
};

#endif
