#include <iostream>
#include "filesystem.h"

int main(int argc, char** argv)
{

    if (argc != 2) 
    {
        return 1;
    }
    std::string path = argv[1];

    for (auto entry: directory_listing(path))
    {
        std::cout << (entry.is_dir ? "D" : "F") << " " << entry.name << " " << std::endl;
    }
    return 0;
}
