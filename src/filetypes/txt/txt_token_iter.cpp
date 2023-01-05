#include "./txt_token_iter.h"

TxtTokenIter::TxtTokenIter(const std::vector<DocToken> &tokens, DocAddr address)
    : tokens(tokens)
{
    seek(address);
}

TxtTokenIter::TxtTokenIter(const TxtTokenIter &other)
    : i(other.i)
    , tokens(other.tokens)
{
}

const DocToken *TxtTokenIter::read(int direction)
{
    uint32_t read_pos;

    if (direction < 0)
    {
        if (i == 0)
        {
            return nullptr;
        }
        read_pos = --i;
    }
    else
    {
        if (i >= tokens.size())
        {
            return nullptr;
        }
        read_pos = i++;
    }

    if (read_pos >= tokens.size())
    {
        return nullptr;
    }

    return &tokens[read_pos];
}

void TxtTokenIter::seek(DocAddr address)
{
    for (uint32_t j = 0; j < tokens.size(); ++j)
    {
        DocAddr other_address = tokens[j].address;
        if (other_address <= address)
        {
            i = j;
            // stop at first match in case of duplicates
            if (other_address == address)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
}

std::shared_ptr<TokenIter> TxtTokenIter::clone() const
{
    return std::make_shared<TxtTokenIter>(*this);
}
