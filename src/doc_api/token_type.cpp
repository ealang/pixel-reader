#include "./token_type.h"

std::string to_string(TokenType type)
{
    switch (type)
    {
        case TokenType::Text:
            return "Text";
        case TokenType::Image:
            return "Image";
        case TokenType::Header:
            return "Header";
        default:
            return "Unknown";
    }
}

