#ifndef TXT_TOKEN_ITER_H_
#define TXT_TOKEN_ITER_H_

#include "doc_api/token_iter.h"

#include <vector>

class TxtTokenIter: public TokenIter
{
    uint32_t i = 0;
    const std::vector<std::unique_ptr<DocToken>> &tokens;

public:
    TxtTokenIter(const std::vector<std::unique_ptr<DocToken>> &tokens, DocAddr address);
    TxtTokenIter(const TxtTokenIter &);

    const DocToken *read(int direction) override;
    void seek(DocAddr address) override;

    std::shared_ptr<TokenIter> clone() const override;
};

#endif
