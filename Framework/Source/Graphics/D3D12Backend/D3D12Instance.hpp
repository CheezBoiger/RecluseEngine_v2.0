//
#pragma once 

#include "Recluse/Graphics/GraphicsInstance.hpp"

#include "D3D12Commons.hpp"

namespace Recluse {


class D3D12Adapter;


class D3D12Instance : public GraphicsInstance 
{
public:
    D3D12Instance() : GraphicsInstance(GRAPHICS_API_D3D12) { }
    virtual ~D3D12Instance() { }

    IDXGIFactory2* get() const { return m_pFactory; }

private:
    void queryGraphicsAdapters() override;
    void freeGraphicsAdapters() override;
    ErrType onInitialize(const ApplicationInfo& appInfo, EnableLayerFlags flags) override;
    void onDestroy() override;

    void enableDebugValidation();

    IDXGIFactory2* m_pFactory;
};
} // Recluse