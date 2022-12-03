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
    SystemStyling(std::string color_theme);
    virtual ~SystemStyling();

    // Color theme definition
    void set_color_theme(std::string color_theme);
    const std::string &get_color_theme() const;
    const ColorTheme &get_loaded_color_theme() const;

    // Subscribe to any changes
    uint32_t subscribe_to_changes(std::function<void()> callback);
    void unsubscribe_from_changes(uint32_t sub_id);
};

#endif
