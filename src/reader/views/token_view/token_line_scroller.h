#ifndef TOKEN_LINE_SCROLLER_H_
#define TOKEN_LINE_SCROLLER_H_

#include "./display_line.h"

#include "doc_api/doc_addr.h"
#include "doc_api/doc_reader.h"
#include "util/indexed_dequeue.h"
#include "util/sdl_image_cache.h"
#include "util/sdl_pointer.h"

#include <functional>
#include <optional>

// Lazy renders tokens into lines of text, and provides access to the lines
// through an infinite-scroll type interface.
class TokenLineScroller
{
    const std::shared_ptr<DocReader> reader;
    std::shared_ptr<TokenIter> forward_it;
    std::shared_ptr<TokenIter> backward_it;
    std::function<bool(const char *, uint32_t)> line_fits;

    std::optional<int> global_first_line;
    std::optional<int> global_last_line;

    uint32_t line_height_pixels;
    uint32_t num_lines_lookahead;
    int current_line = 0;

    IndexedDequeue<std::unique_ptr<DisplayLine>> lines_buf;
    SDLImageCache image_cache;

    std::vector<std::unique_ptr<DisplayLine>> image_to_display_lines(const DocToken &token);
    std::vector<std::unique_ptr<DisplayLine>> token_to_display_lines(const DocToken &token);

    uint32_t get_more_lines_forward(uint32_t num);
    uint32_t get_more_lines_backward(uint32_t num);
    void clear_buffer();
    void initialize_buffer_at(DocAddr address);
    void ensure_lines_around(int line_num);

public:
    TokenLineScroller(
        const std::shared_ptr<DocReader> reader,
        DocAddr address,
        uint32_t num_lines_lookahead,
        std::function<bool(const char *, uint32_t)> line_fits,
        uint32_t line_height_pixels
    );

    const DisplayLine *get_line_relative(int offset);
    int get_line_number() const;
    void seek_lines_relative(int offset);
    void seek_to_address(DocAddr address);
    void reset_buffer();
    void set_line_height_pixels(uint32_t line_height_pixels);

    std::optional<int> first_line_number() const;
    std::optional<int> last_line_number() const;

    SDL_Surface *load_scaled_image(const std::string &path);
};

#endif
