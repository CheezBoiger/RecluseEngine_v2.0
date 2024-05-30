//
#pragma once 

#include "D3D12Commons.hpp"

#include "Recluse/Graphics/GraphicsAdapter.hpp"

#include <vector>
#include <list>

namespace Recluse {
namespace D3D12 {

class D3D12Instance;
class D3D12Device;

// D3D12 Graphics adapter, this defines a physical device, such as the GPGPU, Integrated GPU, Software Rasterizer, etc...
// The adapter is essentially the physical interface that actually performs the rendering. 
class D3D12Adapter : public GraphicsAdapter 
{
public:
    // Get all available adapters from the D3D12 instance.
    static std::vector<IDXGIAdapter*> getAdapters(D3D12Instance* pInstance);
    // Get the D3D12 adapter information, which contains all limitations and supported features.
    ResultCode getAdapterInfo(AdapterInfo* out) const override;
    
    IDXGIAdapter* get() const { return m_pAdapter; }

    D3D12Instance* getInstance() const { return m_pInstance; }

    U32 constantBufferOffsetAlignmentBytes() const;

    ResultCode createDevice(DeviceCreateInfo& info, GraphicsDevice** ppDevice) override;
    ResultCode destroyDevice(GraphicsDevice* pDevice) override;

    void destroy();

private:

    D3D12Adapter(IDXGIAdapter* adapter = NULL);

    IDXGIAdapter* m_pAdapter;
    D3D12Instance* m_pInstance;
    
    std::list<D3D12Device*> m_devices;

    friend class D3D12Instance;
};
} // D3D12
} // Recluse