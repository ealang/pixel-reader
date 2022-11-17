#include "./file_selector.h"
#include "./selection_menu.h"

#include "sys/filesystem.h"
#include "sys/filesystem_path.h"

#include <iostream>
#include <vector>

struct FSState
{
    // file state
    std::string path;
    std::vector<FSEntry> path_entries;
    std::function<void(std::string)> on_file_selected;

    SelectionMenu menu;

    FSState(std::string path, TTF_Font *font)
        : path(path),
          menu(font)
    {
    }
};

namespace {

void refresh_path_entries(FSState *s)
{
    s->path_entries = directory_listing(s->path);
    if (s->path != "/")
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
        std::cerr << "Got selection with no menu entries" << std::endl;
        return;
    }

    const FSEntry &entry = s->path_entries[menu_index];
    if (entry.is_dir)
    {
        std::string highlight_name;
        if (entry.name == "..")
        {
            highlight_name = fs_path_split_dir(s->path).second;
            s->path = fs_path_parent(s->path);
        }
        else
        {
            // go down a directory
            s->path = fs_path_join(s->path, entry.name);
        }

        refresh_path_entries(s);

        // Update selected menu item
        int new_cursor_pos = 1;
        if (!highlight_name.empty())
        {
            for (int i = 0; i < (int)s->path_entries.size(); ++i)
            {
                if (s->path_entries[i].name == highlight_name && s->path_entries[i].is_dir)
                {
                    new_cursor_pos = i;
                    break;
                }
            }
        }

        s->menu.set_cursor_pos(new_cursor_pos);
    }
    else
    {
        // selected a file
        if (s->on_file_selected)
        {
            std::string full_path = fs_path_join(s->path, entry.name);
            s->on_file_selected(full_path);
        }
    }
}

} // namespace

FileSelector::FileSelector(std::string path, TTF_Font *font)
    : state(std::make_unique<FSState>(
          fs_path_make_absolute(path, get_cwd()),
          font
      ))
{
    refresh_path_entries(state.get());

    state->menu.set_on_selection([this](uint32_t menu_index) {
        on_menu_entry_selected(this->state.get(), menu_index);
    });
}

FileSelector::~FileSelector()
{
}

bool FileSelector::render(SDL_Surface *dest_surface)
{
    return state->menu.render(dest_surface);
}

bool FileSelector::on_keypress(SDLKey key)
{
    return state->menu.on_keypress(key);
}

bool FileSelector::is_done()
{
    return state->menu.is_done();
}

void FileSelector::set_on_file_selected(std::function<void(const std::string &)> on_file_selected)
{
    state->on_file_selected = on_file_selected;
}
