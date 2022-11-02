#include <stdio.h>
#include <zip.h>
#include <cstring>
#include <libxml/parser.h>

#define EPUB_CONTAINER_PATH "META-INF/container.xml"

void scan_tree(const xmlNode *cur, int level = 0)
{
    if (cur && cur->type == XML_ELEMENT_NODE) // eliminate text
    {
        for (int i = 0; i < level; ++i)
        {
            printf(" ");
        }
        printf(
            "%s (%s, %s)\n",
            cur->name,
            xmlGetProp(cur, (const unsigned char *)"full-path"),
            xmlGetProp(cur, (const unsigned char *)"media-type")
        );
        const xmlNode *it = cur->children;
        while (it)
        {
            scan_tree(it, level + 1);
            if (it == cur->last)
            {
                break;
            }
            it = it ->next;
        }
    }
}

void parse_container(zip_t *zip)
{
    zip_file_t *container_zip = zip_fopen(zip, EPUB_CONTAINER_PATH , ZIP_FL_UNCHANGED);
    if (container_zip == nullptr)
    {
        printf("Unable to find container.xml\n");
        return;
    }

    char container_buffer[4096];
    memset(container_buffer, 0, 4096 * sizeof(char));
    zip_fread(container_zip, container_buffer, 4095);

    xmlDocPtr container_doc = xmlParseMemory(container_buffer, 4095);
    if (container_doc == nullptr)
    {
        printf("Unable to parse container.xml\n");
        return;
    }

    xmlNodePtr it = xmlDocGetRootElement(container_doc);
    if (it != nullptr)
    {
        printf("Found root: %s\n", it->name);

        scan_tree(it);
    }
    xmlFreeDoc(container_doc);

    zip_fclose(container_zip);
}

void inspec_epub_zip(zip_t *zip) 
{
    parse_container(zip);
}

void inspect_epub_file(const char *filepath)
{
    int err = 0;
    zip_t *zip = zip_open(filepath, ZIP_RDONLY, &err);
    if (zip == nullptr)
    {
        printf("failed to open: %d\n", err);
        return;
    }

    inspec_epub_zip(zip);
    zip_close(zip);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Must provide path to epub\n");
        return 1;
    }

    const char *epub_file = argv[1];
    inspect_epub_file(epub_file);

    xmlCleanupParser();

    return 0;
}
