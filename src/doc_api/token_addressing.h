#ifndef TOKEN_ADDRESSING_H_
#define TOKEN_ADDRESSING_H_

#include "./doc_token.h"
#include <cstdint>

uint32_t get_address_width(const char *str);
uint32_t get_address_width(const std::string &str);
uint32_t get_address_width(const DocToken &token);

#endif
