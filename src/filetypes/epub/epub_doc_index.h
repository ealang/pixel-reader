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
    const std::filesystem::path zip_path;

    mutable bool cache_is_valid;
    mutable std::vector<DocToken> tokens_cache;
    mutable std::unordered_map<std::string, DocAddr> id_to_addr_cache;

    Document();
    Document(std::filesystem::path zip_path);
};

// Provide access to documents listed in the spine.
// Documents are addressed by spine index. Lazy load from zip.
class EpubDocIndex
{
    zip_t *zip;
    std::vector<Document> spine_entries;

    const std::vector<DocToken> &ensure_cached(uint32_t spine_index) const;

public:
    EpubDocIndex(const PackageContents &package, zip_t *zip);
    EpubDocIndex(const EpubDocIndex &) = delete;
    EpubDocIndex &operator=(const EpubDocIndex &) = delete;

    uint32_t spine_size() const;
    bool empty(uint32_t spine_index) const;

    uint32_t token_count(uint32_t spine_index) const;

    const std::vector<DocToken> &tokens(uint32_t spine_index) const;
    const std::unordered_map<std::string, DocAddr> &elem_id_to_address(uint32_t spine_index) const;
};

#endif
