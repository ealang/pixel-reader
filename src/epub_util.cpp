#include <iostream>
#include <cstring>
#include <libxml/parser.h>
#include <libxml/xmlstring.h>

#include "libxml_util.h"
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

    xmlNodePtr node = xmlDocGetRootElement(container_doc);
    node = it_first_child(it_first_by_name(node, (const xmlChar *) "container"));
    node = it_first_child(it_first_by_name(node, (const xmlChar *) "rootfiles"));
    node = it_first_by_name(node, (const xmlChar *) "rootfile");

    std::string rootfile_path;
    if (node)
    {
        auto full_path = xmlGetProp(node, (const xmlChar *)"full-path");
        auto media_type = xmlGetProp(node, (const xmlChar *)"media-type");

        if (xmlStrEqual(media_type, (const xmlChar *)SUPPORTED_DOCROOT_MEDIA_TYPE))
        {
            rootfile_path = std::string((const char*)full_path);
        }
        else
        {
            std::cerr << "Found unsupported docroot media type: " << media_type << std::endl;
        }
    }
    else
    {
        std::cerr << "Unable to find rootfile element" << std::endl;
    }

    xmlFreeDoc(container_doc);
    
    return rootfile_path;
}
