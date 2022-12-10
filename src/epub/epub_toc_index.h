#ifndef EPUB_TOC_INDEX_H_
#define EPUB_TOC_INDEX_H_

#include <cstdint>
#include <optional>
#include <vector>

#include "./epub_doc_index.h"
#include "doc_api/doc_addr.h"

struct TocItemProgressLookup
{
    uint32_t total_size = 0;                             // Total size of the toc item in units of address space
    std::vector<DocAddr> spine_to_start_addr;            // Start address for the toc item within each spine entry
    std::vector<uint32_t> spine_to_cumulative_size;      // Total size of the toc item at the start of each spine entry
};

struct TocItemCache
{
    std::string display_name;
    uint32_t indent_level;
    uint32_t spine_start_index;
    std::string token_id_link;

    // caching
    mutable bool start_address_is_valid = false;
    mutable DocAddr start_address = 0;  // address of first token for this toc item

    mutable bool progress_is_valid = false;
    mutable TocItemProgressLookup progress {0, {}, {}};
};

// Provide information about table of contents entries.
// Lazy load from zip as needed.
class EpubTocIndex
{
    EpubDocIndex &doc_index;
    std::vector<TocItemCache> toc;

    DocAddr resolve_start_address(uint32_t item_index) const;
    DocAddr resolve_upper_address(uint32_t item_index) const;
    const TocItemProgressLookup &resolve_progress_lookup(uint32_t item_index) const;

    mutable uint32_t cached_toc_index = 0;
    mutable DocAddr cached_toc_index_start_address = -1;
    mutable DocAddr cached_toc_index_upper_address = -1;

public:
    EpubTocIndex(const PackageContents &package, const std::vector<NavPoint> &navmap, EpubDocIndex &doc_index);
    EpubTocIndex(const EpubTocIndex &) = delete;
    EpubTocIndex &operator=(const EpubTocIndex &) = delete;

    uint32_t toc_size() const;
    std::string toc_item_display_name(uint32_t toc_item_index) const;
    uint32_t toc_item_indent_level(uint32_t toc_item_index) const;

    DocAddr get_toc_item_address(uint32_t toc_item_index) const;
    std::optional<uint32_t> get_toc_item_index(const DocAddr &address) const;

    // Return (pos inside, size of) the toc item in units of address space.
    std::pair<uint32_t, uint32_t> get_toc_item_progress(const DocAddr &address) const;
};

#endif
