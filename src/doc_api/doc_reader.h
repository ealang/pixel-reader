#ifndef DOC_READER_H_
#define DOC_READER_H_

#include "./token_iter.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct TocItem
{
    std::string display_name;
    uint32_t indent_level;
};

struct TocPosition
{
    uint32_t toc_index;
    uint32_t progress_percent;
};

// Allow readers to cache arbitrary data
class DocReaderCache
{
public:
    virtual std::optional<std::string> read(const std::string &book_id, const std::string &key) const = 0;
    virtual void write(const std::string &book_id, const std::string &key, const std::string &value) = 0;
};

// Interface for interacting with a particular document format.
class DocReader
{
public:
    virtual ~DocReader() = default;

    bool open();
    virtual bool open(DocReaderCache &cache) = 0;
    virtual bool is_open() const = 0;

    virtual std::string get_id() const = 0;

    virtual const std::vector<TocItem> &get_table_of_contents() const = 0;
    virtual TocPosition get_toc_position(const DocAddr &address) const = 0;
    virtual DocAddr get_toc_item_address(uint32_t toc_item_index) const = 0;

    virtual uint32_t get_global_progress_percent(const DocAddr &address) const = 0;

    virtual std::shared_ptr<TokenIter> get_iter(DocAddr address = make_address()) const = 0;

    virtual std::vector<char> load_resource(const std::filesystem::path &path) const = 0;
};

#endif
