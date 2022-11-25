#include "./state_store.h"

#include <fstream>
#include <iostream>

#define ACTIVITY_STORE_VERSION "v1"

namespace
{

void write_activity_store(const std::filesystem::path &path, const StateStore &store)
{
    std::ofstream fp(path);
    fp << ACTIVITY_STORE_VERSION << std::endl;
    fp << store.get_current_browse_path().value_or("").string() << std::endl;
    fp << store.get_current_book_path().value_or("").string() << std::endl;
    fp.close();
}

void load_activity_store(const std::filesystem::path &path, StateStore &store)
{
    std::string browse_path, book_path;

    std::ifstream fp(path);
    if (fp.is_open())
    {
        std::string version;
        std::getline(fp, version);
        if (version == ACTIVITY_STORE_VERSION)
        {
            std::getline(fp, browse_path);
            std::getline(fp, book_path);
        }
        else
        {
            std::cerr << "Unknown activity store version: " << version << std::endl;
        }
        fp.close();
    }

    if (browse_path.size())
    {
        store.set_current_browse_path(browse_path);
    }
    else
    {
        store.remove_current_browse_path();
    }

    if (book_path.size())
    {
        store.set_current_book_path(book_path);
    }
    else
    {
        store.remove_current_book_path();
    }
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
      addresses_root_path(base_dir / "books")
{
    std::filesystem::create_directories(base_dir);
    std::filesystem::create_directories(addresses_root_path);
    load_activity_store(activity_store_path, *this);
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
        dirty = true;
    }
}

void StateStore::remove_current_browse_path()
{
    if (current_browse_path)
    {
        current_browse_path.reset();
        dirty = true;
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
        dirty = true;
    }
}

void StateStore::remove_current_book_path()
{
    if (current_book_path)
    {
        current_book_path.reset();
        dirty = true;
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
        dirty = true;
    }
}

void StateStore::flush() const
{
    if (dirty)
    {
        write_activity_store(activity_store_path, *this);

        for (const auto &[book_path, address] : book_addresses)
        {
            write_book_address(
                address_store_path_for_book(addresses_root_path, book_path),
                address
            );
        }

        book_addresses.clear();
        dirty = false;
    }
}

