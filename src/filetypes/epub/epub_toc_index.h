#ifndef EPUB_TOC_INDEX_H_
#define EPUB_TOC_INDEX_H_

#include <cstdint>
#include <optional>
#include <vector>

#include "./epub_doc_index.h"
#include "doc_api/doc_addr.h"

struct EpubTocIndexState;

// Provide information about table of contents entries.
// Lazy load from zip as needed.
class EpubTocIndex
{
    std::unique_ptr<EpubTocIndexState> state;

public:
    EpubTocIndex(const PackageContents &package, const std::vector<NavPoint> &navmap, EpubDocIndex &doc_index);
    EpubTocIndex(const EpubTocIndex &) = delete;
    EpubTocIndex &operator=(const EpubTocIndex &) = delete;
    virtual ~EpubTocIndex();

    uint32_t toc_size() const;
    std::string toc_item_display_name(uint32_t toc_item_index) const;
    uint32_t toc_item_indent_level(uint32_t toc_item_index) const;

    DocAddr get_toc_item_address(uint32_t toc_item_index) const;
    std::optional<uint32_t> get_toc_item_index(const DocAddr &address) const;

    // Return (pos inside, size of) the toc item in units of address space.
    std::pair<uint32_t, uint32_t> get_toc_item_progress(const DocAddr &address) const;
};

#endif
