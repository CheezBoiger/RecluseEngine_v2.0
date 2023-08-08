//
#include "D3D12Adapter.hpp"
#include "D3D12Instance.hpp"

#include "D3D12Device.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


std::vector<IDXGIAdapter*> D3D12Adapter::getAdapters(D3D12Instance* pContext)
{
    std::vector<IDXGIAdapter*> adapters;

    IDXGIFactory1* pFactory = pContext->get();    
    IDXGIAdapter* adapter   = NULL;

    for 
        (
            U32 i = 0; 
            pFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; 
            ++i
        ) 
    {
        adapters.push_back(adapter);
    }

    return adapters;
}


D3D12Adapter::D3D12Adapter(IDXGIAdapter* adapter)
    : m_pAdapter(adapter)
    , m_pInstance(NULL)
{
}


void D3D12Adapter::destroy()
{
    if (m_pAdapter) 
    {    
        m_pAdapter->Release();
        m_pAdapter = nullptr;
    }
}


ResultCode D3D12Adapter::getAdapterInfo(AdapterInfo* out) const 
{
    DXGI_ADAPTER_DESC desc = { };
    m_pAdapter->GetDesc(&desc);

    U32 vendorId = desc.VendorId;
    U32 deviceId = desc.DeviceId;

    out->vendorId = vendorId;   
    
    switch (desc.VendorId) 
    {
        case INTEL_VENDOR_ID: out->vendorName = "Intel Corporation"; break;
        case NVIDIA_VENDOR_ID: out->vendorName = "Nvidia Corporation"; break;
        case AMD_VENDOR_ID: out->vendorName = "Advanced Micro Devices"; break;
        case MSFT_VENDOR_ID: out->vendorName = "Microsoft"; break;
        default: out->vendorName = "Unknown"; break;
    } 

    WideCharToMultiByte(CP_UTF8, 0, desc.Description, 128, out->deviceName, 256, 0, 0);

    return RecluseResult_Ok;
}


U32 D3D12Adapter::constantBufferOffsetAlignmentBytes() const
{
    return D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
}


ResultCode D3D12Adapter::createDevice(DeviceCreateInfo& info, GraphicsDevice** ppDevice)
{
    D3D12Device* pDevice    = new D3D12Device();
    ResultCode result          = RecluseResult_Ok;
    
    result = pDevice->initialize(this, info);
    
    if (result != RecluseResult_Ok) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to create a d3d12 device!");

        pDevice->destroy();
        delete pDevice;
        
        return result;
    }

    m_devices.push_back(pDevice);

    *ppDevice = pDevice;

    return result;
}


ResultCode D3D12Adapter::destroyDevice(GraphicsDevice* pDevice)
{
    if (!pDevice) return RecluseResult_NullPtrExcept;

    for (auto& iter = m_devices.begin(); iter != m_devices.end(); ++iter) 
    {
        if ((*iter) == pDevice) 
        {   
            (*iter)->destroy();
            delete *iter;
            m_devices.erase(iter);
            
            return RecluseResult_Ok;
        }
    }

    return RecluseResult_Failed;
}
} // Recluse