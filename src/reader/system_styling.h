#ifndef SYSTEM_STYLING_H_
#define SYSTEM_STYLING_H_

#include "reader/color_theme.h"

#include <functional>
#include <memory>
#include <string>

struct SystemStylingState;

class SystemStyling {
    std::unique_ptr<SystemStylingState> state;
    void notify_subscribers() const;

public:
    SystemStyling(
        const std::string &font_name,
        uint32_t font_size,
        const std::string &color_theme
    );
    virtual ~SystemStyling();

    // Font
    void set_font_name(std::string font_name);
    const std::string &get_font_name() const;

    // Font size
    void set_font_size(uint32_t font_size);
    uint32_t get_font_size() const;
    uint32_t get_prev_font_size() const;
    uint32_t get_next_font_size() const;

    // Color theme definition
    void set_color_theme(const std::string &color_theme);
    const std::string &get_color_theme() const;
    const ColorTheme &get_loaded_color_theme() const;

    // Subscribe to any changes
    uint32_t subscribe_to_changes(std::function<void()> callback);
    void unsubscribe_from_changes(uint32_t sub_id);
};

#endif
