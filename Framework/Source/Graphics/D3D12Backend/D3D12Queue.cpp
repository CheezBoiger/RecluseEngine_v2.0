//
#include "D3D12Queue.hpp"
#include "D3D12Device.hpp"
#include "D3D12Resource.hpp"

#include "Recluse/Messaging.hpp"
#include "Recluse/Math/MathCommons.hpp"

namespace Recluse {
namespace D3D12 {

ResultCode D3D12Queue::initialize(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE newType)
{
    R_DEBUG(R_CHANNEL_D3D12, "Creating command queue...");

    D3D12_COMMAND_LIST_TYPE type        = newType;
    ID3D12Device* device                = pDevice;
    HRESULT result                      = S_OK;
    D3D12_COMMAND_QUEUE_DESC desc       = { };
    desc.NodeMask                       = 0;
    desc.Priority                       = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    
    desc.Type                           = type;
    
    result = device->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), (void**)&m_queue);

    if (result != S_OK) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create command queue!");

        return RecluseResult_Failed;        
    }    

    pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&pFence);
    pEvent = CreateEvent(nullptr, false, false, nullptr);

    pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&m_allocator);

    return RecluseResult_Ok;
}


void D3D12Queue::destroy()
{
    if (m_allocator)
    {
        m_allocator->Release();
        m_allocator = nullptr;
    }

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


ID3D12GraphicsCommandList* D3D12Queue::createOneTimeCommandList(U32 nodeMask, ID3D12Device* pDevice)
{
    ID3D12GraphicsCommandList* pList = nullptr;
    HRESULT result = pDevice->CreateCommandList(nodeMask, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocator, nullptr, __uuidof(ID3D12GraphicsCommandList), (void**)&pList);
    
    if (FAILED(result))
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create a one time command list.");
        return nullptr;
    }
    return pList;
}


ResultCode D3D12Queue::endAndSubmitOneTimeCommandList(ID3D12GraphicsCommandList* pList)
{
    ID3D12CommandList* commandList[] = { pList };
    m_queue->ExecuteCommandLists(1, commandList);
    U64 currentFenceValue = pFence->GetCompletedValue() + 1;
    waitForGpu(currentFenceValue);
    
    pList->Release();
    m_allocator->Reset();
    return RecluseResult_Ok;
}


void D3D12Queue::copyResource(D3D12Resource* dst, D3D12Resource* src)
{
    ID3D12Device* pDevice = nullptr;
    m_queue->GetDevice(__uuidof(ID3D12Device), (void**)&pDevice);
    ID3D12GraphicsCommandList* pList = createOneTimeCommandList(0, pDevice);
    generateCopyResourceCommand(pList, dst, src);
    pList->Close();
    endAndSubmitOneTimeCommandList(pList);
    pDevice->Release();
}

R_INTERNAL
Bool isTexture(const D3D12_RESOURCE_DIMENSION dim)
{
    return (dim != D3D12_RESOURCE_DIMENSION_BUFFER  && dim != D3D12_RESOURCE_DIMENSION_UNKNOWN);
}


void D3D12Queue::generateCopyResourceCommand(ID3D12GraphicsCommandList* pList, D3D12Resource* dst, D3D12Resource* src)
{
    ID3D12Resource* pDst = dst->get();
    ID3D12Resource* pSrc = src->get();
    D3D12_RESOURCE_DESC dstDesc = pDst->GetDesc();
    D3D12_RESOURCE_DESC srcDesc = pSrc->GetDesc();
    ID3D12Device* pDevice = nullptr;
    pList->GetDevice(__uuidof(ID3D12Device), (void**)&pDevice);

    if (dstDesc.Dimension == srcDesc.Dimension 
            && dstDesc.MipLevels == srcDesc.MipLevels 
            && dstDesc.DepthOrArraySize == srcDesc.DepthOrArraySize
            && dstDesc.Width == srcDesc.Width
            && dstDesc.Height == srcDesc.Height
            && dstDesc.Format == srcDesc.Format)
    {
        pList->CopyResource(dst->get(), src->get());
    }
    else
    {
        if (isTexture(dstDesc.Dimension))
        {
            U32 subresourceCount = dst->getTotalSubResources();
            U32 srcSubresourceCount = src->getTotalSubResources();
            if (isTexture(srcDesc.Dimension))
            {
                std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> srcFootprint(srcSubresourceCount);
                std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> dstFootprint(subresourceCount);
                pDevice->GetCopyableFootprints(&srcDesc, 0, srcSubresourceCount, 0, srcFootprint.data(), nullptr, nullptr, nullptr);
                pDevice->GetCopyableFootprints(&dstDesc, 0, subresourceCount, 0, dstFootprint.data(), nullptr, nullptr, nullptr);
                for (U32 subresource = 0; subresource < srcSubresourceCount; ++subresource)
                {
                    D3D12_TEXTURE_COPY_LOCATION srcLocation = { };
                    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                    srcLocation.pResource = pSrc;
                    srcLocation.SubresourceIndex = subresource;
                    for (U32 dstSubresource = 0; dstSubresource < subresourceCount; ++dstSubresource)
                    {
                        D3D12_TEXTURE_COPY_LOCATION dstLocation = { };
                        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                        dstLocation.SubresourceIndex = dstSubresource;
                        dstLocation.pResource = pDst;

                        D3D12_BOX srcBox = { };
                        srcBox.left = 0;
                        srcBox.top = 0;
                        srcBox.right = Math::minimum(srcFootprint[subresource].Footprint.Width, dstFootprint[dstSubresource].Footprint.Width);
                        srcBox.bottom = Math::minimum(srcFootprint[subresource].Footprint.Height, dstFootprint[dstSubresource].Footprint.Height);
                        srcBox.front = 0;
                        srcBox.back = 1;
                        pList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &srcBox);
                    }
                }
            }
            else
            {
                std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprint(subresourceCount);
                pDevice->GetCopyableFootprints(&dstDesc, 0, subresourceCount, 0, footprint.data(), nullptr, nullptr, nullptr); 
                U32 offsetBytes = 0;
                U32 rowPitch = footprint[0].Footprint.Width * Dxgi::getNativeFormatSize(footprint[0].Footprint.Format);
                for (U32 subresource = 0; subresource < subresourceCount; ++subresource)
                {
                    D3D12_TEXTURE_COPY_LOCATION dstLocation = { };
                    D3D12_TEXTURE_COPY_LOCATION srcLocation = { };
                    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                    dstLocation.SubresourceIndex = subresource;
                    dstLocation.pResource = pDst;

                    // For the source, since it is a buffer, we need to override the rowPitch, since we are based on the size of the total width!
                    D3D12_PLACED_SUBRESOURCE_FOOTPRINT fp = { };
                    fp.Offset = offsetBytes;
                    fp.Footprint.Depth = footprint[subresource].Footprint.Depth;
                    fp.Footprint.Format = footprint[subresource].Footprint.Format;
                    fp.Footprint.Height = footprint[subresource].Footprint.Height;
                    fp.Footprint.Width = footprint[subresource].Footprint.Width;
                    fp.Footprint.RowPitch = rowPitch;
                    offsetBytes += fp.Footprint.Height * rowPitch;
                    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                    srcLocation.pResource = pSrc;
                    srcLocation.PlacedFootprint = fp;//footprint[subresource];
                    pList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
                }
            }
        }
        else
        {
            if (isTexture(srcDesc.Dimension))
            {
            }
        }
    }
    // We must release the reference of device, otherwise we have a leak.
    pDevice->Release();
}


void D3D12Queue::generateCopyBufferRegionsCommand(ID3D12GraphicsCommandList* pList, D3D12Resource* dst, D3D12Resource* src, const CopyBufferRegion* pRegions, U32 numRegions)
{
    ID3D12Resource* pDst = dst->get();
    ID3D12Resource* pSrc = src->get();
    D3D12_RESOURCE_DESC desc = pDst->GetDesc();
    for (U32 i = 0; i < numRegions; ++i)
    {
        pList->CopyBufferRegion(pDst, pRegions[i].dstOffsetBytes, pSrc, pRegions[i].srcOffsetBytes, pRegions[i].szBytes);
    }
}


void D3D12Queue::copyBufferRegions(D3D12Resource* dst, D3D12Resource* src, const CopyBufferRegion* regions, U32 numRegions)
{
    ID3D12Device* pDevice = nullptr;
    m_queue->GetDevice(__uuidof(ID3D12Device), (void**)&pDevice);
    ID3D12GraphicsCommandList* pList = createOneTimeCommandList(0, pDevice);
    generateCopyBufferRegionsCommand(pList, dst, src, regions, numRegions);
    pList->Close();
    endAndSubmitOneTimeCommandList(pList);
    pDevice->Release();
}
} // D3D12
} // Recluse