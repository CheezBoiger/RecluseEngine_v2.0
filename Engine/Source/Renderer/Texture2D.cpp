//
#include "Recluse/Renderer/Material.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Renderer/Renderer.hpp"

namespace Recluse {
namespace Engine {


ErrType Texture2D::initialize(Renderer* pRenderer, ResourceFormat format, U32 width, U32 height, U32 arrayLevel, U32 mips)
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

    switch (desc.format) {
        case RESOURCE_FORMAT_D24_UNORM_S8_UINT:
        case RESOURCE_FORMAT_D32_FLOAT:
        case RESOURCE_FORMAT_D32_FLOAT_S8_UINT:
            desc.usage |= RESOURCE_USAGE_DEPTH_STENCIL;
            break;
    }

    result = pDevice->createResource(&m_resource, desc, RESOURCE_STATE_SHADER_RESOURCE);

    if (result != REC_RESULT_OK) {
    
        R_ERR("Texture2D", "Failed to create texture2D resource.");

    }

    return result;
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
    
    result = pRenderer->getDevice()->copyResource(m_resource, nullptr);
}


ErrType TextureView::initialize(Renderer* pRenderer, Texture2D* pTexture, ResourceViewDesc& desc)
{
    GraphicsDevice* pDevice = pRenderer->getDevice();
    ErrType result = REC_RESULT_OK;

    desc.pResource = pTexture->getResource();

    result = pDevice->createResourceView(&m_view, desc);

    m_texture = pTexture;

    return result;
}


ErrType TextureView::destroy(Renderer* pRenderer)
{
    if (m_view) {
        pRenderer->getDevice()->destroyResourceView(m_view);
        m_view = nullptr;
    }

    return REC_RESULT_OK;
}
} // Engine
} // Recluse