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


enum RenderPassType : U32 {

    RENDER_PREZ                 = 0x0001,
    RENDER_SHADOW               = 0x0002,
    RENDER_PARTICLE             = 0x0004,
    RENDER_GBUFFER              = 0x0008,
    RENDER_HAS_MATERIAL         = 0x0010,
    RENDER_FORWARD_OPAQUE       = 0x0020,
    RENDER_FORWARD_TRANSPARENT  = 0x0040,
    RENDER_HAZE                 = 0x0080
};


typedef U32 RenderPassTypeFlags;

enum CommandOp {
    COMMAND_OP_DRAWABLE_INSTANCED,
    COMMAND_OP_DRAWABLE_INDEXED_INSTANCED,
    COMMAND_OP_DISPATCH
};


enum VertexAttribFlag {
  VERTEX_ATTRIB_POSITION   = 0x0001,
  VERTEX_ATTRIB_NORMAL     = 0x0002,
  VERTEX_ATTRIB_TEXCOORDS  = 0x0004,
  VERTEX_ATTRIB_TANGENT    = 0x0008,
  VERTEX_ATTRIB_BITANGENT  = 0x0010,
  VERTEX_ATTRIB_BONES      = 0x0020,
};

typedef U32 VertexAttribFlags;
typedef U32 RenderPassTypeFlags;

struct RenderCommand {
    CommandOp               op          : 24;   //
    U32                     stencilRef  : 8;    // 4 B
    RenderPassTypeFlags     flags;              // 8 B
};


struct DrawableRenderCommand : public RenderCommand {
    GraphicsResource**  ppVertexBuffers;                // 16 B
    U64*                pOffsets;                       // 24 B
    PerMeshTransform*   pPerMeshTransform;              // 32 B
    U32                 numVertexBuffers    : 8;        // 
    VertexAttribFlags   vertexTypeFlags     : 24;       // 36 B
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
class R_PUBLIC_API RenderCommandList {
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