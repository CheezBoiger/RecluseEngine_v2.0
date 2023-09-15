
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include "D3D11Commons.hpp"


namespace Recluse {



class D3D11Context : public GraphicsContext
{
public:

private:
    ID3D11DeviceContext* m_pContext;
};


class D3D11Device : public GraphicsDevice
{
public:
    D3D11Device()
        : m_pDevice(nullptr)
    { }

private:
    ID3D11Device* m_pDevice;    
};
} // Recluse