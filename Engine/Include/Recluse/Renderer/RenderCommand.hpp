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


enum PassType : U32 {
    RENDER_HAS_POSITION    = 0x001,
    RENDER_HAS_NORMAL      = 0x002,
    RENDER_HAS_TEXCOORDS   = 0x004,
    RENDER_HAS_TANGENT     = 0x008,
    RENDER_HAS_BITANGENT   = 0x010,
    RENDER_HAS_BONES       = 0x020,
    RENDER_PREZ            = 0x040,
    RENDER_SHADOW          = 0x080,
    RENDER_PARTICLE        = 0x100,
    RENDER_GBUFFER         = 0x200,
    RENDER_HAS_MATERIAL    = 0x400
};


typedef U32 PassTypeFlags;

enum CommandOp {
    COMMAND_OP_DRAWABLE_INSTANCED,
    COMMAND_OP_DRAWABLE_INDEXED_INSTANCED,
    COMMAND_OP_DISPATCH
};


typedef U32 PassTypeFlags;

struct RenderCommand {
    CommandOp               op;                 // 8 B
    PassTypeFlags           flags;              // 16 B
};


struct DrawableRenderCommand : public RenderCommand {
    GraphicsResource**  ppVertexBuffers;        // 24 B
    U64*                pOffsets;               // 32 B
    PerMeshTransform*   pPerMeshTransform;      // 40 B
    U32                 numVertexBuffers;       // 44 B
};


struct IndexedInstancedSubMesh {
    Material* pMaterial;                        // 
    U32 indexCount;                             // 
    U32 firstInstance;                          // 
    U32 firstIndex;                             // 
    U32 vertexOffset;                           // 
    U32 instanceCount;                          // 
};


struct DrawIndexedRenderCommand : public DrawableRenderCommand {
    GraphicsResource*           pIndexBuffer;   // 56 B
    IndexedInstancedSubMesh*    pSubMeshes;     // 64 B
    U64                         offset;         // 72 B
    U32                         numSubMeshes;   // 76 B
    IndexType                   indexType;      // 80 B
};


struct InstancedSubMesh {
    Material* pMaterial;                        //
    U32 vertexCount;                            // 
    U32 instanceCount;                          // 
    U32 firstVertex;                            // 
    U32 firstInstance;                          // 
};


struct DrawRenderCommand : public DrawableRenderCommand {
    InstancedSubMesh*   pSubMeshes;             // 56 B
    U32                 numSubMeshes;           // 60 B
                                                // 64 B
};


// High level render command list, which will be read by the low level backend, once the render thread
// is kicked off. Should reset every frame render.
class R_EXPORT RenderCommandList {
public:
    RenderCommandList()
        : m_pAllocator(nullptr)
        , m_pointerAllocator(nullptr)
        , m_pool(nullptr)
        , m_pointerPool(nullptr) { }

    void initialize();
    void destroy();

    inline ErrType push(const RenderCommand& renderCommand);
    inline void reset();

    RenderCommand** getRenderCommands() const { return (RenderCommand**)m_pointerPool->getBaseAddress(); }    

    U64 getNumberCommands() const { return m_pAllocator->getTotalAllocations(); }

private:
    
    void resize();

    Allocator* m_pAllocator;
    Allocator* m_pointerAllocator;
    MemoryPool* m_pool;
    MemoryPool* m_pointerPool;
};
} // Engine
} // Recluse