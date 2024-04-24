//
#include "D3D12LoaderApi.hpp"
#include "D3D12Instance.hpp"

Recluse::GraphicsInstance* createInstance()
{
    return new Recluse::D3D12::D3D12Instance();
}


Recluse::ResultCode destroyInstance(Recluse::GraphicsInstance* instance)
{
    if (instance)
    {
        delete instance;
    }
    return Recluse::RecluseResult_Ok;
}