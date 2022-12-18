#include "./token_view_styling.h"

#include <unordered_map>

struct TokenViewStylingState
{
    std::string font;
    bool show_title_bar;

    uint32_t next_subscriber_id = 1;
    std::unordered_map<uint32_t, std::function<void()>> subscribers;

    TokenViewStylingState(
        std::string font,
        bool show_title_bar
    ) : font(font),
        show_title_bar(show_title_bar)
    {}
};

TokenViewStyling::TokenViewStyling(std::string font, bool show_title_bar)
    : state(std::make_unique<TokenViewStylingState>(font, show_title_bar))
{
}

TokenViewStyling::~TokenViewStyling()
{
}

void TokenViewStyling::notify_subscribers() const
{
    for (auto &sub: state->subscribers)
    {
        sub.second();
    }
}

const std::string &TokenViewStyling::get_font() const
{
    return state->font;
}

bool TokenViewStyling::get_show_title_bar() const
{
    return state->show_title_bar;
}

void TokenViewStyling::set_show_title_bar(bool show_title_bar)
{
    if (state->show_title_bar != show_title_bar)
    {
        state->show_title_bar = show_title_bar;
        notify_subscribers();
    }
}

uint32_t TokenViewStyling::subscribe_to_changes(std::function<void()> callback)
{
    uint32_t sub_id = state->next_subscriber_id++;
    state->subscribers[sub_id] = callback;
    return sub_id;
}

void TokenViewStyling::unsubscribe_from_changes(uint32_t sub_id)
{
    state->subscribers.erase(sub_id);
}
