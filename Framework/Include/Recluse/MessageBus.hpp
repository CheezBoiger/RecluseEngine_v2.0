//
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/RGUID.hpp"
#include "Recluse/Serialization/Hasher.hpp"
#include "Recluse/Threading/Threading.hpp"

#include "RecluseFramework_exports.hpp"

#include <queue>
#include <functional>
#include <map>

namespace Recluse {


// EventId is in the form of hash values.
typedef Hash64 EventId;
typedef U64 GroupId;


class EventMessage 
{
public:
    static const EventId kBadEventId = ~0;
    ~EventMessage() { }
    EventMessage(EventId eventId = kBadEventId) : m_eventId(eventId) { }

    EventId getEvent() const { return m_eventId; }

private:
    EventId m_eventId;
};



typedef std::function<ResultCode(const EventMessage&)> MessageReceiveFunc;

// Simple Message bus to be used for input messaging. This is a simple bus design,
// We might want something more efficient later on.
class MessageBus 
{
public:
    friend class    Recluse::EventMessage;
    typedef U32     Id;

    // Helper to fire an event.
    static void fireEvent(MessageBus* pBus, EventId id)
    {
        pBus->pushEvent(id);
    }

    RecluseFramework_PUBLIC_API MessageBus();

    ~MessageBus() {} 

    // Initialize the messaging bus system.
    RecluseFramework_PUBLIC_API void initialize(SizeT eventCacheSzBytes = R_MB(2ull));

    RecluseFramework_PUBLIC_API void cleanUp();

    // Add a end point receiver to the message bus.
    RecluseFramework_PUBLIC_API void addReceiver(const std::string& nodeName, MessageReceiveFunc receiver);

    // Push an event
    void pushEvent(EventId eventId) 
    {   
        ScopedLock _(m_messageQueueMutex);
        EventMessage* pMessage   = new (m_pMessageAllocator) EventMessage(eventId);
        m_messages.push(pMessage);
    }

    // Notify all message receivers of the given fired events. This is a syncronous call,
    // so unless you need to call this asyncronously, be sure to call this on a separate thread.
    void notifyAll() 
    {
        // Notify all message receivers.
        while (!m_messages.empty()) 
        {
            // Must lock the mutex and read at a time.
            ScopedLock _(m_messageQueueMutex);
            for (MessageReceiveFunc func : m_messageReceivers) 
            {
                ResultCode result = func(*m_messages.front());
                // TODO: Proper message handling.
            }
            
            m_messages.pop();
        }
    }

    // Only notify one message receiver of the fired events. This is a syncronous call,
    // so unless you need to call this asyncronously, be sure to call this on a separate thread.
    RecluseFramework_PUBLIC_API void notifyOne(const std::string& nodeName);

    // Clears the event queue. This is required after notifying, as the 
    // queue will still contain all allocated events.
    void clearQueue() 
    {
        ScopedLock _(m_messageQueueMutex);
        if (m_messages.empty())
            m_pMessageAllocator->reset();
    }

    Id getId() const { return m_id; }

private:
    MutexGuard                      m_messageQueueMutex;
    Allocator*                      m_pMessageAllocator;
    MemoryPool                      m_messageMemPool;
    std::queue<EventMessage*>       m_messages;             //< The Message queue.
    std::vector<MessageReceiveFunc> m_messageReceivers;
    std::map<std::string, U32>      m_receiverNodeNames;

    // Message bus id.
    Id                              m_id;
    
};
} // Recluse