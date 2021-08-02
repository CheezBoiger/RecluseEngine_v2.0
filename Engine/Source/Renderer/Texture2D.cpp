//
#include "Recluse/Renderer/Material.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Renderer/Renderer.hpp"

namespace Recluse {
namespace Engine {


void Texture2D::initialize(Renderer* pRenderer, ResourceFormat format, U32 width, U32 height, U32 arrayLevel, U32 mips)
{
    GraphicsDevice* pDevice             = pRenderer->getDevice();
    GraphicsResourceDescription desc    = { };
    ErrType result                      = REC_RESULT_OK;

    desc.memoryUsage    = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    desc.usage          = RESOURCE_USAGE_TRANSFER_DESTINATION | RESOURCE_USAGE_SHADER_RESOURCE;
    desc.arrayLevels    = arrayLevel;
    desc.mipLevels      = mips;
    desc.depth          = 1;
    desc.width          = width;
    desc.height         = height;
    desc.dimension      = RESOURCE_DIMENSION_2D;
    desc.samples        = 1;
    desc.format         = format;

    result = pDevice->createResource(&m_resource, desc);

    if (result != REC_RESULT_OK) {
    
        R_ERR("Texture2D", "Failed to create texture2D resource.");
    
    }
}


void Texture2D::destroy(Renderer* pRenderer)
{
    if (m_resource) {
    
        pRenderer->getDevice()->destroyResource(m_resource);
        m_resource = nullptr;
        
    }
}


void Texture2D::load(Renderer* pRenderer, void* pData, U64 szBytes)
{
    ErrType result = REC_RESULT_OK;
    
    result = pRenderer->getGraphicsQueue()->copyResource(m_resource, nullptr);
}
} // Engine
} // Recluse