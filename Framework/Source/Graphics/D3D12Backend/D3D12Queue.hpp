//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

namespace Recluse {

class D3D12Device;

class D3D12Queue 
{
public:
    D3D12Queue(GraphicsQueueTypeFlags queueType)
        : m_queue(nullptr) { }

    ErrType initialize(D3D12Device* pDevice);
    
    //ErrType submit(const QueueSubmit* payload);

    void destroy();

    ID3D12CommandQueue* get() const { return m_queue; }

private:
    ID3D12CommandQueue* m_queue;
};
} // Recluse