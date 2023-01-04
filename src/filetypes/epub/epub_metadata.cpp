#include "./epub_metadata.h"
#include "./libxml_iter.h"
#include "util/str_utils.h"

#include <libxml/parser.h>

#include <cstring>
#include <filesystem>
#include <iostream>
#include <optional>

NavPoint::NavPoint(const std::string &label)
    : NavPoint(label, "", "")
{
}

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

std::string epub_parse_rootfile_path(const char *container_xml)
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

namespace parse_package
{

std::unordered_map<std::string, ManifestItem> parse_package_manifest(const std::filesystem::path &base_path, xmlNodePtr node)
{
    std::unordered_map<std::string, ManifestItem> manifest;

    node = elem_first_child(elem_first_by_name(node, BAD_CAST "package"));
    node = elem_first_child(elem_first_by_name(node, BAD_CAST "manifest"));
    node = elem_first_by_name(node, BAD_CAST "item");

    while (node)
    {
        const char *id = (const char *)xmlGetProp(node, BAD_CAST "id");
        const char *href = (const char *)xmlGetProp(node, BAD_CAST "href");
        const char *media_type = (const char *)xmlGetProp(node, BAD_CAST "media-type");
        const char *properties = (const char *)xmlGetProp(node, BAD_CAST "properties");
        if (id && href && media_type)
        {
            manifest.emplace(
                std::string(id),
                ManifestItem{
                    href,
                    (base_path / href).lexically_normal(),
                    std::string(media_type),
                    std::string(properties ? properties : "")
                }
            );
        }
        node = elem_next_by_name(node, BAD_CAST "item");
    }

    return manifest;
}

std::vector<std::string> parse_package_spine(xmlNodePtr node)
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

}  // namespace parse_package

bool epub_parse_package_contents(const std::string &rootfile_path, const char *package_xml, PackageContents &out_package)
{
    xmlDocPtr package_doc = xmlReadMemory(package_xml, strlen(package_xml), nullptr, nullptr, 0);
    if (package_doc == nullptr)
    {
        std::cerr << "Unable to parse package doc" << std::endl;
        return false;
    }

    xmlNodePtr node = xmlDocGetRootElement(package_doc);
    std::filesystem::path base_path = std::filesystem::path(rootfile_path).parent_path();

    out_package.id_to_manifest_item = parse_package::parse_package_manifest(base_path, node);
    out_package.spine_ids = parse_package::parse_package_spine(node);

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

namespace parse_ncx
{

void parse_nav_point(const std::filesystem::path &base_path, xmlNodePtr node, std::vector<NavPoint> &out)
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
        (base_path / src).lexically_normal()
    );

    // Look for child elements
    {
        std::vector<NavPoint> &children = out.back().children;

        node = elem_first_by_name(node, BAD_CAST "navPoint");
        while (node)
        {
            parse_nav_point(base_path, node, children);
            node = elem_next_by_name(node, BAD_CAST "navPoint");
        }
    }
}

void parse_nav_map(const std::filesystem::path &base_path, xmlNodePtr node, std::vector<NavPoint> &out)
{
    node = elem_first_child(node); // get children of navMap
    node = elem_first_by_name(node, BAD_CAST "navPoint");
    while (node)
    {
        parse_nav_point(base_path, node, out);
        node = elem_next_by_name(node, BAD_CAST "navPoint");
    }
}

}  // namespace parse_ncx

bool epub_parse_ncx(const std::string &ncx_file_path, const char *ncx_xml, std::vector<NavPoint> &out_navmap)
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

    parse_ncx::parse_nav_map(base_path, node, out_navmap);

    xmlFreeDoc(ncx_doc);

    return found_navmap;
}

namespace parse_nav
{

std::string concat_with_whitespace(const std::string &s1, const std::string &s2)
{
    if (s1.empty())
    {
        return s2;
    }
    if (s2.empty())
    {
        return s1;
    }
    return s1 + " " + s2;
}

// Simplistic way to gather label text for a TOC item. TODO: Improved whitespace handling.
std::string collect_text(xmlNodePtr node)
{
    std::string result;

    while (node)
    {
        if (node->type == XML_TEXT_NODE)
        {
            if (node->content)
            {
                result = concat_with_whitespace(result, strip_whitespace((const char*)node->content));
            }
        }
        else if (node->type == XML_ELEMENT_NODE)
        {
            result = concat_with_whitespace(result, collect_text(elem_first_child(node)));
        }
        node = node->next;
    }

    return result;
}

std::optional<NavPoint> try_parse_anchor(xmlNodePtr anchor, const std::filesystem::path &base_path)
{
    if (anchor)
    {
        const xmlChar *href = xmlGetProp(anchor, BAD_CAST "href");
        if (href && xmlStrlen(href))
        {
            auto label = collect_text(elem_first_child(anchor));
            if (!label.empty())
            {
                return NavPoint(
                    label,
                    (const char*)href,
                    (base_path / ((const char*)href)).lexically_normal()
                );
            }
        }
    }
    return std::nullopt;
}

std::optional<NavPoint> try_parse_span(xmlNodePtr span)
{
    if (span)
    {
        auto label = collect_text(elem_first_child(span));
        if (!label.empty())
        {
            return NavPoint(label);
        }
    }
    return std::nullopt;
}

void parse_ol(xmlNodePtr node, const std::filesystem::path &base_path, std::vector<NavPoint> &out);

void parse_li(xmlNodePtr li_node, const std::filesystem::path &base_path, std::vector<NavPoint> &out)
{
    xmlNodePtr node = elem_first_child(li_node);

    xmlNodePtr anchor_node = elem_first_by_name(node, BAD_CAST "a");
    auto anchor_nav_point = try_parse_anchor(anchor_node, base_path);

    xmlNodePtr span_node = elem_first_by_name(node, BAD_CAST "span");
    auto span_nav_point = try_parse_span(span_node);

    if (anchor_nav_point)
    {
        out.emplace_back(std::move(*anchor_nav_point));
    }
    else if (span_nav_point)
    {
        out.emplace_back(std::move(*span_nav_point));
    }
    else
    {
        std::cerr << "Unable to find <a> or <span> in <li>" << std::endl;
        return;
    }

    // May contain an <ol>
    xmlNodePtr ol_node = elem_first_by_name(node, BAD_CAST "ol");
    if (ol_node)
    {
        parse_ol(ol_node, base_path, out.back().children);
    }
}

void parse_ol(xmlNodePtr ol_node, const std::filesystem::path &base_path, std::vector<NavPoint> &out)
{
    xmlNodePtr node = elem_first_child(ol_node);
    node = elem_first_by_name(node, BAD_CAST "li");

    // Expected to have only <li> elements
    while (node)
    {
        parse_li(node, base_path, out);
        node = elem_next_by_name(node, BAD_CAST "li");
    }
}

}  // namespace parse_nav

// https://www.w3.org/TR/epub/#sec-nav
bool epub_parse_nav(const std::string &nav_file_path, const char *nav_xml, std::vector<NavPoint> &out_navmap)
{
    xmlDocPtr nav_doc = xmlReadMemory(nav_xml, strlen(nav_xml), nullptr, nullptr, 0);
    if (nav_doc == nullptr)
    {
        std::cerr << "Unable to parse nav doc" << std::endl;
        return false;
    }

    std::filesystem::path base_path = std::filesystem::path(nav_file_path).parent_path();

    xmlNodePtr node = xmlDocGetRootElement(nav_doc);
    node = elem_first_child(elem_first_by_name(node, BAD_CAST "html"));
    node = elem_first_child(elem_first_by_name(node, BAD_CAST "body"));
    node = elem_first_by_name(node, BAD_CAST "nav");

    bool found_nav = false;
    while (node)
    {
        const xmlChar *type_prop = xmlGetProp(node, BAD_CAST "type");
        if (type_prop && xmlStrEqual(type_prop, BAD_CAST "toc"))
        {
            node = elem_first_by_name(elem_first_child(node), BAD_CAST "ol");
            if (node)
            {
                found_nav = true;
                // Should be exactly one <ol> in the <nav>
                parse_nav::parse_ol(node, base_path, out_navmap);
            }
            break;
        }

        node = elem_next_by_name(node, BAD_CAST "nav");
    }

    xmlFreeDoc(nav_doc);

    return found_nav;
}
