#include "./settings_store.h"
#include "./state_store.h"
#include "./color_theme_def.h"

#include <iostream>
#include <vector>

constexpr const char *SETTINGS_KEY_COLOR_THEME = "color_theme";
constexpr const char *SETTINGS_KEY_SHOW_TITLE_BAR = "show_title_bar";
constexpr const char *SETTINGS_KEY_FONT_SIZE = "font_size";

static const std::vector<uint32_t> FONT_SIZES {
    18,
    22,
    24,
    28,
};
constexpr uint32_t DEFAULT_FONT_SIZE = 22;

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

uint32_t get_next_font_size(uint32_t font_size)
{
    for (uint32_t i = 0; i < FONT_SIZES.size(); i++)
    {
        if (FONT_SIZES[i] == font_size)
        {
            return FONT_SIZES[(i + 1) % FONT_SIZES.size()];
        }
    }
    return DEFAULT_FONT_SIZE;
}
