#ifndef MESSAGE_BROKER_H_
#define MESSAGE_BROKER_H_

#include <functional>
#include <memory>
#include <string>

struct Message
{
    std::string type;
    virtual ~Message() = default;
    Message(std::string type);
};

struct MessageBrokerState;

// Provide a thread safe way for worker threads to communicate with the main thread.
class MessageBroker
{
    std::unique_ptr<MessageBrokerState> state;
public:

    MessageBroker();
    virtual ~MessageBroker();

    uint32_t register_address(std::function<void(std::shared_ptr<Message>)> callback);
    void unregister_address(uint32_t address);

    // Send a message to given address. May be called from any thread.
    void send_message(uint32_t address, std::shared_ptr<Message> message);

    // Return true if at least one message was delivered. Must be called from main thread.
    bool distribute_messages();
};

#endif
