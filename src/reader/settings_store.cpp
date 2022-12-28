#include "./settings_store.h"
#include "./state_store.h"
#include "./color_theme_def.h"
#include "sys/filesystem.h"

#include <algorithm>
#include <iostream>
#include <vector>

namespace
{

constexpr const char *SETTINGS_KEY_COLOR_THEME = "color_theme";
constexpr const char *SETTINGS_KEY_SHOW_TITLE_BAR = "show_title_bar";
constexpr const char *SETTINGS_KEY_FONT_NAME = "font_name";
constexpr const char *SETTINGS_KEY_FONT_SIZE = "font_size";

constexpr const char *FONT_DIR = "resources/fonts";
constexpr const char *DEFAULT_FONT_NAME = "resources/fonts/DejaVuSans.ttf";

const std::vector<uint32_t> FONT_SIZES {
    20,
    24,
    28,
    32,
};
constexpr uint32_t DEFAULT_FONT_SIZE = 24;

std::vector<std::string> available_fonts;

void discover_fonts()
{
    if (available_fonts.size())
    {
        return;
    }

    for (const auto &entry: directory_listing(FONT_DIR))
    {
        std::filesystem::path path = std::filesystem::path(FONT_DIR) / entry.name;
        if (!entry.is_dir && path.extension().string() == ".ttf")
        {
            available_fonts.push_back(path.string());
        }
    }

    if (available_fonts.empty())
    {
        throw std::runtime_error("No fonts found");
    }
}

template <typename T>
static int get_item_index(const std::vector<T> &items, const T &item)
{
    const auto it = std::find(std::begin(items), std::end(items), item);
    uint32_t i = it - items.begin();
    if (i >= items.size())
    {
        return -1;
    }
    return i;
}

} // namespace

bool settings_get_show_title_bar(const StateStore &state_store)
{
    return state_store.get_setting(SETTINGS_KEY_SHOW_TITLE_BAR).value_or("true") == "true";
}

void settings_set_show_title_bar(StateStore &state_store, bool show_title_bar)
{
    state_store.set_setting(SETTINGS_KEY_SHOW_TITLE_BAR, show_title_bar ? "true" : "false");
}

std::string settings_get_color_theme(const StateStore &state_store)
{
    return state_store.get_setting(SETTINGS_KEY_COLOR_THEME).value_or(get_next_theme(""));
}

void settings_set_color_theme(StateStore &state_store, const std::string &color_theme)
{
    state_store.set_setting(SETTINGS_KEY_COLOR_THEME, color_theme);
}

std::string settings_get_font_name(const StateStore &state_store)
{
    discover_fonts();

    auto font_name = state_store.get_setting(SETTINGS_KEY_FONT_NAME).value_or(DEFAULT_FONT_NAME);
    if (get_item_index(available_fonts, font_name) < 0)
    {
        font_name = available_fonts[0];
    }
    return font_name;
}

void settings_set_font_name(StateStore &state_store, const std::string &font_name)
{
    return state_store.set_setting(SETTINGS_KEY_FONT_NAME, font_name);
}

std::string get_prev_font_name(const std::string &font_name)
{
    discover_fonts();
    int i = get_item_index(available_fonts, font_name);
    return available_fonts[(i + available_fonts.size() - 1) % available_fonts.size()];
}

std::string get_next_font_name(const std::string &font_name)
{
    discover_fonts();
    int i = get_item_index(available_fonts, font_name);
    return available_fonts[(i + 1) % available_fonts.size()];
}

uint32_t settings_get_font_size(const StateStore &state_store)
{
    auto font_size = state_store.get_setting(SETTINGS_KEY_FONT_SIZE);
    if (!font_size)
    {
        return DEFAULT_FONT_SIZE;
    }
    try
    {
        return std::stoul(*font_size);
    }
    catch (const std::invalid_argument &)
    {
        std::cerr << "Unable to parse font size in store" << std::endl;
        return DEFAULT_FONT_SIZE;
    }
}

void settings_set_font_size(StateStore &state_store, uint32_t font_size)
{
    state_store.set_setting(SETTINGS_KEY_FONT_SIZE, std::to_string(font_size));
}

uint32_t get_prev_font_size(uint32_t font_size)
{
    int i = get_item_index(FONT_SIZES, font_size);
    if (i == -1)
    {
        return DEFAULT_FONT_SIZE;
    }

    return FONT_SIZES[(i + FONT_SIZES.size() - 1) % FONT_SIZES.size()];
}

uint32_t get_next_font_size(uint32_t font_size)
{
    int i = get_item_index(FONT_SIZES, font_size);
    if (i == -1)
    {
        return DEFAULT_FONT_SIZE;
    }

    return FONT_SIZES[(i + 1) % FONT_SIZES.size()];
}
