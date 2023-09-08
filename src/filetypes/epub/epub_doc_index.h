#ifndef EPUB_DOC_INDEX_H_
#define EPUB_DOC_INDEX_H_

#include "./epub_metadata.h"
#include "doc_api/doc_token.h"

#include <zip.h>

#include <filesystem>
#include <unordered_map>
#include <vector>

struct Document
{
    std::filesystem::path zip_path;

    bool cache_is_valid;
    std::vector<std::unique_ptr<DocToken>> tokens_cache;
    std::unordered_map<std::string, DocAddr> id_to_addr_cache;

    Document();
    Document(std::filesystem::path zip_path);
};

// Provide access to documents listed in the spine.
// Documents are addressed by spine index. Lazy load from zip.
class EpubDocIndex
{
    zip_t *zip;
    mutable std::vector<Document> spine_entries;

    const std::vector<std::unique_ptr<DocToken>> &ensure_cached(uint32_t spine_index) const;

public:
    EpubDocIndex(const PackageContents &package, zip_t *zip);
    EpubDocIndex(const EpubDocIndex &) = delete;
    EpubDocIndex &operator=(const EpubDocIndex &) = delete;

    // Number of spine entries
    uint32_t spine_size() const;

    // Number of tokens in spine entry
    uint32_t token_count(uint32_t spine_index) const;
    // True if spine has no tokens
    bool empty(uint32_t spine_index) const;

    // Address space consumed by spine entry
    uint32_t address_width(uint32_t spine_index) const;

    const std::vector<std::unique_ptr<DocToken>> &tokens(uint32_t spine_index) const;
    const std::unordered_map<std::string, DocAddr> &elem_id_to_address(uint32_t spine_index) const;
};

#endif
