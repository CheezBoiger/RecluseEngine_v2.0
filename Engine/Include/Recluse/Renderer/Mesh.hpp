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


// Per mesh information.
struct RecluseEngine_PUBLIC_API PerMeshTransform 
{
    Matrix44 world;
    Matrix44 worldToViewClip;
    Matrix44 n;
};


// SubMesh Flags tell the behavior of the submesh, how it should be 
// drawn, and how to access the contents of it.
enum SubMeshFlag
{
    // No flags.
    SubMeshFlag_None = 0,

    // Submesh is meant to be drawn with indices.
    SubMeshFlag_Indexed = (1 << 0),

    // Submesh is meant to be indirectly drawn.
    SubMeshFlag_Indirect = (1 << 1)
};
typedef uint SubMeshFlags;



// A Submesh is a portion of a mesh that has a separate rendering method or technique.
// Instinctively it will be a part of the mesh with it's own material, to which the renderer
// will know how to draw. Usually the renderer filters out drawcalls based on the submesh and its
// materials.
struct RecluseEngine_PUBLIC_API SubMesh 
{
    // Name of the submesh.
    std::string     name;
    SubMeshFlags    flags;

    // Material Id to query.
    uint            materialId;

    // Submesh vertex offset.
    uint            offsetElements;
    // Number of vertices that correspond to this mesh
    uint            rangeElements;

    // The bounds of the submesh. Usually for collision or whatnot.
    Math::Bounds3d  bounds;

    VertexBuffer*   vertexBuffer;
    IndexBuffer*    indexBuffer;
};


struct MeshLod
{
    uint offset;
    uint numVertices;
};


class CpuMesh
{
public:
    
};


// Mesh is a generalization of a renderable object that we wish to draw on screen.
// It composes of multiple submeshes, each assigned to separate portions of the vertex buffer and index buffer,
// and is usually sorted by material. Usually, mesh will be the object to pass to multiple parts of the engine,
// but ultimately, submeshes are the ones that contain the precise information of the renderable.
class Mesh : public Serializable, public RecreatableObject
{
public:
    virtual ~Mesh() { }

    RecluseEngine_PUBLIC_API Mesh()
        : m_pVertexBuffer(nullptr)
        , m_pIndexBuffer(nullptr) { }

    RecluseEngine_PUBLIC_API ResultCode initialize(VertexBuffer* pVertexBuffer, IndexBuffer* pIndexBuffer);

    RecluseEngine_PUBLIC_API VertexBuffer* getVertexBuffer() { return m_pVertexBuffer; }
    RecluseEngine_PUBLIC_API IndexBuffer* getIndexBuffer() { return m_pIndexBuffer; }

    RecluseEngine_PUBLIC_API const std::vector<SubMesh*>& getSubMeshes() { return m_submeshes; };

    RecluseEngine_PUBLIC_API void addSubmeshes(U32 numSubmeshes, SubMesh* pSubmeshes) 
    {
        for (U32 i = 0; i < numSubmeshes; ++i) 
        { 
            m_subMeshMap[pSubmeshes[i].name] = pSubmeshes[i];
            m_submeshes.push_back(&m_subMeshMap[pSubmeshes[i].name]);
        }
    }

    RecluseEngine_PUBLIC_API ResultCode serialize(Archive* archive) const override;
    RecluseEngine_PUBLIC_API ResultCode deserialize(Archive* archive) override;
    RecluseEngine_PUBLIC_API ResultCode recreate() override { return RecluseResult_NoImpl; }
    RecluseEngine_PUBLIC_API Bool       isRecreatable() const override { return false; }

    RecluseEngine_PUBLIC_API SubMesh*   getSubMesh(U32 idx) { return m_submeshes[idx]; }

private:
    std::map<std::string, SubMesh>  m_subMeshMap;
    std::vector<SubMesh*>           m_submeshes;
    VertexBuffer*                   m_pVertexBuffer;
    IndexBuffer*                    m_pIndexBuffer;
    RGUID                           m_guid;
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
public:
    enum { Position, Normal, UV, Binormal, Tangent, BoneIndex, BoneWeight };
    typedef u32 Attribute;

    // Stream data from the archive.
    bool streamFrom(Archive* archive);

    // Stream data to the archive.
    bool streamTo(Archive* archive);

    std::vector<Math::Float4>* operator()(Attribute attrib)
    {
        auto it = m_attributes.find(attrib);
        if (it != m_attributes.end())
            return &it->second;
        return nullptr;
    }

    bool contains(Attribute attrib) const
    {
        return (m_attributes.find(attrib) != m_attributes.end());
    }

    template<typename Type>
    bool store(Attribute attribute, const std::vector<Type>& attribs)
    {
        
    }

private:
    std::map<Attribute, std::vector<Math::Float4>> m_attributes;
};
} // Engine
} // Recluse