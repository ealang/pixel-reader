#ifndef EPUB_UTIL_H_
#define EPUB_UTIL_H_

#define EPUB_CONTAINER_PATH "META-INF/container.xml"

#include <string>

std::string epub_get_rootfile_path(const char *container_str);

void epub_global_cleanup();

#endif
