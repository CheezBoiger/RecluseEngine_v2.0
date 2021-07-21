//
#include "D3D12Device.hpp"
#include "D3D12Adapter.hpp"

#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"

namespace Recluse {


ErrType D3D12Device::initialize(D3D12Adapter* adapter, const DeviceCreateInfo& info)
{
    R_DEBUG(R_CHANNEL_D3D12, "Creating D3D12 device...");

    IDXGIAdapter* pAdapter  = adapter->get();
    HRESULT result          = S_OK;

    result = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), (void**)&m_device);

    if (result != S_OK) {
    
        return REC_RESULT_FAILED;
    
    }

    m_pAdapter = adapter;

    R_DEBUG(R_CHANNEL_D3D12, "Successfully created D3D12 device!");
    
    return REC_RESULT_OK;
}


void D3D12Device::destroy()
{
    if (m_device) {
    
        R_DEBUG(R_CHANNEL_D3D12, "Destroying D3D12 device...");

        m_device->Release();
    
    }
}
} // Recluse