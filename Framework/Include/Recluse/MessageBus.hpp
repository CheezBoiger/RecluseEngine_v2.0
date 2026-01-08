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


// Event message used to handle basic events to send to decoupled modules.
// Can be inherited to create custom event messages for certain modules, but 
// it is recommended send as basic a message as possible, to avoid potential 
// compile time increases, and overall binary size.
class EventMessage 
{
public:
    static const EventId kBadEventId = ~0;
    virtual ~EventMessage() { }
    EventMessage(EventId eventId = kBadEventId) : m_eventId(eventId) { }

    EventId getEvent() const { return m_eventId; }

    // Cast an event message to a specific message that was created,
    // that is inherited by EventMessage!
    template<typename EventType>
    static const EventType& castTo(const EventMessage& message)
    {
        static_assert(std::is_base_of<EventMessage, EventType>::value);
        return dynamic_cast<const EventType&>(message);
    }

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

    // Helper to send an event.
    template<typename EventClass = EventMessage, typename... Args>
    static void sendEvent(MessageBus* pBus, EventId id, Args... args)
    {
        pBus->pushEvent<EventClass>(id, args...);
    }

    RecluseFramework_PUBLIC_API MessageBus();

    ~MessageBus() {} 

    // Initialize the messaging bus system.
    RecluseFramework_PUBLIC_API void initialize(SizeT eventCacheSzBytes = R_KB(64ull));

    RecluseFramework_PUBLIC_API void cleanUp();

    // Add a end point receiver to the message bus.
    RecluseFramework_PUBLIC_API void addReceiver(const std::string& nodeName, MessageReceiveFunc receiver);

    // Push an event
    template<typename EventClass = EventMessage, typename... Args>
    void pushEvent(EventId eventId, Args... args) 
    {   
        ScopedLock _(m_messageQueueMutex);
        EventMessage* pMessage   = new (m_pMessageAllocator) EventClass(eventId, args...);
        m_messages.push(pMessage);
    }

    // Notify all message receivers of the given fired events. This is a syncronous call,
    // so unless you need to call this asyncronously, be sure to call this on a separate thread.
    RecluseFramework_PUBLIC_API void notifyAll();

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