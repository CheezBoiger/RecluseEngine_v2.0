//
#include "D3D12Resource.hpp"
#include "D3D12Device.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType D3D12Resource::initialize(D3D12Device* pDevice, const GraphicsResourceDescription& desc, bool makeCommitted)
{
    ID3D12Device* device                    = pDevice->get();
    D3D12_RESOURCE_DESC d3d12desc           = { };
    D3D12_RESOURCE_STATES state             = D3D12_RESOURCE_STATE_COMMON;
    D3D12_CLEAR_VALUE optimizedClearValue   = { };
    HRESULT sResult                         = S_OK;

    if (makeCommitted == true) {
        D3D12_HEAP_PROPERTIES heapProps = { };
        D3D12_HEAP_FLAGS flags          = D3D12_HEAP_FLAG_NONE;

        sResult = device->CreateCommittedResource(&heapProps, flags, &d3d12desc, state, 
            &optimizedClearValue, __uuidof(ID3D12Resource), (void**)&m_pResource);
    }

    if (FAILED(sResult)) {
        R_ERR(R_CHANNEL_D3D12, "Failed to create resource!");

        return REC_RESULT_FAILED;
    }
    
    return REC_RESULT_OK;
}
} // Recluse