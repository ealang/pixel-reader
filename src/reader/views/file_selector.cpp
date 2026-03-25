#include "./file_selector.h"

#include "extern/rotozoom/SDL_rotozoom.h"
#include "filetypes/epub/epub_cover.h"
#include "filetypes/open_doc.h"
#include "reader/shoulder_keymap.h"
#include "reader/state_store.h"
#include "reader/system_styling.h"
#include "sys/filesystem.h"
#include "sys/keymap.h"
#include "sys/screen.h"
#include "util/sdl_font_cache.h"
#include "util/sdl_image_cache.h"
#include "util/sdl_utils.h"
#include "util/str_utils.h"
#include "util/throttled.h"
#include "util/utf8.h"

#include <SDL/SDL_ttf.h>

#include <algorithm>
#include <condition_variable>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <optional>
#include <strings.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{

const int LINE_PADDING = 4;
const int ENTRY_GAP = 5;
const int PREVIEW_PANEL_PADDING = 10;
const int PREVIEW_PANEL_GAP = 8;
const int PREVIEW_BORDER_WIDTH = 2;

struct PreviewRequest
{
    uint64_t request_id;
    std::filesystem::path book_path;
};

struct IndexRequest
{
    uint64_t request_id;
    std::filesystem::path directory_path;
    std::vector<FSEntry> path_entries;
};

struct PreviewLoadResult
{
    uint64_t request_id = 0;
    std::filesystem::path book_path;
    std::string title;
    std::string author;
    std::string progress_label;
    std::filesystem::path cover_path;
    std::string media_type;
    std::vector<char> data;
};

struct IndexProgress
{
    uint64_t request_id = 0;
    std::filesystem::path book_path;
    std::string title;
    std::string author;
    std::string progress_label;
    uint32_t indexed_books = 0;
    uint32_t total_books = 0;
    bool finished = false;
};

struct PreviewTextInfo
{
    std::string title;
    std::string author;
};

enum class SortMode
{
    TITLE,
    AUTHOR,
};

bool path_is_epub(const std::filesystem::path &path)
{
    return to_lower(path.extension()) == ".epub";
}

std::string image_format_for_preview(const PreviewLoadResult &result)
{
    if (result.cover_path.has_extension())
    {
        auto ext = result.cover_path.extension().string();
        if (ext.size() > 1)
        {
            return ext.substr(1);
        }
    }

    if (result.media_type == "image/jpeg")
    {
        return "jpg";
    }
    if (result.media_type == "image/png")
    {
        return "png";
    }
    if (result.media_type == "image/gif")
    {
        return "gif";
    }
    if (result.media_type == "image/bmp")
    {
        return "bmp";
    }

    return {};
}

bool text_fits(TTF_Font *font, const std::string &text, int max_width)
{
    if (max_width <= 0)
    {
        return false;
    }

    int text_width = 0;
    int text_height = 0;
    return TTF_SizeUTF8(font, text.c_str(), &text_width, &text_height) == 0 && text_width <= max_width;
}

std::string truncate_text_to_width(TTF_Font *font, const std::string &text, int max_width)
{
    if (text.empty() || max_width <= 0)
    {
        return {};
    }

    if (text_fits(font, text, max_width))
    {
        return text;
    }

    const std::string suffix = "...";
    if (!text_fits(font, suffix, max_width))
    {
        return {};
    }

    const char *start = text.c_str();
    const char *cur = start;
    const char *best = start;
    while (*cur)
    {
        const char *next = utf8_step(cur);
        std::string candidate(start, next - start);
        candidate += suffix;
        if (!text_fits(font, candidate, max_width))
        {
            break;
        }
        best = next;
        cur = next;
    }

    return std::string(start, best - start) + suffix;
}

std::pair<std::string, std::string> split_text_to_two_lines(
    TTF_Font *font,
    const std::string &text,
    int max_width
)
{
    if (text.empty() || max_width <= 0)
    {
        return {};
    }

    if (text_fits(font, text, max_width))
    {
        return { text, {} };
    }

    const char *start = text.c_str();
    const char *cur = start;
    const char *best = start;

    while (*cur)
    {
        const char *next = utf8_step(cur);
        std::string candidate(start, next - start);
        if (!text_fits(font, candidate, max_width))
        {
            break;
        }
        best = next;
        cur = next;
    }

    if (best == start)
    {
        return { truncate_text_to_width(font, text, max_width), {} };
    }

    std::string first_line(start, best - start);
    std::string second_line = truncate_text_to_width(font, std::string(best), max_width);
    return { first_line, second_line };
}

double preview_scale_to_fit(const SDL_Surface *surface, int max_width, int max_height)
{
    if (!surface || max_width <= 0 || max_height <= 0)
    {
        return 1.0;
    }

    double width_scale = static_cast<double>(max_width) / surface->w;
    double height_scale = static_cast<double>(max_height) / surface->h;
    return std::min(1.0, std::min(width_scale, height_scale));
}

SDL_Rect preview_panel_rect()
{
    int width = std::clamp<int>(SCREEN_WIDTH / 3, 120, 220);
    return SDL_Rect {
        static_cast<Sint16>(SCREEN_WIDTH - width - PREVIEW_PANEL_GAP),
        static_cast<Sint16>(PREVIEW_PANEL_GAP),
        static_cast<Uint16>(width),
        static_cast<Uint16>(SCREEN_HEIGHT - PREVIEW_PANEL_GAP * 2)
    };
}

SDL_Rect preview_image_rect(const SDL_Rect &panel, int line_height, int secondary_line_height)
{
    int header_height = secondary_line_height + LINE_PADDING;
    int footer_height = line_height * 2 + secondary_line_height * 2 + PREVIEW_PANEL_PADDING;
    return SDL_Rect {
        static_cast<Sint16>(panel.x + PREVIEW_PANEL_PADDING),
        static_cast<Sint16>(panel.y + PREVIEW_PANEL_PADDING + header_height),
        static_cast<Uint16>(panel.w - PREVIEW_PANEL_PADDING * 2),
        static_cast<Uint16>(std::max(0, panel.h - footer_height - header_height - PREVIEW_PANEL_PADDING * 2))
    };
}

std::string preview_cache_key(const std::filesystem::path &path, int line_height, int secondary_line_height)
{
    SDL_Rect panel = preview_panel_rect();
    SDL_Rect image_rect = preview_image_rect(panel, line_height, secondary_line_height);
    return path.string() + "#" +
        std::to_string(image_rect.w) + "x" +
        std::to_string(image_rect.h);
}

void render_text_line(
    SDL_Surface *dest_surface,
    TTF_Font *font,
    const std::string &text,
    const SDL_Color &fg_color,
    const SDL_Color &bg_color,
    Sint16 x,
    Sint16 y
)
{
    if (text.empty())
    {
        return;
    }

    auto surface = surface_unique_ptr {
        TTF_RenderUTF8_Shaded(font, text.c_str(), fg_color, bg_color)
    };
    if (!surface)
    {
        return;
    }

    SDL_Rect dest_rect = { x, y, 0, 0 };
    SDL_BlitSurface(surface.get(), NULL, dest_surface, &dest_rect);
}

uint32_t secondary_font_size(uint32_t base_size)
{
    return std::max<uint32_t>(10, base_size > 2 ? base_size - 2 : base_size);
}

std::string load_progress_label(const std::filesystem::path &book_path, const StateStore &state_store)
{
    auto reader = create_doc_reader(book_path);
    if (!reader || !reader->open())
    {
        return {};
    }

    auto book_id = reader->get_id();
    if (book_id.empty())
    {
        return {};
    }

    auto saved_address = state_store.read_book_address_from_disk(book_id);
    if (!saved_address)
    {
        return "Unread";
    }

    uint32_t progress_percent = reader->get_global_progress_percent(*saved_address);
    if (progress_percent >= 99)
    {
        return "Finished";
    }
    if (progress_percent == 0)
    {
        return "Started";
    }

    return "Resume " + std::to_string(progress_percent) + "%";
}

} // namespace

struct FSState
{
    std::filesystem::path path;
    std::vector<FSEntry> path_entries;
    std::function<void(const std::filesystem::path &)> on_file_selected;
    std::function<void(const std::filesystem::path &)> on_file_focus;
    std::function<void()> on_view_focus;
    std::function<void(task_func)> async;

    StateStore &state_store;
    SystemStyling &styling;
    const uint32_t styling_sub_id;

    bool needs_render = true;
    bool is_done = false;
    uint32_t cursor_pos = 0;
    uint32_t scroll_pos = 0;
    int line_height;
    int secondary_line_height;
    SortMode sort_mode = SortMode::TITLE;
    bool index_started = false;
    bool index_loading = false;
    std::string pending_focus_entry_name;
    uint32_t indexed_books = 0;
    uint32_t total_books = 0;

    Throttled scroll_throttle;

    SDLImageCache preview_cache;
    std::unordered_map<std::string, PreviewTextInfo> preview_text_cache;
    std::unordered_map<std::string, std::string> progress_label_cache;
    std::unordered_set<std::string> missing_preview_paths;
    std::filesystem::path focused_path;
    bool preview_loading = false;

    std::mutex preview_mutex;
    std::condition_variable preview_cv;
    bool stop_preview_worker = false;
    std::optional<PreviewRequest> queued_preview_request;
    uint64_t current_preview_request_id = 0;
    std::thread preview_worker;

    std::mutex index_mutex;
    std::condition_variable index_cv;
    bool stop_index_worker = false;
    std::optional<IndexRequest> queued_index_request;
    uint64_t current_index_request_id = 0;
    std::thread index_worker;

    FSState(
        std::filesystem::path path,
        StateStore &state_store,
        SystemStyling &styling,
        std::function<void(task_func)> async
    )
        : path(std::move(path))
        , async(std::move(async))
        , state_store(state_store)
        , styling(styling)
        , styling_sub_id(styling.subscribe_to_changes([this, &styling](SystemStyling::ChangeId) {
            needs_render = true;
            line_height = detect_line_height(
                styling.get_font_name(),
                styling.get_font_size()
            ) + LINE_PADDING;
            secondary_line_height = detect_line_height(
                styling.get_font_name(),
                secondary_font_size(styling.get_font_size())
            ) + LINE_PADDING / 2;
        }))
        , line_height(detect_line_height(
            styling.get_font_name(),
            styling.get_font_size()
        ) + LINE_PADDING)
        , secondary_line_height(detect_line_height(
            styling.get_font_name(),
            secondary_font_size(styling.get_font_size())
        ) + LINE_PADDING / 2)
        , scroll_throttle(250, 100)
    {
    }
};

namespace
{

void focus_menu_index(FSState *s, uint32_t new_cursor_pos);
void focus_menu_entry(FSState *s, const std::string &entry_name);

uint32_t entry_height(const FSState *s)
{
    return s->line_height + s->secondary_line_height + ENTRY_GAP;
}

uint32_t entry_content_height(const FSState *s)
{
    return s->line_height + s->secondary_line_height;
}

uint32_t default_focus_index(const FSState *s)
{
    if (s->path_entries.empty())
    {
        return 0;
    }

    if (s->path_entries[0].name == ".." && s->path_entries.size() > 1)
    {
        return 1;
    }

    return 0;
}

uint32_t num_display_lines(const FSState *s)
{
    return std::max<uint32_t>(1, SCREEN_HEIGHT / entry_height(s));
}

uint32_t excess_pxl_y(const FSState *s)
{
    return SCREEN_HEIGHT - num_display_lines(s) * entry_height(s);
}

TTF_Font *secondary_font(const FSState *s)
{
    return cached_load_font(
        s->styling.get_font_name(),
        secondary_font_size(s->styling.get_font_size())
    );
}

std::string entry_cache_key(const std::filesystem::path &path)
{
    return path.string();
}

std::string entry_title_text(const FSState *s, const std::filesystem::path &path)
{
    auto info_it = s->preview_text_cache.find(entry_cache_key(path));
    if (info_it != s->preview_text_cache.end() && !info_it->second.title.empty())
    {
        return info_it->second.title;
    }

    return path.filename().string();
}

std::string entry_author_text(const FSState *s, const std::filesystem::path &path)
{
    auto info_it = s->preview_text_cache.find(entry_cache_key(path));
    if (info_it != s->preview_text_cache.end())
    {
        return info_it->second.author;
    }

    return {};
}

std::string normalized_sort_value(const std::string &value)
{
    return to_lower(value);
}

uint32_t count_total_books(const std::vector<FSEntry> &entries)
{
    uint32_t total = 0;
    for (const auto &entry : entries)
    {
        if (!entry.is_dir)
        {
            ++total;
        }
    }
    return total;
}

void sort_path_entries(FSState *s)
{
    std::stable_sort(s->path_entries.begin(), s->path_entries.end(), [&s](const FSEntry &a, const FSEntry &b) {
        if (a.name == ".." || b.name == "..")
        {
            return a.name == "..";
        }

        if (a.is_dir != b.is_dir)
        {
            return a.is_dir > b.is_dir;
        }

        if (a.is_dir)
        {
            return strcasecmp(a.name.c_str(), b.name.c_str()) < 0;
        }

        const auto a_path = s->path / a.name;
        const auto b_path = s->path / b.name;

        const auto a_title = normalized_sort_value(entry_title_text(s, a_path));
        const auto b_title = normalized_sort_value(entry_title_text(s, b_path));
        const auto a_author = normalized_sort_value(entry_author_text(s, a_path));
        const auto b_author = normalized_sort_value(entry_author_text(s, b_path));
        const auto a_name = normalized_sort_value(a.name);
        const auto b_name = normalized_sort_value(b.name);

        if (s->sort_mode == SortMode::AUTHOR)
        {
            const auto &a_primary = a_author.empty() ? a_title : a_author;
            const auto &b_primary = b_author.empty() ? b_title : b_author;
            if (a_primary != b_primary)
            {
                return a_primary < b_primary;
            }
        }
        else if (a_title != b_title)
        {
            return a_title < b_title;
        }

        if (a_title != b_title)
        {
            return a_title < b_title;
        }
        return a_name < b_name;
    });
}

void apply_index_progress(const std::shared_ptr<FSState> &s, IndexProgress progress)
{
    if (progress.request_id != s->current_index_request_id)
    {
        return;
    }

    if (!progress.book_path.empty())
    {
        s->preview_text_cache[progress.book_path.string()] = PreviewTextInfo {
            progress.title,
            progress.author
        };
        s->progress_label_cache[progress.book_path.string()] = progress.progress_label;
    }

    s->indexed_books = progress.indexed_books;
    s->total_books = progress.total_books;

    if (progress.finished)
    {
        s->index_loading = false;
        sort_path_entries(s.get());

        if (!s->pending_focus_entry_name.empty())
        {
            focus_menu_entry(s.get(), s->pending_focus_entry_name);
        }
        else
        {
            focus_menu_index(s.get(), default_focus_index(s.get()));
        }
        s->pending_focus_entry_name.clear();
    }

    s->needs_render = true;
}

void apply_preview_result(const std::shared_ptr<FSState> &s, PreviewLoadResult result)
{
    if (result.request_id != s->current_preview_request_id ||
        result.book_path != s->focused_path)
    {
        return;
    }

    s->preview_loading = false;
    s->preview_text_cache[result.book_path.string()] = PreviewTextInfo {
        result.title,
        result.author
    };
    if (!result.progress_label.empty())
    {
        s->progress_label_cache[result.book_path.string()] = result.progress_label;
    }

    if (result.data.empty())
    {
        s->missing_preview_paths.insert(result.book_path.string());
        s->needs_render = true;
        return;
    }

    auto image_format = image_format_for_preview(result);
    if (image_format.empty())
    {
        s->missing_preview_paths.insert(result.book_path.string());
        s->needs_render = true;
        return;
    }

    auto surface = load_surface_from_ptr(
        result.data.data(),
        result.data.size(),
        image_format,
        get_render_surface_format()
    );
    if (!surface)
    {
        s->missing_preview_paths.insert(result.book_path.string());
        s->needs_render = true;
        return;
    }

    SDL_Rect panel = preview_panel_rect();
    SDL_Rect image_rect = preview_image_rect(panel, s->line_height, s->secondary_line_height);
    double scale = preview_scale_to_fit(surface.get(), image_rect.w, image_rect.h);
    s->preview_cache.put_image(
        preview_cache_key(result.book_path, s->line_height, s->secondary_line_height),
        scale < 1.0 ?
            surface_unique_ptr { zoomSurface(surface.get(), scale, scale, 1) } :
            std::move(surface)
    );
    s->needs_render = true;
}

void index_worker_main(std::weak_ptr<FSState> weak_state)
{
    while (true)
    {
        auto s = weak_state.lock();
        if (!s)
        {
            return;
        }

        IndexRequest request;
        {
            std::unique_lock<std::mutex> lock(s->index_mutex);
            s->index_cv.wait(lock, [&s]() {
                return s->stop_index_worker || s->queued_index_request.has_value();
            });

            if (s->stop_index_worker)
            {
                return;
            }

            request = *s->queued_index_request;
            s->queued_index_request.reset();
        }

        uint32_t indexed_books = 0;
        uint32_t total_books = count_total_books(request.path_entries);

        for (const auto &entry : request.path_entries)
        {
            if (entry.is_dir)
            {
                continue;
            }

            const auto book_path = request.directory_path / entry.name;
            IndexProgress progress;
            progress.request_id = request.request_id;
            progress.book_path = book_path;
            progress.total_books = total_books;

            if (path_is_epub(book_path))
            {
                EpubCover metadata;
                if (epub_load_book_metadata(book_path, metadata))
                {
                    progress.title = metadata.title;
                    progress.author = metadata.author;
                }
            }

            progress.indexed_books = ++indexed_books;

            s->async([weak_state, progress = std::move(progress)]() mutable {
                if (auto state = weak_state.lock())
                {
                    apply_index_progress(state, std::move(progress));
                }
            });
        }

        IndexProgress finished_progress;
        finished_progress.request_id = request.request_id;
        finished_progress.total_books = total_books;
        finished_progress.indexed_books = indexed_books;
        finished_progress.finished = true;

        s->async([weak_state, finished_progress = std::move(finished_progress)]() mutable {
            if (auto state = weak_state.lock())
            {
                apply_index_progress(state, std::move(finished_progress));
            }
        });
    }
}

void preview_worker_main(std::weak_ptr<FSState> weak_state)
{
    while (true)
    {
        auto s = weak_state.lock();
        if (!s)
        {
            return;
        }

        PreviewRequest request;
        {
            std::unique_lock<std::mutex> lock(s->preview_mutex);
            s->preview_cv.wait(lock, [&s]() {
                return s->stop_preview_worker || s->queued_preview_request.has_value();
            });

            if (s->stop_preview_worker)
            {
                return;
            }

            request = *s->queued_preview_request;
            s->queued_preview_request.reset();
        }

        PreviewLoadResult result;
        result.request_id = request.request_id;
        result.book_path = request.book_path;

        EpubCover cover;
        if (epub_load_preview_info(request.book_path, cover))
        {
            result.title = cover.title;
            result.author = cover.author;
            result.progress_label = load_progress_label(request.book_path, s->state_store);
            result.cover_path = cover.href_absolute;
            result.media_type = cover.media_type;
            result.data = std::move(cover.data);
        }
        else
        {
            result.progress_label = load_progress_label(request.book_path, s->state_store);
        }

        s->async([weak_state, result = std::move(result)]() mutable {
            if (auto state = weak_state.lock())
            {
                apply_preview_result(state, std::move(result));
            }
        });
    }
}

void cancel_preview_request(FSState *s)
{
    {
        std::lock_guard<std::mutex> lock(s->preview_mutex);
        ++s->current_preview_request_id;
        s->queued_preview_request.reset();
    }
    s->preview_loading = false;
}

void queue_index_request(FSState *s)
{
    {
        std::lock_guard<std::mutex> lock(s->index_mutex);
        s->queued_index_request = IndexRequest {
            ++s->current_index_request_id,
            s->path,
            s->path_entries
        };
    }

    s->index_started = true;
    s->index_loading = s->total_books > 0;
    s->indexed_books = 0;
    s->index_cv.notify_one();
}

void begin_indexing_directory(FSState *s)
{
    if (s->index_loading || s->index_started)
    {
        return;
    }

    cancel_preview_request(s);
    s->focused_path.clear();

    if (s->total_books == 0)
    {
        sort_path_entries(s);
        if (!s->pending_focus_entry_name.empty())
        {
            focus_menu_entry(s, s->pending_focus_entry_name);
            s->pending_focus_entry_name.clear();
        }
        else
        {
            focus_menu_index(s, default_focus_index(s));
        }
        s->needs_render = true;
        return;
    }

    queue_index_request(s);
    s->needs_render = true;
}

void queue_preview_request(FSState *s, const std::filesystem::path &path)
{
    {
        std::lock_guard<std::mutex> lock(s->preview_mutex);
        s->queued_preview_request = PreviewRequest {
            ++s->current_preview_request_id,
            path
        };
    }
    s->preview_loading = true;
    s->preview_cv.notify_one();
}

void refresh_path_entries(FSState *s)
{
    cancel_preview_request(s);
    s->path_entries.clear();
    if (s->path.has_parent_path() && s->path != s->path.root_path())
    {
        s->path_entries.push_back(FSEntry::directory(".."));
    }

    for (const auto &entry : directory_listing(s->path))
    {
        if (entry.is_dir || file_type_is_supported(entry.name))
        {
            s->path_entries.push_back(entry);
        }
    }

    s->total_books = count_total_books(s->path_entries);
    s->index_started = false;
    s->index_loading = false;
    s->cursor_pos = 0;
    s->scroll_pos = 0;
    s->focused_path.clear();
    s->indexed_books = 0;
}

void toggle_sort_mode(FSState *s)
{
    s->sort_mode = (
        s->sort_mode == SortMode::TITLE ? SortMode::AUTHOR : SortMode::TITLE
    );

    if (!s->index_loading && s->index_started)
    {
        const auto focused_name = (
            s->path_entries.empty() ? std::string() : s->path_entries[s->cursor_pos].name
        );
        sort_path_entries(s);
        if (!focused_name.empty())
        {
            focus_menu_entry(s, focused_name);
        }
        else
        {
            focus_menu_index(s, 0);
        }
    }

    s->needs_render = true;
}

void focus_menu_index(FSState *s, uint32_t new_cursor_pos)
{
    if (s->path_entries.empty())
    {
        s->cursor_pos = 0;
        s->scroll_pos = 0;
        s->focused_path.clear();
        cancel_preview_request(s);
        s->needs_render = true;
        return;
    }

    if (new_cursor_pos >= s->path_entries.size())
    {
        new_cursor_pos = 0;
    }

    s->cursor_pos = new_cursor_pos;

    int lines = num_display_lines(s);
    int num_entries = s->path_entries.size();
    s->scroll_pos = std::max(
        0,
        std::min(
            num_entries - lines,
            static_cast<int>(new_cursor_pos) - lines / 4 - 1
        )
    );

    const auto &entry = s->path_entries[s->cursor_pos];
    s->focused_path = s->path / entry.name;
    if (s->on_file_focus)
    {
        s->on_file_focus(s->focused_path);
    }

    if (entry.is_dir)
    {
        cancel_preview_request(s);
    }
    else if (file_type_is_supported(s->focused_path))
    {
        const bool cover_known = (
            !path_is_epub(s->focused_path) ||
            s->preview_cache.get_image(preview_cache_key(s->focused_path, s->line_height, s->secondary_line_height)) ||
            s->missing_preview_paths.count(s->focused_path.string())
        );
        const bool progress_known = s->progress_label_cache.count(s->focused_path.string());

        if (cover_known && progress_known)
        {
            cancel_preview_request(s);
        }
        else
        {
            queue_preview_request(s, s->focused_path);
        }
    }
    else
    {
        cancel_preview_request(s);
    }

    s->needs_render = true;
}

void focus_menu_entry(FSState *s, const std::string &entry_name)
{
    for (uint32_t i = 0; i < s->path_entries.size(); ++i)
    {
        if (s->path_entries[i].name == entry_name)
        {
            focus_menu_index(s, i);
            return;
        }
    }

    focus_menu_index(s, 0);
}

void on_menu_entry_selected(FSState *s)
{
    if (s->index_loading)
    {
        return;
    }

    if (s->path_entries.empty())
    {
        return;
    }

    const FSEntry &entry = s->path_entries[s->cursor_pos];
    if (entry.is_dir)
    {
        if (entry.name == "..")
        {
            std::string highlight_name = s->path.filename().string();

            s->path = s->path.parent_path();
            refresh_path_entries(s);
            s->pending_focus_entry_name = highlight_name;
            begin_indexing_directory(s);
        }
        else
        {
            s->path /= entry.name;
            refresh_path_entries(s);
            s->pending_focus_entry_name.clear();
            begin_indexing_directory(s);
        }
    }
    else if (s->on_file_selected)
    {
        s->on_file_selected(s->path / entry.name);
    }
}

std::filesystem::path sanitize_starting_path(std::filesystem::path path)
{
    path = std::filesystem::absolute(path);

    if (path.has_parent_path())
    {
        path = path.parent_path();
    }

    while (!std::filesystem::is_directory(path))
    {
        std::cerr << "Directory " << path << " does not exist" << std::endl;
        if (path.has_parent_path() && path != path.root_path())
        {
            path = path.parent_path();
        }
        else
        {
            path = std::filesystem::current_path();
            break;
        }
    }

    return path;
}

std::string preview_status_text(const FSState *s)
{
    if (s->focused_path.empty())
    {
        return "Browse your library";
    }

    if (s->preview_loading)
    {
        return "Loading details...";
    }
    if (s->preview_cache.get_image(preview_cache_key(s->focused_path, s->line_height, s->secondary_line_height)))
    {
        return "Embedded cover";
    }
    if (s->missing_preview_paths.count(s->focused_path.string()))
    {
        return "No embedded cover";
    }

    auto entry_it = std::find_if(
        s->path_entries.begin(),
        s->path_entries.end(),
        [&s](const auto &entry) {
            return s->focused_path == s->path / entry.name;
        }
    );
    if (entry_it != s->path_entries.end() && entry_it->is_dir)
    {
        return "Folder";
    }
    if (path_is_epub(s->focused_path))
    {
        return "Cover preview unavailable";
    }
    if (file_type_is_supported(s->focused_path))
    {
        return "Preview available for EPUB";
    }
    return {};
}

std::string preview_title_text(const FSState *s)
{
    if (s->focused_path.empty())
    {
        return "Browse your library";
    }

    auto info_it = s->preview_text_cache.find(s->focused_path.string());
    if (info_it != s->preview_text_cache.end() && !info_it->second.title.empty())
    {
        return info_it->second.title;
    }

    return s->focused_path.filename().string();
}

std::string preview_subtitle_text(const FSState *s)
{
    if (s->focused_path.empty())
    {
        return {};
    }

    auto info_it = s->preview_text_cache.find(s->focused_path.string());
    if (info_it != s->preview_text_cache.end() && !info_it->second.author.empty())
    {
        return info_it->second.author;
    }

    return preview_status_text(s);
}

std::string preview_progress_text(const FSState *s)
{
    if (s->focused_path.empty())
    {
        return {};
    }

    auto it = s->progress_label_cache.find(s->focused_path.string());
    if (it != s->progress_label_cache.end() && !it->second.empty())
    {
        return it->second;
    }

    if (file_type_is_supported(s->focused_path) && !s->preview_loading)
    {
        return "Unread";
    }

    return {};
}

std::string preview_sort_text(const FSState *s)
{
    return s->sort_mode == SortMode::AUTHOR ? "Sort: Author" : "Sort: Title";
}

void render_loading_message(FSState *s, SDL_Surface *dest_surface)
{
    TTF_Font *font = s->styling.get_loaded_font();
    const auto &theme = s->styling.get_loaded_color_theme();
    std::string text = "Loading " + std::to_string(s->indexed_books) +
        " of " + std::to_string(s->total_books) + " books...";

    int text_width = 0;
    int text_height = 0;
    TTF_SizeUTF8(font, text.c_str(), &text_width, &text_height);

    Sint16 x = std::max<Sint16>(LINE_PADDING, (static_cast<int>(SCREEN_WIDTH) - text_width) / 2);
    Sint16 y = std::max<Sint16>(LINE_PADDING, (SCREEN_HEIGHT - text_height) / 2);
    render_text_line(
        dest_surface,
        font,
        text,
        theme.main_text,
        theme.background,
        x,
        y
    );
}

void render_preview_panel(FSState *s, SDL_Surface *dest_surface)
{
    TTF_Font *font = s->styling.get_loaded_font();
    TTF_Font *small_font = secondary_font(s);
    const auto &theme = s->styling.get_loaded_color_theme();
    const auto &bg_color = theme.background;

    SDL_Rect panel = preview_panel_rect();

    SDL_FillRect(
        dest_surface,
        &panel,
        SDL_MapRGB(dest_surface->format, theme.main_text.r, theme.main_text.g, theme.main_text.b)
    );

    SDL_Rect panel_inner = panel;
    panel_inner.x += PREVIEW_BORDER_WIDTH;
    panel_inner.y += PREVIEW_BORDER_WIDTH;
    panel_inner.w -= PREVIEW_BORDER_WIDTH * 2;
    panel_inner.h -= PREVIEW_BORDER_WIDTH * 2;
    SDL_FillRect(
        dest_surface,
        &panel_inner,
        SDL_MapRGB(dest_surface->format, bg_color.r, bg_color.g, bg_color.b)
    );

    SDL_Rect image_rect = preview_image_rect(panel_inner, s->line_height, s->secondary_line_height);
    render_text_line(
        dest_surface,
        small_font,
        preview_sort_text(s),
        theme.secondary_text,
        bg_color,
        panel_inner.x + PREVIEW_PANEL_PADDING,
        panel_inner.y + PREVIEW_PANEL_PADDING
    );

    if (auto *cover = s->preview_cache.get_image(preview_cache_key(s->focused_path, s->line_height, s->secondary_line_height)))
    {
        SDL_Rect dest_rect = {
            static_cast<Sint16>(image_rect.x + (image_rect.w - cover->w) / 2),
            static_cast<Sint16>(image_rect.y + (image_rect.h - cover->h) / 2),
            0,
            0
        };
        SDL_BlitSurface(cover, NULL, dest_surface, &dest_rect);
    }

    const int text_width = panel_inner.w - PREVIEW_PANEL_PADDING * 2;
    const Sint16 title_y = panel_inner.y + panel_inner.h -
        (s->line_height * 2 + s->secondary_line_height * 2);
    const auto [title_line_1, title_line_2] = split_text_to_two_lines(
        font,
        preview_title_text(s),
        text_width
    );
    render_text_line(
        dest_surface,
        font,
        title_line_1,
        theme.main_text,
        bg_color,
        panel_inner.x + PREVIEW_PANEL_PADDING,
        title_y
    );
    render_text_line(
        dest_surface,
        font,
        title_line_2,
        theme.main_text,
        bg_color,
        panel_inner.x + PREVIEW_PANEL_PADDING,
        title_y + s->line_height
    );

    auto subtitle = truncate_text_to_width(
        small_font,
        preview_subtitle_text(s),
        text_width
    );
    render_text_line(
        dest_surface,
        small_font,
        subtitle,
        theme.secondary_text,
        bg_color,
        panel_inner.x + PREVIEW_PANEL_PADDING,
        title_y + s->line_height * 2
    );

    auto progress = truncate_text_to_width(
        small_font,
        preview_progress_text(s),
        text_width
    );
    render_text_line(
        dest_surface,
        small_font,
        progress,
        theme.secondary_text,
        bg_color,
        panel_inner.x + PREVIEW_PANEL_PADDING,
        title_y + s->line_height * 2 + s->secondary_line_height
    );
}

void render_file_list(FSState *s, SDL_Surface *dest_surface)
{
    TTF_Font *font = s->styling.get_loaded_font();
    TTF_Font *small_font = secondary_font(s);
    const auto &theme = s->styling.get_loaded_color_theme();
    const SDL_Color &fg_color = theme.main_text;
    const SDL_Color &secondary_fg_color = theme.secondary_text;
    const SDL_Color &bg_color = theme.background;
    const SDL_Color &hl_bg_color = theme.highlight_background;
    const SDL_Color &hl_text_color = theme.highlight_text;

    SDL_Rect panel = preview_panel_rect();
    int list_width = panel.x - PREVIEW_PANEL_GAP;

    Sint16 x = LINE_PADDING;
    Sint16 y = excess_pxl_y(s) / 2;

    uint32_t num_lines = num_display_lines(s);
    for (uint32_t i = 0; i < num_lines; ++i)
    {
        uint32_t global_i = i + s->scroll_pos;
        if (global_i >= s->path_entries.size())
        {
            break;
        }

        const auto &entry = s->path_entries[global_i];
        const bool is_highlighted = (global_i == s->cursor_pos);

        if (is_highlighted)
        {
            SDL_Rect rect = {0, y, static_cast<Uint16>(list_width), static_cast<Uint16>(entry_content_height(s))};
            SDL_FillRect(
                dest_surface,
                &rect,
                SDL_MapRGB(dest_surface->format, hl_bg_color.r, hl_bg_color.g, hl_bg_color.b)
            );
        }

        const auto entry_path = s->path / entry.name;
        const auto author = entry.is_dir ? std::string() : entry_author_text(s, entry_path);
        const bool emphasize_author = (
            !is_highlighted &&
            !entry.is_dir &&
            s->sort_mode == SortMode::AUTHOR &&
            !author.empty()
        );
        const SDL_Color &title_color = (
            is_highlighted ? hl_text_color :
            emphasize_author ? secondary_fg_color :
            fg_color
        );
        const SDL_Color &author_color = (
            is_highlighted ? hl_text_color :
            emphasize_author ? fg_color :
            secondary_fg_color
        );
        auto display_name = truncate_text_to_width(
            font,
            entry_title_text(s, entry_path),
            list_width - LINE_PADDING * 2
        );
        render_text_line(
            dest_surface,
            font,
            display_name,
            title_color,
            is_highlighted ? hl_bg_color : bg_color,
            x,
            y + LINE_PADDING / 2
        );

        auto subtitle = truncate_text_to_width(
            small_font,
            author,
            list_width - LINE_PADDING * 2
        );
        render_text_line(
            dest_surface,
            small_font,
            subtitle,
            author_color,
            is_highlighted ? hl_bg_color : bg_color,
            x,
            y + s->line_height
        );

        y += entry_height(s);
    }
}

} // namespace

FileSelector::FileSelector(
    std::filesystem::path path,
    StateStore &state_store,
    SystemStyling &styling,
    std::function<void(task_func)> async
)
    : state(std::shared_ptr<FSState>(new FSState(
          sanitize_starting_path(path),
          state_store,
          styling,
          std::move(async)
      )))
{
    refresh_path_entries(state.get());
    state->pending_focus_entry_name = path.has_filename() ? path.filename().string() : std::string();

    state->preview_worker = std::thread(preview_worker_main, std::weak_ptr<FSState>(state));
    state->index_worker = std::thread(index_worker_main, std::weak_ptr<FSState>(state));
}

FileSelector::~FileSelector()
{
    {
        std::lock_guard<std::mutex> lock(state->preview_mutex);
        state->stop_preview_worker = true;
        state->queued_preview_request.reset();
    }
    state->preview_cv.notify_one();
    if (state->preview_worker.joinable())
    {
        state->preview_worker.join();
    }

    {
        std::lock_guard<std::mutex> lock(state->index_mutex);
        state->stop_index_worker = true;
        state->queued_index_request.reset();
    }
    state->index_cv.notify_one();
    if (state->index_worker.joinable())
    {
        state->index_worker.join();
    }

    state->styling.unsubscribe_from_changes(state->styling_sub_id);
}

bool FileSelector::render(SDL_Surface *dest_surface, bool force_render)
{
    if (!state->needs_render && !force_render)
    {
        return false;
    }
    state->needs_render = false;

    const auto &bg_color = state->styling.get_loaded_color_theme().background;
    SDL_Rect rect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_FillRect(
        dest_surface,
        &rect,
        SDL_MapRGB(dest_surface->format, bg_color.r, bg_color.g, bg_color.b)
    );

    if (state->index_loading)
    {
        render_loading_message(state.get(), dest_surface);
    }
    else
    {
        render_file_list(state.get(), dest_surface);
        render_preview_panel(state.get(), dest_surface);
    }
    return true;
}

bool FileSelector::is_done()
{
    return state->is_done;
}

void FileSelector::on_keypress(SDLKey key)
{
    if (state->index_loading)
    {
        switch (key)
        {
            case SW_BTN_Y:
                toggle_sort_mode(state.get());
                break;
            case SW_BTN_B:
                state->is_done = true;
                break;
            default:
                break;
        }
        return;
    }

    switch (key)
    {
        case SW_BTN_UP:
            if (state->cursor_pos > 0)
            {
                focus_menu_index(state.get(), state->cursor_pos - 1);
                state->scroll_pos = std::min(state->scroll_pos, state->cursor_pos);
            }
            break;
        case SW_BTN_DOWN:
            if (!state->path_entries.empty() && state->cursor_pos < state->path_entries.size() - 1)
            {
                focus_menu_index(state.get(), state->cursor_pos + 1);
                if (state->cursor_pos >= state->scroll_pos + num_display_lines(state.get()))
                {
                    state->scroll_pos = state->cursor_pos - num_display_lines(state.get()) + 1;
                }
            }
            break;
        case SW_BTN_L1:
        case SW_BTN_R1:
        case SW_BTN_L2:
        case SW_BTN_R2:
            {
                auto [l_key, r_key] = get_shoulder_keymap_lr(
                    state->styling.get_shoulder_keymap()
                );

                if (key == l_key)
                {
                    key = SW_BTN_LEFT;
                }
                else if (key == r_key)
                {
                    key = SW_BTN_RIGHT;
                }
            }
            // fallthrough
        case SW_BTN_LEFT:
            if (!state->path_entries.empty())
            {
                uint32_t step = std::max<uint32_t>(1, num_display_lines(state.get()) / 2);
                focus_menu_index(
                    state.get(),
                    state->cursor_pos <= step ? 0 : state->cursor_pos - step
                );
                state->scroll_pos = std::min(state->scroll_pos, state->cursor_pos);
            }
            break;
        case SW_BTN_RIGHT:
            if (!state->path_entries.empty())
            {
                uint32_t step = std::max<uint32_t>(1, num_display_lines(state.get()) / 2);
                focus_menu_index(
                    state.get(),
                    std::min<uint32_t>(state->cursor_pos + step, state->path_entries.size() - 1)
                );
                if (state->cursor_pos >= state->scroll_pos + num_display_lines(state.get()))
                {
                    state->scroll_pos = state->cursor_pos - num_display_lines(state.get()) + 1;
                }
            }
            break;
        case SW_BTN_A:
            on_menu_entry_selected(state.get());
            break;
        case SW_BTN_Y:
            toggle_sort_mode(state.get());
            break;
        case SW_BTN_B:
            state->is_done = true;
            break;
        default:
            break;
    }
}

void FileSelector::on_keyheld(SDLKey key, uint32_t held_time_ms)
{
    switch (key)
    {
        case SW_BTN_UP:
        case SW_BTN_DOWN:
        case SW_BTN_LEFT:
        case SW_BTN_RIGHT:
        case SW_BTN_L1:
        case SW_BTN_R1:
        case SW_BTN_L2:
        case SW_BTN_R2:
            if (state->scroll_throttle(held_time_ms))
            {
                on_keypress(key);
            }
            break;
        default:
            break;
    }
}

void FileSelector::on_focus()
{
    if (state->on_view_focus)
    {
        state->on_view_focus();
    }

    begin_indexing_directory(state.get());
}

void FileSelector::set_on_file_selected(std::function<void(const std::filesystem::path &)> callback)
{
    state->on_file_selected = callback;
}

void FileSelector::set_on_file_focus(std::function<void(const std::filesystem::path &)> callback)
{
    state->on_file_focus = callback;
}

void FileSelector::set_on_view_focus(std::function<void()> callback)
{
    state->on_view_focus = callback;
}
