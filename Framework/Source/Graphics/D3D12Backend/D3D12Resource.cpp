//
#include "D3D12Resource.hpp"
#include "D3D12Device.hpp"
#include "D3D12Allocator.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


D3D12_RESOURCE_DIMENSION getDimension(ResourceDimension dim)
{
    switch (dim) {
        case RESOURCE_DIMENSION_BUFFER: return D3D12_RESOURCE_DIMENSION_BUFFER;
        case RESOURCE_DIMENSION_1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case RESOURCE_DIMENSION_2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case RESOURCE_DIMENSION_3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default: return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
}


ErrType D3D12Resource::initialize(D3D12Device* pDevice, const GraphicsResourceDescription& desc, Bool makeCommitted)
{
    ID3D12Device* device                    = pDevice->get();
    D3D12_RESOURCE_DESC d3d12desc           = { };
    D3D12_RESOURCE_STATES state             = D3D12_RESOURCE_STATE_COMMON;
    D3D12_CLEAR_VALUE optimizedClearValue   = { };
    D3D12Allocator* pAllocator              = nullptr;
    HRESULT sResult                         = S_OK;

    d3d12desc.Dimension         = getDimension(desc.dimension);
    d3d12desc.DepthOrArraySize  = desc.depth;
    d3d12desc.Width             = desc.width;
    d3d12desc.Height            = desc.height;
    d3d12desc.MipLevels         = desc.mipLevels;
    d3d12desc.Format            = Dxgi::getNativeFormat(desc.format);

    if (makeCommitted == true) {
        D3D12_HEAP_PROPERTIES heapProps = { };
        D3D12_HEAP_FLAGS flags          = D3D12_HEAP_FLAG_NONE;

        sResult = device->CreateCommittedResource(&heapProps, flags, &d3d12desc, state, 
            &optimizedClearValue, __uuidof(ID3D12Resource), (void**)&m_pResource);

    } else {

        R_ASSERT_MSG(pAllocator, "Allocator for d3d12 resources is NULL!");

        U64 szBytes     = 0;
        U64 alignment   = 0;
        ErrType result  = pAllocator->allocate(&m_pResource, szBytes, alignment);

        if (result != REC_RESULT_OK) {
            R_ERR(R_CHANNEL_D3D12, "Failed to allocate a D3D12 resource!");
        } else {
            
        }
    }

    if (FAILED(sResult)) {
        R_ERR(R_CHANNEL_D3D12, "Failed to create resource!");

        return REC_RESULT_FAILED;
    }
    
    m_isCommitted = makeCommitted;
    return REC_RESULT_OK;
}


ErrType D3D12Resource::destroy()
{
    if (m_pResource) {

        m_pResource->Release();
        m_pResource = nullptr;

    }

    return REC_RESULT_OK;
}
} // Recluse