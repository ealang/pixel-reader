#include "./color_theme_def.h"

#include <string>
#include <utility>
#include <vector>

static const std::vector<std::pair<std::string, ColorTheme>> theme_defs = {
    {
        "night_contrast",
        {
            {0, 0, 0, 0},
            {240, 240, 240, 0},
            {64, 64, 64, 0},
            {150, 38, 200, 0}
        }
    },
    {
        "vampire",
        {
            {0, 0, 0, 0},
            {192, 0, 0, 0},
            {64, 64, 64, 0},
            {80, 0, 0, 0}
        }
    },
    {
        "light_contrast",
        {
            {250, 250, 250, 0},
            {0, 0, 0, 0},
            {160, 160, 160, 0},
            {163, 81, 200, 0}
        }
    },
    {
        "light_sepia",
        {
            {250, 240, 220, 0},
            {32, 32, 32, 0},
            {160, 160, 160, 0},
            {163, 81, 200, 0}
        }
    },
};

static int get_theme_index(const std::string &name)
{
    for (uint32_t i = 0; i < theme_defs.size(); i++)
    {
        if (theme_defs[i].first == name)
        {
            return i;
        }
    }

    return -1;
}

const ColorTheme& get_color_theme(const std::string &name)
{
    int i = get_theme_index(name);
    if (i < 0)
    {
        i = 0;
    }

    return theme_defs[i].second;
}

std::string get_prev_theme(const std::string &name)
{
    int i = get_theme_index(name);
    if (i < 0)
    {
        return theme_defs[0].first;
    }

    return theme_defs[
        (i - 1 + theme_defs.size()) % theme_defs.size()
    ].first;
}

std::string get_next_theme(const std::string &name)
{
    int i = get_theme_index(name);
    if (i < 0)
    {
        return theme_defs[0].first;
    }

    return theme_defs[
        (i + 1) % theme_defs.size()
    ].first;
}
