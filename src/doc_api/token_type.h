#ifndef TOKEN_TYPE_H_
#define TOKEN_TYPE_H_

#include <string>

enum class TokenType
{
    Text,
    Block,
    Section,
};

std::string to_string(TokenType type);

#endif
