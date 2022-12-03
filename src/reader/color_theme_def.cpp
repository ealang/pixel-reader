#include "./color_theme_def.h"

#include <string>
#include <utility>
#include <vector>

static const std::vector<std::pair<std::string, ColorTheme>> theme_defs = {
    {
        "light_contrast",
        {
            {255, 255, 255, 0},
            {0, 0, 0, 0},
            {64, 64, 64, 0},
            {200, 0, 255, 0}
        }
    },
    {
        "light_gray",
        {
            {220, 220, 220, 0},
            {32, 32, 32, 0},
            {64, 64, 64, 0},
            {220, 0, 220, 0}
        }
    },
    {
        "light_sepia",
        {
            {240, 240, 200, 0},
            {32, 32, 32, 0},
            {64, 64, 64, 0},
            {240, 0, 200, 0}
        }
    },
    {
        "night_contrast",
        {
            {0, 0, 0, 0},
            {240, 240, 240, 0},
            {128, 128, 128, 0},
            {128, 0, 128, 0}
        }
    },
    {
        "night_warm",
        {
            {0, 0, 0, 0},
            {188, 182, 128, 0},
            {188 / 2, 182 / 2, 128 / 2, 0},
            {188 / 2, 182 / 2, 128 / 2, 0}
        }
    },
    {
        "vampire",
        {
            {0, 0, 0, 0},
            {188, 0, 0, 0},
            {188 / 2, 182 / 2, 128 / 2, 0},
            {64, 0, 0, 0}
        }
    }
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
