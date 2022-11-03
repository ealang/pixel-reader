#include <iostream>
#include <cstring>
#include <libxml/parser.h>
#include <libxml/xmlstring.h>

#include "epub_util.h"

#define SUPPORTED_DOCROOT_MEDIA_TYPE "application/oebps-package+xml"

std::string epub_get_rootfile_path(const char *xml_str)
{
    xmlDocPtr container_doc = xmlReadMemory(xml_str, strlen(xml_str), nullptr, nullptr, 0);
    if (container_doc == nullptr)
    {
        std::cerr << "Unable to parse container.xml" << std::endl;
        return "";
    }

    xmlNodePtr it = xmlDocGetRootElement(container_doc);
    if (it == nullptr)
    {
        std::cerr << "Unable to get room elem in container.xml" << std::endl;
        return "";
    }

    std::string rootfile_path;
    for (; it; it = it->next)
    {
        std::cout << "name " << it->name << "\n";
        if (it->type == XML_ELEMENT_NODE && xmlStrEqual(it->name, (const xmlChar *)"rootfile"))
        {
            auto full_path = xmlGetProp(it, (const xmlChar *)"full-path");
            auto media_type = xmlGetProp(it, (const xmlChar *)"media-type");
            if (full_path && media_type)
            {
                if (xmlStrEqual(media_type, (const xmlChar *)SUPPORTED_DOCROOT_MEDIA_TYPE))
                {
                    rootfile_path = std::string((const char*)full_path);
                }
                else
                {
                    std::cerr << "Found unsupported docroot media type: " << media_type << std::endl;
                }
            }
        }
    }

    if (!rootfile_path.size())
    {
        std::cerr << "Unable to find docroot element" << std::endl;
    }

    xmlFreeDoc(container_doc);
    
    return rootfile_path;
}

void epub_global_cleanup()
{
    xmlCleanupParser();
}
