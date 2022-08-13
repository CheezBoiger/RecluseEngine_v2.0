//
#include "D3D12Instance.hpp"
#include "D3D12Adapter.hpp"

#include "Recluse/Messaging.hpp"


namespace Recluse {


void D3D12Instance::queryGraphicsAdapters()
{
    std::vector<IDXGIAdapter*> adapters = D3D12Adapter::getAdapters(this);
    m_graphicsAdapters.resize(adapters.size());

    if (adapters.size() > 1) 
    { 
        R_DEBUG(R_CHANNEL_D3D12, "There are %llu D3D12 devices.", adapters.size());
    } 
    else 
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


ErrType D3D12Instance::onInitialize(const ApplicationInfo& appInfo, EnableLayerFlags flags)
{
    R_DEBUG(R_CHANNEL_D3D12, "Initializing D3D12 context...");
    HRESULT result = S_OK;

    if (flags & LAYER_FEATURE_DEBUG_VALIDATION_BIT) 
    {
        enableDebugValidation();
    }    

    result = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)&m_pFactory);

    if (result != S_OK) 
    {
        return R_RESULT_FAILED;
    }

    return R_RESULT_OK;
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


void D3D12Instance::enableDebugValidation()
{
    R_DEBUG(R_CHANNEL_D3D12, "Enabling Debug and GPU validation...");
#if !defined(D3D12_IGNORE_SDK_LAYERS)

    ID3D12Debug* spDebugController0     = nullptr;
    ID3D12Debug1* spDebugController1    = nullptr;
    HRESULT result                      = S_OK;

    result = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&spDebugController0);

    if (FAILED(result)) 
    {
        R_ERR(R_CHANNEL_D3D12, "Failed to enable gpu validation!");    
    }

    spDebugController0->EnableDebugLayer();
    spDebugController0->QueryInterface<ID3D12Debug1>(&spDebugController1);

    spDebugController1->SetEnableGPUBasedValidation(true);

    spDebugController1->Release();
    spDebugController0->Release();

#else
    R_WARN(R_CHANNEL_D3D12, "Can not enable GPU based validation for d3d12 device.");
#endif
}
} // Recluse