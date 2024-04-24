//
#include "D3D11LoaderApi.hpp"
#include "D3D11Instance.hpp"

Recluse::GraphicsInstance* createInstance()
{
    return nullptr;//new Recluse::D3D11::D3D11Instance();
}


Recluse::ResultCode destroyInstance(Recluse::GraphicsInstance* instance)
{
    if (instance)
    {
        delete instance;
    }
    return Recluse::RecluseResult_Ok;
}