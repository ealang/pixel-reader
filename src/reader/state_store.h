#ifndef STATE_STORE_H_
#define STATE_STORE_H_

#include <filesystem>
#include <optional>

class StateStore {
    std::filesystem::path store_path;

    std::filesystem::path current_browse_path;
    std::optional<std::filesystem::path> current_book_path;

public:
    StateStore(std::filesystem::path store_dir);
    virtual ~StateStore();

    const std::filesystem::path &get_current_browse_path() const;
    void store_current_browse_path(std::filesystem::path path);

    const std::optional<std::filesystem::path> &get_current_book_path() const;
    void store_current_book_path(std::filesystem::path path);
    void remove_current_book_path();

    void flush() const;
};

#endif
