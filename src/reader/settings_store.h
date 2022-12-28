#ifndef SETTINGS_STORE_H_
#define SETTINGS_STORE_H_

#include <string>

struct StateStore;

// Title bar
bool settings_get_show_title_bar(const StateStore &state_store);
void settings_set_show_title_bar(StateStore &state_store, bool show_title_bar);

// Color theme
std::string settings_get_color_theme(const StateStore &state_store);
void settings_set_color_theme(StateStore &state_store, const std::string &color_theme);

std::string settings_get_font_name(const StateStore &state_store);
void settings_set_font_name(StateStore &state_store, const std::string &font_name);
std::string get_prev_font_name(const std::string &font_name);
std::string get_next_font_name(const std::string &font_name);

// Font size
uint32_t settings_get_font_size(const StateStore &state_store);
void settings_set_font_size(StateStore &state_store, uint32_t font_size);
uint32_t get_prev_font_size(uint32_t font_size);
uint32_t get_next_font_size(uint32_t font_size);

#endif
