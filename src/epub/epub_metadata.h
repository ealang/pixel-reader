#ifndef EPUB_METADATA_H_
#define EPUB_METADATA_H_

#define EPUB_CONTAINER_PATH "META-INF/container.xml"

#define APPLICATION_XHTML_XML "application/xhtml+xml"
#define APPLICATION_X_DTBNCX_XML "application/x-dtbncx+xml"

#include <string>
#include <unordered_map>
#include <vector>

struct ManifestItem
{
    std::string href;
    std::string href_absolute; // doc path in epub
    std::string media_type;
    std::string properties;
};

struct NavPoint
{
    std::string label;
    std::string src;
    std::string src_absolute; // doc path in epub
    std::vector<NavPoint> children;

    NavPoint(const std::string &label);
    NavPoint(const std::string &label, const std::string &src, const std::string &src_absolute);
    NavPoint(const std::string &label, const std::string &src, const std::string &src_absolute, std::vector<NavPoint> children);

    bool operator==(const NavPoint &other) const;
};

// Contents of rootfile
struct PackageContents
{
    std::unordered_map<std::string, ManifestItem> id_to_manifest_item;
    std::vector<std::string> spine_ids;
    std::string toc_id;
};

std::string epub_parse_rootfile_path(const char *container_xml);
bool epub_parse_package_contents(const std::string &rootfile_path, const char *package_xml, PackageContents &out_package);
bool epub_parse_ncx(const std::string &ncx_file_path, const char *ncx_xml, std::vector<NavPoint> &out_navmap);
bool epub_parse_nav(const std::string &nav_file_path, const char *nav_xml, std::vector<NavPoint> &out_navmap);

#endif
