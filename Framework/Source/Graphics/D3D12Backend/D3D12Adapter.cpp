//
#include "D3D12Adapter.hpp"
#include "D3D12Context.hpp"

namespace Recluse {


std::vector<IDXGIAdapter*> D3D12Adapter::getAdapters(D3D12Context* pContext)
{
    std::vector<IDXGIAdapter*> adapters;

    IDXGIFactory1* pFactory = pContext->get();    
    IDXGIAdapter* adapter   = NULL;

    for (U32 i = 0; pFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        adapters.push_back(adapter);
    }

    return adapters;
}


D3D12Adapter::D3D12Adapter(IDXGIAdapter* adapter)
    : m_pAdapter(adapter)
    , m_pContext(NULL)
{
}


ErrType D3D12Adapter::getAdapterInfo(AdapterInfo* out) const 
{
    DXGI_ADAPTER_DESC desc = { };
    m_pAdapter->GetDesc(&desc);

    U32 vendorId = desc.VendorId;
    U32 deviceId = desc.DeviceId;

    out->vendorId = vendorId;   
    
    switch (desc.VendorId) {
        case INTEL_VENDOR_ID: out->vendor = VENDOR_INTEL; break;
        case NVIDIA_VENDOR_ID: out->vendor = VENDOR_NVIDIA; break;
        case AMD_VENDOR_ID: out->vendor = VENDOR_AMD; break;
        default: out->vendor = VENDOR_UNKNOWN; break;
    } 

    WideCharToMultiByte(CP_UTF8, 0, desc.Description, 128, out->deviceName, 256, 0, 0);

    return REC_RESULT_OK;
}
} // Recluse