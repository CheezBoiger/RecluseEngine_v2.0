//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Quaternion.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Renderer/RendererResources.hpp"
#include "Recluse/Renderer/Renderer.hpp"
#include "Recluse/RGUID.hpp"

#include "RecluseEngine_exports.hpp"
#include "Recluse/Math/Bounds3D.hpp"

#include <vector>
#include <map>

namespace Recluse {

class GraphicsDevice;
class GraphicsResource;
class DescriptorSetLayout;
class DescriptorSet;
struct Bounds3d;
class Material;

namespace Engine {

using namespace Math;

typedef Quaternion  MeshRotationData;
typedef Float3      MeshTranslateData;
typedef Float3      MeshScaleData;
typedef U8          MeshLod;


// SubMesh Flags tell the behavior of the submesh, how it should be 
// drawn, and how to access the contents of it.
enum SubMeshFlag
{
    // No flags.
    SubMeshFlag_None = 0,

    // Submesh is meant to be drawn with indices. This means that SubMesh::offsetElements and SubMesh::rangeElements
    // will be read as indices, instead of vertices.
    SubMeshFlag_Indexed = (1 << 0),

    // Submesh is meant to be indirectly drawn.
    SubMeshFlag_Indirect = (1 << 1),
    
    // Uses the mesh shader pipeline.
    SubMeshFlag_MeshShader = (1 << 2),
};
typedef uint SubMeshFlags;


// A Submesh is a portion of a mesh that has a separate rendering method or technique.
// Instinctively it will be a part of the mesh with it's own material, to which the renderer
// will know how to draw. Usually the renderer filters out drawcalls based on the submesh and its
// materials.
struct RecluseEngine_PUBLIC_API SubMesh
{
    // Name of the submesh.
    const char*     name;
    SubMeshFlags    flags;

    // Material Id to query.
    uint            materialId;

    // Submesh vertex offset.
    uint            offsetElements;
    // Number of elements that correspond to this submesh
    uint            rangeElements;

    // The bounds of the submesh. Usually for collision or whatnot.
    // Animation should consider updating this as well.
    Math::Bounds3d  bounds;
};


// Native mesh struct, contains the vertex and index buffers, for the given mesh.
struct RecluseEngine_PUBLIC_API Mesh 
{
    typedef u32 Attribute;

    enum { Position, Normal, UV0, UV1, Binormal, Tangent, BoneIndex, BoneWeight };

    std::vector<SubMesh> submeshes;
};


template<typename GpuBuffers>
struct MeshType : public Mesh
{
    GpuBuffers buffers;
};


struct TraditionalMesh
{
    // The bound vertex buffer.
    VertexBuffer*   vertexBuffer;

    // The bound index buffer. 
    IndexBuffer*    indexBuffer;
};


// MeshObject is a generalization of a renderable object that we wish to draw on screen.
// It composes of multiple submeshes, each assigned to separate portions of the vertex buffer and index buffer,
// and is usually sorted by material. Usually, mesh will be the object to pass to multiple parts of the engine,
// but ultimately, submeshes are the ones that contain the precise information of the renderable.
class MeshObject : public Serializable, public RecreatableObject
{
public:

    struct MeshLOD
    {
        MeshType<TraditionalMesh>* mesh;
    };

    virtual ~MeshObject() { }

    RecluseEngine_PUBLIC_API MeshObject() { }

    RecluseEngine_PUBLIC_API ResultCode     initialize(VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer);

    RecluseEngine_PUBLIC_API ResultCode     serialize(Archive* archive) const override;
    RecluseEngine_PUBLIC_API ResultCode     deserialize(Archive* archive) override;
    RecluseEngine_PUBLIC_API ResultCode     recreate(GraphicsContext* context) override { return RecluseResult_NoImpl; }
    RecluseEngine_PUBLIC_API Bool           isRecreatable() const override { return false; }
    RecluseEngine_PUBLIC_API const char*    getDebugName() const { return m_debugName; }
    RecluseEngine_PUBLIC_API MeshLOD*       getLod(uint lodIndex);

private:

    typedef std::vector<MeshLOD> MeshLodArray;

    MeshLodArray                    m_meshArray;
    RGUID                           m_guid;

    const char*                     m_debugName;
};


typedef GPUBuffer PerInstancedMeshBuffer;
typedef GPUBuffer InstancedMeshBuffer;
typedef U32       InstancedMeshId;          // Instanced Id, this is used to lookup matrix info from buffer.


// Instanced version of the mesh handler.
class InstancedMeshHandler
{
public:

    //< Register a mesh id. Get the assigned mesh id to update with.
    RecluseEngine_PUBLIC_API InstancedMeshId RegisterMeshId();
    
    //< Unregister a mesh id. 
    RecluseEngine_PUBLIC_API Bool            UnregisterMeshId();

    //< Update the mesh handler.
    RecluseEngine_PUBLIC_API void            update();

private:
    InstancedMeshBuffer             m_gpu4x4Matrices;   //< gpu matrix data. 
    
    // CPU side transform data.
    std::vector<MeshTranslateData>  m_dataTranslates;   //< Mesh Translations.
    std::vector<MeshRotationData>   m_dataRotations;    //< Mesh Rotations;
    std::vector<MeshScaleData>      m_dataScales;       //< Mesh Scales;
};

class InstancedMesh 
{
public:
    
    RecluseEngine_PUBLIC_API ResultCode initializeInstanced();

    RecluseEngine_PUBLIC_API InstancedMeshHandler* getHandler() { return m_instancedMeshHandler; } 

private:
    InstancedMeshHandler* m_instancedMeshHandler;
    PerInstancedMeshBuffer m_perInstancedMeshBuffer;
};


// Mesh Streamer, streams a host visible memory mesh from disk to ram. 
struct RecluseEngine_PUBLIC_API MeshStreamer
{
private:
    struct MeshHeader
    {
        typedef U32 Version;
        Version     meshVersion;
        RGUID       meshGuid;
        U32         attributeCount;
    };

    struct MeshAttributeHeader
    {
        Mesh::Attribute attribute;
        U32             elementCount;
        U32             bytesPerStride;
    };
public:

    // Stream data from the archive. If required version is 0, any version is allowed.
    bool streamFrom(Archive* archive, MeshHeader::Version requiredVersion = 0);

    // Stream data to the archive.
    bool streamTo(Archive* archive);

    std::vector<Math::Float4>* operator()(Mesh::Attribute attrib)
    {
        auto it = m_attributes.find(attrib);
        if (it != m_attributes.end())
            return &it->second;
        return nullptr;
    }

    bool contains(Mesh::Attribute attrib) const
    {
        return (m_attributes.find(attrib) != m_attributes.end());
    }

    template<typename Type>
    bool store(Mesh::Attribute attribute, const std::vector<Type>& attribs)
    {
        m_attributes[attribute] = attribs;
        return true;
    }

private:
    // Mesh attribute map loaded, unloaded from the streamer.
    std::map<Mesh::Attribute, std::vector<Math::Float4>> m_attributes;
};
} // Engine
} // Recluse