//
#include "Recluse/Renderer/Mesh.hpp"

#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/CommandList.hpp"

namespace Recluse {
namespace Engine {


ErrType GPUBuffer::initialize(GraphicsDevice* pDevice, U64 totalSzBytes, ResourceUsageFlags usage)
{
    ErrType result                      = RecluseResult_Ok;
    GraphicsResourceDescription desc    = { };
    desc.usage                          = usage | ResourceUsage_TransferDestination;
    desc.memoryUsage                    = ResourceMemoryUsage_GpuOnly;
    desc.dimension                      = ResourceDimension_Buffer;
    desc.arrayLevels                    = 1;
    desc.mipLevels                      = 1;
    desc.height                         = 1;
    desc.depth                          = 1;
    desc.width                          = totalSzBytes;
    
    result = pDevice->createResource(&m_pResource, desc, ResourceState_CopyDestination);

    m_pDevice       = pDevice;
    m_totalSzBytes  = totalSzBytes;

    return result;
}


ErrType GPUBuffer::stream(GraphicsDevice* pDevice, void* ptr, U64 offsetBytes, U64 szBytes)
{
    ErrType result = RecluseResult_Ok;
    GraphicsResource* pStaging = nullptr;

    GraphicsResourceDescription stageDesc = { };
    stageDesc.dimension     = ResourceDimension_Buffer;
    stageDesc.memoryUsage   = ResourceMemoryUsage_CpuToGpu;
    stageDesc.width         = szBytes;
    stageDesc.usage         = ResourceUsage_TransferSource;
    stageDesc.arrayLevels   = 1;
    stageDesc.mipLevels     = 1;
    stageDesc.height        = 1;

    result = m_pDevice->createResource(&pStaging, stageDesc, ResourceState_CopySource);

    if (result != RecluseResult_Ok) 
    {
        return result;
    }

    CopyBufferRegion region = { };
    region.dstOffsetBytes   = offsetBytes;
    region.srcOffsetBytes   = 0;
    region.szBytes          = szBytes;

    if (m_pResource->getCurrentResourceState() != ResourceState_CopyDestination)
    {
        // TODO():
        // We will need to transition the resource before hand. This will require some work...
        return RecluseResult_NeedsUpdate;
    }

    result = pDevice->getContext()->copyBufferRegions(m_pResource, pStaging, &region, 1);

    m_pDevice->destroyResource(pStaging);

    return result;
}


ErrType GPUBuffer::destroy()
{
    if (m_pResource) 
    {
        m_pDevice->destroyResource(m_pResource);
        m_pResource = nullptr;   
    }

    return RecluseResult_Ok;
}


ErrType Mesh::initialize(VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer)
{
    return RecluseResult_Ok;
}
} // Engine
} // Recluse