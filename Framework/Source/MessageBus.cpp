//
#include "Recluse/MessageBus.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Memory/LinearAllocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

namespace Recluse {


MessageBus::MessageBus()
    : m_pMessageAllocator(nullptr)
    , m_messageMemPool({})
{

}


void MessageBus::initialize(SizeT eventCacheSzBytes)
{
    static U32 idCount = 0;
    // Pre-allocate a sizeable pool. Include room for our allocator!
    m_messageMemPool.preAllocate(eventCacheSzBytes + sizeof(LinearAllocator));

    // We will allocate our allocator into the pool too!
    m_pMessageAllocator = new (reinterpret_cast<void*>(m_messageMemPool.getBaseAddress())) LinearAllocator();
    m_pMessageAllocator->initialize
            (
                m_messageMemPool.getPtrAddressAt(sizeof(LinearAllocator)), 
                m_messageMemPool.getTotalSizeBytes() - sizeof(LinearAllocator)
            );

    m_id = ++idCount;
}


void MessageBus::cleanUp()
{
    // Make sure to finish all notifications before we clean up.
    notifyAll();
    if (m_pMessageAllocator) 
    {
        // For this case, we don't need to delete the allocator, as it was allocated inside the memory pool.
        m_pMessageAllocator->cleanUp();
    }

    m_messageMemPool.release();
}


void MessageBus::addReceiver(const std::string& nodeName, MessageReceiveFunc receiver)
{
    m_messageReceivers.push_back(receiver);
    m_receiverNodeNames[nodeName] = m_messageReceivers.size() - 1;
}


void MessageBus::notifyOne(const std::string& nodeName)
{
    if (m_receiverNodeNames.find(nodeName) == m_receiverNodeNames.end()) 
    {
        R_WARN(__FUNCTION__, "Unable to find the node name=%s", nodeName.c_str());
        return;
    }

    MessageReceiveFunc func =  m_messageReceivers[m_receiverNodeNames[nodeName]];

     ScopedLock _(m_messageQueueMutex);
    while (!m_messages.empty()) 
    {
        ResultCode result = func(*m_messages.front());
        // TODO: Handle result.
        m_messages.pop();    
    }

    m_pMessageAllocator->reset();
}


void MessageBus::notifyAll()
{
    R_ASSERT(m_messageMemPool.isAllocated(), "This message bus was not initialized.");
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

    m_pMessageAllocator->reset();
}
} // Recluse