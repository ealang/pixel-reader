#ifndef TOKEN_LINE_WRAPPING_H_
#define TOKEN_LINE_WRAPPING_H_

#include "doc_api/doc_token.h"

#include <functional>
#include <vector>

// Perform line wrapping pre-processing on tokens.
// Output split/recombined tokens such that each new token occupies the width of a single line.
void line_wrap_token(
    const DocToken &token,
    std::function<bool(const char *, uint32_t)> text_fits_on_line,
    std::function<void(const DocToken &)> for_each
);

#endif
