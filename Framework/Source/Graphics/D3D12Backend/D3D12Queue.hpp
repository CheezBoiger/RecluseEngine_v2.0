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
        : m_queue(nullptr)
        , pFence(nullptr)
        , pEvent(nullptr) { }

    ResultCode initialize(D3D12Device* pDevice);
    
    //ErrType submit(const QueueSubmit* payload);

    void destroy();

    ID3D12CommandQueue* get() const { return m_queue; }

    // Returns the expected new fence value.
    U64                    waitForGpu(U64 currentFenceValue);

    ID3D12Fence* getFence() const { return pFence; }
    HANDLE getEvent() const { return pEvent; }

private:
    ID3D12CommandQueue*         m_queue;
    ID3D12Fence*                pFence;
    HANDLE                      pEvent;
};
} // Recluse