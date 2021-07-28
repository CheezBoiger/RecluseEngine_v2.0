//
#pragma once

#include "Recluse/Graphics/CommandQueue.hpp"
#include "D3D12Commons.hpp"

namespace Recluse {

class D3D12Device;

class D3D12Queue : public GraphicsQueue {
public:
    D3D12Queue(GraphicsQueueTypeFlags queueType)
        : m_queue(nullptr)
        , GraphicsQueue(queueType) { }

    ErrType initialize(D3D12Device* pDevice);
    
    ErrType submit(const QueueSubmit* payload) override;

    void destroy();

    ID3D12CommandQueue* get() const { return m_queue; }

private:
    ID3D12CommandQueue* m_queue;
};
} // Recluse