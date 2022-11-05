#include <iostream>
#include <zip.h>

#include "epub_util.h"
#include "epub_reader.h"

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
    :_path(path), _zip(0)
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
        auto container_str = _read_zip_file_str(_zip, EPUB_CONTAINER_PATH);
        if (container_str.empty())
        {
            std::cerr << "Failed to read epub container" << std::endl;
            return false;
        }

        rootfile_path = epub_get_rootfile_path(container_str.data());
        if (rootfile_path.empty())
        {
            std::cerr << "Unable to get docroot path" << std::endl;
            return false;
        }
    }

    // read package document
    {
        auto package_str = _read_zip_file_str(_zip, rootfile_path);
        if (package_str.empty())
        {
            std::cerr << "Failed to read " << rootfile_path << std::endl;
            return false;
        }
        bool parsed_package = epub_get_package_contents(package_str.data(), _package);

        for (auto it: _package.spine_ids)
        {
            std::cout << it << " " << _package.id_to_manifest_item[it].href << std::endl;
        }

        return parsed_package;
    }

    return true;
}
