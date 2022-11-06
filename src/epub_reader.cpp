#include "epub_reader.h"
#include "epub_util.h"
#include "xhtml_parser.h"

#include <iostream>
#include <zip.h>

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
            std::cerr << "Failed to read " << rootfile_path << std::endl;
            return false;
        }

        return epub_get_package_contents(rootfile_path, package_xml.data(), _package);
    }

    return true;
}

const PackageContents& EPubReader::get_package() const
{
    return _package;
}

std::vector<char> EPubReader::get_file_as_bytes(std::string href) const
{
    return _read_zip_file_str(_zip, href);
}

std::vector<std::string> EPubReader::get_item_as_text(std::string item_id) const
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

    if (item.media_type == "application/xhtml+xml")
    {
        return parse_xhtml_lines(bytes.data(), item_id);
    }

    std::cerr << "Unable to parse item " << item_id << " with media type " << item.media_type << std::endl;
    return {};
}

