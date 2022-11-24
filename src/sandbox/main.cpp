#include <iostream>
#include <libxml/parser.h>
#include <unistd.h>

#include "sys/filesystem.h"

void ls(std::string path)
{
    for (auto entry: directory_listing(path))
    {
        std::cout << (entry.is_dir ? "D" : "F") << " " << entry.name << " " << std::endl;
    }
}

void display_epub(std::string path);
void display_xhtml(std::string path);

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
    }
    else
    {
    }

    xmlCleanupParser();
    return 0;
}
