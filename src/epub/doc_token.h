#ifndef DOC_TOKEN_H_
#define DOC_TOKEN_H_

#include <string>

enum class TokenType
{
    Text,
    Block,
    Section,
};

struct DocToken
{
    const TokenType type;
    const std::string text;

    DocToken(TokenType type, std::string text = "");
    std::string to_string() const;

    static DocToken text_token(std::string text)
    {
        return DocToken(TokenType::Text, text);
    }

    static DocToken block_token()
    {
        return DocToken(TokenType::Block);
    }

    static DocToken section_token()
    {
        return DocToken(TokenType::Section);
    }
};

#endif
