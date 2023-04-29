//
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/Shader.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"

namespace Recluse {
namespace Engine {

class Texture2D;
class TextureView;


const std::string shaderDir                    = Filesystem::getCurrentDir() + "/" + "Shaders";
const ShaderIntermediateCode intermediateCode  = ShaderIntermediateCode_Spirv;

static const char* getBinaryShaderExtension()
{
    switch (intermediateCode) 
    {
        case ShaderIntermediateCode_Dxbc: return "dxbc";
        case ShaderIntermediateCode_Dxil: return "dxil";
        case ShaderIntermediateCode_Spirv: return "spv";
        default: return "unknown";
    }
}

static std::string getBinaryShaderPath(const std::string& relativePath)
{
    return shaderDir + "/" + relativePath + "." + getBinaryShaderExtension();
}


class GpuToCpuBuffer
{
    float* dataPtr;
};


class R_PUBLIC_API GPUBuffer 
{
public:
    GPUBuffer()
        : m_pResource(nullptr)
        , m_pDevice(nullptr)
        , m_totalSzBytes(0) { }

    virtual ~GPUBuffer() { }
    
    ResultCode             initialize(GraphicsDevice* pDevice, U64 totalSzBytes, ResourceUsageFlags usage);
    ResultCode             destroy();

    // Stream a staging resource back to the gpu.
    ResultCode             stream(GraphicsContext* pContext, void* ptr, U64 offsetBytes, U64 szBytes);

    // Obtain a staging buffer based on the offset of this gpu resource, and the total size bytes to work with.
    GpuToCpuBuffer      stage(U64 offsetBytes, U64 szBytes);

    GraphicsResource*   get() const { return m_pResource; }
private:
    ReferenceObject<U32>  m_stagingCount;
    GraphicsResource*   m_pResource;
    GraphicsDevice*     m_pDevice;
    U64                 m_totalSzBytes;
};


class R_PUBLIC_API IndexBuffer : public GPUBuffer 
{
public:
    IndexBuffer() { }

    ResultCode initializeIndices(GraphicsDevice* pDevice, IndexType type, U64 totalIndices) 
    {
        U64 szIndex = 0;
        switch (type) 
        {
            case IndexType_Unsigned16: szIndex = 2ull; break;
            case IndexType_Unsigned32:
            default: szIndex = 4ull; break;
        }
        m_indexType     = type;
        m_totalIndices  = totalIndices;
        return initialize(pDevice, szIndex * totalIndices, ResourceUsage_IndexBuffer);
    }

    IndexType   getIndexType() const { return m_indexType; }
    U64         getTotalIndices() const { return m_totalIndices; }

private:
    U64         m_totalIndices;
    IndexType   m_indexType;
};


class R_PUBLIC_API VertexBuffer : public GPUBuffer 
{
public:
    VertexBuffer() { }

    ResultCode initializeVertices(GraphicsDevice* pDevice, U64 perVertexSzBytes, U64 totalVertices) 
    {
        m_perVertexSzBytes  = perVertexSzBytes;
        m_totalVertices     = totalVertices;
        return initialize(pDevice, perVertexSzBytes * totalVertices, ResourceUsage_VertexBuffer);
    }

private:
    U64 m_perVertexSzBytes;
    U64 m_totalVertices;
};


enum GBuffer
{
    GBuffer_Albedo,
    GBuffer_Normal,
    GBuffer_Materials,
    GBuffer_Depth,
    GBuffer_MaxBufferCount
};


struct SceneBufferDefinitions 
{
    Texture2D*              gbuffer[GBuffer_MaxBufferCount];
    TextureView*            gbufferViews[GBuffer_MaxBufferCount];
};

namespace RenderDB {

typedef U32 RenderID;

extern R_PUBLIC_API Bool         registerTexture2D(RenderID id);
extern R_PUBLIC_API Bool         registerGPUBuffer(RenderID id);

extern R_PUBLIC_API Bool         isCachedGPUBuffer(RenderID id);
extern R_PUBLIC_API Bool         isCachedTexture2D(RenderID id);

extern R_PUBLIC_API Texture2D*   getTexture2D(RenderID id);
extern R_PUBLIC_API GPUBuffer*   getGPUBuffer(RenderID id);

extern R_PUBLIC_API ResultCode      cacheGPUBuffer(RenderID id, GPUBuffer* pBuffer);
extern R_PUBLIC_API ResultCode      cacheTexture2D(RenderID id, Texture2D* pTexture);

extern R_PUBLIC_API Bool         removeTexture2D(RenderID id);
extern R_PUBLIC_API Bool         removeGPUBuffer(RenderID id);
} // RenderDB
} // Engine
} // Recluse