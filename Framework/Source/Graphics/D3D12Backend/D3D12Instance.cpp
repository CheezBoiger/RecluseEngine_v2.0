//
#include "D3D12Instance.hpp"
#include "D3D12Adapter.hpp"

#include "Recluse/Messaging.hpp"
#include <dxgi1_5.h>

namespace Recluse {


void D3D12Instance::queryGraphicsAdapters()
{
    std::vector<IDXGIAdapter*> adapters = D3D12Adapter::getAdapters(this);
    m_graphicsAdapters.resize(adapters.size());

    if (adapters.size() > 1) 
    { 
        R_DEBUG(R_CHANNEL_D3D12, "There are %llu D3D12 devices.", adapters.size());
    } 
    else if (adapters.size() == 1)
    {
        R_DEBUG(R_CHANNEL_D3D12, "There is 1 D3D12 device.");
    }

    for (U32 i = 0; i < m_graphicsAdapters.size(); ++i) 
    {
        D3D12Adapter* pAdapter  = new D3D12Adapter(adapters[i]);
        pAdapter->m_pInstance   = this;
        m_graphicsAdapters[i]   = pAdapter;
    }
}


void D3D12Instance::freeGraphicsAdapters()
{
    for (U32 i = 0; i < m_graphicsAdapters.size(); ++i) 
    {
        D3D12Adapter* pAdapter = static_cast<D3D12Adapter*>(m_graphicsAdapters[i]);
        pAdapter->destroy();
        delete pAdapter;
    }

    m_graphicsAdapters.clear();
}


ResultCode D3D12Instance::onInitialize(const ApplicationInfo& appInfo, LayerFeatureFlags flags)
{
    R_DEBUG(R_CHANNEL_D3D12, "Initializing D3D12 context...");
    HRESULT result = S_OK;

    if (flags & LayerFeatureFlag_DebugValidation) 
    {
        Bool enableGpuValidation = (flags & LayerFeatureFlag_GpuDebugValidation) ? true : false;
        enableDebugValidation(enableGpuValidation);
    }    

    result = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)&m_pFactory);

    if (result != S_OK) 
    {
        return RecluseResult_Failed;
    }

    return RecluseResult_Ok;
}


void D3D12Instance::onDestroy()
{
    R_DEBUG(R_CHANNEL_D3D12, "Destroying D3D12 context...");

    if (m_pFactory) 
    {
        m_pFactory->Release();
        m_pFactory = nullptr;
    }

    R_DEBUG(R_CHANNEL_D3D12, "Successfully destroyed context!");
}


void D3D12Instance::enableDebugValidation(Bool enableGpuValidation)
{
    R_DEBUG(R_CHANNEL_D3D12, "Enabling Debug validation...");
#if !defined(D3D12_IGNORE_SDK_LAYERS)

    ID3D12Debug* spDebugController0     = nullptr;
    HRESULT result                      = S_OK;

    result = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&spDebugController0);

    if (FAILED(result)) 
    {
        R_ERROR(R_CHANNEL_D3D12, "Failed to enable gpu validation!");    
    }

    spDebugController0->EnableDebugLayer();

    if (enableGpuValidation)
    {
        R_DEBUG(R_CHANNEL_D3D12, "Enabling Gpu Debug Validation..."); 
        ID3D12Debug1* spDebugController1    = nullptr;
        spDebugController0->QueryInterface<ID3D12Debug1>(&spDebugController1);
        spDebugController1->SetEnableGPUBasedValidation(true);
        spDebugController1->Release();
    }
    spDebugController0->Release();

#else
    R_WARN(R_CHANNEL_D3D12, "Can not enable GPU based validation for d3d12 device.");
#endif
}


Bool D3D12Instance::hasTearingSupport() const
{
    IDXGIFactory5* factory5;
    HRESULT result = m_pFactory->QueryInterface<IDXGIFactory5>(&factory5);
    if (SUCCEEDED(result))
    {
        BOOL allowTearing = false;
        result = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        factory5->Release();
        return SUCCEEDED(result) && allowTearing;
    }
    return false;
}
} // Recluse