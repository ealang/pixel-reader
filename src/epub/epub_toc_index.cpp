#include "./epub_toc_index.h"
#include "doc_api/token_addressing.h"

#include <iostream>

#define DEBUG 0

namespace {

std::pair<std::string, std::string> separate_fragment(const std::string &url)
{
    auto fragment_pos = url.find('#');
    if (fragment_pos != std::string::npos)
    {
        return { url.substr(0, fragment_pos), url.substr(fragment_pos + 1) };
    }
    return {url, ""};
}

void _flatten_navmap_to_toc(
    const PackageContents &package,
    const std::unordered_map<std::string, uint32_t> &path_to_spine_idx,
    const std::vector<NavPoint> &navmap,
    std::vector<TocItemCache> &out_toc,
    uint32_t indent_level = 0
)
{
    for (const auto &navpoint : navmap)
    {
        auto [path, fragment] = separate_fragment(navpoint.src_absolute);
        auto spine_it = path_to_spine_idx.find(path);
        if (spine_it != path_to_spine_idx.end())
        {
            out_toc.emplace_back(TocItemCache{
                navpoint.label,
                indent_level,
                spine_it->second,
                fragment
            });
        }
        else
        {
            std::cerr << "TOC: Unable to find document for \"" << navpoint.label << "\"" << std::endl;
        }

        _flatten_navmap_to_toc(
            package,
            path_to_spine_idx,
            navpoint.children,
            out_toc,
            indent_level + 1
        );
    }
}

void flatten_navmap_to_toc(
    const PackageContents &package,
    const std::vector<NavPoint> &navmap,
    std::vector<TocItemCache> &out_toc
)
{
    // Build lookup
    std::unordered_map<std::string, uint32_t> path_to_spine_idx;
    for (uint32_t spine_idx = 0; spine_idx < package.spine_ids.size(); ++spine_idx)
    {
        const auto &doc_id = package.spine_ids[spine_idx];
        auto item_it = package.id_to_manifest_item.find(doc_id);
        if (item_it != package.id_to_manifest_item.end())
        {
            if (item_it->second.media_type == APPLICATION_XHTML_XML)
            {
                path_to_spine_idx[item_it->second.href_absolute] = spine_idx;
            }
            else
            {
                std::cerr << "Skipping toc item of media type " << item_it->second.media_type << std::endl;
            }
        }
        else
        {
            std::cerr << "Failed to find spine doc " << doc_id << " in manifest" << std::endl;
        }
    }

    return _flatten_navmap_to_toc(
        package,
        path_to_spine_idx,
        navmap,
        out_toc
    );
}

// Address above all documents.
DocAddr spine_upper_address(const EpubDocIndex &doc_index)
{
    return make_address(doc_index.spine_size());
}

// Address just above the given document.
DocAddr document_upper_address(const EpubDocIndex &doc_index, uint32_t spine_index)
{
    const auto &tokens = doc_index.tokens(spine_index);
    if (tokens.size())
    {
        const auto &last_token = tokens.back();
        return last_token.address + get_address_width(last_token);
    }
    return make_address(spine_index);
}

}  // namespace

DocAddr EpubTocIndex::resolve_start_address(uint32_t item_index) const
{
    if (item_index >= toc.size())
    {
        std::cout << "Requested start address for invalid toc item index: " << item_index << std::endl;
        return make_address();
    }

    auto &toc_item = toc[item_index];
    if (toc_item.start_address_is_valid)
    {
        return toc_item.start_address;
    }

    toc_item.start_address = make_address(toc_item.spine_start_index);
    if (!toc_item.token_id_link.empty())
    {
        // Need to match fragment
        bool found = false;
        for (const auto &id: doc_index.elem_id_to_address(toc_item.spine_start_index))
        {
            if (id.first == toc_item.token_id_link)
            {
                toc_item.start_address = id.second;
                found = true;
                break;
            }
        }
        if (!found)
        {
            #if DEBUG
            std::cerr << "Failed to find id=" << toc_item.token_id_link << " in spine " << toc_item.spine_start_index << std::endl;
            #endif
        }
    }

    toc_item.start_address_is_valid = true;
    return toc_item.start_address;
}

DocAddr EpubTocIndex::resolve_upper_address(uint32_t item_index) const
{
    static const DocAddr null_address = make_address();

    if (item_index >= toc.size())
    {
        std::cout << "Requested end address for invalid toc item index: " << item_index << std::endl;
        return null_address;
    }

    if (item_index + 1 < toc.size())
    {
        return resolve_start_address(item_index + 1);
    }
    return spine_upper_address(doc_index);
}

const TocItemProgressLookup &EpubTocIndex::resolve_progress_lookup(uint32_t item_index) const
{
    if (item_index >= toc.size())
    {
        throw std::runtime_error("Requested progress for invalid toc item index" + std::to_string(item_index));
    }

    auto &toc_item = toc[item_index];
    auto &progress = toc_item.progress;
    if (toc_item.progress_is_valid)
    {
        return progress;
    }

    // Compute size of the toc item + lookup table to compute progress for any address within the toc item.
    {
        progress.spine_to_start_addr.resize(doc_index.spine_size());
        progress.spine_to_cumulative_size.resize(doc_index.spine_size());

        DocAddr start_address = resolve_start_address(item_index);
        DocAddr upper_address = resolve_upper_address(item_index);

        uint32_t start_spine = get_chapter_number(start_address);
        uint32_t end_spine = get_chapter_number(upper_address);

        uint32_t cumulative_size = 0;
        for (uint32_t i = start_spine; i <= end_spine; ++i)
        {
            DocAddr local_start = std::max(
                start_address,
                make_address(i)
            );
            if (local_start >= upper_address)
            {
                break;
            }

            DocAddr local_end = std::min(
                upper_address,
                document_upper_address(doc_index, i)
            );

            progress.spine_to_start_addr[i] = local_start;
            progress.spine_to_cumulative_size[i] = cumulative_size;
            cumulative_size += get_text_number(local_end) - get_text_number(local_start);
        }
        progress.total_size = cumulative_size;
        toc_item.progress_is_valid = true;
    }

    return progress;
}

EpubTocIndex::EpubTocIndex(const PackageContents &package, const std::vector<NavPoint> &navmap, EpubDocIndex &doc_index)
    : doc_index(doc_index)
{
    flatten_navmap_to_toc(package, navmap, toc);

    if (toc.empty())
    {
        std::cerr << "Falling back to spine for TOC" << std::endl;
        for (uint32_t spine_idx = 0; spine_idx < package.spine_ids.size(); ++spine_idx)
        {
            const auto &doc_id = package.spine_ids[spine_idx];
            auto item_it = package.id_to_manifest_item.find(doc_id);
            if (item_it != package.id_to_manifest_item.end())
            {
                if (item_it->second.media_type == APPLICATION_XHTML_XML)
                {
                    toc.emplace_back(TocItemCache {
                        std::filesystem::path(item_it->second.href).filename(),  // fallback name
                        0,
                        spine_idx,
                        ""
                    });
                }
                else
                {
                    std::cerr << "Skipping toc item of media type " << item_it->second.media_type << std::endl;
                }
            }
            else
            {
                std::cerr << "Failed to find spine doc " << doc_id << " in manifest" << std::endl;
            }
        }
    }

    #if DEBUG
    {
        std::cerr << "TOC:" << std::endl;
        for (uint32_t i = 0; i < toc.size(); ++i)
        {
            const auto &toc_item = toc[i];
            std::cerr << "  " << std::string(toc_item.indent_level * 2, ' ')
                      << toc_item.display_name << " ("
                      << "spine_idx=" << toc_item.spine_start_index << " "
                      << "frag_link=\"" << toc_item.token_id_link << "\")" << std::endl;
        }
    }
    #endif

    {
        uint32_t last_spine_index = 0;
        for (const auto &toc_item: toc)
        {
            if (toc_item.spine_start_index < last_spine_index)
            {
                std::cerr << "Toc item " << toc_item.display_name << " is out of order" << std::endl;
            }
            last_spine_index = toc_item.spine_start_index;
        }
    }
}

uint32_t EpubTocIndex::toc_size() const
{
    return toc.size();
}

std::string EpubTocIndex::toc_item_display_name(uint32_t toc_item_index) const
{
    if (toc_item_index >= toc.size())
    {
        return "";
    }
    return toc[toc_item_index].display_name;
}

uint32_t EpubTocIndex::toc_item_indent_level(uint32_t toc_item_index) const
{
    if (toc_item_index >= toc.size())
    {
        return 0;
    }
    return toc[toc_item_index].indent_level;
}

DocAddr EpubTocIndex::get_toc_item_address(uint32_t toc_item_index) const
{
    return resolve_start_address(toc_item_index);
}

std::optional<uint32_t> EpubTocIndex::get_toc_item_index(const DocAddr &address) const
{
    if (toc.empty())
    {
        return std::nullopt;
    }

    if (address >= cached_toc_index_start_address && address < cached_toc_index_upper_address)
    {
        return cached_toc_index;
    }

    uint32_t spine_index = get_chapter_number(address);
    for (uint32_t i = 0; i < toc.size() - 1; ++i)
    {
        auto &toc_item = toc[i];
        auto &next_toc_item = toc[i + 1];

        uint32_t toc_item_spine_start = toc_item.spine_start_index;
        uint32_t toc_item_spine_end = next_toc_item.spine_start_index;
        if (spine_index >= toc_item_spine_start && spine_index <= toc_item_spine_end)
        {
            // Next chapter may not start at the beginning of the doc. Need to check addresses.
            const auto &toc_item_start_addr = resolve_start_address(i);
            const auto &toc_item_upper_addr = resolve_upper_address(i);
            if (address >= toc_item_start_addr && address < toc_item_upper_addr)
            {
                cached_toc_index = i;
                cached_toc_index_start_address  = toc_item_start_addr;
                cached_toc_index_upper_address = toc_item_upper_addr;
                return i;
            }
        }
    }

    {
        uint32_t i = toc.size() - 1;
        if (address >= resolve_start_address(i))
        {
            return i;
        }
    }

    return std::nullopt;
}

// Excluding empty address space, return (pos inside, size of) the toc item.
std::pair<uint32_t, uint32_t> EpubTocIndex::get_toc_item_progress(const DocAddr &address) const
{
    auto toc_index = get_toc_item_index(address);
    if (!toc_index || *toc_index >= toc.size())
    {
        return {0, 0};
    }

    const auto &progress = resolve_progress_lookup(*toc_index);
    uint32_t spine_index = get_chapter_number(address);

    uint32_t pos = (
        progress.spine_to_cumulative_size[spine_index] +
        static_cast<uint32_t>(address - progress.spine_to_start_addr[spine_index])
    );
    return {pos, progress.total_size};
}
