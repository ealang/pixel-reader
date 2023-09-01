#include "./epub_reader.h"

#include "./epub_doc_index.h"
#include "./epub_metadata.h"
#include "./epub_toc_index.h"
#include "./epub_token_iter.h"
#include "util/zip_utils.h"

#include "extern/hash-library/md5.h"

#include <algorithm>
#include <iostream>
#include <zip.h>

struct EpubReaderState
{
    std::filesystem::path path;
    zip_t *zip = nullptr;

    std::string package_md5;

    std::unique_ptr<EpubDocIndex> doc_index;
    std::unique_ptr<EpubTocIndex> toc_index;
    std::vector<TocItem> user_toc;

    EpubReaderState(std::string path) : path(std::move(path)) {}
};

EPubReader::EPubReader(std::filesystem::path path)
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

bool EPubReader::open(DocReaderCache &)
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

        rootfile_path = epub_parse_rootfile_path(container_xml.data());
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

        state->package_md5 = MD5()(package_xml.data(), package_xml.size());

        if (!epub_parse_package_contents(rootfile_path, package_xml.data(), package))
        {
            std::cerr << "Failed to parse " << rootfile_path << std::endl;
            return false;
        }
    }

    std::vector<NavPoint> navmap;

    // Parse ncx file (if avail)
    if (!package.toc_id.empty())
    {
        auto item = package.id_to_manifest_item.find(package.toc_id);
        if (item != package.id_to_manifest_item.end() && item->second.media_type == APPLICATION_X_DTBNCX_XML)
        {
            auto ncx_path = item->second.href_absolute;
            auto ncx_xml = read_zip_file_str(state->zip, ncx_path);

            epub_parse_ncx(ncx_path, ncx_xml.data(), navmap);
        }
        else
        {
            std::cerr << "Failed to find toc document id " << package.toc_id << " or unknown media type" << std::endl;
        }
    }

    // Parse nav file (if avail)
    if (navmap.empty())
    {
        auto nav_item = std::find_if(
            package.id_to_manifest_item.begin(),
            package.id_to_manifest_item.end(),
            [](const auto &item) {
                return item.second.media_type == APPLICATION_XHTML_XML && item.second.properties == "nav";
            }
        );
        if (nav_item != package.id_to_manifest_item.end())
        {
            auto nav_path = nav_item->second.href_absolute;
            auto nav_xml = read_zip_file_str(state->zip, nav_path);

            epub_parse_nav(nav_path, nav_xml.data(), navmap);
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

std::string EPubReader::get_id() const
{
    return state->package_md5;
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

std::shared_ptr<TokenIter> EPubReader::get_iter(DocAddr address) const
{
    return std::make_shared<EPubTokenIter>(
        state->doc_index.get(),
        address
    );
}

std::vector<char> EPubReader::load_resource(const std::filesystem::path &path) const
{
    return read_zip_file_str(state->zip, path);
}
