#include "./doc_addr.h"

#include <iomanip>

std::string to_string(const DocAddr &address)
{
    uint32_t upper = address >> 32;
    uint32_t lower = address & 0xFFFFFFFF;

    std::stringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(2) << upper << "-"
       << std::setw(4) << lower;

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
        return 0;
    }

    DocAddr high = std::stoi(address.substr(0, 8), nullptr, 16);
    DocAddr low = std::stoi(address.substr(8, 16), nullptr, 16);
    return (high << 32 ) | low;
}
