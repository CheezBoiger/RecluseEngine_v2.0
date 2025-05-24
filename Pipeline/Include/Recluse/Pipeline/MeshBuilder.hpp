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
        Optimize            = (1<<0),
        Quantize            = (1<<1),
        Simplify            = (1<<2),
        Optimize_Aggressive = (1<<3),

        // Triangulate the mesh, if there are polygons that are more than 3 vertices.
        Triangulate = (1<<3)
    };
    typedef U32 MeshBuilderFlags;

    // Mesh information.
    struct MeshData
    {
        // Name of the mesh.
        std::string                             name;
        // Id of this mesh.
        RGUID                                   guid;
        RGUID                                   parent; // Parent guid.

        // Some information about polygons in a mesh.
        // According to the url: https://download.autodesk.com/us/fbx/SDKdocs/FBX_SDK_Help/files/fbxsdkref/class_k_fbx_mesh.html
        // Fbx identifies a group of vertices as a Polygon, which needs to be 3 vertices, otherwise will require triangulation.
        // 

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

    // Destroy the mesh builder.
    static ResultCode           destroy(MeshBuilder* builder);

                                MeshBuilder(FileFormat fileFormat, const char* ext) : m_fileFormat(fileFormat), m_ext(ext) { }

    virtual                     ~MeshBuilder() { }

    virtual ResultCode          build(Importer* importer, MeshBuilderFlags flags) = 0;

    virtual ResultCode          serialize(Archive* archive) const override { return RecluseResult_NoImpl; }
    virtual ResultCode          deserialize(Archive* archive) override { return RecluseResult_NoImpl; }
    ResultCode                  clear() { m_data.clear(); m_dataMap.clear(); return RecluseResult_Ok; }

    // Get the data by rguid.
    MeshData*                   getData(const RGUID& rguid) { auto it = m_dataMap.find(rguid); if (it != m_dataMap.end()) return &m_data[it->second.index]; }

    // Get the data by index.
    const MeshData*             getData(uint index) const { return &m_data[index]; }
    MeshData*                   getData(uint index) { return &m_data[index]; }

    // Get the number of meshes added in this builder.
    u32                         getNumberOfMeshes() const { return m_data.size(); }
    // Raw gather for the mesh datas.
    MeshData*                   getAll() { return m_data.data(); }

    // Obtain the bone data with a mesh id.
    BoneData*                   getBoneData(i32 meshId) { auto it = m_boneMap.find(meshId); if (it != m_boneMap.end()) return &it->second; }

    // File format.
    FileFormat                  getFileFormat() const { return m_fileFormat; }
    // Get extension.
    const char*                 getExtension() const { return m_ext; }

protected:
    struct MeshDataInfo
    {
        u32 index;
    };
    std::map<RGUID, MeshDataInfo, RGUID::Less>          m_dataMap;
    std::map<i32, BoneData>                             m_boneMap;
    std::vector<MeshBuilder::MeshData>                  m_data;

    // Simplify the mesh when possible.
    void                        performSimplify();
    // Optimize the mesh where possible.
    void                        performOptimize();

private:
    FileFormat  m_fileFormat;
    const char* m_ext;
};
} // Builder
} // Pipeline
} // Recluse