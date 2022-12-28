#include "./system_styling.h"

#include "reader/color_theme_def.h"

#include <unordered_map>

struct SystemStylingState {
    std::string font_name;
    uint32_t font_size;

    std::string color_theme;
    ColorTheme loaded_color_theme;

    uint32_t next_subscriber_id = 1;
    std::unordered_map<uint32_t, std::function<void()>> subscribers;

    SystemStylingState(
        const std::string &font_name,
        uint32_t font_size,
        const std::string &color_theme
    ) : font_name(font_name),
        font_size(font_size),
        color_theme(color_theme),
        loaded_color_theme(get_color_theme(color_theme))
    {}
};

SystemStyling::SystemStyling(const std::string &font_name, uint32_t font_size, const std::string &color_theme)
    : state(std::make_unique<SystemStylingState>(font_name, font_size, color_theme))
{
}

SystemStyling::~SystemStyling()
{
}

void SystemStyling::set_font_name(std::string font_name)
{
    if (state->font_name != font_name) {
        state->font_name = font_name;
        notify_subscribers();
    }
}

const std::string &SystemStyling::get_font_name() const
{
    return state->font_name;
}

void SystemStyling::set_font_size(uint32_t font_size)
{
    if (state->font_size != font_size)
    {
        state->font_size = font_size;
        notify_subscribers();
    }
}

uint32_t SystemStyling::get_font_size() const
{
    return state->font_size;
}

void SystemStyling::notify_subscribers() const
{
    for (auto &sub: state->subscribers)
    {
        sub.second();
    }
}

void SystemStyling::set_color_theme(const std::string &color_theme)
{
    if (state->color_theme != color_theme)
    {
        state->color_theme = color_theme;
        state->loaded_color_theme = ::get_color_theme(color_theme);
        notify_subscribers();
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

uint32_t SystemStyling::subscribe_to_changes(std::function<void()> callback)
{
    uint32_t sub_id = state->next_subscriber_id++;
    state->subscribers[sub_id] = callback;
    return sub_id;
}

void SystemStyling::unsubscribe_from_changes(uint32_t sub_id)
{
    state->subscribers.erase(sub_id);
}
