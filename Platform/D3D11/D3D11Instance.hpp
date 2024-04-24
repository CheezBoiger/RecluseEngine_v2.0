
#include "D3D11Commons.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"


namespace Recluse {
namespace D3D11 {

class D3D11Instance : public GraphicsInstance
{
public:
    D3D11Instance()
        : GraphicsInstance(GraphicsApi_Direct3D11) 
    { }
    virtual ~D3D11Instance() { }

    ResultCode      onInitialize(const ApplicationInfo& appInfo, LayerFeatureFlags flags) override;

    IDXGIFactory2*  get() const { return m_pFactory; }
    Bool            hasTearingSupport() const;
    
private:
    void            queryGraphicsAdapters() override;
    void            freeGraphicsAdapters() override;
    void            onDestroy() override;

    IDXGIFactory2* m_pFactory;
};
} // D3D11
} // Recluse