#ifndef EPUB_METADATA_H_
#define EPUB_METADATA_H_

#define EPUB_CONTAINER_PATH "META-INF/container.xml"

#include <string>
#include <unordered_map>
#include <vector>

struct ManifestItem
{
    std::string href;
    std::string href_absolute; // doc path in epub
    std::string media_type;
};

struct NavPoint
{
    std::string label;
    std::string src;
    std::string src_absolute; // doc path in epub
    std::vector<NavPoint> children;

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

std::string epub_get_rootfile_path(const char *container_xml);
bool epub_get_package_contents(const std::string &rootfile_path, const char *package_xml, PackageContents &out_package);
bool epub_get_ncx(const std::string &ncx_file_path, const char *ncx_xml, std::vector<NavPoint> &out_navmap);

#endif
