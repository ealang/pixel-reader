#include "./token_type.h"

std::string to_string(TokenType type)
{
    switch (type)
    {
        case TokenType::Text:
            return "Text";
        case TokenType::TextBreak:
            return "TextBreak";
        case TokenType::Section:
            return "Section";
        case TokenType::Image:
            return "Image";
        default:
            return "Unknown";
    }
}

