#ifndef TEXT_VIEW_STYLING_H_
#define TEXT_VIEW_STYLING_H_

#include <functional>
#include <memory>
#include <string>

struct TextViewStylingState;

class TextViewStyling
{
    std::unique_ptr<TextViewStylingState> state;
    void notify_subscribers() const;

public:
    TextViewStyling(std::string font, bool show_title_bar);
    virtual ~TextViewStyling();

    // Font
    const std::string &get_font() const;

    // Title bar
    bool get_show_title_bar() const;
    void set_show_title_bar(bool show_title_bar);

    // Subscribe to any changes
    uint32_t subscribe_to_changes(std::function<void()> callback);
    void unsubscribe_from_changes(uint32_t sub_id);
};

#endif
