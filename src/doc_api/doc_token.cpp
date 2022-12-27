#include "./doc_token.h"

DocToken::DocToken(TokenType type, DocAddr address, std::string data)
    : type(type), address(address), data(std::move(data))
{
}

bool DocToken::operator==(const DocToken &other) const
{
    return type == other.type && address == other.address && data == other.data;
}

std::string to_string(const DocToken &token)
{
    return (
        "[DocToken "
        "address=" + to_string(token.address) + ", "
        "type=" + to_string(token.type) +
        (
            token.data.empty()
            ? ""
            : ", data=\"" + token.data + "\""
        ) +
        "]"
    );
}

