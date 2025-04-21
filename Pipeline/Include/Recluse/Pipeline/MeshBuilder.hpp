//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/Renderer/Mesh.hpp"
#include "Recluse/RGUID.hpp"
#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Math/Bounds3D.hpp"
#include "Recluse/Pipeline/Importer.hpp"

#include "ReclusePipeline_exports.hpp"

#include <vector>

namespace Recluse {
namespace Pipeline {
namespace Builder {


// Mesh Builder helps in building out meshes and materials,
// and holds that data for use later.
class ReclusePipeline_PUBLIC_API MeshBuilder : public Serializable
{
public:
    // 
    enum 
    {
        Optimize = (1<<0),
        Quantize = (1<<1),
        Simplify = (1<<2)
    };
    typedef U32 MeshBuilderFlags;

    enum 
    {
        Position,
        Normal,
        UV,
        Binormal,
        Tangent,
        BoneIndex,
        BoneWeight
    };

    // Mesh information.
    struct Data
    {
        // Name of the mesh.
        std::string                             name;
        // Id of this mesh.
        RGUID                                   guid;

        // Engine submeshes.
        // Some information about submeshes.
        // According to the url: https://download.autodesk.com/us/fbx/SDKdocs/FBX_SDK_Help/files/fbxsdkref/class_k_fbx_mesh.html
        // Fbx identifies a group of vertices as a Polygon, this can be akin to a submesh, that we identify in Recluse.
        // 
        std::map<std::string, Engine::SubMesh*> submeshMap;
        std::vector<Engine::SubMesh>            submeshes;

        // Data related to the mesh is arbitrary, especially if we end up quantizing.
        std::vector<Math::Float3>               positions;
        std::vector<Math::Float3>               normals;
        std::vector<Math::Float4>               uvs;
        std::vector<Math::Float3>               binormals;
        std::vector<Math::Float3>               tangents;

        // For animation data. If the mesh has bone information
        i32                                     boneId;

        // Vertex indices, if the mesh is indexed.
        std::vector<U32>                        vertexIndices;

        std::vector<Material*>                  materials;
    };
    
    struct BoneData
    {
        std::vector<Math::Float4>               boneWeights;
        std::vector<Math::UInt4>                boneIndices;
    };

    // Create the mesh builder.
    static MeshBuilder*         create(FileFormat format);
    static ResultCode           destroy(MeshBuilder* builder);

    MeshBuilder() { }

    virtual ~MeshBuilder() { }

    virtual ResultCode          build(Importer* importer, MeshBuilderFlags flags) = 0;

    virtual ResultCode          serialize(Archive* archive) const override { return RecluseResult_NoImpl; }
    virtual ResultCode          deserialize(Archive* archive) override { return RecluseResult_NoImpl; }
    ResultCode                  clear() { m_data.clear(); m_dataMap.clear(); return RecluseResult_Ok; }

    Data*                       getData(const RGUID& rguid) { auto it = m_dataMap.find(rguid); if (it != m_dataMap.end()) return &m_data[it->second.index]; }
    u32                         getNumberOfMeshes() const { return m_data.size(); }
    Data*                       getAll() { return m_data.data(); }
    BoneData*                   getBoneData(i32 boneId) { auto it = m_boneMap.find(boneId); if (it != m_boneMap.end()) return &it->second; }
protected:
    struct MeshDataInfo
    {
        u32 index;
    };
    std::map<RGUID, MeshDataInfo, RGUID::Less>          m_dataMap;
    std::map<i32, BoneData>                             m_boneMap;
    std::vector<MeshBuilder::Data>                      m_data;
};
} // Builder
} // Pipeline
} // Recluse