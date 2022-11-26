#include "sys/screen.h"
#include "sys/keymap.h"

#include "./state_store.h"
#include "./view_stack.h"
#include "./views/file_selector.h"
#include "./views/reader_view.h"

#include <SDL/SDL.h>
#include <iostream>
#include <libxml/parser.h>

namespace
{

void initialize_views(ViewStack &view_stack, StateStore &state_store, TTF_Font *font)
{
    auto browse_path = state_store.get_current_browse_path().value_or(std::filesystem::current_path() / "");
    std::shared_ptr<FileSelector> fs = std::make_shared<FileSelector>(
        browse_path,
        font
    );

    auto load_book = [&view_stack, &state_store, font](std::filesystem::path path) {
        if (path.extension() != ".epub")
        {
            return;
        }
        state_store.set_current_book_path(path);

        std::cerr << "Loading " << path << std::endl;
        auto reader_view = std::make_shared<ReaderView>(
            path,
            state_store.get_book_address(path).value_or(0),
            font,
            view_stack
        );

        reader_view->set_on_change_address([&state_store, path_str=path.string()](const DocAddr &addr) {
            state_store.set_book_address(path_str, addr);
        });
        reader_view->set_on_quit([&state_store]() {
            state_store.remove_current_book_path();
        });

        view_stack.push(reader_view);
    };

    fs->set_on_file_selected(load_book);
    fs->set_on_file_focus([&state_store](std::string path) {
        state_store.set_current_browse_path(path);
    });

    view_stack.push(fs);

    if (state_store.get_current_book_path())
    {
        load_book(state_store.get_current_book_path().value());
    }
}

} // namespace

int main (int, char *[])
{
    // SDL Init
    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(SDL_DISABLE);
    TTF_Init();

    SDL_Surface *video = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE);
    SDL_Surface *screen = SDL_CreateRGBSurface(SDL_HWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);

    // Font
    int font_size = 24;
    TTF_Font *font = TTF_OpenFont("fonts/DejaVuSerif.ttf", font_size);

    if (!font)
    {
        std::cerr << "TTF_OpenFont: " << TTF_GetError() << std::endl;
        return 1;
    }

    // Stores
    auto base_dir = std::filesystem::current_path() / ".state";
    StateStore state_store(base_dir);

    // Views
    ViewStack view_stack;
    initialize_views(view_stack, state_store, font);
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

    state_store.flush();

    TTF_CloseFont(font);
    SDL_FreeSurface(screen);
    SDL_Quit();
    xmlCleanupParser();
    
    return 0;
}
