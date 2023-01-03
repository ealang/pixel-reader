#ifndef TOKEN_VIEW_H_
#define TOKEN_VIEW_H_

#include "reader/view.h"
#include "doc_api/doc_addr.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

struct DocReader;
struct SystemStyling;
struct TokenViewState;
struct TokenViewStyling;

class TokenView: public View
{
    std::unique_ptr<TokenViewState> state;

    void scroll(int num_lines);

public:
    TokenView(
        std::shared_ptr<DocReader> reader,
        DocAddr address,
        SystemStyling &sys_styling,
        TokenViewStyling &token_view_styling
    );
    virtual ~TokenView();

    bool render(SDL_Surface *dest_surface, bool force_render) override;
    bool is_done() override;
    void on_keypress(SDLKey key) override;
    void on_keyheld(SDLKey key, uint32_t held_time_ms) override;

    DocAddr get_address() const;
    void seek_to_address(DocAddr address);

    void set_title(const std::string &title);
    void set_title_progress(int percent);

    void set_on_scroll(std::function<void(DocAddr)> callback);
};

#endif
