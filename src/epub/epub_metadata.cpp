#include "./epub_metadata.h"
#include "./libxml_iter.h"
#include "./xhtml_string_util.h"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <libxml/parser.h>


NavPoint::NavPoint(const std::string &label, const std::string &src, const std::string &src_absolute)
    : label(label), src(src), src_absolute(src_absolute)
{
}

NavPoint::NavPoint(const std::string &label, const std::string &src, const std::string &src_absolute, std::vector<NavPoint> children)
    : label(label), src(src), src_absolute(src_absolute), children(children)
{
}

bool NavPoint::operator==(const NavPoint &other) const
{
    return label == other.label &&
        src == other.src &&
        src_absolute == other.src_absolute &&
        children == other.children;
}

std::string epub_get_rootfile_path(const char *container_xml)
{
    xmlDocPtr container_doc = xmlReadMemory(container_xml, strlen(container_xml), nullptr, nullptr, 0);
    if (container_doc == nullptr)
    {
        std::cerr << "Unable to parse container xml" << std::endl;
        return {};
    }

    xmlNodePtr node = xmlDocGetRootElement(container_doc);
    node = elem_first_child(elem_first_by_name(node, BAD_CAST "container"));
    node = elem_first_child(elem_first_by_name(node, BAD_CAST "rootfiles"));
    node = elem_first_by_name(node, BAD_CAST "rootfile");

    std::string rootfile_path;
    if (node)
    {
        auto full_path = xmlGetProp(node, BAD_CAST "full-path");
        auto media_type = xmlGetProp(node, BAD_CAST "media-type");

        if (xmlStrEqual(media_type, BAD_CAST "application/oebps-package+xml"))
        {
            rootfile_path = std::string((const char*) full_path);
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

static std::unordered_map<std::string, ManifestItem> _parse_package_manifest(const std::filesystem::path &base_path, xmlNodePtr node)
{
    std::unordered_map<std::string, ManifestItem> manifest;

    node = elem_first_child(elem_first_by_name(node, BAD_CAST "package"));
    node = elem_first_child(elem_first_by_name(node, BAD_CAST "manifest"));
    node = elem_first_by_name(node, BAD_CAST "item");

    while (node)
    {
        const xmlChar *id = xmlGetProp(node, BAD_CAST "id");
        const xmlChar *href = xmlGetProp(node, BAD_CAST "href");
        const xmlChar *media_type = xmlGetProp(node, BAD_CAST "media-type");
        if (id && href && media_type)
        {
            manifest.emplace(
                std::string((const char*) id),
                ManifestItem{
                    (const char*) href,
                    base_path / ((const char*) href),
                    std::string((const char*) media_type)
                }
            );
        }
        node = elem_next_by_name(node, BAD_CAST "item");
    }

    return manifest;
}

static std::vector<std::string> _parse_package_spine(xmlNodePtr node)
{
    std::vector<std::string> spine_ids;

    node = elem_first_child(elem_first_by_name(node, BAD_CAST "package"));
    node = elem_first_child(elem_first_by_name(node, BAD_CAST "spine"));
    node = elem_first_by_name(node, BAD_CAST "itemref");

    while (node)
    {
        const xmlChar *idref = xmlGetProp(node, BAD_CAST "idref");
        if (idref)
        {
            spine_ids.emplace_back((const char*)idref);
        }

        node = elem_next_by_name(node, BAD_CAST "itemref");
    }

    return spine_ids;
}

bool epub_get_package_contents(const std::string &rootfile_path, const char *package_xml, PackageContents &out_package)
{
    xmlDocPtr package_doc = xmlReadMemory(package_xml, strlen(package_xml), nullptr, nullptr, 0);
    if (package_doc == nullptr)
    {
        std::cerr << "Unable to parse package doc" << std::endl;
        return false;
    }

    xmlNodePtr node = xmlDocGetRootElement(package_doc);
    std::filesystem::path base_path = std::filesystem::path(rootfile_path).parent_path();

    out_package.id_to_manifest_item = _parse_package_manifest(base_path, node);
    out_package.spine_ids = _parse_package_spine(node);

    // get toc id from spine
    {
        xmlNodePtr spine = elem_first_by_name(
            elem_first_child(elem_first_by_name(node, BAD_CAST "package")),
            BAD_CAST "spine"
        );
        if (spine)
        {
            const xmlChar *toc_attr = xmlGetProp(spine, BAD_CAST "toc");
            if (toc_attr)
            {
                out_package.toc_id = (const char*)toc_attr;
            }
        }
    }

    xmlFreeDoc(package_doc);

    return true;
}

static void _parse_nav_point(const std::filesystem::path &base_path, xmlNodePtr node, std::vector<NavPoint> &out)
{
    node = elem_first_child(node); // get children of navPoint

    std::string label, src;
    {
        xmlNodePtr text_node = elem_first_child(elem_first_by_name(
            elem_first_child(
                elem_first_by_name(node, BAD_CAST "navLabel")
            ),
            BAD_CAST "text"
        ));
        if (!text_node || text_node->type != XML_TEXT_NODE || !text_node->content)
        {
            return;
        }

        label = strip_whitespace((const char*)text_node->content);
        if (label.empty())
        {
            return;
        }
    }
    {
        xmlNodePtr content_node = elem_first_by_name(node, BAD_CAST "content");
        if (!content_node)
        {
            return;
        }
        const xmlChar *src_str = xmlGetProp(content_node, BAD_CAST "src");
        if (!src_str || xmlStrlen(src_str) == 0)
        {
            return;
        }
        src = (const char*)src_str;
    }
    out.emplace_back(
        label,
        src,
        base_path / src
    );

    // Look for child elements
    {
        std::vector<NavPoint> &children = out.back().children;

        node = elem_first_by_name(node, BAD_CAST "navPoint");
        while (node)
        {
            _parse_nav_point(base_path, node, children);
            node = elem_next_by_name(node, BAD_CAST "navPoint");
        }
    }
}

static void _parse_nav_map(const std::filesystem::path &base_path, xmlNodePtr node, std::vector<NavPoint> &out)
{
    node = elem_first_child(node); // get children of navMap
    node = elem_first_by_name(node, BAD_CAST "navPoint");
    while (node)
    {
        _parse_nav_point(base_path, node, out);
        node = elem_next_by_name(node, BAD_CAST "navPoint");
    }
}

bool epub_get_ncx(const std::string &ncx_file_path, const char *ncx_xml, std::vector<NavPoint> &out_navmap)
{
    xmlDocPtr ncx_doc = xmlReadMemory(ncx_xml, strlen(ncx_xml), nullptr, nullptr, 0);
    if (ncx_doc == nullptr)
    {
        std::cerr << "Unable to parse ncx doc" << std::endl;
        return false;
    }

    std::filesystem::path base_path = std::filesystem::path(ncx_file_path).parent_path();

    xmlNodePtr node = xmlDocGetRootElement(ncx_doc);
    node = elem_first_child(elem_first_by_name(node, BAD_CAST "ncx"));
    node = elem_first_by_name(node, BAD_CAST "navMap");
    bool found_navmap = node != nullptr;

    _parse_nav_map(base_path, node, out_navmap);

    xmlFreeDoc(ncx_doc);

    return found_navmap;
}
