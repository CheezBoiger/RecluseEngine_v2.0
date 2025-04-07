//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Quaternion.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Renderer/RendererResources.hpp"
#include "Recluse/RGUID.hpp"

#include "RecluseEngine_exports.hpp"

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


// A Submesh is a portion of a mesh that has a separate rendering method or technique.
// Instinctively it will be a part of the mesh with it's own material.
struct RecluseEngine_PUBLIC_API SubMesh 
{
    std::string name;
    Material*   material;
    U64         offset;
    U64         numVertices;
};


struct MeshLod
{
    U64 index;
    U64 offsetVert;
};


class CpuMesh
{
public:
    
};


class Mesh : public Serializable
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


//< 
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
} // Engine
} // Recluse