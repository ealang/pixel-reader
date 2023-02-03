#include "./txt_reader.h"
#include "./txt_token_iter.h"
#include "doc_api/token_addressing.h"
#include "util/str_utils.h"
#include "util/text_encoding.h"

#include "extern/hash-library/md5.h"

#include <fstream>
#include <iostream>

namespace
{

constexpr uint32_t SPACES_PER_TAB = 4;

std::unique_ptr<std::vector<char>> load_utf8_text(const std::filesystem::path &path)
{
    // Load file
    auto original_data = std::make_unique<std::vector<char>>();
    if (!load_binary_file(path, *original_data))
    {
        return {};
    }

    auto encoding = detect_text_encoding(original_data->data(), original_data->size());
    if (!encoding)
    {
        std::cerr << "Unable to detect encoding for " << path << std::endl;
    }
    else if (encoding && *encoding != "UTF-8" && *encoding != "ASCII")
    {
        std::cerr << "Encoding for " << path << " detected as " << *encoding << std::endl;

        auto utf8_data = std::make_unique<std::vector<char>>();
        if (re_encode_text(
            original_data->data(),
            original_data->size(),
            *encoding, "UTF-8",
            *utf8_data
        ))
        {
            return utf8_data;
        }
        else
        {
            std::cerr << "Failed to re-encode as UTF-8" << std::endl;
        }
    }

    original_data->push_back(0); // null-terminate

    return original_data;
}

bool tokenize_text_file(const std::filesystem::path &path, std::vector<std::unique_ptr<DocToken>> &tokens_out, std::string &md5_out)
{
    auto text = load_utf8_text(path);
    if (!text)
    {
        return false;
    }

    MD5 md5;
    DocAddr cur_address = make_address();
    if (text->size())
    {
        std::istringstream iss(text->data());
        std::string line;
        while (std::getline(iss, line))
        {
            md5.add(line.c_str(), line.size());
            md5.add("\n", 1);

            line = strip_whitespace_right(
                convert_tabs_to_space(
                    remove_carriage_returns(line),
                    SPACES_PER_TAB 
                )
            );

            tokens_out.emplace_back(std::make_unique<TextDocToken>(cur_address, line));
            cur_address += get_address_width(*tokens_out.back().get());
        }
    }

    md5_out = md5.getHash();

    return true;
}

} // namespace

struct TxtReaderState
{
    std::filesystem::path path;
    std::vector<TocItem> toc;
    std::vector<std::unique_ptr<DocToken>> tokens;
    std::string md5;
    bool is_open = false;
    uint32_t total_address_width = 0;

    TxtReaderState(const std::filesystem::path &path)
        : path(path)
    {
    }
};

TxtReader::TxtReader(const std::filesystem::path &path)
    : state(std::make_unique<TxtReaderState>(path))
{
}

TxtReader::~TxtReader()
{
}

bool TxtReader::open()
{
    if (state->is_open)
    {
        return true;
    }
    state->is_open = tokenize_text_file(state->path, state->tokens, state->md5);
    if (state->is_open && state->tokens.size())
    {
        const auto *last_token = state->tokens.back().get();
        state->total_address_width = get_text_number(last_token->address) + get_address_width(*last_token);
    }

    return state->is_open;
}

bool TxtReader::is_open() const
{
    return state->is_open;
}

std::string TxtReader::get_id() const
{
    return state->md5;
}

const std::vector<TocItem> &TxtReader::get_table_of_contents() const
{
    return state->toc;
}

TocPosition TxtReader::get_toc_position(const DocAddr &address) const
{
    uint32_t pos = get_text_number(address);
    uint32_t size = state->total_address_width;

    return {
        0,
        size == 0 ? 100 : (
            (std::min(pos, size) * 100) / size
        )
    };
}

DocAddr TxtReader::get_toc_item_address(uint32_t) const
{
    return 0;
}

std::shared_ptr<TokenIter> TxtReader::get_iter(DocAddr address) const
{
    return std::make_shared<TxtTokenIter>(state->tokens, address);
}

std::vector<char> TxtReader::load_resource(const std::filesystem::path &) const
{
    throw std::runtime_error("Load resource is not supported for txt");
}
