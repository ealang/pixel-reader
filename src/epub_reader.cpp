#include <iostream>
#include <zip.h>

#include "epub_util.h"
#include "epub_reader.h"

// Read contents as a null-terminated string
static char *_read_zip_file_str(zip_t *zip, const char *filepath)
{
    zip_stat_t stats;
    if (zip_stat(zip, filepath, 0, &stats) != 0) 
    {
        std::cerr << "Unable to stat " << filepath << " in epub" << std::endl;
        return nullptr;
    }

    if (!(stats.valid & ZIP_STAT_SIZE))
    {
        std::cerr << "Unable to get size of " << filepath << " in epub" << std::endl;
        return nullptr;
    }

    zip_uint64_t size = stats.size;
    char *buffer = new char[size + 1]();

    zip_file_t *fp = zip_fopen(zip, filepath, 0);
    if (fp == nullptr)
    {
        std::cerr << "Unable to open " << filepath << " in epub" << std::endl;
        return nullptr;
    }

    auto read_size = zip_fread(fp, buffer, size);
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
            std::cerr << "Failed to open " << _path
                << " code: " << err
                << std::endl;
            return false;
        }
    }

    // read container.xml
    {
        const char *container_str = _read_zip_file_str(_zip, EPUB_CONTAINER_PATH);
        if (container_str == nullptr)
        {
            std::cerr << "Failed to read epub container" << std::endl;
            return false;
        }

        std::string rootfile_path = epub_get_rootfile_path(container_str);
        delete[] container_str;

        if (!rootfile_path.size())
        {
            std::cerr << "Unable to get docroot path" << std::endl;
            return false;
        }
        std::cout << "Root file is " << rootfile_path << std::endl;
    }

    return true;
}
