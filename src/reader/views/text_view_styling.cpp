#include "./text_view_styling.h"

#include "util/sdl_pointer.h"

#include <iostream>
#include <unordered_map>

struct TextViewStylingState
{
    ttf_font_unique_ptr loaded_font;
    bool show_title_bar;

    uint32_t next_subscriber_id = 1;
    std::unordered_map<uint32_t, std::function<void()>> subscribers;

    TextViewStylingState(
        std::string font,
        uint32_t font_size,
        bool show_title_bar
    ) : loaded_font(TTF_OpenFont(font.c_str(), font_size)),
        show_title_bar(show_title_bar)
    {}
};

TextViewStyling::TextViewStyling(std::string font, uint32_t font_size, bool show_title_bar)
    : state(std::make_unique<TextViewStylingState>(font, font_size, show_title_bar))
{
    if (!state->loaded_font)
    {
        std::cerr << "Unable to load font: " << font << std::endl;
    }
}

TextViewStyling::~TextViewStyling()
{
}

void TextViewStyling::notify_subscribers() const
{
    for (auto &sub: state->subscribers)
    {
        sub.second();
    }
}

TTF_Font *TextViewStyling::get_loaded_font() const
{
    return state->loaded_font.get();
}

bool TextViewStyling::get_show_title_bar() const
{
    return state->show_title_bar;
}

void TextViewStyling::set_show_title_bar(bool show_title_bar)
{
    if (state->show_title_bar != show_title_bar)
    {
        state->show_title_bar = show_title_bar;
        notify_subscribers();
    }
}

uint32_t TextViewStyling::subscribe_to_changes(std::function<void()> callback)
{
    uint32_t sub_id = state->next_subscriber_id++;
    state->subscribers[sub_id] = callback;
    return sub_id;
}

void TextViewStyling::unsubscribe_from_changes(uint32_t sub_id)
{
    state->subscribers.erase(sub_id);
}
