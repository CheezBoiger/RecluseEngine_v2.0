//
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Types.hpp"
#include <queue>
#include <functional>
#include <map>

namespace Recluse {


class AMessage 
{
public:
    virtual ~AMessage() { }

    virtual std::string getEvent() = 0;
};

typedef std::function<void(AMessage*)> MessageReceiveFunc;

// Simple Message bus to be used for input messaging.
class MessageBus 
{
public:
    friend class Recluse::AMessage;

    MessageBus()
        : m_pMessageAllocator(nullptr)
        , m_messageMemPool(nullptr)
    { }

    ~MessageBus() {} 

    R_PUBLIC_API void initialize();
    R_PUBLIC_API void cleanUp();

    R_PUBLIC_API void addReceiver
                        (
                            const std::string& nodeName, 
                            MessageReceiveFunc receiver
                        );

    template<typename MessageType>
    void pushMessage(const MessageType& message) 
    {
        static_assert(std::is_base_of<AMessage, MessageType>::value, "Must be a base of AMessage!");
        
        MessageType* pMessage   = new (m_pMessageAllocator) MessageType();
        *pMessage               = message;
        
        m_messages.push(pMessage);
    }

    void notifyAll() 
    {
        // Notify all message receivers.
        while (!m_messages.empty()) 
        {
            for (MessageReceiveFunc func : m_messageReceivers) 
            {
                func(m_messages.front());
            }
            
            m_messages.pop();
        }
    }

    R_PUBLIC_API void notifyOne(const std::string& nodeName);

    void clearQueue() 
    {
        if (m_messages.empty())
            m_pMessageAllocator->reset();
    }

private:
    Allocator* m_pMessageAllocator;
    MemoryPool* m_messageMemPool;
    std::queue<AMessage*> m_messages;
    std::vector<MessageReceiveFunc> m_messageReceivers;
    std::map<std::string, U32> m_receiverNodeNames;
};
} // Recluse