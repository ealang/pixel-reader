#ifndef EPUB_TOKEN_ITER_H_
#define EPUB_TOKEN_ITER_H_

#include "doc_api/doc_addr.h"
#include "doc_api/doc_token.h"

struct EpubDocIndex;

class EPubTokenIter
{
    EpubDocIndex *index;
    uint32_t current_spine_idx = 0;
    uint32_t current_token_idx = 0;

    bool seek_to_first();
    bool seek_to_prev();

public:
    EPubTokenIter(EpubDocIndex *index, DocAddr address = make_address());

    const DocToken *read(int direction);
    void seek(DocAddr address);
};

#endif
