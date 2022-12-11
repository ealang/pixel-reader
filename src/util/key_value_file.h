#ifndef KEY_VALUE_FILE_H_
#define KEY_VALUE_FILE_H_

#include <filesystem>
#include <string>
#include <unordered_map>

void write_key_value(const std::filesystem::path &path, const std::unordered_map<std::string, std::string> &settings);
std::unordered_map<std::string, std::string> load_key_value(const std::filesystem::path &path);

#endif
