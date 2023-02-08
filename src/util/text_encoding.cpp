#include "./text_encoding.h"

#include <cstring>
#include <fstream>
#include <iostream>

#include <iconv.h>
#include <uchardet/uchardet.h>

bool load_binary_file(const std::filesystem::path &file_path, std::vector<char> &data_out, bool reserve_null)
{
    std::ifstream fp(file_path, std::ios::binary);
    if (!fp.is_open())
    {
        return false;
    }

    fp.seekg(0, fp.end);
    int file_size = fp.tellg();
    fp.seekg(0, fp.beg);

    data_out.reserve(file_size + (reserve_null ? 1 : 0));
    data_out.insert(
        data_out.end(),
        std::istreambuf_iterator<char>(fp),
        std::istreambuf_iterator<char>()
    );

    return true;
}

std::optional<std::string> detect_text_encoding(const char *data, uint32_t size)
{
    std::optional<std::string> result;

    uchardet_t detector = uchardet_new();
    if (detector)
    {
        int code = uchardet_handle_data(detector, data, size);
        if (code == 0)
        {
            uchardet_data_end(detector);
            std::string encoding_str = uchardet_get_charset(detector);
            if (encoding_str != "")
            {
                result = encoding_str;
            }
        }
        else
        {
            std::cerr << "uchardet failed to read data: " << code << std::endl;
        }

    }
    else
    {
        std::cerr << "Failed to start uchardet" << std::endl;
    }

    return result;
}

bool re_encode_text(const char *_source, uint32_t source_size, std::string from_encoding, std::string to_encoding, std::vector<char> &data_out)
{
    iconv_t converter = iconv_open(to_encoding.c_str(), from_encoding.c_str());
    if (converter == (iconv_t)(-1))
    {
        std::cerr << "iconv_open failed: " << strerror(errno) << std::endl;
        return false;
    }

    char *source = (char *)_source;
    size_t source_size_left = source_size;

    std::vector<char> buffer;
    constexpr size_t buffer_size = 1024 * 256;
    buffer.reserve(buffer_size);

    bool success = true;
    while (source_size_left > 0)
    {
        char *dest = buffer.data();
        size_t dest_size_left = buffer_size;

        if (iconv(converter, &source, &source_size_left, &dest, &dest_size_left) == (size_t)(-1))
        {
            if (errno != E2BIG)
            {
                std::cerr << "iconv failed: " << strerror(errno) << std::endl;
                success = false;
                break;
            }
        }

        size_t buffer_used = buffer_size - dest_size_left;
        if (buffer_used == 0)
        {
            break;
        }
        data_out.insert(data_out.end(), buffer.begin(), buffer.begin() + buffer_used);
    }

    // null terminate
    data_out.push_back(0);

    iconv_close(converter);

    return success;
}
