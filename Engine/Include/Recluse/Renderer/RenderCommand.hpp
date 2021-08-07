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
    COMMAND_OP_DRAW_INSTANCED,
    COMMAND_OP_DRAW_INDEXED_INSTANCED
};

typedef U32 PassTypeFlags;

struct RenderCommand {
    GraphicsResource**      ppVertexBuffers;    // 8  B
    U64*                    pOffsets;           // 16 B
    Material*               pMaterial;          // 24 B
    U32                     numVertexBuffers;   // 28 B 
    CommandOp               op;                 // 32 B
    TransformPerMesh*       pTransformPerMesh;  // 40 B 
    U32                     numInstances;       // 44 B
    PassTypeFlags           flags;              // 48 B
};


struct DrawIndexedRenderCommand : public RenderCommand {
    GraphicsResource* pIndexBuffer;             // 56 B
    U32 indexCount;                             // 60 B
    U32 firstInstance;                          // 64 B
    U32 firstIndex;                             // 68 B
    U32 vertexOffset;                           // 72 B 
    U32 instanceCount;                          // 76 B
                                                // 80 B
};


struct DrawRenderCommand : public RenderCommand {
    U32 vertexCount;                            // 56 B
    U32 instanceCount;                          // 60 B
    U32 firstVertex;                            // 64 B
    U32 firstInstance;                          // 68 B
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