#include "filesystem_path.h"

std::pair<std::string, std::string> fs_path_split_dir(std::string path)
{
    size_t i = path.find_last_of('/');
    if (i == std::string::npos) {
        return std::make_pair("", path);
    }
    return std::make_pair(path.substr(0, i), path.substr(i + 1));
}

std::string fs_path_join(std::string path1, std::string path2)
{
    if (path1.empty()) {
        return path2;
    }
    if (path2.empty()) {
        return path1;
    }
    if (path1.back() == '/') {
        return path1 + path2;
    }
    return path1 + "/" + path2;
}

std::string fs_path_parent(std::string path)
{
    return fs_path_split_dir(path).first;
}
