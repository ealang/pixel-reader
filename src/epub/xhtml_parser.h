#ifndef XHTML_PARSER_H_
#define XHTML_PARSER_H_

#include "./doc_token.h"

#include <string>
#include <vector>

std::vector<DocToken> parse_xhtml_tokens(const char *xml_str, std::string name);

#endif
