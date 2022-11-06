#ifndef EPUB_READER_H_
#define EPUB_READER_H_

#include "epub_util.h"

#include <string>
#include <vector>

typedef struct zip zip_t;

class EPubReader
{
    const std::string _path;
    zip_t *_zip;
    PackageContents _package;
public:
    EPubReader(std::string path);
    virtual ~EPubReader();

    bool open();
    const PackageContents& get_package() const;
    std::vector<char> get_file_as_bytes(std::string href) const;
    std::vector<std::string> get_item_as_text(std::string item_id) const;
};

#endif
