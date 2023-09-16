//
#include "D3D12Instance.hpp"
#include "D3D12Adapter.hpp"

#include "Recluse/Messaging.hpp"
#include <dxgi1_5.h>

#include "Recluse/Threading/Threading.hpp"

namespace Recluse {


R_INTERNAL
void pfnD3D12MessageFunc(
    D3D12_MESSAGE_CATEGORY Category, 
    D3D12_MESSAGE_SEVERITY Severity, 
    D3D12_MESSAGE_ID ID, 
    LPCSTR pDescription, 
    void* pContext)
{
    switch (Severity)
    {
        case D3D12_MESSAGE_SEVERITY_CORRUPTION:
            R_ERROR(R_CHANNEL_D3D12, "CORRUPTION: %s", pDescription);
            break;
        case D3D12_MESSAGE_SEVERITY_ERROR:
            R_ERROR(R_CHANNEL_D3D12, "ERROR: %s", pDescription);
            break;
        case D3D12_MESSAGE_SEVERITY_INFO:
            R_INFO(R_CHANNEL_D3D12, "INFO: %s", pDescription);
            break;
        case D3D12_MESSAGE_SEVERITY_WARNING:
            R_WARN(R_CHANNEL_D3D12, "WARNING: %s", pDescription);
        case D3D12_MESSAGE_SEVERITY_MESSAGE:
        default:    
            R_INFO(R_CHANNEL_D3D12, "MESSAGE: %s", pDescription);
            break;
    }
}

#if !defined(__ID3D12InfoQueue1_FWD_DEFINED__)
struct ThreadInfo
{
    U32 cookie;
    ID3D12Device* device;
    MutexGuard mut;
    Bool closed;
    Thread thr;
};
MutexGuard finishMutex;
std::map<U32, ThreadInfo> g_threadFinish;
U32 D3D12HandleDebugMessageCallback(void* data)
{
    U32 id = *(U32*)data;
    Bool shouldClose = false;

    do
    {
        
        ThreadInfo* info = nullptr;
        {
            ScopedLock _(finishMutex);
            info = &g_threadFinish[id];
        }
        ScopedLock _(info->mut);
        shouldClose = info->closed;
        if (!shouldClose)
        {
            ID3D12InfoQueue* infoQueue = nullptr;
            info->device->QueryInterface<ID3D12InfoQueue>(&infoQueue);
            if (infoQueue)
            {
                UINT64 messageCount = infoQueue->GetNumStoredMessages();
                for (UINT64 messageIdx = 0; messageIdx < messageCount; ++messageIdx)
                {
                    SIZE_T sizeLength = 0;
                    HRESULT result = infoQueue->GetMessageA(messageIdx, nullptr, &sizeLength);
                    D3D12_MESSAGE* message = (D3D12_MESSAGE*)malloc(sizeLength);
                    result = infoQueue->GetMessageA(messageIdx, message, &sizeLength);
                    if (result == S_OK)
                    {
                        pfnD3D12MessageFunc(message->Category, message->Severity, message->ID, message->pDescription, nullptr);
                    }
                    free(message);
                }
                infoQueue->ClearStoredMessages();
                infoQueue->Release();
            }
        }
    } while (!shouldClose);
    return 0;
}
#endif

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

    m_enabledFlags = flags;

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


DWORD D3D12Instance::registerDebugMessageCallback(ID3D12Device* pDevice)
{
    DWORD cookie = 0u;
#if defined(__ID3D12InfoQueue1_FWD_DEFINED__)
    ID3D12InfoQueue1* infoQueue = nullptr;
    pDevice->QueryInterface<ID3D12InfoQueue1>(&infoQueue);
    infoQueue->RegisterMessageCallback(pfnD3D12MessageFunc, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &cookie);
    infoQueue->Release();
#else
    static DWORD cookei = 0;
    cookie = ++cookei;
    ScopedLock _(finishMutex);
    ThreadInfo& info = g_threadFinish[cookie];
    info.device = pDevice;
    info.closed = false;
    info.cookie = cookie;
    info.thr.payload = &g_threadFinish[cookie].cookie;
    createThread(&info.thr, D3D12HandleDebugMessageCallback);
#endif
    return cookie;
}


void D3D12Instance::unregisterDebugMessageCallback(ID3D12Device* pDevice, DWORD cookie)
{
#if defined(__ID3D12InfoQueue1_FWD_DEFINED__)
    ID3D12InfoQueue1* infoQueue = nullptr;
    pDevice->QueryInterface<ID3D12InfoQueue1>(&infoQueue);
    infoQueue->UnregisterMessageCallback(cookie);
    infoQueue->Release();
#else
    ThreadInfo& info = g_threadFinish[cookie];
    info.closed = true;
    joinThread(&info.thr);
    ScopedLock _(info.mut);
    g_threadFinish.erase(cookie);
#endif
}
} // Recluse