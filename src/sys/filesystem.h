#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <vector>
#include <string>

struct FSEntry
{
    std::string name;
    bool is_dir;

    static FSEntry directory(const std::string& path)
    {
        return { path, true };
    }
};

std::vector<FSEntry> directory_listing(const std::string& path);
std::string get_cwd();

#endif
