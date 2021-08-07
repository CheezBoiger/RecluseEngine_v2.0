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
const ShaderIntermediateCode intermediateCode  = INTERMEDIATE_SPIRV;

static const char* getBinaryShaderExtension()
{
    switch (intermediateCode) {
    
        case INTERMEDIATE_DXBC: return "dxbc";
        case INTERMEDIATE_DXIL: return "dxil";
        case INTERMEDIATE_SPIRV: return "spv";
        default: return "unknown";
    }
}

static std::string getBinaryShaderPath(const std::string& relativePath)
{
    return shaderDir + "/" + relativePath + "." + getBinaryShaderExtension();
}


class R_EXPORT GPUBuffer {
public:
    GPUBuffer()
        : m_pResource(nullptr)
        , m_pDevice(nullptr)
        , m_totalSzBytes(0) { }

    virtual ~GPUBuffer() { }
    
    ErrType initialize(GraphicsDevice* pDevice, U64 totalSzBytes, ResourceUsageFlags usage);
    ErrType destroy();

    ErrType stream(GraphicsQueue* pQueue, void* ptr, U64 offsetBytes, U64 szBytes);

    GraphicsResource* get() const { return m_pResource; }
private:

    GraphicsResource*   m_pResource;
    GraphicsDevice*     m_pDevice;
    U64                 m_totalSzBytes;
};


class R_EXPORT IndexBuffer : public GPUBuffer {
public:
    IndexBuffer() { }

    ErrType initializeIndices(GraphicsDevice* pDevice, IndexType type, U64 totalIndices) {
        U64 szIndex = 0;
        switch (type) {
            case INDEX_TYPE_UINT16: szIndex = 2ull; break;
            case INDEX_TYPE_UINT32:
            default: szIndex = 4ull; break;
        }
        m_indexType = type;
        m_totalIndices = totalIndices;
        return initialize(pDevice, szIndex * totalIndices, RESOURCE_USAGE_INDEX_BUFFER);
    }

    IndexType getIndexType() const { return m_indexType; }
    U64 getTotalIndices() const { return m_totalIndices; }

private:
    U64 m_totalIndices;
    IndexType m_indexType;
};


class R_EXPORT VertexBuffer : public GPUBuffer {
public:
    VertexBuffer() { }

    ErrType initializeVertices(GraphicsDevice* pDevice, U64 perVertexSzBytes, U64 totalVertices) {
        m_perVertexSzBytes = perVertexSzBytes;
        m_totalVertices = totalVertices;
        return initialize(pDevice, perVertexSzBytes * totalVertices, RESOURCE_USAGE_VERTEX_BUFFER);
    }

private:
    U64 m_perVertexSzBytes;
    U64 m_totalVertices;
};


struct SceneBuffers {
    std::vector<GPUBuffer*> pSceneViewBuffers;
    Texture2D* pSceneAlbedo;
    Texture2D* pSceneAo;
    Texture2D* pSceneMat;
    Texture2D* pSceneNormal;
    Texture2D* pSceneBrightness;
    Texture2D* pSceneHDRTexture;
    Texture2D* pSceneDepth;
    TextureView* pDepthStencilView;
};
} // Engine
} // Recluse