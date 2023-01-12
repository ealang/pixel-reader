#include "./token_line_scroller.h"

#include "doc_api/token_addressing.h"
#include "reader/text_wrap.h"
#include "sys/screen.h"
#include "util/sdl_utils.h"
#include "util/str_utils.h"

#include "extern/SDL_rotozoom.h"

#include <filesystem>
#include <iostream>

namespace
{

uint32_t get_line_for_address(const IndexedDequeue<std::unique_ptr<DisplayLine>> &lines, DocAddr address)
{
    int best_line = lines.start_index();
    for (int i = lines.start_index(); i < lines.end_index(); ++i)
    {
        const auto &line = lines[i];
        if (line && line->address <= address)
        {
            best_line = i;
            if (line->address == address)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    return best_line;
}

float scale_to_fit_width(int w)
{
    if (w > SCREEN_WIDTH)
    {
        return SCREEN_WIDTH / static_cast<float>(w);
    }
    return 1;
}

} // namespace

std::vector<std::unique_ptr<DisplayLine>> TokenLineScroller::image_to_display_lines(const ImageDocToken &token)
{
    SDL_Surface *image = load_scaled_image(token.path);

    std::vector<std::unique_ptr<DisplayLine>> lines;
    if (image && image->h)
    {
        int num_lines = (image->h + line_height_pixels - 1) / line_height_pixels;
        lines.emplace_back(std::make_unique<ImageLine>(token.address, token.path, num_lines, image->w, image->h));
        for (int i = 1; i < num_lines; ++i)
        {
            lines.emplace_back(std::make_unique<ImageRefLine>(token.address, i));
        }
    }
    else
    {
        // Fallback for error loading image
        lines.emplace_back(std::make_unique<TextLine>(token.address, ""));
        lines.emplace_back(std::make_unique<TextLine>(token.address, "[Image " + token.path.string() + "]"));
        lines.emplace_back(std::make_unique<TextLine>(token.address, ""));
    }
    return lines;
}


std::vector<std::unique_ptr<DisplayLine>> TokenLineScroller::render_display_lines(const DocToken &token)
{
    if (token.type == TokenType::Image)
    {
        return image_to_display_lines(static_cast<const ImageDocToken &>(token));
    }
    else
    {
        std::string text;

        if (token.type == TokenType::Text)
        {
            text = static_cast<const TextDocToken &>(token).text;
        }
        else if (token.type == TokenType::Header)
        {
            text = static_cast<const HeaderDocToken &>(token).text;
        }
        else
        {
            throw std::runtime_error("Unknown token type");
        }

        DocAddr address = token.address;
        std::vector<std::unique_ptr<DisplayLine>> lines;
        wrap_lines(text.c_str(), line_fits, [type=token.type, &lines, &address](const char *str, uint32_t len) {
            std::string line_text(str, len);
            bool centered = type == TokenType::Header;

            lines.push_back(
                std::make_unique<TextLine>(address, line_text, centered)
            );

            address += get_address_width(line_text);
        });

        return lines;
    }
}

void TokenLineScroller::get_more_lines_forward(uint32_t num_lines)
{
    while (num_lines > 0)
    {
        const DocToken *token = forward_it->read(1);
        if (!token)
        {
            global_end_line = lines_buf.end_index();
            break;
        }

        for (auto &line : render_display_lines(*token))
        {
            lines_buf.append(std::move(line));
            if (num_lines > 0)
            {
                --num_lines;
            }
        }
    }
}

void TokenLineScroller::get_more_lines_backward(uint32_t num_lines)
{
    while (num_lines > 0)
    {
        const DocToken *token = backward_it->read(-1);
        if (!token)
        {
            global_first_line = lines_buf.start_index();
            break;
        }

        std::vector<std::unique_ptr<DisplayLine>> lines = render_display_lines(*token);
        for (auto it = lines.rbegin(); it != lines.rend(); ++it)
        {
            lines_buf.prepend(std::move(*it));
            if (num_lines > 0)
            {
                --num_lines;
            }
        }
    }
}

void TokenLineScroller::clear_buffer()
{
    lines_buf.clear();
    current_line = 0;
    global_first_line = std::nullopt;
    global_end_line = std::nullopt;
}

void TokenLineScroller::initialize_buffer_at(DocAddr address)
{
    clear_buffer();

    backward_it = reader->get_iter(address);
    forward_it = backward_it->clone();

    materialize_line(0);
    current_line = get_line_for_address(lines_buf, address);
}

TokenLineScroller::TokenLineScroller(
    const std::shared_ptr<DocReader> reader,
    DocAddr address,
    std::function<bool(const char *, uint32_t)> line_fits,
    uint32_t line_height_pixels
) : reader(reader),
    forward_it(nullptr),
    backward_it(nullptr),
    line_fits(line_fits),
    line_height_pixels(line_height_pixels)
{
    initialize_buffer_at(address);
}

void TokenLineScroller::materialize_line(int line_num)
{
    int forward_needed = line_num - lines_buf.end_index() + 1;
    if (forward_needed > 0)
    {
        get_more_lines_forward(forward_needed);
    }

    int backwards_lines_needed = lines_buf.start_index() - line_num;
    if (backwards_lines_needed > 0)
    {
        get_more_lines_backward(backwards_lines_needed);
    }
}

const DisplayLine *TokenLineScroller::get_line_relative(int offset)
{
    int line = current_line + offset;
    materialize_line(line);

    if (line < lines_buf.start_index() || line >= lines_buf.end_index())
    {
        return nullptr;
    }
    return lines_buf[line].get();
}

int TokenLineScroller::get_line_number() const
{
    return current_line;
}

void TokenLineScroller::seek_lines_relative(int offset)
{
    current_line += offset;
    materialize_line(current_line);
}

void TokenLineScroller::seek_to_address(DocAddr address)
{
    initialize_buffer_at(address);
}

void TokenLineScroller::reset_buffer()
{
    const DisplayLine *line = get_line_relative(0);
    if (line)
    {
        DocAddr cur_address = line->address;
        clear_buffer();
        initialize_buffer_at(cur_address);
    }
}

void TokenLineScroller::set_line_height_pixels(uint32_t new_height)
{
    line_height_pixels = new_height;
}

std::optional<int> TokenLineScroller::first_line_number() const
{
    return global_first_line;
}

std::optional<int> TokenLineScroller::end_line_number() const
{
    return global_end_line;
}

SDL_Surface *TokenLineScroller::load_scaled_image(const std::filesystem::path &path)
{
    {
        SDL_Surface *image = image_cache.get_image(path);
        if (image)
        {
            return image;
        }
    }

    auto img_data = reader->load_resource(path);
    if (img_data.empty())
    {
        std::cerr << "Failed to read image data: " << path << std::endl;
        return nullptr;
    }

    std::string file_ext;
    try
    {
        file_ext = path.extension().string().substr(1);
    }
    catch (const std::out_of_range &)
    {
        return nullptr;
    }

    auto img_surface = load_surface_from_ptr(
        img_data.data(),
        img_data.size(),
        file_ext.c_str(),
        get_render_surface_format()
    );
    if (!img_surface)
    {
        std::cerr << "Failed to load image: " << path << std::endl;
        return nullptr;
    }

    float scale = scale_to_fit_width(img_surface->w);
    image_cache.put_image(
        path,
        scale != 1 ? 
            surface_unique_ptr { zoomSurface(img_surface.get(), scale, scale, 1) } :
            std::move(img_surface)
    );

    return image_cache.get_image(path);
}
