#include <iostream>
#include "sys/filesystem.h"
#include "epub/epub_reader.h"
#include "epub/epub_metadata.h"
#include <libxml/parser.h>
#include <unistd.h>

void ls(std::string path)
{
    for (auto entry: directory_listing(path))
    {
        std::cout << (entry.is_dir ? "D" : "F") << " " << entry.name << " " << std::endl;
    }
}

void read(std::string epub_file)
{
    EPubReader reader(epub_file);
    if (reader.open())
    {
        auto package = reader.get_package();
        for (auto it: package.spine_ids)
        {
            std::cout << "---------------------------------" << std::endl;
            std::cout << it << " " << package.id_to_manifest_item[it].href << std::endl;

            for (auto line: reader.get_item_as_text(it))
            {
                std::cout << line;
            }
        }
    }
    else
    {
        std::cout << "Failed to open" << std::endl;
    }

    xmlCleanupParser();
}

void default_demo()
{

    char *cwd1 = get_current_dir_name();
    char *cwd2 = getcwd(NULL, 0);
    char *cwd3 = getenv("PWD");
    std::cout << "cwd1: " << cwd1 << std::endl;
    std::cout << "cwd2: " << cwd2 << std::endl;
    std::cout << "cwd3: " << cwd3 << std::endl;

    if (cwd1) free(cwd1);
    //if (cwd2) free(cwd2);
    // if (cwd3) free(cwd3);
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
        else if (mode == "read" && argc > 2)
        {
            read(argv[2]);
        }
    }
    else
    {
        default_demo();
    }

    return 0;
}
