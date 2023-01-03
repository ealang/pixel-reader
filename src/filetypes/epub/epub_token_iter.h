#ifndef EPUB_TOKEN_ITER_H_
#define EPUB_TOKEN_ITER_H_

#include "doc_api/token_iter.h"

struct EpubDocIndex;

class EPubTokenIter: public TokenIter
{
    EpubDocIndex *index;
    uint32_t current_spine_idx = 0;
    uint32_t current_token_idx = 0;

    bool seek_to_first();
    bool seek_to_prev();

public:
    EPubTokenIter(EpubDocIndex *index, DocAddr address);
    EPubTokenIter(const EPubTokenIter &);

    const DocToken *read(int direction) override;
    void seek(DocAddr address) override;

    std::shared_ptr<TokenIter> clone() const override;
};

#endif
