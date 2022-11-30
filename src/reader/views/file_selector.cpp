#include "./file_selector.h"
#include "./selection_menu.h"

#include "sys/filesystem.h"

#include <filesystem>
#include <iostream>
#include <vector>

struct FSState
{
    std::filesystem::path path;
    std::vector<FSEntry> path_entries;
    std::function<void(const std::filesystem::path &)> on_file_selected;
    std::function<void(const std::filesystem::path &)> on_file_focus;

    SelectionMenu menu;

    FSState(std::filesystem::path path, TTF_Font *font)
        : path(path),
          menu(font)
    {
    }
};

namespace {

void refresh_path_entries(FSState *s)
{
    s->path_entries = directory_listing(s->path);
    if (s->path.has_parent_path() && s->path != s->path.root_path())
    {
        s->path_entries.insert(s->path_entries.begin(), FSEntry::directory(".."));
    }

    std::vector<std::string> menu_entries;
    for (const auto &entry : s->path_entries)
    {
        menu_entries.push_back(entry.name);
    }
    s->menu.set_entries(menu_entries);
}

void on_menu_entry_selected(FSState *s, uint32_t menu_index)
{
    if (s->path_entries.empty())
    {
        return;
    }

    const FSEntry &entry = s->path_entries[menu_index];
    if (entry.is_dir)
    {
        if (entry.name == "..")
        {
            std::string highlight_name = s->path.filename();

            s->path = s->path.parent_path();
            refresh_path_entries(s);
            s->menu.set_cursor_pos(highlight_name);
        }
        else
        {
            // Go down a directory
            s->path /= entry.name;
            refresh_path_entries(s);
            s->menu.set_cursor_pos(1); // get past ".." entry
        }
    }
    else
    {
        if (s->on_file_selected)
        {
            s->on_file_selected(s->path / entry.name);
        }
    }
}

void on_menu_entry_focused(FSState *s, uint32_t menu_index)
{
    if (!s->path_entries.empty() && s->on_file_focus)
    {
        const auto &entry = s->path_entries[menu_index];
        s->on_file_focus(s->path / entry.name);
    }
}

std::filesystem::path sanitize_starting_path(std::filesystem::path path)
{
    path = std::filesystem::absolute(path);

    if (path.has_parent_path())
    {
        // get the directory component
        path = path.parent_path();
    }

    // make sure path exists
    while (!std::filesystem::is_directory(path))
    {
        std::cerr << "Directory " << path << " does not exist" << std::endl;
        if (path.has_parent_path() && path != path.root_path())
        {
            path = path.parent_path();
        }
        else
        {
            path = std::filesystem::current_path();
            break;
        }
    }

    return path;
}

} // namespace

FileSelector::FileSelector(std::filesystem::path path, TTF_Font *font)
    : state(std::make_unique<FSState>(
          sanitize_starting_path(path),
          font
      ))
{
    state->menu.set_on_selection([this](uint32_t menu_index) {
        on_menu_entry_selected(this->state.get(), menu_index);
    });

    state->menu.set_on_focus([this](uint32_t menu_index) {
        on_menu_entry_focused(this->state.get(), menu_index);
    });

    refresh_path_entries(state.get());
    if (path.has_filename())
    {
        state->menu.set_cursor_pos(path.filename());
    }
    else
    {
        state->menu.set_cursor_pos(1); // get past ".." entry
    }
}

FileSelector::~FileSelector()
{
}

bool FileSelector::render(SDL_Surface *dest_surface)
{
    return state->menu.render(dest_surface);
}

bool FileSelector::is_done()
{
    return state->menu.is_done();
}

void FileSelector::on_keypress(SDLKey key)
{
    state->menu.on_keypress(key);
}

void FileSelector::on_keyheld(SDLKey key, uint32_t held_time_ms)
{
    state->menu.on_keyheld(key, held_time_ms);
}

void FileSelector::set_on_file_selected(std::function<void(const std::filesystem::path &)> callback)
{
    state->on_file_selected = callback;
}

void FileSelector::set_on_file_focus(std::function<void(const std::filesystem::path &)> callback)
{
    state->on_file_focus = callback;
}
