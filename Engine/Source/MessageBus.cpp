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


void MessageBus::addReceiver(const std::string& nodeName, MessageReceiveFunc receiver)
{
    m_messageReceivers.push_back(receiver);
    m_receiverNodeNames[nodeName] = m_messageReceivers.size() - 1;
}


void MessageBus::notifyOne(const std::string& nodeName)
{
    if (m_receiverNodeNames.find(nodeName) == m_receiverNodeNames.end()) {
        R_ERR(__FUNCTION__, "Unable to find the node name=%s", nodeName.c_str());
        return;
    }

    MessageReceiveFunc func =  m_messageReceivers[m_receiverNodeNames[nodeName]];

    while (!m_messages.empty()) {
        func(m_messages.front());
        m_messages.pop();    
    }
}
} // Recluse