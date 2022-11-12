#include "sys/screen.h"
#include "sys/keymap.h"
#include "sys/filesystem.h"

#include "epub/epub_reader.h"

#include "./file_selector.h"
#include "./text_view.h"

#include <SDL/SDL.h>
#include <iostream>

static std::string get_chapter()
{
    EPubReader reader("samples/epub/deep-learning-illustrated.epub");
    if (reader.open())
    {
        auto package = reader.get_package();
        std::string result;
        for (auto &line : reader.get_item_as_text("ch01"))
        {
            result += line;
        }
        return result;
    }
    else
    {
        std::cerr << "Failed to open" << std::endl;
    }
    return "error opening";
}

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
    TTF_Font *font = TTF_OpenFont("fonts/DejaVuSansMono.ttf", font_size);

    if (!font) {
        std::cerr << "TTF_OpenFont: " << TTF_GetError() << std::endl;
        return 1;
    }

    // FileSelector selector(starting_path, font, 4);
    // selector.render(screen);


    auto text = get_chapter();

    TextView view(text, font, 0);
    view.render(screen);

    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    bool quit = false;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            bool rerender = false;
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    if (view.on_keypress(event.key.keysym.sym)) {
                        rerender = true;
                    }
                    // else if (selector.on_keypress(event.key.keysym.sym))
                    // {
                    //     rerender = selector.render(screen);
                    //     if (selector.file_is_selected())
                    //     {
                    //         std::cout << "Selected file: " << selector.get_selected_file() << std::endl;
                    //         quit = true;
                    //     }
                    // }
                    else
                    {
                        switch (event.key.keysym.sym) {
                            case SW_BTN_LEFT:
                                break;
                            case SW_BTN_RIGHT:
                                break;
                            case SW_BTN_A:
                                quit = true;
                                break;
                            case SW_BTN_MENU:
                                quit = true;
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }

            if (rerender)
            {
                // selector.render(screen);
                view.render(screen);
                SDL_BlitSurface(screen, NULL, video, NULL);
                SDL_Flip(video);
            }
        }
    }

    TTF_CloseFont(font);
    SDL_FreeSurface(screen);
    SDL_Quit();
    
    return 0;
}
