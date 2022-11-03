#ifndef EPUB_READER_H_
#define EPUB_READER_H_

#include <string>

typedef struct zip zip_t;

class EPubReader
{
    const std::string _path;
    zip_t *_zip;
public:
    EPubReader(std::string path);
    virtual ~EPubReader();

    bool open();
};

void global_cleanup();

#endif
