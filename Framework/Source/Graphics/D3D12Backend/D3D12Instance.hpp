//
#pragma once 

#include "Recluse/Graphics/GraphicsInstance.hpp"

#include "D3D12Commons.hpp"

namespace Recluse {


class D3D12Adapter;


class D3D12Instance : public GraphicsInstance 
{
public:
    D3D12Instance() : GraphicsInstance(GraphicsApi_Direct3D12) { }
    virtual ~D3D12Instance() { }

    IDXGIFactory2* get() const { return m_pFactory; }

private:
    void queryGraphicsAdapters() override;
    void freeGraphicsAdapters() override;
    ResultCode onInitialize(const ApplicationInfo& appInfo, LayerFeatureFlags flags) override;
    void onDestroy() override;

    void enableDebugValidation(Bool enableGpuValidation);

    IDXGIFactory2* m_pFactory;
};
} // Recluse