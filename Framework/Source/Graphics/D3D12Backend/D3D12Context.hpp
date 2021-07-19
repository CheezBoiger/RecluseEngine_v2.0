//
#pragma once 

#include "Recluse/Graphics/GraphicsContext.hpp"

#include "D3D12Commons.hpp"

namespace Recluse {


class D3D12Adapter;


class D3D12Context : public GraphicsContext {
public:

    IDXGIFactory1* get() const { return m_pFactory; }

private:
    void queryGraphicsAdapters() override;
    void freeGraphicsAdapters() override;
    ErrType onInitialize(const ApplicationInfo& appInfo, EnableLayerFlags flags) override;
    void onDestroy() override;

    IDXGIFactory1* m_pFactory;
};
} // Recluse