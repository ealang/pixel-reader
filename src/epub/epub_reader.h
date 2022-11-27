#ifndef EPUB_READER_H_
#define EPUB_READER_H_

#include "./epub_metadata.h"
#include "doc_api/doc_token.h"

#include <optional>
#include <string>
#include <vector>

typedef struct zip zip_t;

struct TocItem
{
    DocAddr address;
    std::string display_name;
    uint32_t indent_level;

    TocItem(
        DocAddr address,
        const std::string &display_name,
        uint32_t indent_level
    );
};

class EPubReader
{
    const std::string path;
    zip_t *zip;

    PackageContents package;
    std::vector<TocItem> table_of_contents;     // Table of contents (may not include all documents)
    std::vector<std::string> document_id_order; // All documents in order

    std::vector<char> read_file_as_bytes(std::string href) const;

    // Get index of the address in the `document_id_order` vector
    std::optional<uint32_t> get_document_id_order_index(const DocAddr &address) const;

public:
    EPubReader(std::string path);
    virtual ~EPubReader();

    bool open();
    bool is_open() const;

    const std::vector<TocItem> &get_table_of_contents() const;
    uint32_t get_toc_index(const DocAddr &address) const;

    std::vector<DocToken> get_tokenized_document(const DocAddr &address) const;

    std::optional<DocAddr> get_start_address() const;
    std::optional<DocAddr> get_previous_doc_address(const DocAddr &address) const;
    std::optional<DocAddr> get_next_doc_address(const DocAddr &address) const;
};

#endif
