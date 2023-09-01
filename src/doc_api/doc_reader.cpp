#include "./doc_reader.h"

namespace
{

class NullCache : public DocReaderCache
{
public:
    std::optional<std::string> read(const std::string &, const std::string &) const override
    {
        return {};
    }

    void write(const std::string &, const std::string &, const std::string &) override
    {
    }
};

} // namespace

bool DocReader::open()
{
    NullCache cache;
    return open(cache);
}
