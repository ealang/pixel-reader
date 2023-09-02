#include "./system_styling.h"
#include "./config.h"

#include "reader/color_theme_def.h"
#include "util/sdl_font_cache.h"

#include <unordered_map>

struct SystemStylingState {
    std::string font_name;
    uint32_t font_size;

    std::string color_theme;
    ColorTheme loaded_color_theme;

    std::string shoulder_keymap;

    uint32_t next_subscriber_id = 1;
    std::unordered_map<uint32_t, std::function<void(SystemStyling::ChangeId)>> subscribers;

    SystemStylingState(
        const std::string &font_name,
        uint32_t font_size,
        const std::string &color_theme,
        const std::string &shoulder_keymap
    ) : font_name(font_name),
        font_size(font_size),
        color_theme(color_theme),
        loaded_color_theme(get_color_theme(color_theme)),
        shoulder_keymap(shoulder_keymap)
    {}
};

SystemStyling::SystemStyling(const std::string &font_name, uint32_t font_size, const std::string &color_theme, const std::string &shoulder_keymap)
    : state(std::make_unique<SystemStylingState>(font_name, font_size, color_theme, shoulder_keymap))
{
}

SystemStyling::~SystemStyling()
{
}

void SystemStyling::notify_subscribers(ChangeId id) const
{
    for (auto &sub: state->subscribers)
    {
        sub.second(id);
    }
}

void SystemStyling::set_font_name(std::string font_name)
{
    if (state->font_name != font_name) {
        state->font_name = font_name;
        notify_subscribers(ChangeId::FONT_NAME);
    }
}

const std::string &SystemStyling::get_font_name() const
{
    return state->font_name;
}

TTF_Font *SystemStyling::get_loaded_font() const
{
    return cached_load_font(state->font_name, state->font_size);
}

void SystemStyling::set_font_size(uint32_t font_size)
{
    if (state->font_size != font_size)
    {
        state->font_size = font_size;
        notify_subscribers(ChangeId::FONT_SIZE);
    }
}

uint32_t SystemStyling::get_font_size() const
{
    return state->font_size;
}

uint32_t SystemStyling::get_prev_font_size() const
{
    uint32_t range = MAX_FONT_SIZE - MIN_FONT_SIZE + FONT_SIZE_STEP;
    return (state->font_size - MIN_FONT_SIZE + range - FONT_SIZE_STEP) % range + MIN_FONT_SIZE;
}

uint32_t SystemStyling::get_next_font_size() const
{
    uint32_t range = MAX_FONT_SIZE - MIN_FONT_SIZE + FONT_SIZE_STEP;
    return (state->font_size - MIN_FONT_SIZE + FONT_SIZE_STEP) % range + MIN_FONT_SIZE;
}

void SystemStyling::set_color_theme(const std::string &color_theme)
{
    if (state->color_theme != color_theme)
    {
        state->color_theme = color_theme;
        state->loaded_color_theme = ::get_color_theme(color_theme);
        notify_subscribers(ChangeId::COLOR_THEME);
    }
}

const std::string &SystemStyling::get_color_theme() const
{
    return state->color_theme;
}

const ColorTheme &SystemStyling::get_loaded_color_theme() const
{
    return state->loaded_color_theme;
}

const std::string &SystemStyling::get_shoulder_keymap() const
{
    return state->shoulder_keymap;
}

void SystemStyling::set_shoulder_keymap(const std::string &keymap)
{
    if (state->shoulder_keymap != keymap)
    {
        state->shoulder_keymap = keymap;
        notify_subscribers(ChangeId::SHOULDER_KEYMAP);
    }
}

uint32_t SystemStyling::subscribe_to_changes(std::function<void(ChangeId)> callback)
{
    uint32_t sub_id = state->next_subscriber_id++;
    state->subscribers[sub_id] = callback;
    return sub_id;
}

void SystemStyling::unsubscribe_from_changes(uint32_t sub_id)
{
    state->subscribers.erase(sub_id);
}
