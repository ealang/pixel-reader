#include "./key_value_file.h"

#include <fstream>

void write_key_value(const std::filesystem::path &path, const std::unordered_map<std::string, std::string> &settings)
{
    std::ofstream fp(path);
    for (const auto& [key, value]: settings)
    {
        fp << key << "=" << value << std::endl;
    }
    fp.close();
}

std::unordered_map<std::string, std::string> load_key_value(const std::filesystem::path &path)
{
    std::unordered_map<std::string, std::string> settings;
    std::ifstream fp(path);

    std::string line;
    while (std::getline(fp, line))
    {
        auto pos = line.find('=');
        if (pos == std::string::npos)
        {
            continue;
        }
        auto key = line.substr(0, pos);
        auto value = line.substr(pos + 1);
        settings[key] = value;
    }
    fp.close();

    return settings;
}
