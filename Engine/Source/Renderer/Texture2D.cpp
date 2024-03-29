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
        return recluseHashFast(&h, sizeof(TextureViewID));
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

ResultCode Texture2D::initialize(Renderer* pRenderer, ResourceFormat format, U32 width, U32 height, U32 arrayLevel, U32 mips)
{
    GraphicsDevice* pDevice             = pRenderer->getDevice();
    GraphicsResourceDescription desc    = { };
    ResultCode result                      = RecluseResult_Ok;

    desc.memoryUsage    = ResourceMemoryUsage_GpuOnly;
    desc.usage          = ResourceUsage_TransferDestination | ResourceUsage_ShaderResource;
    desc.depthOrArraySize = arrayLevel;
    desc.mipLevels      = mips;
    desc.width          = width;
    desc.height         = height;
    desc.dimension      = ResourceDimension_2d;
    desc.samples        = 1;
    desc.format         = format;

    switch (desc.format) 
    {
        case ResourceFormat_D24_Unorm_S8_Uint:
        case ResourceFormat_D32_Float:
        case ResourceFormat_D32_Float_S8_Uint:
            desc.usage |= ResourceUsage_DepthStencil;
            break;
    }

    result = pDevice->createResource(&m_resource, desc, ResourceState_ShaderResource);

    if (result != RecluseResult_Ok) 
    {
        R_ERROR("Texture2D", "Failed to create texture2D resource.");
        return result;
    }

    // Generate the CRC using just the pointer.
    genCrC((void*)m_resource, sizeof(GraphicsResource));

    return result;
}


void TextureResource::genCrC(void* pUnique, U64 sz) 
{
    m_crc = recluseHashFast(pUnique, sz);
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
    pRenderer->getContext()->copyResource(m_resource, nullptr);
}


ResultCode TextureView::initialize(Renderer* pRenderer, Texture2D* pTexture, ResourceViewDescription& desc)
{
    GraphicsDevice* pDevice = pRenderer->getDevice();
    ResultCode result = RecluseResult_Ok;

    m_texture = pTexture;

    genCrC((void*)m_view, sizeof(TextureView));

    return result;
}


ResultCode TextureView::destroy(Renderer* pRenderer)
{

    return RecluseResult_Ok;
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