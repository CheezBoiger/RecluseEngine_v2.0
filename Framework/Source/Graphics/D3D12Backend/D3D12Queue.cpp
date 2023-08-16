//
#include "D3D12Queue.hpp"
#include "D3D12Device.hpp"

#include "Recluse/Messaging.hpp"


namespace Recluse {


ResultCode D3D12Queue::initialize(D3D12Device* pDevice)
{
    R_DEBUG(R_CHANNEL_D3D12, "Creating command queue...");

    D3D12_COMMAND_LIST_TYPE type        = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ID3D12Device* device                = pDevice->get();
    HRESULT result                      = S_OK;
    D3D12_COMMAND_QUEUE_DESC desc       = { };
    desc.NodeMask                       = 0;
    desc.Priority                       = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    GraphicsQueueTypeFlags queueType    = QUEUE_TYPE_PRESENT;

    if (queueType & QUEUE_TYPE_COMPUTE) type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
    if (queueType & QUEUE_TYPE_COPY) type = D3D12_COMMAND_LIST_TYPE_COPY;
    if (queueType & QUEUE_TYPE_PRESENT) type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (queueType & QUEUE_TYPE_GRAPHICS) type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    
    desc.Type                           = type;
    
    result = device->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), (void**)&m_queue);

    if (result != S_OK) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create command queue!");

        return RecluseResult_Failed;        
    }    

    pDevice->get()->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&pFence);
    pEvent = CreateEvent(nullptr, false, false, nullptr);

    return RecluseResult_Ok;
}


void D3D12Queue::destroy()
{
    if (m_queue) 
    {
        R_DEBUG(R_CHANNEL_D3D12, "Releasing D3D12 queue...");
        
        m_queue->Release();
        m_queue = nullptr;    
    }

    if (pFence)
    {
        pFence->Release();
        pFence = nullptr;
    }

    if (pEvent)
    {
        CloseHandle(pEvent);
        pEvent = nullptr;
    }
}


U64 D3D12Queue::waitForGpu(U64 currentFenceValue)
{ 
    const U64 newFenceValue = currentFenceValue;
    m_queue->Signal(pFence, newFenceValue);
    pFence->SetEventOnCompletion(newFenceValue, pEvent);
    WaitForSingleObject(pEvent, INFINITE);
    return newFenceValue + 1;
}

/*
ErrType D3D12Queue::submit(const QueueSubmit* payload) 
{
    // TODO: Implement d3d12 command lists.
   // m_queue->ExecuteCommandLists(payload->numCommandLists, );
    return REC_RESULT_NOT_IMPLEMENTED;
}
*/
} // Recluse