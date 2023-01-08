#include "./token_view.h"

#include "./token_line_scroller.h"
#include "./token_view_styling.h"

#include "doc_api/doc_reader.h"
#include "reader/system_styling.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"
#include "util/sdl_utils.h"
#include "util/throttled.h"

#include <stdexcept>
#include <iostream>
namespace {

bool line_fits_on_screen(TTF_Font *font, int avail_width, const char *s, uint32_t len)
{
    int w = avail_width, h;

    char *mut_s = (char*)s;
    char replaced = mut_s[len];
    mut_s[len] = 0;

    TTF_SizeUTF8(font, mut_s, &w, &h);
    mut_s[len] = replaced;

    return w <= avail_width;
}

}  // namespace

struct TokenViewState
{
    SystemStyling &sys_styling;
    TokenViewStyling &token_view_styling;
    const uint32_t sys_styling_sub_id;
    const uint32_t token_view_styling_sub_id;

    TTF_Font *current_font = nullptr;

    const int line_padding = 4;
    int line_height;

    TokenLineScroller line_scroller;

    bool needs_render = true;

    std::string title;
    int title_progress_percent = 0;

    std::function<void(DocAddr)> on_scroll;

    Throttled line_scroll_throttle;
    Throttled page_scroll_throttle;

    int num_display_lines() const
    {
        return SCREEN_HEIGHT / line_height;
    }

    int num_text_display_lines() const
    {
        bool show_title_bar = token_view_styling.get_show_title_bar();
        return num_display_lines() - (show_title_bar ? 1 : 0);
    }

    int excess_pxl_y() const
    {
        return SCREEN_HEIGHT - num_display_lines() * line_height;
    }

    int line_pxl_limit_y() const
    {
        if (!token_view_styling.get_show_title_bar())
        {
            return SCREEN_HEIGHT;
        }

        return SCREEN_HEIGHT - line_height - excess_pxl_y() / 2;
    }

    TokenViewState(std::shared_ptr<DocReader> reader, DocAddr address, SystemStyling &sys_styling, TokenViewStyling &token_view_styling)
        : sys_styling(sys_styling),
          token_view_styling(token_view_styling),
          sys_styling_sub_id(sys_styling.subscribe_to_changes([this]() {
              // Color theme and font size
              current_font = cached_load_font(
                  this->sys_styling.get_font_name(),
                  this->sys_styling.get_font_size()
              );
              line_height = detect_line_height(current_font) + line_padding;
              line_scroller.set_line_height_pixels(line_height);
              line_scroller.reset_buffer();  // need to re-wrap lines if font-size changed
              needs_render = true;
          })),
          token_view_styling_sub_id(token_view_styling.subscribe_to_changes([this]() {
              needs_render = true;
          })),
          current_font(cached_load_font(
              sys_styling.get_font_name(),
              sys_styling.get_font_size()
          )),
          line_height(detect_line_height(sys_styling.get_font_name(), sys_styling.get_font_size()) + line_padding),
          line_scroller(
              reader,
              address,
              [this](const char *s, uint32_t len) {
                  return line_fits_on_screen(
                      current_font,
                      SCREEN_WIDTH - line_padding * 2,
                      s,
                      len
                  );
              },
              line_height
          ),
          line_scroll_throttle(250, 50),
          page_scroll_throttle(750, 150)
    {
    }

    ~TokenViewState()
    {
        sys_styling.unsubscribe_from_changes(sys_styling_sub_id);
        token_view_styling.unsubscribe_from_changes(token_view_styling_sub_id);
    }
};

TokenView::TokenView(std::shared_ptr<DocReader> reader, DocAddr address, SystemStyling &sys_styling, TokenViewStyling &token_view_styling)
    : state(std::make_unique<TokenViewState>(reader, address, sys_styling, token_view_styling))
{
}

TokenView::~TokenView()
{
}

bool TokenView::render(SDL_Surface *dest_surface, bool force_render)
{
    if (!state->needs_render && !force_render)
    {
        return false;
    }
    state->needs_render = false;

    scroll(0);  // Will adjust scroll position if necessary for end of book

    TTF_Font *font = state->current_font;
    const auto &theme = state->sys_styling.get_loaded_color_theme();
    const int line_height = state->line_height;
    const int line_padding = state->line_padding;

    // Clear screen
    {
        SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        const auto &bgcolor = theme.background;

        SDL_FillRect(
            dest_surface,
            &rect,
            SDL_MapRGB(dest_surface->format, bgcolor.r, bgcolor.g, bgcolor.b)
        );
    }

    int num_text_display_lines = state->num_text_display_lines();
    const Uint16 padding_y = state->excess_pxl_y() / 2;
    Sint16 line_y = padding_y;

    for (int i = 0; i < num_text_display_lines; ++i)
    {
        const DisplayLine *line = state->line_scroller.get_line_relative(i);
        if (line)
        {
            if (line->type == DisplayLine::Type::Text)
            {
                const char *s = static_cast<const TextLine *>(line)->text.c_str();
                auto surface = surface_unique_ptr { TTF_RenderUTF8_Shaded(font, s, theme.main_text, theme.background) };
                SDL_Rect dest_rect = {
                    static_cast<Sint16>(line_padding),
                    static_cast<Sint16>(line_y + line_padding / 2),
                    0, 0
                };
                SDL_BlitSurface(surface.get(), nullptr, dest_surface, &dest_rect);
            }
            else if (line->type == DisplayLine::Type::Image || (line->type == DisplayLine::Type::ImageRef && i == 0))
            {
                const ImageLine *image_line = nullptr;
                uint32_t line_offset = 0;

                if (line->type == DisplayLine::Type::ImageRef)
                {
                    line_offset = static_cast<const ImageRefLine *>(line)->offset;
                    const DisplayLine *ref_line = state->line_scroller.get_line_relative(i - line_offset);
                    if (ref_line)
                    {
                        if (ref_line->type != DisplayLine::Type::Image)
                        {
                            throw std::runtime_error("ImageRefLine points to non image");
                        }
                        image_line = static_cast<const ImageLine *>(ref_line);
                    }
                }
                else
                {
                    image_line = static_cast<const ImageLine *>(line);
                }

                if (image_line)
                {
                    auto *surface = state->line_scroller.load_scaled_image(image_line->image_path);

                    // Amount of line height not used by image
                    uint32_t img_excess_y = image_line->num_lines * line_height - image_line->height;
                    // Y coordinate of image in screen space
                    int screen_start_y = line_y + img_excess_y / 2 - line_height * line_offset;

                    // Crop off-screen part of image. Allow to extend to edge of screen.
                    Sint16 src_y = std::max(-screen_start_y, 0);
                    Sint16 dst_y = std::max(screen_start_y, 0);

                    if (surface && src_y < (Sint16)image_line->height)
                    {
                        Uint16 width = image_line->width;
                        Uint16 height = image_line->height - src_y;

                        // Crop bottom
                        auto dst_y_bottom = dst_y + height;
                        Uint16 y_limit = state->line_pxl_limit_y();
                        if (dst_y_bottom > y_limit)
                        {
                            height -= dst_y_bottom - y_limit;
                        }

                        SDL_Rect src_rect = {0, src_y, width, height};
                        SDL_Rect dest_rect = {
                            static_cast<Sint16>((SCREEN_WIDTH - width) / 2),
                            dst_y,
                            0,
                            0
                        };
                        SDL_BlitSurface(surface, &src_rect, dest_surface, &dest_rect);
                    }
                }
            }
        }
        else
        {
            break;
        }

        line_y += line_height;
    }

    if (state->token_view_styling.get_show_title_bar())
    {
        // Recompute for short book case
        line_y = padding_y + num_text_display_lines * line_height;
        SDL_Rect title_crop_rect = {0, 0, 0, (Uint16)line_height};

        // Progress
        {
            char percent_str[32];
            snprintf(percent_str, sizeof(percent_str), " %d%%", state->title_progress_percent);

            SDL_Surface *page_surface = TTF_RenderUTF8_Shaded(font, percent_str, theme.secondary_text, theme.background);

            SDL_Rect dest_rect = {
                static_cast<Sint16>(SCREEN_WIDTH - page_surface->w - line_padding),
                static_cast<Sint16>(line_y + line_padding / 2),
                0, 0
            };
            title_crop_rect.w = SCREEN_WIDTH - line_padding * 2 - page_surface->w;

            SDL_BlitSurface(page_surface, nullptr, dest_surface, &dest_rect);
            SDL_FreeSurface(page_surface);
        }

        // Toc item
        if (state->title.size() > 0)
        {
            SDL_Rect dest_rect = {
                static_cast<Sint16>(line_padding),
                static_cast<Sint16>(line_y + line_padding / 2),
                0, 0
            };
            SDL_Surface *surface = TTF_RenderUTF8_Shaded(font, state->title.c_str(), theme.secondary_text, theme.background);
            SDL_BlitSurface(surface, &title_crop_rect, dest_surface, &dest_rect);
            SDL_FreeSurface(surface);
        }
    }

    return true;
}

// Adjust scroll amount to avoid going beyond start or end of book.
static int get_bounded_scroll_amount(TokenLineScroller &line_scroller, int num_display_lines, int num_lines)
{
    {
        // if start/end of book is within reach, make sure it is discovered
        line_scroller.get_line_relative(num_display_lines);
        line_scroller.get_line_relative(-num_display_lines);
    }

    int cur_line = line_scroller.get_line_number();
    int new_line = cur_line + num_lines;

    auto end_line = line_scroller.end_line_number();
    if (end_line)
    {
        new_line = std::min(
            *end_line - num_display_lines,
            new_line
        );
    }

    auto first_line = line_scroller.first_line_number();
    if (first_line)
    {
        new_line = std::max(
            *first_line,
            new_line
        );
    }

    return new_line - cur_line;
}

void TokenView::scroll(int num_lines)
{
    num_lines = get_bounded_scroll_amount(
        state->line_scroller,
        state->num_text_display_lines(),
        num_lines
    );
    if (num_lines != 0)
    {
        state->needs_render = true;
        state->line_scroller.seek_lines_relative(num_lines);
        if (state->on_scroll)
        {
            state->on_scroll(get_address());
        }
    }
}

void TokenView::on_keypress(SDLKey key)
{
    switch (key) {
        case SW_BTN_UP:
            scroll(-1);
            break;
        case SW_BTN_DOWN:
            scroll(1);
            break;
        case SW_BTN_LEFT:
        case SW_BTN_L1:
            scroll(-state->num_text_display_lines());
            break;
        case SW_BTN_RIGHT:
        case SW_BTN_R1:
            scroll(state->num_text_display_lines());
            break;
        default:
            break;
    }
}

void TokenView::on_keyheld(SDLKey key, uint32_t held_time_ms)
{
    switch (key) {
        case SW_BTN_UP:
        case SW_BTN_DOWN:
            if (state->line_scroll_throttle(held_time_ms))
            {
                on_keypress(key);
            }
            break;
        case SW_BTN_LEFT:
        case SW_BTN_RIGHT:
        case SW_BTN_L1:
        case SW_BTN_R1:
            if (state->page_scroll_throttle(held_time_ms))
            {
                on_keypress(key);
            }
            break;
        default:
            break;
    }
}

bool TokenView::is_done()
{
    return false;
}

DocAddr TokenView::get_address() const
{
    const DisplayLine *line = state->line_scroller.get_line_relative(0);
    if (line)
    {
        return line->address;
    }
    return make_address();
}

void TokenView::seek_to_address(DocAddr address)
{
    state->line_scroller.seek_to_address(address);
    state->needs_render = true;
}

void TokenView::set_title(const std::string &title)
{
    if (title != state->title)
    {
        state->title = title;
        state->needs_render = true;
    }
}

void TokenView::set_title_progress(int percent)
{
    if (percent != state->title_progress_percent)
    {
        state->title_progress_percent = percent;
        state->needs_render = true;
    }
}

void TokenView::set_on_scroll(std::function<void(DocAddr)> callback)
{
    state->on_scroll = callback;
}
