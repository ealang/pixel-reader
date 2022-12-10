#ifndef ZIP_UTILS_H_
#define ZIP_UTILS_H_

#include <string>
#include <vector>

typedef struct zip zip_t;
std::vector<char> read_zip_file_str(zip_t *zip, const std::string &filepath);

#endif
