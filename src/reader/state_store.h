#ifndef STATE_STORE_H_
#define STATE_STORE_H_

#include "doc_api/doc_addr.h"

#include <filesystem>
#include <optional>
#include <unordered_map>

class StateStore {
    mutable bool activity_dirty = false;
    mutable bool settings_dirty = false;

    // activity
    std::filesystem::path activity_store_path;
    mutable std::unordered_map<std::string, DocAddr> book_addresses; // using string key instead of path due to compile error on gcc 8.3.0
    std::optional<std::filesystem::path> current_browse_path;
    std::optional<std::filesystem::path> current_book_path;

    // book addresses
    std::filesystem::path addresses_root_path;

    // settings
    std::filesystem::path settings_store_path;
    std::unordered_map<std::string, std::string> settings;

public:
    StateStore(std::filesystem::path base_dir);
    virtual ~StateStore();

    // activity
    const std::optional<std::filesystem::path> &get_current_browse_path() const;
    void set_current_browse_path(std::filesystem::path path);
    void remove_current_browse_path();

    const std::optional<std::filesystem::path> &get_current_book_path() const;
    void set_current_book_path(std::filesystem::path path);
    void remove_current_book_path();

    // book addresses
    std::optional<DocAddr> get_book_address(const std::string &book_id) const;
    void set_book_address(const std::string &book_id, DocAddr address);

    // generic settings
    std::optional<std::string> get_setting(const std::string &name) const;
    void set_setting(const std::string &name, const std::string &value);

    void flush() const;
};

#endif
