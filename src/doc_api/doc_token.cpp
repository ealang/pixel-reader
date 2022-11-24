#include "./doc_token.h"

DocToken::DocToken(TokenType type, DocAddr address, std::string text)
    : type(type), address(address), text(text)
{
}

std::string to_string(const DocToken &token)
{
    return (
        "[DocToken "
        "address=" + to_string(token.address) + ", "
        "type=" + to_string(token.type) +
        (
            token.text.empty()
            ? ""
            : ", text=\"" + token.text + "\""
        ) +
        "]"
    );
}

