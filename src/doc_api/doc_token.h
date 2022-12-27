#ifndef DOC_TOKEN_H_
#define DOC_TOKEN_H_

#include "./doc_addr.h"
#include "./token_type.h"
#include <string>

struct DocToken
{
    TokenType type;
    DocAddr address;
    std::string data;

    DocToken(TokenType type, DocAddr address, std::string data = "");
    bool operator==(const DocToken &other) const;
};

std::string to_string(const DocToken &token);

#endif
