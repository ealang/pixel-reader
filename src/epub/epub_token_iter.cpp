#include "./epub_token_iter.h"

#include "./epub_doc_index.h"

EPubTokenIter::EPubTokenIter(EpubDocIndex *index, DocAddr address)
    : index(index)
{
    seek(address);
}

bool EPubTokenIter::seek_to_first()
{
    while (current_spine_idx < index->spine_size())
    {
        if (current_token_idx < index->token_count(current_spine_idx))
        {
            return true;
        }

        ++current_spine_idx;
        current_token_idx = 0;
    }

    return false;
}

bool EPubTokenIter::seek_to_prev()
{
    if (current_token_idx > 0)
    {
        --current_token_idx;
        return true;
    }

    while (current_spine_idx > 0)
    {
        if (--current_spine_idx < index->spine_size())
        {
            uint32_t token_count = index->token_count(current_spine_idx);
            if (token_count)
            {
                current_token_idx = token_count - 1;
                return true;
            }
        }
    }

    return false;
}

const DocToken *EPubTokenIter::read(int direction)
{
    const DocToken *token = nullptr;

    if (direction < 0)
    {
        if (seek_to_prev())
        {
            token = &index->tokens(current_spine_idx)[current_token_idx];
        }
    }
    else
    {
        if (seek_to_first())
        {
            token = &index->tokens(current_spine_idx)[current_token_idx++];
        }
    }

    return token;
}

void EPubTokenIter::seek(DocAddr address)
{
    uint32_t new_spine_idx = std::min(
        get_chapter_number(address),
        index->spine_size()
    );
    uint32_t new_token_idx = 0;
    if (new_spine_idx < index->spine_size())
    {
        const auto &tokens = index->tokens(new_spine_idx);
        uint32_t token_idx = 0;
        for (const auto &token: tokens)
        {
            if (token.address <= address)
            {
                new_token_idx = token_idx;
            }
            else
            {
                break;
            }
            ++token_idx;
        }
    }

    current_spine_idx = new_spine_idx;
    current_token_idx = new_token_idx;
}

