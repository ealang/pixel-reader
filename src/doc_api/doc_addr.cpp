#include "./doc_addr.h"

#include <iomanip>

#define TEXT_START 0
#define TEXT_BITS 32
#define TEXT_MASK (((DocAddr)1 << TEXT_BITS) - 1)

#define CHAPTER_START 32
#define CHAPTER_BITS 16
#define CHAPTER_MASK ((1 << CHAPTER_BITS) - 1)

DocAddr make_address(uint32_t chapter_num, uint32_t text_num)
{
    return (
        ((static_cast<DocAddr>(chapter_num) & CHAPTER_MASK) << CHAPTER_START) |
        ((static_cast<DocAddr>(text_num) & TEXT_MASK) << TEXT_START)
    );
}

DocAddr increment_address(DocAddr addr, uint32_t text_increment)
{
    return make_address(
        get_chapter_number(addr),
        get_text_number(addr) + text_increment
    );
}

uint32_t get_chapter_number(DocAddr addr)
{
    return (addr >> CHAPTER_START) & CHAPTER_MASK;
}

uint32_t get_text_number(DocAddr addr)
{
    return (addr >> TEXT_START) & TEXT_MASK;
}

std::string to_string(const DocAddr &address)
{
    uint32_t chapter_number = get_chapter_number(address);
    uint32_t text_number = get_text_number(address);

    std::stringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(2) << chapter_number << "-"
       << std::setw(4) << text_number;

    return ss.str();
}

std::string encode_address(const DocAddr &address)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16)
       << address;
    return ss.str();
}

DocAddr decode_address(const std::string &address)
{
    if (address.size() != 16) {
        return make_address();
    }

    DocAddr high = std::stoi(address.substr(0, 8), nullptr, 16);
    DocAddr low = std::stoi(address.substr(8, 16), nullptr, 16);
    return make_address(high, low);
}
