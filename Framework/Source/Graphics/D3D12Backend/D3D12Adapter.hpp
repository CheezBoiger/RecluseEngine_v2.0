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

class D3D12Adapter : public GraphicsAdapter 
{
public:
    static std::vector<IDXGIAdapter*> getAdapters(D3D12Instance* pInstance);

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