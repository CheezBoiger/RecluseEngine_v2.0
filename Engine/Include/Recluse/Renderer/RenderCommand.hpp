//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Bounds3D.hpp"

#include "Recluse/Renderer/SceneView.hpp"
#include "Recluse/Renderer/Material.hpp"

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryPool.hpp"
#include "Recluse/Structures/HashMap.hpp"

#include "RecluseEngine_exports.hpp"

#include <vector>

namespace Recluse {
namespace Engine {

// Per mesh information.
struct RecluseEngine_PUBLIC_API PerMeshTransform 
{
    Matrix44 world;
    Matrix44 worldToViewClip;
    Matrix44 n;
};

enum CommandOp 
{
    CommandOp_DrawInstanced,
    CommandOp_DrawIndexedInstanced,
    CommandOp_Dispatch,
    CommandOp_BindResources,
};



enum ResourceBindOp
{
    R_BIND_RTV = (1 << 0),
    R_BIND_SRV = (1 << 1),
    R_BIND_UAV = (1 << 2),
    R_BIND_CBV = (1 << 3)
};


enum VertexAttribFlag 
{
    VertexAttrib_Position   = 0x0001,
    VertexAttrib_Normal     = 0x0002,
    VertexAttrib_Texcoord0  = 0x0004,
    VertexAttrib_Texcoord1  = 0x0008,
    VertexAttrib_Tangent    = 0x0010,
    VertexAttrib_Bitangent  = 0x0020,
    VertexAttrib_Bones      = 0x0040,
    VertexAttrib_BoneIndices = 0x0080,
};

typedef U32 VertexAttribFlags;
typedef U32 RenderPassTypeFlags;
typedef U32 ResourceBindOpFlags;

typedef U32                                 KeyId;
typedef std::vector<KeyId>                  KeyArray;
typedef MapContainer<U32, KeyArray>         CommandKeyContainer;

struct RenderCommand 
{
    CommandOp               op          : 24;   //
    U32                     stencilRef  : 8;    // 4 B
    void*                   opData;             //
};


struct ResourceBindCommand
{
    ResourceBindOpFlags bindFlags;
    GraphicsResource**  pRtvs;
    GraphicsResource**  pCbvs;
    GraphicsResource**  pSrvs;
    GraphicsResource**  pUavs;
    U16                 numRtvs;
    U16                 numCbvs;
    U16                 numUavs;
    U16                 numSrvs;
};


struct DrawBatch
{
    GraphicsResource**  ppVertexBuffers;                // 16 B
    U64*                pOffsets;                       // 24 B
    PerMeshTransform*   pPerMeshTransform;              // 32 B
    U32                 numVertexBuffers    : 8;        // 
    VertexAttribFlags   vertexTypeFlags     : 24;       // 36 B
    PrimitiveTopology   topology;
};


struct IndexedInstancedSubMesh 
{
    Material*   pMaterial;                        // 
    U32         indexCount;                             // 
    U32         firstInstance;                          // 
    U32         firstIndex;                             // 
    U32         vertexOffset;                           // 
    U32         instanceCount;                          // 
};


struct DrawIndexedBatch : public DrawBatch
{
    GraphicsResource*           pIndexBuffer;   // 56 B
    IndexedInstancedSubMesh*    pSubMeshes;     // 64 B
    U64                         offset;         // 72 B
    U32                         numSubMeshes;   // 76 B
    IndexType                   indexType;      // 80 B
};


struct InstancedSubMesh 
{
    Material*   pMaterial;                        //
    U32         vertexCount;                            // 
    U32         instanceCount;                          // 
    U32         firstVertex;                            // 
    U32         firstInstance;                          // 
};


struct DrawInstancedBatch : public DrawBatch
{
    InstancedSubMesh*   pSubMeshes;             // 56 B
    U32                 numSubMeshes;           // 60 B
                                                // 64 B
};


// Draw filter filters out draws that pertain to certain passes.
struct DrawFilter
{
    U32         passFilter;
    KeyArray    drawKey; // DrawKey pertains to the drawcall that works with it.
};


// High level render command list, which will be read by the low level backend, once the render thread
// is kicked off. Should reset every frame render.
class RecluseEngine_PUBLIC_API CommandList 
{
public:
    // Cast void data from render command to another type.
    template<typename Type>
    static Type* cast(void* data)
    {
        return static_cast<Type*>(data);
    }

    CommandList();
    ~CommandList();
    
    // Initialize the render commandlist, which will preallocate the necessary amount 
    // of scratch memory.
    void                initialize(U32 scratchSizeBytes = R_KB(2));
    void                destroy();

    inline ResultCode   push(const RenderCommand& renderCommand);
    inline void         reset();

    RenderCommand*      getRenderCommands() const { return (RenderCommand*)m_pAllocator->getBaseAddr(); }    
    U64                 getNumberCommands() const;

private:
    
    void resize();

    // Memory allocation that is used to iterate through commands.
    Allocator*  m_pAllocator;
    MemoryPool* m_pool;

    // Scratch memory used for rendering command information.
    MemoryPool* m_scratch;
    Allocator*  m_scratchAllocator;
};
} // Engine
} // Recluse