#include "sys/screen.h"
#include "sys/keymap.h"
#include "sys/filesystem.h"
#include "sys/timer.h"

#include "epub/epub_reader.h"

#include "./display_lines.h"
#include "./file_selector.h"
#include "./text_view.h"
#include "./view_stack.h"

#include <SDL/SDL.h>
#include <iostream>
#include <libxml/parser.h>

namespace
{

std::vector<std::string> get_book_display_lines(std::string epub_path, TTF_Font *font)
{
    auto line_fits_on_screen = [font](const char *s, int len) {

        int w = SCREEN_WIDTH, h;

        // TODO
        char *mut_s = (char*)s;
        char replaced = mut_s[len];
        mut_s[len] = 0;

        if (TTF_SizeUTF8(font, mut_s, &w, &h) == 0)
        {
            // TODO
        }

        mut_s[len] = replaced;

        return w <= SCREEN_WIDTH;
    };

    EPubReader reader(epub_path);
    if (reader.open())
    {
        Timer timer;

        std::vector<std::string> lines;
        std::cerr << "Found " << reader.get_tok().size() << " chapters" << std::endl;
        int chapter_limit = 16;
        for (auto &tok : reader.get_tok())
        {
            if (chapter_limit-- == 0)
            {
                break;
            }
            // std::cerr << tok.doc_id << " " << tok.name << std::endl;
            auto tokens = reader.get_tokenized_document(tok.doc_id);
            // std::cerr << "Contains " << tokens.size() << " tokens" << std::endl;

            std::vector<Line> display_lines;
            get_display_lines(
                tokens,
                line_fits_on_screen,
                display_lines
            );

            for (const auto &line: display_lines)
            {
                lines.push_back(line.text);
            }
        }

        std::cerr << "Took " << timer.elapsed_ms() << " ms to format " << lines.size() << " lines" << std::endl;
        return lines;
    }
    else
    {
        std::cerr << "Failed to open" << std::endl;
    }
    return {"error opening"};
}

std::shared_ptr<View> app_main(ViewStack &view_stack, std::string starting_path, TTF_Font *font)
{
    std::shared_ptr<FileSelector> fs = std::make_shared<FileSelector>(starting_path, font, 4);

    fs->set_on_file_selected([font, &view_stack](std::string path) {
        auto text = get_book_display_lines(path, font);
        view_stack.push(std::make_shared<TextView>(text, font, 0));
    });

    return fs;
}

} // namespace

int main (int argc, char *argv[])
{
    std::string starting_path = get_cwd();
    if (argc == 2)
    {
        starting_path = argv[1];
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    TTF_Init();

    SDL_Surface *video = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE);
    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);

    int font_size = 24;
    TTF_Font *font = TTF_OpenFont("fonts/DejaVuSerif.ttf", font_size);

    if (!font)
    {
        std::cerr << "TTF_OpenFont: " << TTF_GetError() << std::endl;
        return 1;
    }

    ViewStack view_stack;
    view_stack.push(app_main(view_stack, starting_path, font));
    view_stack.render(screen);

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    bool quit = false;

    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    {
                        SDLKey key = event.key.keysym.sym;
                        if (key == SW_BTN_MENU)
                        {
                            quit = true;
                        }
                        else
                        {
                            view_stack.on_keypress(key);
                        }

                        if (view_stack.is_done())
                        {
                            quit = true;
                        }

                        if (view_stack.render(screen))
                        {
                            SDL_BlitSurface(screen, NULL, video, NULL);
                            SDL_Flip(video);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    TTF_CloseFont(font);
    SDL_FreeSurface(screen);
    SDL_Quit();
    xmlCleanupParser();
    
    return 0;
}
