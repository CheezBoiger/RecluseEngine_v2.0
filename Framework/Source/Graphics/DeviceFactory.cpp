//
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/System/DLLLoader.hpp"

namespace Recluse {

typedef Recluse::GraphicsInstance*(*CreateInstanceFunc)();
typedef Recluse::ResultCode(*FreeInstanceFunc)(Recluse::GraphicsInstance*);

R_INTERNAL const char* vulkan_dll_name  = "RecluseVulkan.dll";
R_INTERNAL const char* d3d12_dll_name   = "RecluseD3D12.dll";
R_INTERNAL const char* d3d11_dll_name   = "RecluseD3D11.dll";

R_INTERNAL
GraphicsInstance* g_currentInstance = nullptr;
DllLoader           graphicsLibrary = { };
CreateInstanceFunc CreateInstance = nullptr;
FreeInstanceFunc FreeInstance = nullptr;

GraphicsInstance* GraphicsInstance::create(enum GraphicsAPI api)
{
    if (g_currentInstance)
    {
        R_ERROR("Graphics", "An instance already exists for this application! Be sure to destroy the current instance, before creating a new one!");
        return g_currentInstance;
    }
    const char* dll_name = nullptr;
    switch (api) 
    {
        case GraphicsApi_Direct3D12:
        {
            R_DEBUG("Graphics", "Creating D3D12 instance...");
            dll_name = d3d12_dll_name;
            break;
        } 
        case GraphicsApi_Vulkan:
        { 
            R_DEBUG("Graphics", "Creating Vulkan instance...");
            dll_name = vulkan_dll_name;
            break;
        }
        case GraphicsApi_Direct3D11:
        {
            R_NO_IMPL();
            R_ERROR("Graphics", "D3D11 is not fully supported yet!");
            break;
        }
        case GraphicsApi_OpenGL:
        case GraphicsApi_SoftwareRasterizer:
        case GraphicsApi_SoftwareRaytracer:
        default:
            R_ERROR("Graphics", "Unsupported graphics api. Can not create instance!");

    }

    if (dll_name)
    {
        graphicsLibrary.load(dll_name);
        if (graphicsLibrary.isLoaded())
        {
            CreateInstance = (CreateInstanceFunc)graphicsLibrary.procAddress("createInstance");
            FreeInstance = (FreeInstanceFunc)graphicsLibrary.procAddress("destroyInstance");

            g_currentInstance = CreateInstance();
        }
        else
        {
            R_ERROR("Graphics", "Could not load %s for graphics rendering!", dll_name);
        }
    }
    else
        R_ERROR("Graphics", "Not able to find the correct graphics library to load!");

    return g_currentInstance;
}


ResultCode GraphicsInstance::destroyInstance(GraphicsInstance* pInstance)
{
    if (!pInstance) 
    {
        R_ERROR("Graphics", "Null pointer passed to %s", __FUNCTION__);
        return RecluseResult_NullPtrExcept;
    }

    // Instance must be the current one, in order to destroy properly.
    if (pInstance == g_currentInstance)
    {
        g_currentInstance = nullptr;
    }

    pInstance->destroy();

    if (graphicsLibrary.isLoaded())
    {
        ResultCode result = FreeInstance(pInstance);
        if (result != RecluseResult_Ok)
        {
            R_ERROR("Graphics", "Failed to destroy instance!");
        }
        graphicsLibrary.unload();
    }

    return RecluseResult_Ok;
}
} // Recluse