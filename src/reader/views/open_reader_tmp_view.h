#ifndef OPEN_READER_TMP_VIEW_H_
#define OPEN_READER_TMP_VIEW_H_

#include "reader/view.h"

#include <filesystem>
#include <memory>
#include <thread>

struct DocReader;
struct MessageBroker;
struct ReaderDataCache;
struct SystemStyling;
struct ViewStack;

struct ReaderArguments
{
    std::filesystem::path path;
    SystemStyling &sys_styling;
    std::shared_ptr<ReaderDataCache> reader_cache;

    ReaderArguments(
        std::filesystem::path path,
        SystemStyling &sys_styling,
        std::shared_ptr<ReaderDataCache> reader_cache
    ) : path(path), sys_styling(sys_styling), reader_cache(reader_cache)
    {
    }
};

// Transitional view to open a file
class OpenReaderTmpView: public View
{
    ViewStack &view_stack;
    MessageBroker &msg_broker;
    uint32_t my_address;
    std::thread load_thread;

    bool done = false;

    void on_file_load_success(std::shared_ptr<DocReader> reader);
    void on_file_load_failure();

public:
    OpenReaderTmpView(ViewStack &view_stack, MessageBroker &msg_broker, std::unique_ptr<ReaderArguments> reader_args);
    virtual ~OpenReaderTmpView();

    bool render(SDL_Surface *dest_surface, bool force_render) override;
    bool is_done() override;
    bool is_modal() override;
    void on_keypress(SDLKey key) override;
};

#endif
