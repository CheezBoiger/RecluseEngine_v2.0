//
#include "Recluse/MessageBus.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Memory/StackAllocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

namespace Recluse {


void MessageBus::initialize()
{
    m_messageMemPool = new MemoryPool(R_1MB * 2ull);
    m_pMessageAllocator = new StackAllocator();
    m_pMessageAllocator->initialize(m_messageMemPool->getBaseAddress(), 
        m_messageMemPool->getTotalSizeBytes());
}


void MessageBus::cleanUp()
{
    if (m_pMessageAllocator) {
        m_pMessageAllocator->cleanUp();
        delete m_pMessageAllocator;
        m_pMessageAllocator = nullptr;
    }

    if (m_messageMemPool) {
        delete m_messageMemPool;
        m_messageMemPool = nullptr;
    }
}


void MessageBus::addReceiver(MessageReceiveFunc receiver)
{
    m_messageReceivers.push_back(receiver);
}
} // Recluse