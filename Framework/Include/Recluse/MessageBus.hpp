//
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/RGUID.hpp"
#include <queue>
#include <functional>
#include <map>

namespace Recluse {

typedef U64 EventId;
typedef U64 GroupId;

class EventMessage 
{
public:
    static const EventId kBadEventId = ~0;
    ~EventMessage() { }
    EventMessage(EventId eventId = kBadEventId) : m_eventId(eventId) { }

    EventId getEvent() { return m_eventId; }

private:
    EventId m_eventId;
};

typedef std::function<void(EventMessage*)> MessageReceiveFunc;

// Simple Message bus to be used for input messaging. This is a simple bus design,
// We might want something more efficient later on.
class MessageBus 
{
public:
    friend class Recluse::EventMessage;

    static void sendEvent(MessageBus* pBus, EventId id)
    {
        pBus->pushMessage(id);
    }

    R_PUBLIC_API MessageBus();

    ~MessageBus() {} 

    // Initialize the messaging bus system.
    R_PUBLIC_API void initialize(SizeT eventCacheSzBytes = R_MB(2ull));

    R_PUBLIC_API void cleanUp();

    R_PUBLIC_API void addReceiver
                        (
                            const std::string& nodeName, 
                            MessageReceiveFunc receiver
                        );

    void pushMessage(EventId eventId) 
    {   
        EventMessage* pMessage   = new (m_pMessageAllocator) EventMessage(eventId);
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
    Allocator*                      m_pMessageAllocator;
    MemoryPool                      m_messageMemPool;
    std::queue<EventMessage*>       m_messages;
    std::vector<MessageReceiveFunc> m_messageReceivers;
    std::map<std::string, U32>      m_receiverNodeNames;
};
} // Recluse