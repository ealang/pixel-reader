#include "./epub_reader.h"
#include "./xhtml_parser.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <zip.h>

#define APPLICATION_XHTML_XML "application/xhtml+xml"

TokItem::TokItem(std::string doc_id, std::string name)
    : doc_id(doc_id), name(name)
{
}

// Read contents as a null-terminated string
static std::vector<char> _read_zip_file_str(zip_t *zip, std::string filepath)
{
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

static void assert_is_open(const EPubReader *reader)
{
    if (!reader->is_open())
    {
        throw std::runtime_error("EpubReader is not open");
    }
}

EPubReader::EPubReader(std::string path)
    :_path(path), _zip(nullptr)
{
}

EPubReader::~EPubReader()
{
    if (!_zip)
    {
        zip_close(_zip);
    }
}

bool EPubReader::open()
{
    if (_zip)
    {
        return true;
    }

    // open zip
    {
        int err = 0;
        _zip = zip_open(_path.c_str(), ZIP_RDONLY, &err);
        if (_zip == nullptr)
        {
            std::cerr << "Failed to epub " << _path
                << " code: " << err
                << std::endl;
            return false;
        }
    }

    // read container.xml
    std::string rootfile_path;
    {
        auto container_xml = _read_zip_file_str(_zip, EPUB_CONTAINER_PATH);
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
        auto package_xml = _read_zip_file_str(_zip, rootfile_path);
        if (package_xml.empty())
        {
            std::cerr << "Failed to open " << rootfile_path << std::endl;
            return false;
        }

        if (!epub_get_package_contents(rootfile_path, package_xml.data(), _package))
        {
            std::cerr << "Failed to parse " << rootfile_path << std::endl;
            return false;
        }
    }

    // compile toc
    {
        for (auto &doc_id : _package.spine_ids)
        {
            auto &item = _package.id_to_manifest_item[doc_id];
            if (item.media_type == APPLICATION_XHTML_XML)
            {
                auto name = std::filesystem::path(item.href).filename(); // TODO: better name
                _tok_items.emplace_back(doc_id, name);
            }
        }
    }

    return true;
}

bool EPubReader::is_open() const
{
    return _zip != nullptr;
}

std::vector<char> EPubReader::get_file_as_bytes(std::string href) const
{
    assert_is_open(this);

    return _read_zip_file_str(_zip, href);
}

const std::vector<TokItem> &EPubReader::get_tok() const
{
    assert_is_open(this);

    return _tok_items;
}

std::vector<DocToken> EPubReader::get_tokenized_document(std::string item_id) const
{
    auto it = _package.id_to_manifest_item.find(item_id);
    if (it == _package.id_to_manifest_item.end())
    {
        std::cerr << "Unable to find item in manifest " << item_id << std::endl;
        return {};
    }
    auto item = it->second;

    auto bytes = get_file_as_bytes(item.href);
    if (bytes.empty())
    {
        std::cerr << "Unable to read item " << item_id << std::endl;
        return {};
    }

    if (item.media_type != APPLICATION_XHTML_XML)
    {
        std::cerr << "Unable to parse item " << item_id << " with media type " << item.media_type << std::endl;
        return {};
    }

    uint32_t tok_index = (
        std::find(_package.spine_ids.begin(), _package.spine_ids.end(), item_id) -
        _package.spine_ids.begin()
    );
    if (tok_index >= _tok_items.size())
    {
        std::cerr << "Unable to find item in toc " << item_id << std::endl;
        return {};
    }

    return parse_xhtml_tokens(bytes.data(), item_id, tok_index);
}
