#include "./open_reader_tmp_view.h"

#include "filetypes/open_doc.h"
#include "reader/view_stack.h"
#include "util/message_broker.h"

#include <iostream>
#include <stdexcept>
#include <unistd.h>

namespace
{

constexpr const char *MSG_LOAD_FAILURE = "load_failure";
constexpr const char *MSG_LOAD_SUCCESS = "load_success";

struct FileLoadedMessage: public Message
{
    std::shared_ptr<DocReader> reader;
    FileLoadedMessage(std::shared_ptr<DocReader> reader) : Message(MSG_LOAD_SUCCESS), reader(reader)
    {}
};

void load_file(MessageBroker *broker, uint32_t return_address, std::unique_ptr<ReaderArguments> args)
{
    std::cout << "Worker starting with path: " << args->path << std::endl;
    sleep(1);

    std::shared_ptr<DocReader> reader = create_doc_reader(args->path);
    if (!reader || !reader->open(args->reader_cache))
    {
        std::cerr << "Failed to open " << args->path << std::endl;
        broker->send_message(return_address, std::make_shared<Message>(MSG_LOAD_FAILURE));
        //reader_cache->report_load_complete();
        //view_stack.push(std::make_shared<PopupView>("Error opening", true, SYSTEM_FONT, sys_styling));
        return;
    }

    broker->send_message(return_address, std::make_shared<FileLoadedMessage>(reader));
    /*
    reader_cache->report_load_complete();

    state_store.set_current_book_path(path);

    auto book_id = reader->get_id();

    RangeDB highlights(state_store.get_book_highlights(book_id));
    auto reader_view = std::make_shared<ReaderView>(
        path,
        reader,
        state_store.get_book_address(book_id).value_or(make_address()),
        highlights,
        sys_styling,
        token_view_styling,
        view_stack
    );

    reader_view->set_on_change_address([&state_store, book_id](DocAddr addr) {
        state_store.set_book_address(book_id, addr);
    });

    reader_view->set_on_highlight([&state_store, book_id, &view_stack, &sys_styling, &notes_export_queue, path](const RangeDB &highlights) {
        state_store.set_book_highlights(book_id, highlights.get_ranges());
        notes_export_queue.add_job(path);
    });
    reader_view->set_on_quit_requested([&state_store]() {
        state_store.remove_current_book_path();
    });
    */
}

}

OpenReaderTmpView::OpenReaderTmpView(ViewStack &view_stack, MessageBroker &msg_broker, std::unique_ptr<ReaderArguments> reader_args)
    : view_stack(view_stack)
    , msg_broker(msg_broker)
    , my_address(msg_broker.register_address([this](std::shared_ptr<Message> msg) {
        if (msg->type == MSG_LOAD_FAILURE)
        {
            on_file_load_failure();
        }
        else if (msg->type == MSG_LOAD_SUCCESS)
        {
            on_file_load_success(
                static_cast<FileLoadedMessage*>(msg.get())->reader
            );
        }
        else
        {
            throw std::runtime_error("Invalid message");
        }
      }))
    , load_thread(load_file, &msg_broker, my_address, std::move(reader_args))
{
}

OpenReaderTmpView::~OpenReaderTmpView()
{
    load_thread.join();
    msg_broker.unregister_address(my_address);
}

void OpenReaderTmpView::on_file_load_success(std::shared_ptr<DocReader> )
{
    std::cout << "Load success callback\n";
    done = true;
}

void OpenReaderTmpView::on_file_load_failure()
{
    std::cout << "Load failed callback\n";
    done = true;
}

bool OpenReaderTmpView::render(SDL_Surface *, bool )
{
    return false;
}

bool OpenReaderTmpView::is_done()
{
    return done;
}

bool OpenReaderTmpView::is_modal()
{
    // Render view behind it
    return false;
}

void OpenReaderTmpView::on_keypress(SDLKey)
{
}
