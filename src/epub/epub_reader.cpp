#include "./epub_reader.h"
#include "./xhtml_parser.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <zip.h>

#define APPLICATION_XHTML_XML "application/xhtml+xml"
#define APPLICATION_X_DTBNCX_XML "application/x-dtbncx+xml"

namespace {

// Read zip file contents as a null-terminated string
std::vector<char> read_zip_file_str(zip_t *zip, const std::string &filepath)
{
    if (zip == nullptr)
    {
        throw std::runtime_error("Zip is not open");
    }

    zip_stat_t stats;
    if (zip_stat(zip, filepath.c_str(), 0, &stats) != 0 || !(stats.valid & ZIP_STAT_SIZE))
    {
        std::cerr << "Unable to get size of " << filepath << " in epub" << std::endl;
        return {};
    }

    zip_uint64_t size = stats.size;
    std::vector<char> buffer(size + 1);

    zip_file_t *fp = zip_fopen(zip, filepath.c_str(), 0);
    if (fp == nullptr)
    {
        std::cerr << "Unable to open " << filepath << " in epub" << std::endl;
        return {};
    }

    auto read_size = zip_fread(fp, buffer.data(), size);
    if ((zip_uint64_t)read_size != size)
    {
        std::cerr << "Read unexpected number of bytes for " << filepath << " in epub"
            << " expected " << size
            << " got " << read_size
            << std::endl;
    }

    zip_fclose(fp);

    return buffer;
}

std::string strip_fragment(const std::string &url)
{
    auto fragment_pos = url.find('#');
    if (fragment_pos != std::string::npos)
    {
        return url.substr(0, fragment_pos);
    }
    return url;
}

void _flatten_navmap_to_toc(
    const PackageContents &package,
    const std::unordered_map<std::string, std::string> &path_to_document_id,
    const std::vector<NavPoint> &navmap,
    std::vector<TocItem> &out_toc,
    uint32_t indent_level = 0
)
{
    for (const auto &navpoint : navmap)
    {
        auto doc_id_it = path_to_document_id.find(strip_fragment(navpoint.src_absolute));
        if (doc_id_it != path_to_document_id.end())
        {
            const auto &doc_id = doc_id_it->second;
            const auto &item = package.id_to_manifest_item.at(doc_id);
            if (item.media_type == APPLICATION_XHTML_XML)
            {
                out_toc.emplace_back(
                    doc_id,
                    navpoint.label,
                    indent_level
                );
            }
            else
            {
                std::cerr << "TOC: Unknown media type for path " << navpoint.src_absolute << std::endl;
            }
        }
        else
        {
            std::cerr << "TOC: Unable to find document id for path " << navpoint.src_absolute << std::endl;
        }

        _flatten_navmap_to_toc(
            package,
            path_to_document_id,
            navpoint.children,
            out_toc,
            indent_level + 1
        );
    }
}

void flatten_navmap_to_toc(
    const PackageContents &package,
    const std::vector<NavPoint> &navmap,
    std::vector<TocItem> &out_toc
)
{
    // Build path lookup
    std::unordered_map<std::string, std::string> path_to_document_id;
    for (const auto &[doc_id, item] : package.id_to_manifest_item)
    {
        path_to_document_id[item.href_absolute] = doc_id;
    }

    return _flatten_navmap_to_toc(
        package,
        path_to_document_id,
        navmap,
        out_toc
    );
}


}  // namespace

TocItem::TocItem(const std::string &document_id, const std::string &display_name, uint32_t indent_level)
    : document_id(document_id)
    , display_name(display_name)
    , indent_level(indent_level)
{
}

EPubReader::EPubReader(std::string path)
    : path(path), zip(nullptr)
{
}

EPubReader::~EPubReader()
{
    if (!zip)
    {
        zip_close(zip);
    }
}

bool EPubReader::open()
{
    if (zip)
    {
        return true;
    }

    // open zip
    {
        int err = 0;
        zip = zip_open(path.c_str(), ZIP_RDONLY, &err);
        if (zip == nullptr)
        {
            std::cerr << "Failed to epub " << path
                << " code: " << err
                << std::endl;
            return false;
        }
    }

    // read container.xml
    std::string rootfile_path;
    {
        auto container_xml = read_zip_file_str(zip, EPUB_CONTAINER_PATH);
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
    {
        auto package_xml = read_zip_file_str(zip, rootfile_path);
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

    // compile document order
    {
        for (auto &doc_id : package.spine_ids)
        {
            auto item = package.id_to_manifest_item.find(doc_id);
            if (item != package.id_to_manifest_item.end() && item->second.media_type == APPLICATION_XHTML_XML)
            {
                document_id_order.emplace_back(doc_id);
            }
        }
    }

    // compile toc
    bool created_toc = false;
    if (!package.toc_id.empty())
    {
        auto item = package.id_to_manifest_item.find(package.toc_id);
        if (item != package.id_to_manifest_item.end() && item->second.media_type == APPLICATION_X_DTBNCX_XML)
        {
            std::string ncx_path = item->second.href_absolute;
            auto ncx_xml = read_zip_file_str(zip, ncx_path);

            std::vector<NavPoint> navmap;
            if (epub_get_ncx(ncx_path, ncx_xml.data(), navmap))
            {
                created_toc = true;
                flatten_navmap_to_toc(package, navmap, table_of_contents);
            }
        }
        else
        {
            std::cerr << "Failed to find toc " << package.toc_id << " or unknown media type" << std::endl;
        }
    }

    if (!created_toc)
    {
        std::cerr << "No toc, falling back to spine" << std::endl;
        for (const auto &doc_id : document_id_order)
        {
            const auto &item = package.id_to_manifest_item.at(doc_id);
            table_of_contents.emplace_back(doc_id, std::filesystem::path(item.href).filename(), 0);
        }
    }

    return true;
}

bool EPubReader::is_open() const
{
    return zip != nullptr;
}

std::vector<char> EPubReader::read_file_as_bytes(std::string href) const
{
    return read_zip_file_str(zip, href);
}

const std::vector<TocItem> &EPubReader::get_table_of_contents() const
{
    return table_of_contents;
}

uint32_t EPubReader::get_toc_index(DocAddr address) const
{
    auto document_id = get_document_id(address);
    if (!document_id)
    {
        return 0;
    }

    for (uint32_t i = 0; i < table_of_contents.size(); i++)
    {
        if (table_of_contents[i].document_id == document_id)
        {
            return i;
        }
    }
    return 0;
}

const std::vector<std::string> &EPubReader::get_document_id_order() const
{
    return document_id_order;
}

std::optional<std::string> EPubReader::get_document_id(DocAddr address) const
{
    uint32_t spine_index = get_chapter_number(address);
    if (spine_index >= package.spine_ids.size())
    {
        return std::nullopt;
    }

    return package.spine_ids[spine_index];
}

std::vector<DocToken> EPubReader::get_tokenized_document(std::string doc_id) const
{
    auto it = package.id_to_manifest_item.find(doc_id);
    if (it == package.id_to_manifest_item.end())
    {
        std::cerr << "Unable to find item in manifest " << doc_id << std::endl;
        return {};
    }
    auto item = it->second;

    auto bytes = read_file_as_bytes(item.href_absolute);
    if (bytes.empty())
    {
        std::cerr << "Unable to read item " << doc_id << std::endl;
        return {};
    }

    if (item.media_type != APPLICATION_XHTML_XML)
    {
        std::cerr << "Unable to parse item " << doc_id << " with media type " << item.media_type << std::endl;
        return {};
    }

    uint32_t chapter_index = (
        std::find(package.spine_ids.begin(), package.spine_ids.end(), doc_id) -
        package.spine_ids.begin()
    );
    if (chapter_index >= package.spine_ids.size())
    {
        std::cerr << "Unable to find item in toc " << doc_id << std::endl;
        return {};
    }

    return parse_xhtml_tokens(bytes.data(), doc_id, chapter_index);
}
