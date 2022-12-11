#include "./state_store.h"
#include "util/key_value_file.h"

#include <fstream>
#include <unordered_map>

namespace
{

constexpr const char *ACTIVITY_KEY_BROWSER_PATH = "browser_path";
constexpr const char *ACTIVITY_KEY_BOOK_PATH = "book_path";

void write_activity_store(const std::filesystem::path &path, const StateStore &store)
{
    std::unordered_map<std::string, std::string> kv;

    const auto &browse_path = store.get_current_browse_path();
    if (browse_path)
    {
        kv[ACTIVITY_KEY_BROWSER_PATH] = browse_path.value().string();
    }

    const auto &book_path = store.get_current_book_path();
    if (book_path)
    {
        kv[ACTIVITY_KEY_BOOK_PATH] = book_path.value().string();
    }

    write_key_value(path, kv);
}

std::pair<std::optional<std::string>, std::optional<std::string>> load_activity_store(const std::filesystem::path &path)
{
    std::optional<std::string> browse_path, book_path;

    auto kv = load_key_value(path);

    auto browse_it = kv.find(ACTIVITY_KEY_BROWSER_PATH);
    if (browse_it != kv.end())
    {
        browse_path = browse_it->second;
    }

    auto book_it = kv.find(ACTIVITY_KEY_BOOK_PATH);
    if (book_it != kv.end())
    {
        book_path = book_it->second;
    }

    return {browse_path, book_path};
}

std::filesystem::path address_store_path_for_book(const std::filesystem::path &base_path, const std::filesystem::path &book_path)
{
    auto store_filename = book_path.filename();
    store_filename += ".address";

    return base_path / store_filename;
}

void write_book_address(const std::filesystem::path &path, const DocAddr &address)
{
    std::ofstream fp(path);
    fp << encode_address(address);
    fp.close();
}

std::optional<DocAddr> load_book_address(const std::filesystem::path &path)
{
    std::ifstream fp(path);
    if (fp.is_open())
    {
        std::string contents;
        std::getline(fp, contents);
        fp.close();

        return decode_address(contents);
    }
    return std::nullopt;
}

} // namespace

StateStore::StateStore(std::filesystem::path base_dir)
    : activity_store_path(base_dir / "activity"),
      addresses_root_path(base_dir / "books"),
      settings_store_path(base_dir / "settings"),
      settings(load_key_value(settings_store_path))
{
    std::filesystem::create_directories(base_dir);
    std::filesystem::create_directories(addresses_root_path);

    auto [browse_path, book_path] = load_activity_store(activity_store_path);
    current_browse_path = browse_path;
    current_book_path = book_path;
}

StateStore::~StateStore()
{
}

const std::optional<std::filesystem::path> &StateStore::get_current_browse_path() const
{
    return current_browse_path;
}

void StateStore::set_current_browse_path(std::filesystem::path path)
{
    if (current_browse_path != path)
    {
        current_browse_path = path;
        activity_dirty = true;
    }
}

void StateStore::remove_current_browse_path()
{
    if (current_browse_path)
    {
        current_browse_path.reset();
        activity_dirty = true;
    }
}

const std::optional<std::filesystem::path> &StateStore::get_current_book_path() const
{
    return current_book_path;
}

void StateStore::set_current_book_path(std::filesystem::path path)
{
    if (current_book_path != path)
    {
        current_book_path = path;
        activity_dirty = true;
    }
}

void StateStore::remove_current_book_path()
{
    if (current_book_path)
    {
        current_book_path.reset();
        activity_dirty = true;
    }
}

std::optional<DocAddr> StateStore::get_book_address(const std::string &book_path) const
{
    auto it = book_addresses.find(book_path);
    if (it != book_addresses.end())
    {
        return it->second;
    }

    auto cache = load_book_address(
        address_store_path_for_book(addresses_root_path, book_path)
    );
    if (cache)
    {
        book_addresses[book_path] = *cache;
    }

    return cache;
}

void StateStore::set_book_address(const std::string &book_path, const DocAddr &address)
{
    auto it = book_addresses.find(book_path);
    if (it == book_addresses.end() || it->second != address)
    {
        book_addresses[book_path] = address;
    }
}

void StateStore::flush() const
{
    if (activity_dirty)
    {
        write_activity_store(activity_store_path, *this);
        activity_dirty = false;
    }

    // book addresses
    {
        for (const auto &[book_path, address] : book_addresses)
        {
            write_book_address(
                address_store_path_for_book(addresses_root_path, book_path),
                address
            );
        }
        book_addresses.clear();
    }

    if (settings_dirty)
    {
        write_key_value(settings_store_path, settings);
        settings_dirty = false;
    }
}

std::optional<std::string> StateStore::get_setting(const std::string &name) const
{
    auto it = settings.find(name);
    if (it != settings.end())
    {
        return it->second;
    }
    return std::nullopt;
}

void StateStore::set_setting(const std::string &name, const std::string &value)
{
    auto it = settings.find(name);
    if (it == settings.end() || it->second != value)
    {
        settings[name] = value;
        settings_dirty = true;
    }
}
