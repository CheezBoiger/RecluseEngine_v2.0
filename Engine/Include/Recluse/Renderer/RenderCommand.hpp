//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Bounds3D.hpp"

#include "Recluse/Renderer/SceneView.hpp"
#include "Recluse/Renderer/Mesh.hpp"
#include "Recluse/Renderer/Material.hpp"

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"

namespace Recluse {
namespace Engine {


enum SubMeshRender : U32 {
    RENDER_HAS_POSITION    = 0x01,
    RENDER_HAS_NORMAL      = 0x02,
    RENDER_HAS_TEXCOORDS   = 0x04,
    RENDER_HAS_TANGENT     = 0x08,
    RENDER_HAS_BITANGENT   = 0x10,
    RENDER_HAS_BONES       = 0x20
};


typedef U32 SubMeshRenderFlags;


struct SubmeshRenderCommand {
    Material* pMaterial;        // The material that is assigned to these submeshes.
    SubMesh** ppSubmeshes;      // All submeshes to be rendered.
    U32       numSubmeshes;     // Number of submeshes.
    U32       vbIndex;          // Vertex Buffer index in RenderCommand
    Bounds3D  aabb;
    SubMeshRenderFlags flags;      // 
};

struct RenderCommand {
    VertexBuffer**          pVertexBuffers;
    IndexBuffer*            pIndexBuffer;
    U32                     numVertexBuffers;
    U32                     numSubMeshCommands;
    TransformPerMesh*       pTransformPerMesh;
    SubmeshRenderCommand*   pSubmeshes;
    U32                     numInstances;
    Bounds3D                aabb;
};


// High level render command list, which will be read by the low level backend, once the render thread
// is kicked off. Should reset every frame render.
class R_EXPORT RenderCommandList {
public:

    void initialize(U32 numRenderCommands);
    void destroy();

    inline void push(const RenderCommand& renderCommand);
    inline void reset();

    RenderCommand* getRenderCommands() const { return (RenderCommand*)m_pool->getBaseAddress(); }
    U64 getNumberCommands() const { return m_pAllocator->getTotalAllocations(); }

private:
    
    void resize();

    Allocator* m_pAllocator;
    MemoryPool* m_pool;
};
} // Engine
} // Recluse