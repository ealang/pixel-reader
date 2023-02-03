#include <iostream>
#include <libxml/parser.h>
#include <unistd.h>

#include "sys/filesystem.h"
#include "util/text_encoding.h"

void display_epub(std::string path);
void display_xhtml(std::string path);

static void ls(std::string path)
{
    for (auto entry: directory_listing(path))
    {
        std::cout << (entry.is_dir ? "D" : "F") << " " << entry.name << " " << std::endl;
    }
}

static void detect_encoding(std::string path)
{
    std::vector<char> data;
    if (!load_binary_file(path, data))
    {
        std::cout << "Unable to open file" << std::endl;
        return;
    }

    auto encoding = detect_text_encoding(data.data(), data.size());
    std::cout << encoding.value_or("Unknown") << std::endl;
}

static void utf8_cat(std::string path)
{
    std::vector<char> data;
    if (!load_binary_file(path, data))
    {
        std::cerr << "Unable to open file" << std::endl;
        return;
    }

    auto encoding = detect_text_encoding(data.data(), data.size());
    if (!encoding)
    {
        std::cerr << "Unable to detect encoding" << std::endl;
        return;
    }

    std::vector<char> data_out;
    if (!re_encode_text(data.data(), data.size(), *encoding, "UTF-8", data_out))
    {
        std::cerr << "Failed to re-encode" << std::endl;
        return;
    }

    std::cout << data_out.data() << std::endl;
}

int main(int argc, char** argv)
{
    if (argc >= 2)
    {
        std::string mode = argv[1];
        if (mode == "ls" && argc > 2)
        {
            ls(argv[2]);
        }
        else if (mode == "epub" && argc > 2)
        {
            display_epub(argv[2]);
        }
        else if (mode == "xhtml" && argc > 2)
        {
            display_xhtml(argv[2]);
        }
        else if (mode == "detect" && argc > 2)
        {
            detect_encoding(argv[2]);
        }
        else if (mode == "utf8-cat" && argc > 2)
        {
            utf8_cat(argv[2]);
        }
        else
        {
            std::cout << "Unknown args" << std::endl;
        }
    }
    else
    {
        std::cout << "Unknown args" << std::endl;
    }

    xmlCleanupParser();
    return 0;
}
