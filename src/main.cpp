#include "epub_reader.h"
#include "epub_util.h"

#include <iostream>
#include <libxml/parser.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Must provide path to epub file" << std::endl;
        return 1;
    }

    const char *epub_file = argv[1];

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

    return 0;
}
