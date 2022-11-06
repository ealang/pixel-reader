#include "epub_reader.h"
#include "epub_util.h"

#include <iostream>
#include <libxml/parser.h>

static void _display_chapter(EPubReader &reader, std::string item_id)
{
    for (auto line: reader.get_item_as_text(item_id))
    {
        std::cout << line;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "Must provide path to epub file" << std::endl;
        return 1;
    }

    const char *epub_file = argv[1];
    const char *item_id = argc == 3 ? argv[2] : nullptr;

    EPubReader reader(epub_file);
    if (reader.open())
    {
        auto package = reader.get_package();
        if (item_id)
        {
            _display_chapter(reader, item_id);
        }
        else
        {
            for (auto it: package.spine_ids)
            {
                std::cout << "---------------------------------" << std::endl;
                std::cout << it << " " << package.id_to_manifest_item[it].href << std::endl;

                _display_chapter(reader, it);
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
