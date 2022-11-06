#ifndef PATH_OPS_H_
#define PATH_OPS_H

#include <string>

std::pair<std::string, std::string> path_split_dir(std::string path);
std::string path_join(std::string path1, std::string path2);

#endif
