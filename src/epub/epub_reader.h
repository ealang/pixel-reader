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
    std::string document_id;
    std::string display_name;
    uint32_t indent_level;

    TocItem(
        const std::string &document_id,
        const std::string &display_name,
        uint32_t indent_level
    );
};

// Abstract interface to a specific ebook format
class Reader
{
public:
    virtual bool open() = 0;
    virtual bool is_open() const = 0;

    virtual const std::vector<TocItem> &get_table_of_contents() const = 0;
    virtual uint32_t get_toc_index(DocAddr address) const = 0;

    // Return item ids in linear document order
    virtual const std::vector<std::string> &get_document_id_order() const = 0;

    virtual std::optional<std::string> get_document_id(DocAddr address) const = 0;

    virtual std::vector<DocToken> get_tokenized_document(std::string document_id) const = 0;
};

class EPubReader: public Reader
{
    const std::string path;
    zip_t *zip;

    PackageContents package;
    std::vector<TocItem> table_of_contents;
    std::vector<std::string> document_id_order;

    std::vector<char> read_file_as_bytes(std::string href) const;

public:
    EPubReader(std::string path);
    virtual ~EPubReader();

    bool open() override;
    bool is_open() const override;

    virtual const std::vector<TocItem> &get_table_of_contents() const override;
    virtual uint32_t get_toc_index(DocAddr address) const override;

    // Return item ids in linear document order
    virtual const std::vector<std::string> &get_document_id_order() const override;

    virtual std::optional<std::string> get_document_id(DocAddr address) const override;

    virtual std::vector<DocToken> get_tokenized_document(std::string document_id) const override;
};

#endif
