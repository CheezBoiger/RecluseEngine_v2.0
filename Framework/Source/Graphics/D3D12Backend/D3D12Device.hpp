//
#pragma once

#include "Win32/Win32Common.hpp"
#include "D3D12Commons.hpp"

#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

namespace Recluse {

class D3D12Adapter;

class D3D12Device : public GraphicsDevice {
public:

    ErrType initialize(D3D12Adapter* adapter, const DeviceCreateInfo& info);
    void destroy();

    ID3D12Device* get() const { return m_device; }
    
    D3D12Adapter* getAdapter() const { return m_pAdapter; }

private:
    ID3D12Device* m_device;
    D3D12Adapter* m_pAdapter;
};
} // Recluse