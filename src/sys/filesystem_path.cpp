#include "./filesystem_path.h"

#define FS_SEP '/'
#define FS_SEP_STR "/"

std::string fs_path_strip_character(std::string path, char character)
{
    int start, end;
    int str_len = static_cast<int>(path.length());
    for (start = 0; start < str_len && path[start] == character; start++);
    for (end = str_len - 1; end >= 0 && path[end] == character; end--);

    return path.substr(start, end - start + 1);
}

static bool is_absolute_path(std::string path)
{
    return path.size() > 0 && path[0] == FS_SEP;
}

std::pair<std::string, std::string> fs_path_split_dir(std::string path)
{
    auto final_prefix = is_absolute_path(path) ? FS_SEP_STR : "";

    path = fs_path_strip_character(path, FS_SEP);

    size_t i = path.find_last_of(FS_SEP);
    if (i == std::string::npos) {
        return std::make_pair(final_prefix, path);
    }
    return std::make_pair(final_prefix + path.substr(0, i), path.substr(i + 1));
}

std::string fs_path_join(std::string path1, std::string path2)
{
    if (path1.empty()) {
        return path2;
    }
    if (path2.empty()) {
        return path1;
    }
    if (path1.back() == FS_SEP) {
        return path1 + path2;
    }
    return path1 + FS_SEP_STR + path2;
}

std::string fs_path_parent(std::string path)
{
    return fs_path_split_dir(path).first;
}

std::string fs_path_make_absolute(std::string path, std::string cwd)
{
    if (path == ".")
    {
        path = "";
    }
    if (is_absolute_path(path)) {
        return path;
    }
    return fs_path_join(cwd, path);
}
