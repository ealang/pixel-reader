#ifndef DOC_TOKEN_H_
#define DOC_TOKEN_H_

#include "./doc_addr.h"
#include "./token_type.h"
#include <string>

struct DocToken
{
    const TokenType type;
    const DocAddr address;
    const std::string text;

    DocToken(TokenType type, DocAddr address, std::string text = "");
};

std::string to_string(const DocToken &token);

#endif