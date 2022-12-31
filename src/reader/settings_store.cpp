#include "./settings_store.h"
#include "./state_store.h"

#include <iostream>
#include <vector>

namespace
{

constexpr const char *SETTINGS_KEY_SHOW_TITLE_BAR = "show_title_bar";
constexpr const char *SETTINGS_KEY_COLOR_THEME = "color_theme";
constexpr const char *SETTINGS_KEY_FONT_NAME = "font_name";
constexpr const char *SETTINGS_KEY_FONT_SIZE = "font_size";

} // namespace

std::optional<bool> settings_get_show_title_bar(const StateStore &state_store)
{
    const auto &opt = state_store.get_setting(SETTINGS_KEY_SHOW_TITLE_BAR);
    if (!opt)
    {
        return std::nullopt;
    }
    return *opt == "true";
}

void settings_set_show_title_bar(StateStore &state_store, bool show_title_bar)
{
    state_store.set_setting(SETTINGS_KEY_SHOW_TITLE_BAR, show_title_bar ? "true" : "false");
}

std::optional<std::string> settings_get_color_theme(const StateStore &state_store)
{
    return state_store.get_setting(SETTINGS_KEY_COLOR_THEME);
}

void settings_set_color_theme(StateStore &state_store, const std::string &color_theme)
{
    state_store.set_setting(SETTINGS_KEY_COLOR_THEME, color_theme);
}

std::optional<std::string> settings_get_font_name(const StateStore &state_store)
{
    return state_store.get_setting(SETTINGS_KEY_FONT_NAME);
}

void settings_set_font_name(StateStore &state_store, const std::string &font_name)
{
    state_store.set_setting(SETTINGS_KEY_FONT_NAME, font_name);
}

std::optional<uint32_t> settings_get_font_size(const StateStore &state_store)
{
    auto font_size = state_store.get_setting(SETTINGS_KEY_FONT_SIZE);
    if (!font_size)
    {
        return std::nullopt;
    }
    try
    {
        return std::stoul(*font_size);
    }
    catch (const std::invalid_argument &)
    {
        std::cerr << "Unable to parse font size in store" << std::endl;
    }
    return std::nullopt;
}

void settings_set_font_size(StateStore &state_store, uint32_t font_size)
{
    state_store.set_setting(SETTINGS_KEY_FONT_SIZE, std::to_string(font_size));
}
