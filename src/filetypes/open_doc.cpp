#include "./open_doc.h"

#include "./epub/epub_reader.h"
#include "./txt/txt_reader.h"
#include "util/str_utils.h"

#include <set>
#include <string>

namespace 
{

const std::string EPUB_EXT = ".epub";
const std::set<std::string> TEXT_EXTS = {
    ".txt",
    ".md",
    ".markdown"
};

std::string norm_extension(const std::filesystem::path &path)
{
    return to_lower(path.extension());
}

} // namespace

bool file_type_is_supported(const std::filesystem::path &path)
{
    auto ext = norm_extension(path);
    return ext == EPUB_EXT || TEXT_EXTS.count(ext) > 0;
}

std::shared_ptr<DocReader> create_doc_reader(const std::filesystem::path &path)
{
    auto ext = norm_extension(path);
    if (ext == EPUB_EXT)
    {
        return std::make_shared<EPubReader>(path);
    }
    if (TEXT_EXTS.count(ext) > 0)
    {
        return std::make_shared<TxtReader>(path);
    }

    throw std::runtime_error("Unsupported file type: " + path.string());
}
