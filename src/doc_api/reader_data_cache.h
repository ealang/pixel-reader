#ifndef READER_DATA_CACHE_H_
#define READER_DATA_CACHE_H_

#include <memory>
#include <unordered_map>

class ReaderDataCache
{
public:
    // Generic data caching support
    virtual std::unordered_map<std::string, std::string> load_book_cache(const std::string &book_id) = 0;
    virtual void set_book_cache(const std::string &book_id, std::unordered_map<std::string, std::string> &data) = 0;

    // Status reporting
    virtual void report_load_status(const std::string &message) = 0;
};

#endif
