#ifndef EPUB_READER_H_
#define EPUB_READER_H_

#include "doc_api/doc_reader.h"

struct EpubReaderState;

class EPubReader: public DocReader
{
    std::unique_ptr<EpubReaderState> state;

public:
    EPubReader(std::filesystem::path path);
    EPubReader(const EPubReader &) = delete;
    EPubReader &operator=(const EPubReader &) = delete;

    virtual ~EPubReader();

    bool open() override;
    bool is_open() const override;

    const std::vector<TocItem> &get_table_of_contents() const override;
    TocPosition get_toc_position(const DocAddr &address) const override;
    DocAddr get_toc_item_address(uint32_t toc_item_index) const override;

    std::shared_ptr<TokenIter> get_iter(DocAddr address = make_address()) const override;

    std::vector<char> load_resource(std::filesystem::path path) const override;
};

#endif
