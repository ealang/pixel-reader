#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <vector>
#include <string>

struct FSEntry
{
    std::string name;
    bool is_dir;
};

std::vector<FSEntry> directory_listing(const std::string& path);

#endif
