//
#include "D3D12Queue.hpp"
#include "D3D12Device.hpp"

#include "Recluse/Messaging.hpp"


namespace Recluse {


ErrType D3D12Queue::initialize(D3D12Device* pDevice)
{
    R_DEBUG(R_CHANNEL_D3D12, "Creating command queue...");

    D3D12_COMMAND_LIST_TYPE type        = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ID3D12Device* device                = pDevice->get();
    HRESULT result                      = S_OK;
    D3D12_COMMAND_QUEUE_DESC desc       = { };
    desc.NodeMask                       = 0;
    desc.Priority                       = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    GraphicsQueueTypeFlags queueType    = getType();

    if (queueType & QUEUE_TYPE_COMPUTE) type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
    if (queueType & QUEUE_TYPE_COPY) type = D3D12_COMMAND_LIST_TYPE_COPY;
    if (queueType & QUEUE_TYPE_PRESENT) type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (queueType & QUEUE_TYPE_GRAPHICS) type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    
    desc.Type                           = type;
    
    result = device->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), (void**)&m_queue);

    if (result != S_OK) {
    
        R_ERR(R_CHANNEL_D3D12, "Failed to create command queue!");

        return REC_RESULT_FAILED;        
    }    

    return REC_RESULT_OK;
}


void D3D12Queue::destroy()
{
    if (m_queue) {
    
        R_DEBUG(R_CHANNEL_D3D12, "Releasing D3D12 queue...");
        
        m_queue->Release();
        m_queue = nullptr;
        
    }
}


ErrType D3D12Queue::submit(const QueueSubmit* payload) 
{
    // TODO: Implement d3d12 command lists.
   // m_queue->ExecuteCommandLists(payload->numCommandLists, );
    return REC_RESULT_NOT_IMPLEMENTED;
}
} // Recluse