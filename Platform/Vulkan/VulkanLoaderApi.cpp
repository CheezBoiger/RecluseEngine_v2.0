//
#include "VulkanLoaderApi.hpp"
#include "VulkanInstance.hpp"

Recluse::GraphicsInstance* createInstance()
{
    return new Recluse::Vulkan::VulkanInstance();
}

Recluse::ResultCode destroyInstance(Recluse::GraphicsInstance* instance)
{
    if (instance)
    {
        delete instance;
    }
    return Recluse::RecluseResult_Ok;
}