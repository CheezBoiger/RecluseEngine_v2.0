//
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"


namespace Recluse {
namespace Engine {

class Texture2D;

struct SceneBuffers {
    Texture2D* pSceneAlbedo;
    Texture2D* pSceneAo;
    Texture2D* pSceneMat;
    Texture2D* pSceneNormal;
    Texture2D* pSceneBrightness;
    Texture2D* pSceneHDRTexture;
    Texture2D* pSceneDepth;
};


class GPUBuffer {
public:
    GPUBuffer()
        : m_pResource(nullptr)
        , m_pDevice(nullptr)
        , m_totalSzBytes(0) { }
    
    ErrType initialize(GraphicsDevice* pDevice, U64 totalSzBytes, ResourceUsageFlags usage);
    ErrType destroy();

    ErrType stream(GraphicsQueue* pQueue, void* ptr, U64 offsetBytes, U64 szBytes);

    GraphicsResource* get() const { return m_pResource; }
private:

    GraphicsResource*   m_pResource;
    GraphicsDevice*     m_pDevice;
    U64                 m_totalSzBytes;
};


class IndexBuffer {
public:
    IndexBuffer()
        : m_pBuffer(nullptr) { }

    ErrType initialize(GraphicsDevice* pDevice, U64 indices, IndexType type);

private:
    GPUBuffer* m_pBuffer;
};

typedef GPUBuffer VertexBuffer;
} // Engine
} // Recluse