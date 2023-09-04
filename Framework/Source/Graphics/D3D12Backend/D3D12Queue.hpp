//
#pragma once

#include "D3D12Commons.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

namespace Recluse {

class D3D12Device;
class D3D12Resource;

class D3D12Queue 
{
public:
    static void generateCopyResourceCommand(ID3D12GraphicsCommandList* pList, D3D12Resource* dst, D3D12Resource* src);
    static void generateCopyBufferRegionsCommand(ID3D12GraphicsCommandList* pList, D3D12Resource* dst, D3D12Resource* src, const CopyBufferRegion* pRegions, U32 numRegions);

    D3D12Queue()
        : m_queue(nullptr)
        , pFence(nullptr)
        , pEvent(nullptr) { }

    ResultCode initialize(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type);
    
    //ErrType submit(const QueueSubmit* payload);

    void destroy();

    ID3D12CommandQueue*     get() const { return m_queue; }

    // Returns the expected new fence value.
    U64                     waitForGpu(U64 currentFenceValue);
    void                    copyResource(D3D12Resource* dst, D3D12Resource* src);
    void                    copyBufferRegions(D3D12Resource* dst, D3D12Resource* src, const CopyBufferRegion* regions, U32 numRegions);

    ID3D12Fence* getFence() const { return pFence; }
    HANDLE getEvent() const { return pEvent; }

    ID3D12GraphicsCommandList*  createOneTimeCommandList(U32 nodeMask, ID3D12Device* pDevice);
    ResultCode                  endAndSubmitOneTimeCommandList(ID3D12GraphicsCommandList* pList);
private:
    ID3D12CommandAllocator*     m_allocator;
    ID3D12CommandQueue*         m_queue;
    ID3D12Fence*                pFence;
    HANDLE                      pEvent;
};
} // Recluse