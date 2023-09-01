#include "./ss_doc_reader_cache.h"
#include "./state_store.h"

SSDocReaderCache::SSDocReaderCache(StateStore &store): store(store)
{
}

std::optional<std::string> SSDocReaderCache::read(const std::string &book_id, const std::string &key) const
{
    const auto &kv = store.get_reader_cache(book_id);
    const auto it = kv.find(key);
    if (it == kv.end())
    {
        return {};
    }
    return it->second;
}

void SSDocReaderCache::write(const std::string &book_id, const std::string &key, const std::string &value)
{
    if (read(book_id, key) != value)
    {
        auto kv = store.get_reader_cache(book_id);
        kv[key] = value;
        store.set_reader_cache(book_id, kv);
    }
}
