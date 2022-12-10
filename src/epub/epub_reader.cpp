#include "./epub_reader.h"

#include "./epub_doc_index.h"
#include "./epub_metadata.h"
#include "./epub_toc_index.h"
#include "util/zip_utils.h"

#include <iostream>
#include <zip.h>

struct EpubReaderState
{
    const std::string path;
    zip_t *zip = nullptr;

    std::unique_ptr<EpubDocIndex> doc_index;
    std::unique_ptr<EpubTocIndex> toc_index;
    std::vector<TocItem> user_toc;

    EpubReaderState(std::string path) : path(std::move(path)) {}
};

EPubReader::EPubReader(std::string path)
    : state(std::make_unique<EpubReaderState>(std::move(path)))
{
}

EPubReader::~EPubReader()
{
    if (!state->zip)
    {
        zip_close(state->zip);
    }
}

bool EPubReader::open()
{
    if (state->zip)
    {
        return true;
    }

    // open zip
    {
        int err = 0;
        state->zip = zip_open(state->path.c_str(), ZIP_RDONLY, &err);
        if (state->zip == nullptr)
        {
            std::cerr << "Failed to epub " << state->path
                << " code: " << err
                << std::endl;
            return false;
        }
    }

    // read container.xml
    std::string rootfile_path;
    {
        auto container_xml = read_zip_file_str(state->zip, EPUB_CONTAINER_PATH);
        if (container_xml.empty())
        {
            std::cerr << "Failed to read epub container" << std::endl;
            return false;
        }

        rootfile_path = epub_get_rootfile_path(container_xml.data());
        if (rootfile_path.empty())
        {
            std::cerr << "Unable to get docroot path" << std::endl;
            return false;
        }
    }

    // read package document
    PackageContents package;
    {
        auto package_xml = read_zip_file_str(state->zip, rootfile_path);
        if (package_xml.empty())
        {
            std::cerr << "Failed to open " << rootfile_path << std::endl;
            return false;
        }

        if (!epub_get_package_contents(rootfile_path, package_xml.data(), package))
        {
            std::cerr << "Failed to parse " << rootfile_path << std::endl;
            return false;
        }
    }

    // Parse ncx file (if avail)
    std::vector<NavPoint> navmap;
    if (!package.toc_id.empty())
    {
        auto item = package.id_to_manifest_item.find(package.toc_id);
        if (item != package.id_to_manifest_item.end() && item->second.media_type == APPLICATION_X_DTBNCX_XML)
        {
            std::string ncx_path = item->second.href_absolute;
            auto ncx_xml = read_zip_file_str(state->zip, ncx_path);

            epub_get_ncx(ncx_path, ncx_xml.data(), navmap);
        }
        else
        {
            std::cerr << "Failed to find toc document id " << package.toc_id << " or unknown media type" << std::endl;
        }
    }

    state->doc_index = std::make_unique<EpubDocIndex>(package, state->zip);
    state->toc_index = std::make_unique<EpubTocIndex>(package, navmap, *state->doc_index.get());

    // Compile user table of contents
    state->user_toc.reserve(state->toc_index->toc_size());
    for (uint32_t i = 0; i < state->toc_index->toc_size(); ++i)
    {
        state->user_toc.push_back(TocItem {
            state->toc_index->toc_item_display_name(i),
            state->toc_index->toc_item_indent_level(i),
        });
    }

    return true;
}

bool EPubReader::is_open() const
{
    return state->zip != nullptr;
}

const std::vector<TocItem> &EPubReader::get_table_of_contents() const
{
    return state->user_toc;
}

TocPosition EPubReader::get_toc_position(const DocAddr &address) const
{
    auto [pos, size] = state->toc_index->get_toc_item_progress(address);
    
    uint32_t percent = 100;
    if (size)
    {
        percent = pos * 100 / size;
        if (percent > 100)
        {
            std::cerr << "Got unexpected progress value for address " << to_string(address) << std::endl;
            percent = 100;
        }
    }
    return {
        state->toc_index->get_toc_item_index(address).value_or(0),
        percent
    };
}

DocAddr EPubReader::get_toc_item_address(uint32_t toc_item_index) const
{
    return state->toc_index->get_toc_item_address(toc_item_index);
}

const std::vector<DocToken> &EPubReader::load_chapter(const DocAddr &address) const
{
    uint32_t spine_index = get_chapter_number(address);
    return state->doc_index->tokens(spine_index);
}

DocAddr EPubReader::get_first_chapter_address() const
{
    for (uint32_t spine_index = 0; spine_index < state->doc_index->spine_size(); ++spine_index)
    {
        if (!state->doc_index->empty(spine_index))
        {
            return make_address(spine_index);
        }
    }
    return make_address();
}

std::optional<DocAddr> EPubReader::get_prev_chapter_address(const DocAddr &address) const
{
    uint32_t spine_index = std::min(
        get_chapter_number(address),
        state->doc_index->spine_size() - 1
    );
    while (spine_index-- > 0)
    {
        if (!state->doc_index->empty(spine_index))
        {
            return make_address(spine_index);
        }
    }
    return std::nullopt;
}

std::optional<DocAddr> EPubReader::get_next_chapter_address(const DocAddr &address) const
{
    uint32_t spine_index = get_chapter_number(address);
    while (++spine_index < state->doc_index->spine_size())
    {
        if (!state->doc_index->empty(spine_index))
        {
            return make_address(spine_index);
        }
    }
    return std::nullopt;
}
