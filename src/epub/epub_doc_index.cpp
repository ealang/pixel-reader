#include "./epub_doc_index.h"

#include "./xhtml_parser.h"
#include "util/zip_utils.h"

#include <iostream>

#define DEBUG 0

Document::Document() : cache_is_valid(true) {}

Document::Document(std::filesystem::path zip_path)
    : zip_path(zip_path), cache_is_valid(false)
{}

const std::vector<DocToken> &EpubDocIndex::ensure_cached(uint32_t spine_index) const
{
    static const std::vector<DocToken> empty_tokens;

    if (spine_index >= spine_entries.size())
    {
        std::cerr << "Requested tokens in invalid spine index: " << spine_index << std::endl;
        return empty_tokens;
    }

    auto &document = spine_entries[spine_index];
    if (!document.cache_is_valid)
    {
        #if DEBUG
        std::cerr << "Loading " << document.zip_path << std::endl;
        #endif
        auto bytes = read_zip_file_str(zip, document.zip_path);

        if (bytes.empty())
        {
            std::cerr << "Unable to read item " << document.zip_path << std::endl;
            return empty_tokens;
        }

        parse_xhtml_tokens(
            bytes.data(),
            document.zip_path.filename(),
            spine_index,
            document.tokens_cache,
            document.id_to_addr_cache
        );
        document.cache_is_valid = true;
    }

    return document.tokens_cache;
}

EpubDocIndex::EpubDocIndex(const PackageContents &package, zip_t *zip)
    : zip(zip)
{
    for (uint32_t spine_index = 0; spine_index < package.spine_ids.size(); ++spine_index)
    {
        const auto &doc_id = package.spine_ids[spine_index];
        auto item_it = package.id_to_manifest_item.find(doc_id);
        if (item_it != package.id_to_manifest_item.end() && item_it->second.media_type == APPLICATION_XHTML_XML)
        {
            spine_entries.emplace_back(item_it->second.href_absolute);
        }
        else
        {
            std::cerr << "Skipping spine doc " << doc_id << " in manifest" << std::endl;
            spine_entries.emplace_back();
        }
    }
}

uint32_t EpubDocIndex::spine_size() const
{
    return spine_entries.size();
}

bool EpubDocIndex::empty(uint32_t spine_index) const
{
    return ensure_cached(spine_index).size() == 0;
}

const std::vector<DocToken> &EpubDocIndex::tokens(uint32_t spine_index) const
{
    return ensure_cached(spine_index);
}

const std::unordered_map<std::string, DocAddr> &EpubDocIndex::elem_id_to_address(uint32_t spine_index) const
{
    ensure_cached(spine_index);
    return spine_entries[spine_index].id_to_addr_cache;
}
