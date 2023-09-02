#ifndef SETTINGS_STORE_H_
#define SETTINGS_STORE_H_

#include "./reader/progress_reporting.h"

#include <optional>
#include <string>

struct StateStore;

// Title bar
std::optional<bool> settings_get_show_title_bar(const StateStore &state_store);
void settings_set_show_title_bar(StateStore &state_store, bool show_title_bar);

// Shoulder keymap
std::optional<std::string> settings_get_shoulder_keymap(const StateStore &state_store);
void settings_set_shoulder_keymap(StateStore &state_store, std::string keymap);

// Progress reporting
std::optional<ProgressReporting> settings_get_progress_reporting(const StateStore &state_store);
void settings_set_progress_reporting(StateStore &state_store, ProgressReporting progress_reporting);

// Color theme
std::optional<std::string> settings_get_color_theme(const StateStore &state_store);
void settings_set_color_theme(StateStore &state_store, const std::string &color_theme);

// Font name
std::optional<std::string> settings_get_font_name(const StateStore &state_store);
void settings_set_font_name(StateStore &state_store, const std::string &font_name);

// Font size
std::optional<uint32_t> settings_get_font_size(const StateStore &state_store);
void settings_set_font_size(StateStore &state_store, uint32_t font_size);

#endif
