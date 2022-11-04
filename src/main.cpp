#include <iostream>
#include <libxml/parser.h>

#include "epub_reader.h"
#include "epub_util.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Must provide path to epub\n");
        return 1;
    }

    const char *epub_file = argv[1];

    EPubReader reader(epub_file);
    if (!reader.open())
    {
        std::cout << "Failed to open" << std::endl;
    }

    xmlCleanupParser();

    return 0;
}
