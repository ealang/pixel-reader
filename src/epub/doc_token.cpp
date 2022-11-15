#include "./doc_token.h"

DocToken::DocToken(TokenType type, std::string text)
    : type(type), text(text)
{
}

std::string DocToken::to_string() const
{
    switch (type)
    {
        case TokenType::Text:
            return "[Text=\"" + text + "\"]";
        case TokenType::Block:
            return "[Block]";
        case TokenType::Section:
            return "[Section]";
        default:
            return "[Other]";
    }
}

