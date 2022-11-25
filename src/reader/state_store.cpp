#include "./state_store.h"

#include <fstream>
#include <iostream>

#define STORE_VERSION "v1"

namespace
{

void write_store(const std::filesystem::path &path, const StateStore &store)
{
    std::ofstream fp(path);
    fp << STORE_VERSION << std::endl;
    fp << store.get_current_browse_path().string() << std::endl;
    fp << store.get_current_book_path().value_or("").string() << std::endl;
    fp.close();
}

void load_store(const std::filesystem::path &path, StateStore &store)
{
    std::string browse_path, book_path;

    std::ifstream fp(path);
    if (fp.is_open())
    {
        std::string version;
        std::getline(fp, version);
        if (version == STORE_VERSION)
        {
            std::getline(fp, browse_path);
            std::getline(fp, book_path);
        }
        else
        {
            std::cerr << "Unknown store version: " << version << std::endl;
        }
        fp.close();
    }
    else
    {
        std::cerr << "Store not found: " << path << std::endl;
    }

    if (browse_path.size())
    {
        store.store_current_browse_path(browse_path);
    }
    else
    {
        store.store_current_browse_path(std::filesystem::current_path());
    }

    if (book_path.size())
    {
        store.store_current_book_path(book_path);
    }
    else
    {
        store.remove_current_book_path();
    }
}

} // namespace

StateStore::StateStore(std::filesystem::path store_dir)
    : store_path(store_dir / "state.txt")
{
    std::filesystem::create_directories(store_dir);
    load_store(store_path, *this);
}

StateStore::~StateStore()
{
}

const std::filesystem::path &StateStore::get_current_browse_path() const
{
    return current_browse_path;
}

void StateStore::store_current_browse_path(std::filesystem::path path)
{
    current_browse_path = path;
}

const std::optional<std::filesystem::path> &StateStore::get_current_book_path() const
{
    return current_book_path;
}

void StateStore::store_current_book_path(std::filesystem::path path)
{
    current_book_path = path;
}

void StateStore::remove_current_book_path()
{
    current_book_path.reset();
}

void StateStore::flush() const
{
    write_store(store_path, *this);
}

