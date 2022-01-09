//
#include "Recluse/Renderer/Mesh.hpp"

#include "Recluse/Graphics/GraphicsDevice.hpp"

namespace Recluse {
namespace Engine {


ErrType GPUBuffer::initialize(GraphicsDevice* pDevice, U64 totalSzBytes, ResourceUsageFlags usage)
{
    ErrType result                      = REC_RESULT_OK;
    GraphicsResourceDescription desc    = { };
    desc.usage                          = usage | RESOURCE_USAGE_TRANSFER_DESTINATION;
    desc.memoryUsage                    = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    desc.dimension                      = RESOURCE_DIMENSION_BUFFER;
    desc.arrayLevels                    = 1;
    desc.mipLevels                      = 1;
    desc.height                         = 1;
    desc.depth                          = 1;
    desc.width                          = totalSzBytes;
    
    result = pDevice->createResource(&m_pResource, desc, RESOURCE_STATE_COPY_SRC);

    m_pDevice       = pDevice;
    m_totalSzBytes  = totalSzBytes;

    return result;
}


ErrType GPUBuffer::stream(GraphicsDevice* pDevice, void* ptr, U64 offsetBytes, U64 szBytes)
{
    ErrType result = REC_RESULT_OK;
    GraphicsResource* pStaging = nullptr;

    GraphicsResourceDescription stageDesc = { };
    stageDesc.dimension     = RESOURCE_DIMENSION_BUFFER;
    stageDesc.memoryUsage   = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    stageDesc.width         = szBytes;
    stageDesc.usage         = RESOURCE_USAGE_TRANSFER_SOURCE;
    stageDesc.arrayLevels   = 1;
    stageDesc.mipLevels     = 1;
    stageDesc.height        = 1;

    result = m_pDevice->createResource(&pStaging, stageDesc, RESOURCE_STATE_COPY_DST);

    if (result != REC_RESULT_OK) 
    {
        return result;
    }

    CopyBufferRegion region = { };
    region.dstOffsetBytes   = offsetBytes;
    region.srcOffsetBytes   = 0;
    region.szBytes          = szBytes;

    result = pDevice->copyBufferRegions(m_pResource, pStaging, &region, 1);

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

    return REC_RESULT_OK;
}


ErrType Mesh::initialize(VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer)
{
    return REC_RESULT_OK;
}
} // Engine
} // Recluse