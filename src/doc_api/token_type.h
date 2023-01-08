#ifndef TOKEN_TYPE_H_
#define TOKEN_TYPE_H_

#include <string>

enum class TokenType
{
    Text,
    Image,
};

std::string to_string(TokenType type);

#endif
