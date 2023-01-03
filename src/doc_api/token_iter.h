#ifndef TOKEN_ITER_H_
#define TOKEN_ITER_H_

#include "./doc_token.h"
#include "./doc_addr.h"

#include <memory>

// Interface for iterating over a token stream.
class TokenIter
{
public:
    virtual const DocToken *read(int direction) = 0;
    virtual void seek(DocAddr address) = 0;
    virtual ~TokenIter() = default;

    virtual std::shared_ptr<TokenIter> clone() const = 0;
};

#endif
