//
#include "Recluse/Renderer/Material.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/Serialization/Hasher.hpp"

#include <unordered_map>

namespace Recluse {
namespace Engine {


class TextureViewIDHasher {
public:
    SizeT operator()(const TextureViewID& h) const 
    {
        // Not idea, but we gotta...
        return recluseHash((void*)&h, sizeof(TextureViewID));
    }
};


class TextureViewIDComparer 
{
public:
    Bool operator()(const TextureViewID& lh, const TextureViewID& rh) const 
    {
        return (lh.dimension == rh.dimension) && 
                (lh.format == rh.format) &&
                (lh.resourceCrC == rh.resourceCrC) && 
                (lh.type == rh.type);
    }
};

// Texture Management Handler.
static std::unordered_map
    <
        TextureViewID, 
        TextureView*, 
        TextureViewIDHasher,
        TextureViewIDComparer
    > gTextureViewLUT;

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

    switch (desc.format) 
    {
        case RESOURCE_FORMAT_D24_UNORM_S8_UINT:
        case RESOURCE_FORMAT_D32_FLOAT:
        case RESOURCE_FORMAT_D32_FLOAT_S8_UINT:
            desc.usage |= RESOURCE_USAGE_DEPTH_STENCIL;
            break;
    }

    result = pDevice->createResource(&m_resource, desc, RESOURCE_STATE_SHADER_RESOURCE);

    if (result != REC_RESULT_OK) 
    {
        R_ERR("Texture2D", "Failed to create texture2D resource.");
        return result;
    }

    // Generate the CRC using just the pointer.
    genCrC((void*)m_resource, sizeof(GraphicsResource));

    return result;
}


void TextureResource::genCrC(void* pUnique, U64 sz) 
{
    m_crc = recluseHash(pUnique, sz);
}


void Texture2D::destroy(Renderer* pRenderer)
{
    if (m_resource) 
    {
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

    genCrC((void*)m_view, sizeof(TextureView));

    return result;
}


ErrType TextureView::destroy(Renderer* pRenderer)
{
    if (m_view) 
    {
        pRenderer->getDevice()->destroyResourceView(m_view);
        m_view = nullptr;
    }

    return REC_RESULT_OK;
}


TextureView* Texture2D::getTextureView(const TextureViewID& id)
{
    return lookupTextureView(id);
}


TextureView* lookupTextureView(const TextureViewID& id)
{
    if (gTextureViewLUT.find(id) == gTextureViewLUT.end()) 
    {
        return nullptr;
    }

    return gTextureViewLUT[id];
}
} // Engine
} // Recluse