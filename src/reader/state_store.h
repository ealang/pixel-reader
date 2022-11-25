#ifndef STATE_STORE_H_
#define STATE_STORE_H_

#include "doc_api/doc_addr.h"

#include <filesystem>
#include <optional>
#include <unordered_map>

class StateStore {
    mutable bool dirty = false;

    std::filesystem::path activity_store_path;
    std::filesystem::path addresses_root_path;

    mutable std::unordered_map<std::string, DocAddr> book_addresses; // using string key instead of path due to compile error on gcc 8.3.0
    std::optional<std::filesystem::path> current_browse_path;
    std::optional<std::filesystem::path> current_book_path;

public:
    StateStore(std::filesystem::path base_dir);
    virtual ~StateStore();

    const std::optional<std::filesystem::path> &get_current_browse_path() const;
    void set_current_browse_path(std::filesystem::path path);
    void remove_current_browse_path();

    const std::optional<std::filesystem::path> &get_current_book_path() const;
    void set_current_book_path(std::filesystem::path path);
    void remove_current_book_path();

    std::optional<DocAddr> get_book_address(const std::string &book_path) const;
    void set_book_address(const std::string &book_path, const DocAddr &address);

    void flush() const;
};

#endif
