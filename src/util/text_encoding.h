#ifndef TEXT_ENCODING_H_
#define TEXT_ENCODING_H_

#include <filesystem>
#include <optional>
#include <vector>

bool load_binary_file(const std::filesystem::path &file_path, std::vector<char> &data_out, bool reserve_null = true);

std::optional<std::string> detect_text_encoding(const char *data, uint32_t size);
bool re_encode_text(const char *source, uint32_t source_size, std::string from_encoding, std::string to_encoding, std::vector<char> &data_out);

#endif
