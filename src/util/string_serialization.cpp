#include "./string_serialization.h"

#include <sstream>
#include <stdexcept>

std::optional<uint32_t> try_decode_uint(const std::string &str)
{
    try
    {
        return std::stoul(str);
    }
    catch (const std::invalid_argument &)
    {
    }
    return std::nullopt;
}

bool try_decode_uint_vector(std::string encoded, std::vector<uint32_t> &out)
{
    std::istringstream stream(encoded);
    std::string next_str;
    while (std::getline(stream, next_str, ',')) {
        auto int_opt = try_decode_uint(next_str);
        if (!int_opt)
        {
            return false;
        }
        out.emplace_back(*int_opt);
    }

    return true;
}

std::string encode_uint_vector(const std::vector<uint32_t> &numbers)
{
    std::ostringstream ss;

    uint32_t n = numbers.size();
    for (uint32_t i = 0; i < n; ++i)
    {
        ss << numbers[i];
        if (i < n - 1)
        {
            ss << ',';
        }
    }

    return ss.str();
}
