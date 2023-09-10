#ifndef DOC_ADDR_H_
#define DOC_ADDR_H_

#include <string>

using DocAddr = uint64_t;

// Human readable address
std::string to_string(const DocAddr &address);

std::string encode_address(const DocAddr &address);
DocAddr decode_address(const std::string &address);

#endif
