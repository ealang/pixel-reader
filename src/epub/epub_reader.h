#ifndef EPUB_READER_H_
#define EPUB_READER_H_

#include "./epub_metadata.h"
#include "doc_api/doc_token.h"

#include <string>
#include <vector>

typedef struct zip zip_t;

struct TokItem
{
    std::string doc_id;
    std::string name;

    TokItem(std::string doc_id, std::string name);
};

class EPubReader
{
    const std::string _path;
    zip_t *_zip;
    PackageContents _package;
    std::vector<TokItem> _tok_items;
public:
    EPubReader(std::string path);
    virtual ~EPubReader();

    bool open();
    bool is_open() const;
    std::vector<char> get_file_as_bytes(std::string href) const;

    const std::vector<TokItem> &get_tok() const;
    std::vector<DocToken> get_tokenized_document(std::string item_id) const;
};

#endif
