#ifndef DOC_READER_H_
#define DOC_READER_H_

#include "./token_iter.h"
#include "./reader_data_cache.h"

#include <filesystem>
#include <memory>
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

// Interface for interacting with a particular document format.
class DocReader
{
public:
    virtual ~DocReader() = default;

    virtual bool open(std::shared_ptr<ReaderDataCache> load_system = {}) = 0;
    virtual bool is_open() const = 0;

    virtual std::string get_id() const = 0;

    virtual const std::vector<TocItem> &get_table_of_contents() const = 0;
    virtual TocPosition get_toc_position(const DocAddr &address) const = 0;
    virtual DocAddr get_toc_item_address(uint32_t toc_item_index) const = 0;

    virtual std::shared_ptr<TokenIter> get_iter(DocAddr address = make_address()) const = 0;

    virtual std::vector<char> load_resource(const std::filesystem::path &path) const = 0;
};

#endif
