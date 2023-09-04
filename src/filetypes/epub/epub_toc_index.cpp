#include "./epub_toc_index.h"
#include "doc_api/token_addressing.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>

#define DEBUG 0
#define EMPTY_STRING ""

struct EpubTocItem
{
    std::string display_name;
    uint32_t indent_level;
    uint32_t spine_start_index;
    std::string token_id_link;
};

// Provides chapter/global progress lookup for part of a spine document.
struct ProgressFragment
{
    DocAddr start_address;
    uint32_t toc_index;

    ProgressFragment(
        DocAddr start_address,
        uint32_t toc_index
    ) : start_address(start_address), toc_index(toc_index) {}

    uint32_t toc_offset = 0;    // How far into current chapter
    uint32_t toc_width = 0;     // Total chapter size

    uint32_t book_offset = 0;   // How far into book
    uint32_t book_width = 0;    // Total book size
};

struct EpubTocIndexState
{
    std::vector<EpubTocItem> toc;
    std::vector<ProgressFragment> progress_lookup;
};

namespace {

std::pair<std::string, std::string> split_path_fragment(const std::string &url)
{
    auto fragment_pos = url.find('#');
    if (fragment_pos != std::string::npos)
    {
        return { url.substr(0, fragment_pos), url.substr(fragment_pos + 1) };
    }
    return {url, EMPTY_STRING};
}

void _flatten_navmap_to_toc(
    const PackageContents &package,
    const std::unordered_map<std::string, uint32_t> &path_to_spine_idx,
    const std::vector<NavPoint> &navmap,
    std::vector<EpubTocItem> &out_toc,
    uint32_t indent_level = 0
)
{
    for (const auto &navpoint : navmap)
    {
        auto [path, fragment] = split_path_fragment(navpoint.src_absolute);
        auto spine_it = path_to_spine_idx.find(path);
        if (spine_it != path_to_spine_idx.end())
        {
            out_toc.emplace_back(EpubTocItem{
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
    std::vector<EpubTocItem> &out_toc
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

void fallback_spine_to_toc(
    const PackageContents &package,
    std::vector<EpubTocItem> &out_toc
)
{
    for (uint32_t spine_idx = 0; spine_idx < package.spine_ids.size(); ++spine_idx)
    {
        const auto &doc_id = package.spine_ids[spine_idx];
        auto item_it = package.id_to_manifest_item.find(doc_id);
        if (item_it != package.id_to_manifest_item.end())
        {
            if (item_it->second.media_type == APPLICATION_XHTML_XML)
            {
                out_toc.emplace_back(EpubTocItem {
                    std::filesystem::path(item_it->second.href).filename(),  // fallback name
                    0,
                    spine_idx,
                    EMPTY_STRING
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

void resolve_progress_cache(
    const std::vector<EpubTocItem> &toc,
    const EpubDocIndex &doc_index,
    std::vector<ProgressFragment> &fragments
)
{
    // Chapter fragments
    std::vector<ProgressFragment> chapter_fragments;
    {
        chapter_fragments.reserve(toc.size());
        for (uint32_t toc_index = 0; toc_index < toc.size(); ++toc_index)
        {
            const auto &toc_item = toc[toc_index];

            DocAddr address = make_address(toc_item.spine_start_index);
            if (toc_item.token_id_link != EMPTY_STRING)
            {
                const auto &elem_id_to_addr = doc_index.elem_id_to_address(toc_item.spine_start_index);
                const auto addr_it = elem_id_to_addr.find(toc_item.token_id_link);
                if (addr_it != elem_id_to_addr.end())
                {
                    address = addr_it->second;
                }
                else
                {
                    std::cerr << "Unable to find elem '"
                        << toc_item.token_id_link
                        << "' in spine " << toc_item.spine_start_index
                        << " for toc " << toc_index << std::endl;
                }
            }
            chapter_fragments.emplace_back(address, toc_index);
        }

        auto cmp_addr = [](const ProgressFragment &a, const ProgressFragment &b) {
            return a.start_address < b.start_address;
        };
        std::sort(chapter_fragments.begin(), chapter_fragments.end(), cmp_addr);
    }

    // Spine fragments
    std::vector<DocAddr> spine_fragments;
    {
        uint32_t spine_size = doc_index.spine_size();
        spine_fragments.reserve(spine_size);
        for (uint32_t spine_index = 0; spine_index < spine_size; ++spine_index)
        {
            spine_fragments.emplace_back(make_address(spine_index));
        }
    }

    // Merge fragments
    fragments.reserve(spine_fragments.size() + chapter_fragments.size());
    {
        uint32_t si = 0;
        uint32_t ci = 0;
        uint32_t last_toc_index = 0;
        while (true)
        {
            bool has_next_si = si < spine_fragments.size();
            bool has_next_ci = ci < chapter_fragments.size();

            if (!has_next_si && !has_next_ci)
            {
                break;
            }
            else if (has_next_ci && (!has_next_si || chapter_fragments[ci].start_address <= spine_fragments[si]))
            {
                last_toc_index = chapter_fragments[ci].toc_index;
                fragments.emplace_back(chapter_fragments[ci++]);
            }
            else
            {
                fragments.emplace_back(spine_fragments[si++], last_toc_index);
            }
        };
    }

    // Compute fragment widths
    std::vector<uint32_t> fragment_widths;
    {
        fragment_widths.reserve(fragments.size());
        for (uint32_t i = 0; i < fragments.size(); ++i)
        {
            uint32_t fragment_width;

            auto &frag = fragments[i];
            uint32_t spine_idx = get_chapter_number(frag.start_address);
            if (
                i == fragments.size() - 1 ||
                spine_idx != get_chapter_number(fragments[i + 1].start_address)
            )
            {
                fragment_width = doc_index.upper_address(spine_idx) - frag.start_address;
            }
            else
            {
                fragment_width = fragments[i + 1].start_address - frag.start_address;
            }

            fragment_widths.emplace_back(fragment_width);
        }
    }

    // Compute chapter, book offsets
    std::vector<uint32_t> toc_offsets;
    std::vector<uint32_t> toc_widths;
    std::vector<uint32_t> book_offsets;
    uint32_t book_width = 0;
    {
        toc_offsets.reserve(fragments.size());
        toc_widths.resize(toc.size());
        book_offsets.reserve(fragments.size());

        uint32_t last_toc_index = 0;
        uint32_t cur_toc_offset = 0;
        uint32_t cur_book_offset = 0;

        for (uint32_t i = 0; i < fragments.size(); ++i)
        {
            uint32_t cur_toc_index = fragments[i].toc_index;
            if (cur_toc_index != last_toc_index)
            {
                last_toc_index = cur_toc_index;
                cur_toc_offset = 0;
            }

            toc_offsets.emplace_back(cur_toc_offset);
            book_offsets.emplace_back(cur_book_offset);

            uint32_t fragment_width = fragment_widths[i];
            cur_toc_offset += fragment_width;
            cur_book_offset += fragment_width;
            toc_widths[cur_toc_index] += fragment_width;
        }

        book_width = cur_book_offset;
    }

    // Complete lookup table
    for (uint32_t i = 0; i < fragments.size(); ++i)
    {
        auto &fragment = fragments[i];
        fragment.toc_offset = toc_offsets[i];
        fragment.toc_width = toc_widths[fragment.toc_index];
        fragment.book_offset = book_offsets[i];
        fragment.book_width = book_width;

        // std::cout
        //     << "toc=" << fragment.toc_index
        //     << " addr=" << to_string(fragment.start_address)
        //     << " toc_offs=" << fragment.toc_offset
        //     << " toc_width=" << fragment.toc_width
        //     << " book_offs=" << fragment.book_offset
        //     << " frag_width=" << std::hex << fragment_widths[i]
        //     << "\n";
    }
}

}  // namespace

EpubTocIndex::EpubTocIndex(const PackageContents &package, const std::vector<NavPoint> &navmap, EpubDocIndex &doc_index)
    : state(std::make_unique<EpubTocIndexState>())
{
    // Create table of contents
    auto &toc = state->toc;
    flatten_navmap_to_toc(package, navmap, toc);

    if (toc.empty())
    {
        std::cerr << "Falling back to spine for TOC" << std::endl;
        fallback_spine_to_toc(package, toc);
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

    // Resolve TOC addresses
    resolve_progress_cache(toc, doc_index, state->progress_lookup);
}

EpubTocIndex::~EpubTocIndex() {}

uint32_t EpubTocIndex::toc_size() const
{
    return state->toc.size();
}

std::string EpubTocIndex::toc_item_display_name(uint32_t toc_item_index) const
{
    if (toc_item_index >= state->toc.size())
    {
        return EMPTY_STRING;
    }
    return state->toc[toc_item_index].display_name;
}

uint32_t EpubTocIndex::toc_item_indent_level(uint32_t toc_item_index) const
{
    if (toc_item_index >= state->toc.size())
    {
        return 0;
    }
    return state->toc[toc_item_index].indent_level;
}

DocAddr EpubTocIndex::get_toc_item_address(uint32_t toc_item_index) const
{
    DocAddr addr = 0;
    for (const auto &fragment : state->progress_lookup)
    {
        if (fragment.toc_index > toc_item_index)
        {
            break;
        }

        addr = fragment.start_address;
    }
    return addr;
}

std::optional<uint32_t> EpubTocIndex::get_toc_item_index(const DocAddr &address) const
{
    uint32_t toc = 0;
    for (const auto &fragment : state->progress_lookup)
    {
        if (fragment.start_address > address)
        {
            break;
        }

        toc = fragment.toc_index;
    }
    return toc;
}

std::pair<uint32_t, uint32_t> EpubTocIndex::get_toc_item_progress(const DocAddr &address) const
{
    const ProgressFragment *last_frag = nullptr;
    for (const auto &fragment : state->progress_lookup)
    {
        if (fragment.start_address > address)
        {
            break;
        }

        last_frag = &fragment;
    }

    if (!last_frag)
    {
        return {0, 0};
    }

    /*
    std::cout << "------------\n";
    std::cout << "Cur addr is " << to_string(address) << "\n";
    std::cout << "Frag start is " << to_string(last_frag->start_address) << "\n";
    std::cout << "Toc offset is " << last_frag->toc_offset << "\n";
    std::cout << "Toc width is " << last_frag->toc_width << "\n";
    std::cout << "Toc idx is " << last_frag->toc_index << "\n";
    */

    // book
    return {
        last_frag->book_offset + address - last_frag->start_address,
        last_frag->book_width
    };

    // toc
    // return {
    //     last_frag->toc_offset + address - last_frag->start_address,
    //     last_frag->toc_width
    // };
}
