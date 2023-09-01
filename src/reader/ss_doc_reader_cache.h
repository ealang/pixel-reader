#ifndef SS_DOC_READER_CACHE_H_
#define SS_DOC_READER_CACHE_H_

#include "doc_api/doc_reader.h"

struct StateStore;

// StateStore backed DocReaderCache
class SSDocReaderCache : public DocReaderCache
{
    StateStore &store;
public:
    SSDocReaderCache(StateStore &store);

    std::optional<std::string> read(const std::string &book_id, const std::string &key) const override;
    void write(const std::string &book_id, const std::string &key, const std::string &value) override;
};

#endif
