#ifndef STRING_SERIALIZATION_H_
#define STRING_SERIALIZATION_H_

#include <optional>
#include <string>
#include <vector>

std::optional<uint32_t> try_decode_uint(const std::string &str);

bool try_decode_uint_vector(std::string encoded, std::vector<uint32_t> &out);
std::string encode_uint_vector(const std::vector<uint32_t> &numbers);

#endif
