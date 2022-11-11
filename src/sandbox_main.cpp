#include <iostream>
#include "filesystem.h"

int main(int argc, char** argv)
{
    std::string path = argc == 2 ? argv[1] : ".";

    for (auto entry: directory_listing(path))
    {
        std::cout << (entry.is_dir ? "D" : "F") << " " << entry.name << " " << std::endl;
    }
    return 0;
}
