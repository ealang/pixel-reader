#ifndef TOKEN_TYPE_H_
#define TOKEN_TYPE_H_

#include <string>

enum class TokenType
{
    Text,
    TextBreak,
    Section,
    Image,
};

std::string to_string(TokenType type);

#endif
