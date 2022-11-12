#ifndef FILESYSTEM_PATH_H
#define FILESYSTEM_PATH_H

#include <string>

std::pair<std::string, std::string> fs_path_split_dir(std::string path);
std::string fs_path_join(std::string path1, std::string path2);
std::string fs_path_parent(std::string path);
std::string fs_path_make_absolute(std::string path, std::string cwd);

#endif
