//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Serializable.hpp"
#include "Recluse/Renderer/Mesh.hpp"
#include "Recluse/RGUID.hpp"
#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Math/Bounds3D.hpp"
#include "Recluse/Pipeline/Importer.hpp"
#include "Recluse/Pipeline/Material.hpp"

#include "ReclusePipeline_exports.hpp"

#include <vector>

namespace Recluse {
namespace Pipeline {
namespace Builder {

struct MeshletMetadata
{
    U32 maxVerticesPerMeshlet;
    U32 maxPrimitivesPerMeshlet;
    F32 coneWeight;
};


// Mesh Builder helps in building out meshes and materials,
// and holds that data for use later.
class ReclusePipeline_PUBLIC_API MeshBuilder
{
public:
    // 
    enum 
    {
        None                = 0,
        Optimize            = (1<<0),
        Simplify            = (1<<1),
        Optimize_Aggressive = (1<<2),

        // Generate meshlets for clustered rendering.
        GenerateMeshlets    = (1<<3),
        
        // Generate Level of Detail meshes. These are stored inside the meshdata map.
        GenerateLods        = (1<<4),

        // Triangulate the mesh, if there are polygons that are more than 3 vertices.
        Triangulate         = (1<<5)
    };
    typedef U32 MeshBuilderFlags;
    typedef I32 BoneId;

    static const U32 kDefaultVerticesPerMeshlet     = 64;
    static const U32 kDefaultPrimitivesPerMeshlet   = 128;

    // Info for constructed meshlet.
    struct Meshlet
    {
        U32                 vertexCount;
        U32                 vertexOffset;
        
        U32                 primCount;
        U32                 primOffset;

        Math::BoundsSphere  bounds;
        
        Math::Float3        coneApex;
        Math::Float3        coneAxis;
        F32                 coneCutoff;
    };

    struct SubMesh
    {
        uint                materialId;
        uint                offsetElements;
        uint                rangeElements;
        Math::Bounds3d      bounds;
    };

    // Mesh information.
    struct MeshData
    {
        enum 
        { 
            None        = 0,
            Indexed     = (1 << 0), 
            Rigid       = (1 << 1), 
            Skinned     = (1 << 2) 
        };
        typedef U32 MeshAttributeFlags;
        
        static const u32 kInvalidBoneId     = ~0;
        static const u32 kInvalidLodId      = ~0;
        static const u32 kInvalidMaterialId = ~0;

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
        BoneId                                  boneId      = kInvalidBoneId;
        u32                                     lodId       = kInvalidLodId;
        
        // Global material id for this mesh.
        u32                                     materialId  = kInvalidMaterialId;

        // Atribute flags that define any special cases of the mesh.
        MeshAttributeFlags                      flags       = None;

        // Vertex indices, if the mesh is indexed.
        std::vector<U32>                        vertexIndices;

        // Materials within this mesh object. Usually this will only house null materials after export, as these will need to be 
        // manually set up in the engine editor.
        std::map<u32, Material*>                materials;

        // Submesh info. This is built per material.
        std::vector<MeshBuilder::SubMesh>       submeshes;

        // Resize the mesh data attributes.
        void resizeAttributes(U32 newSize);

        // Resize the indices.
        void resizeIndices(U32 newSize);
    };
    
    struct BoneData
    {
        std::vector<Math::Float4>               boneWeights;
        std::vector<Math::UInt4>                boneIndices;

        void resize(U32 newSize);
    };

    struct MeshletData
    {
        std::vector<Meshlet>                    meshlets;
        std::vector<U32>                        primitives;
        std::vector<U32>                        vertices;
    };

    struct LodData
    {
        // Each index in the vector corresponds to an lod guid, which the mesh itself is stored in the meshmap.
        std::vector<RGUID>                      lods;
    };

    struct MaterialProperties
    {
        struct Parameter
        {
            Material::data_type dataType;
            const char* name;
        };

        std::vector<Parameter> parameters;
    };

    // Create the mesh builder.
    static MeshBuilder*         create(FileFormat format);

    // Destroy the mesh builder.
    static ResultCode           destroy(MeshBuilder* builder);

                                MeshBuilder(FileFormat fileFormat, const char* ext);

    virtual                     ~MeshBuilder() { }

    ResultCode                  build(Importer* importer, MeshBuilderFlags flags = MeshBuilder::None, const MaterialProperties& properties={});

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
    BoneData*                   getBoneData(BoneId boneId) { auto it = m_boneMap.find(boneId); return (it != m_boneMap.end()) ? &it->second : nullptr; }
    MeshletData*                getMeshletData(const RGUID& meshId) { auto it = m_meshletMap.find(meshId); return (it != m_meshletMap.end()) ? &it->second : nullptr; }
    LodData*                    getLodData(i32 lodId) { auto it = m_lodMap.find(lodId); return (it != m_lodMap.end()) ? &it->second : nullptr; }

    // File format.
    FileFormat                  getFileFormat() const { return m_fileFormat; }
    // Get extension.
    const char*                 getExtension() const { return m_ext; }

    void                        setMeshletMetadata(const MeshletMetadata& metadata) { m_meshletMetadata = metadata; }
    MeshletMetadata             getMeshletMetadata() const { return m_meshletMetadata; }

protected:

    virtual ResultCode          onBuild(Importer* importer, const MaterialProperties& properties, MeshBuilderFlags flags) = 0;

    struct MeshDataInfo
    {
        u32 index;
    };

    std::map<RGUID, MeshDataInfo, RGUID::Less>          m_dataMap;
    std::map<BoneId, BoneData, RGUID::Less>             m_boneMap;
    std::map<u32, LodData>                              m_lodMap;
    std::map<u32, Material*>                            m_matMap;
    std::map<RGUID, MeshletData, RGUID::Less>           m_meshletMap;

    // The actual meshes.
    std::vector<MeshBuilder::MeshData>                  m_data;

    // Simplify the mesh when possible.
    void                        performSimplify(MeshData& meshData);
    // Optimize the mesh where possible.
    template<typename Vertex, typename VertexEncoder, typename VertexDecoder>
    void                        performOptimize(MeshData& meshData);

    // Generate meshlets for this mesh data.
    void                        generateMeshlets(MeshData& meshData);

private:
    FileFormat      m_fileFormat;
    const char*     m_ext;

    MeshletMetadata m_meshletMetadata;
};


// MeshBuilder to Engine exporter handles exporting the mesh builder content to something engine readable 
// 
class MeshBuilderToEngineExporter : public Serializable
{
public:
    MeshBuilderToEngineExporter(const MeshBuilder& builder)
        : meshBuilder(builder) { }

    virtual ResultCode          serialize(Archive* archive) const override { return RecluseResult_NoImpl; }
    virtual ResultCode          deserialize(Archive* archive) override { return RecluseResult_NoImpl; }
private:
    const MeshBuilder& meshBuilder;
};
} // Builder
} // Pipeline
} // Recluse