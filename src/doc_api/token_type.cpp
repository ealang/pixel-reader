#include "./token_type.h"

std::string to_string(TokenType type)
{
    switch (type)
    {
        case TokenType::Text:
            return "Text";
        case TokenType::Block:
            return "Block";
        case TokenType::Section:
            return "Section";
        default:
            return "Unknown";
    }
}

