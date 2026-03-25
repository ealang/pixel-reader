#include "./epub_cover.h"

#include "./epub_metadata.h"
#include "util/zip_utils.h"

#include <iostream>
#include <zip.h>

namespace
{

bool media_type_is_image(const std::string &media_type)
{
    return media_type.rfind("image/", 0) == 0;
}

bool load_epub_info(
    const std::filesystem::path &book_path,
    EpubCover &out_cover,
    bool include_cover_data
)
{
    int err = 0;
    zip_t *zip = zip_open(book_path.c_str(), ZIP_RDONLY, &err);
    if (zip == nullptr)
    {
        std::cerr << "Failed to open epub " << book_path
            << " code: " << err
            << std::endl;
        return false;
    }

    auto close_zip = [&zip]() {
        if (zip)
        {
            zip_close(zip);
            zip = nullptr;
        }
    };

    auto container_xml = read_zip_file_str(zip, EPUB_CONTAINER_PATH);
    if (container_xml.empty())
    {
        close_zip();
        return false;
    }

    auto rootfile_path = epub_parse_rootfile_path(container_xml.data());
    if (rootfile_path.empty())
    {
        close_zip();
        return false;
    }

    auto package_xml = read_zip_file_str(zip, rootfile_path);
    if (package_xml.empty())
    {
        close_zip();
        return false;
    }

    PackageContents package;
    if (!epub_parse_package_contents(rootfile_path, package_xml.data(), package))
    {
        close_zip();
        return false;
    }

    out_cover.title = package.title;
    out_cover.author = package.author;
    out_cover.href_absolute = package.cover_href_absolute;
    out_cover.media_type = package.cover_media_type;
    out_cover.data.clear();
    if (include_cover_data &&
        !package.cover_href_absolute.empty() &&
        media_type_is_image(package.cover_media_type))
    {
        out_cover.data = read_zip_file_str(zip, package.cover_href_absolute);
    }

    close_zip();

    return true;
}

} // namespace

bool epub_load_preview_info(const std::filesystem::path &book_path, EpubCover &out_cover)
{
    return load_epub_info(book_path, out_cover, true);
}

bool epub_load_book_metadata(const std::filesystem::path &book_path, EpubCover &out_cover)
{
    return load_epub_info(book_path, out_cover, false);
}

bool epub_load_cover(const std::filesystem::path &book_path, EpubCover &out_cover)
{
    if (!epub_load_preview_info(book_path, out_cover))
    {
        return false;
    }

    return !out_cover.data.empty();
}
