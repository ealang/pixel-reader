#ifndef SYSTEM_STYLING_H_
#define SYSTEM_STYLING_H_

#include "reader/color_theme.h"

#include <functional>
#include <memory>
#include <string>

#include <SDL/SDL_ttf.h>

struct SystemStylingState;

class SystemStyling {
public:
    enum class ChangeId {
        FONT_NAME,
        FONT_SIZE,
        COLOR_THEME,
        SHOULDER_KEYMAP,
    };

private:
    std::unique_ptr<SystemStylingState> state;
    void notify_subscribers(ChangeId) const;

public:
    SystemStyling(
        const std::string &font_name,
        uint32_t font_size,
        const std::string &color_theme,
        const std::string &shoulder_keymap
    );
    virtual ~SystemStyling();

    // Font
    void set_font_name(std::string font_name);
    const std::string &get_font_name() const;
    TTF_Font *get_loaded_font() const;

    // Font size
    void set_font_size(uint32_t font_size);
    uint32_t get_font_size() const;
    uint32_t get_prev_font_size() const;
    uint32_t get_next_font_size() const;

    // Color theme definition
    void set_color_theme(const std::string &color_theme);
    const std::string &get_color_theme() const;
    const ColorTheme &get_loaded_color_theme() const;

    // shoulder keymap
    const std::string &get_shoulder_keymap() const;
    void set_shoulder_keymap(const std::string &keymap);

    // Subscribe to any changes
    uint32_t subscribe_to_changes(std::function<void(ChangeId)> callback);
    void unsubscribe_from_changes(uint32_t sub_id);
};

#endif
