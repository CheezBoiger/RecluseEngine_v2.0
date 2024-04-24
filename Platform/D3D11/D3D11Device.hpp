
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include "D3D11Commons.hpp"


namespace Recluse {



class D3D11Context : public GraphicsContext
{
public:
    
    void bindRenderTargets(U32 count, ResourceViewId* ppResources, ResourceViewId pDepthStencil = 0) override
    {
        m_pContext->OMSetRenderTargets(count, nullptr, nullptr);
    }

private:
    ID3D11DeviceContext* m_pContext;
};


class D3D11Device : public GraphicsDevice
{
public:
    D3D11Device()
        : m_pDevice(nullptr)
    { }

    GraphicsContext* createContext() override;
    ResultCode releaseContext(GraphicsContext* context) override;

private:
    ID3D11Device* m_pDevice;
};
} // Recluse