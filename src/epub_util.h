#ifndef EPUB_UTIL_H_
#define EPUB_UTIL_H_

#define EPUB_CONTAINER_PATH "META-INF/container.xml"

#include <string>
#include <unordered_map>
#include <vector>

struct ManifestItem
{
    std::string href;
    std::string media_type;
};

// Contents of rootfile
struct PackageContents
{
    std::unordered_map<std::string, ManifestItem> id_to_manifest_item;
    std::vector<std::string> spine_ids;
};

std::string epub_get_rootfile_path(const char *container_str);
bool epub_get_package_contents(const char *package_str, PackageContents &out_package);

#endif
