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

DocAddr get_document_start_address(const PackageContents &package, const std::string &document_id)
{
    auto it = std::find(package.spine_ids.begin(), package.spine_ids.end(), document_id);
    if (it == package.spine_ids.end())
    {
        return 0;
    }

    return make_address(
        it - package.spine_ids.begin(),
        0
    );
}

std::optional<std::string> get_document_id_for_address(const PackageContents &package, const DocAddr &address)
{
    uint32_t spine_index = get_chapter_number(address);
    if (spine_index >= package.spine_ids.size())
    {
        return std::nullopt;
    }

    return package.spine_ids[spine_index];
}

void _flatten_navmap_to_toc(
    const PackageContents &package,
    const std::unordered_map<std::string, DocAddr> &toc_path_to_address,
    const std::vector<NavPoint> &navmap,
    std::vector<TocItem> &out_toc,
    uint32_t indent_level = 0
)
{
    for (const auto &navpoint : navmap)
    {
        // TODO: need to pay attention to fragment be able to link within document
        auto address_it = toc_path_to_address.find(strip_fragment(navpoint.src_absolute));
        if (address_it != toc_path_to_address.end())
        {
            out_toc.emplace_back(
                address_it->second,
                navpoint.label,
                indent_level
            );
        }
        else
        {
            std::cerr << "TOC: Unable to find document for " << navpoint.src << std::endl;
        }

        _flatten_navmap_to_toc(
            package,
            toc_path_to_address,
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
    std::unordered_map<std::string, DocAddr> toc_path_to_address;
    for (const auto &[doc_id, item] : package.id_to_manifest_item)
    {
        if (item.media_type == APPLICATION_XHTML_XML)
        {
            toc_path_to_address[item.href_absolute] = get_document_start_address(package, doc_id);
        }
    }

    return _flatten_navmap_to_toc(
        package,
        toc_path_to_address,
        navmap,
        out_toc
    );
}


}  // namespace

TocItem::TocItem(DocAddr address, const std::string &display_name, uint32_t indent_level)
    : address(address)
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
            table_of_contents.emplace_back(
                get_document_start_address(package, doc_id),
                std::filesystem::path(item.href).filename(),
                0
            );
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

uint32_t EPubReader::get_toc_index(const DocAddr &address) const
{
    uint32_t best_index = 0;
    for (uint32_t i = 0; i < table_of_contents.size(); i++)
    {
        if (table_of_contents[i].address > address)
        {
            break;
        }
        best_index = i;
    }

    return best_index;
}

std::vector<DocToken> EPubReader::get_tokenized_document(const DocAddr &address) const
{
    auto doc_id_opt = get_document_id_for_address(package, address);
    if (!doc_id_opt)
    {
        std::cerr << "Unable to get document for address " << to_string(address) << std::endl;
        return {};
    }

    const std::string &doc_id = doc_id_opt.value();

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

std::optional<uint32_t> EPubReader::get_document_id_order_index(const DocAddr &address) const
{
    // Get the document id for the current address
    auto cur_doc_id_opt = get_document_id_for_address(package, address);
    if (!cur_doc_id_opt)
    {
        return std::nullopt;
    }
    auto &cur_doc_id = cur_doc_id_opt.value();

    // Lookup index in document order
    uint32_t order_index = 
        std::find(document_id_order.begin(), document_id_order.end(), cur_doc_id) - 
        document_id_order.begin();

    if (order_index >= document_id_order.size())
    {
        return std::nullopt;
    }

    return order_index;
}


std::optional<DocAddr> EPubReader::get_start_address() const
{
    if (document_id_order.empty())
    {
        return std::nullopt;
    }

    return get_document_start_address(package, document_id_order.front());
}

std::optional<DocAddr> EPubReader::get_previous_doc_address(const DocAddr &address) const
{
    // Get index of document id order
    auto doc_index_opt = get_document_id_order_index(address);
    if (!doc_index_opt)
    {
        return std::nullopt;
    }

    // Get previous document id
    uint32_t doc_index = *doc_index_opt;
    if (doc_index == 0)
    {
        return std::nullopt;
    }

    return get_document_start_address(package, document_id_order[doc_index - 1]);
}

std::optional<DocAddr> EPubReader::get_next_doc_address(const DocAddr &address) const
{
    auto doc_index_opt = get_document_id_order_index(address);
    if (!doc_index_opt)
    {
        return std::nullopt;
    }

    uint32_t doc_index = *doc_index_opt;
    if (doc_index >= document_id_order.size() - 1)
    {
        return std::nullopt;
    }

    return get_document_start_address(package, document_id_order[doc_index + 1]);
}
