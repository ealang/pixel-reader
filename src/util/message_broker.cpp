#include "./message_broker.h"

#include <deque>
#include <mutex>
#include <unordered_map>

Message::Message(std::string type)
    : type(std::move(type))
{
}


struct MessageBrokerState
{
    uint32_t next_address = 1;

    std::mutex lock;

    std::unordered_map<uint32_t, std::function<void(std::shared_ptr<Message>)>> callbacks;
    std::deque<std::pair<uint32_t, std::shared_ptr<Message>>> messages;
};

MessageBroker::MessageBroker()
    : state(std::make_unique<MessageBrokerState>())
{
}

MessageBroker::~MessageBroker()
{
}

uint32_t MessageBroker::register_address(std::function<void(std::shared_ptr<Message>)> callback)
{
    state->callbacks[state->next_address] = callback;
    return state->next_address++;
}

void MessageBroker::unregister_address(uint32_t address)
{
    state->callbacks.erase(address);
}

void MessageBroker::send_message(uint32_t address, std::shared_ptr<Message> message)
{
    std::lock_guard<std::mutex> guard(state->lock);
    state->messages.emplace_back(address, message);
}

bool MessageBroker::distribute_messages()
{
    bool sent_message = false;
    for (auto [address, message] : state->messages)
    {
        auto callback = state->callbacks.find(address);
        if (callback != state->callbacks.end())
        {
            sent_message = true;
            callback->second(message);
        }
    }
    state->messages.clear();

    return sent_message;
}
