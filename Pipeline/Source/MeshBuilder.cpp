//
#include "Recluse/Pipeline/MeshBuilder.hpp"

#include "SceneImporter/FBX/FBXMeshBuilder.hpp"

#include <meshoptimizer.h>

namespace Recluse {
namespace Pipeline {
namespace Builder {


struct RigidVertex
{
    Math::Float3 position;
    Math::Float3 normal;
    Math::Float4 uvs;
    Math::Float3 binormal;
    Math::Float3 tangent;
};


class RigidVertexEncoder
{
public:
    std::vector<RigidVertex> operator()(MeshBuilder& builder, const MeshBuilder::MeshData& meshData)
    {
        std::vector<RigidVertex> dest(meshData.positions.size());
        for (U32 i = 0; i < dest.size(); ++i)
        {
            RigidVertex v{};
            v.position  = meshData.positions[i];
            v.normal    = meshData.normals[i];
            v.uvs       = meshData.uvs[i];
            v.binormal  = meshData.binormals[i];
            v.tangent   = meshData.tangents[i];
            dest[i] = v;
        }
        return dest;
    }
};


class RigidVertexDecoder
{
public:
    bool operator()(MeshBuilder& builder, MeshBuilder::MeshData& meshData, const std::vector<RigidVertex>& srcVertices, const std::vector<U32>& srcIndices)
    {
        // Final values should reinitialize the meshData.
        meshData.resizeAttributes(srcVertices.size());
        meshData.resizeIndices(srcIndices.size());
    
        for (U32 i = 0; i < srcVertices.size(); ++i)
        {
            meshData.positions[i]   = srcVertices[i].position;
            meshData.normals[i]     = srcVertices[i].normal;
            meshData.uvs[i]         = srcVertices[i].uvs;
            meshData.binormals[i]   = srcVertices[i].binormal;
            meshData.tangents[i]    = srcVertices[i].tangent;
        }

        for (U32 i = 0; i < srcIndices.size(); ++i)
        {
            meshData.vertexIndices[i] = srcIndices[i];
        }

        return true;
    }
};


struct SkinnedVertex
{
    Math::Float3 position;
    Math::Float3 normal;
    Math::Float4 uvs;
    Math::Float3 binormal;
    Math::Float3 tangent;
    Math::Float4 boneWeight;
    Math::UInt4  boneIndices;
};


class SkinnedVertexEncoder
{
public:
    std::vector<SkinnedVertex> operator()(MeshBuilder& builder, const MeshBuilder::MeshData& meshData)
    {
        std::vector<SkinnedVertex> dest(meshData.positions.size());
        MeshBuilder::BoneData* boneData = builder.getBoneData(meshData.boneId);

        R_ASSERT(boneData);

        for (U32 i = 0; i < dest.size(); ++i)
        {
            SkinnedVertex v{};
            v.position  = meshData.positions[i];
            v.normal    = meshData.normals[i];
            v.uvs       = meshData.uvs[i];
            v.binormal  = meshData.binormals[i];
            v.tangent   = meshData.tangents[i];
            v.boneWeight = boneData->boneWeights[i];
            v.boneIndices = boneData->boneIndices[i]; 
            dest[i] = v;
        }
        return dest;
    }
};


class SkinnedVertexDecoder
{
public:
    bool operator()(MeshBuilder& builder, MeshBuilder::MeshData& meshData, const std::vector<SkinnedVertex>& srcVertices, const std::vector<U32>& srcIndices)
    {
        // Final values should reinitialize the meshData.
        meshData.resizeAttributes(srcVertices.size());
        meshData.resizeIndices(srcIndices.size());
        
        MeshBuilder::BoneData* boneData = builder.getBoneData(meshData.boneId);
        R_ASSERT(boneData);
    
        for (U32 i = 0; i < srcVertices.size(); ++i)
        {
            meshData.positions[i]   = srcVertices[i].position;
            meshData.normals[i]     = srcVertices[i].normal;
            meshData.uvs[i]         = srcVertices[i].uvs;
            meshData.binormals[i]   = srcVertices[i].binormal;
            meshData.tangents[i]    = srcVertices[i].tangent;
            boneData->boneWeights[i] = srcVertices[i].boneWeight;
            boneData->boneIndices[i] = srcVertices[i].boneIndices;
        }

        for (U32 i = 0; i < srcIndices.size(); ++i)
        {
            meshData.vertexIndices[i] = srcIndices[i];
        }

        return true;
    }
};


void MeshBuilder::MeshData::resizeAttributes(U32 newSize)
{
    positions.resize(newSize);
    normals.resize(newSize);
    uvs.resize(newSize);
    binormals.resize(newSize);
    tangents.resize(newSize);
}


void MeshBuilder::MeshData::resizeIndices(U32 newSize)
{
    vertexIndices.resize(newSize);
}


void MeshBuilder::BoneData::resize(U32 newSize)
{
    boneWeights.resize(newSize);
    boneIndices.resize(newSize);
}


MeshBuilder* MeshBuilder::create(FileFormat fileFormat)
{
    MeshBuilder* meshBuilder = nullptr;

    switch (fileFormat)
    {
        case FileFormat_GLTF:
            break;

        case FileFormat_FBX:
        default:
            meshBuilder = FBX::FbxMeshBuilder::create();
            break;
    }

    return meshBuilder;
}


ResultCode MeshBuilder::destroy(MeshBuilder* builder)
{
    if (!builder)
        return RecluseResult_NullPtrExcept;

    ResultCode result = RecluseResult_Failed;

    switch (builder->getFileFormat())
    {
        case FileFormat_GLTF:
            break;
        
        case FileFormat_FBX:
        default:
            result = FBX::FbxMeshBuilder::destroy(builder);
            break;
    }

    return result;
}

MeshBuilder::MeshBuilder(FileFormat fileFormat, const char* ext)
    : m_fileFormat(fileFormat)
    , m_ext(ext)
{
    m_meshletMetadata.coneWeight                = 0.0f; 
    m_meshletMetadata.maxPrimitivesPerMeshlet   = kDefaultPrimitivesPerMeshlet; 
    m_meshletMetadata.maxVerticesPerMeshlet     = kDefaultVerticesPerMeshlet; 
}


void MeshBuilder::performSimplify(MeshData& meshData)
{
}


void MeshBuilder::performQuantize(MeshData& meshData)
{
}


template<typename Vertex, typename VertexEncoder, typename VertexDecoder>
void MeshBuilder::performOptimize(MeshData& meshData)
{
    R_INFO("Mesh Builder", "Optimizing mesh...");
    size_t indexCount               = meshData.vertexIndices.size();
    size_t unindexedVertexCount     = meshData.positions.size();
    U32* inputIndices               = (meshData.flags & MeshData::Indexed) ? meshData.vertexIndices.data() : nullptr;

    std::vector<U32> remap(unindexedVertexCount);

    VertexEncoder encoder;
    VertexDecoder decoder;

    std::vector<Vertex> unindexedVertices = encoder(*this, meshData);
    
    // Start with the indexing.
    size_t vertexCount = meshopt_generateVertexRemap(remap.data(), inputIndices, indexCount, unindexedVertices.data(), unindexedVertexCount, sizeof(Vertex));

    std::vector<U32> optimizedIndices(indexCount);
    std::vector<Vertex> optimizedVertices(vertexCount);
    
    meshopt_remapIndexBuffer(optimizedIndices.data(), inputIndices, indexCount, remap.data());
    meshopt_remapVertexBuffer(optimizedVertices.data(), unindexedVertices.data(), unindexedVertexCount, sizeof(Vertex), remap.data());

    // Cache optimiztion.
    meshopt_optimizeVertexCache(optimizedIndices.data(), optimizedIndices.data(), indexCount, vertexCount);

    // Overdraw optimization.
    meshopt_optimizeOverdraw(optimizedIndices.data(), optimizedIndices.data(), indexCount, &optimizedVertices[0].position.x, vertexCount, sizeof(Vertex), 1.05f);
    
    // Vertex fetch optimization.
    meshopt_optimizeVertexFetch(optimizedVertices.data(), optimizedIndices.data(), indexCount, optimizedVertices.data(), vertexCount, sizeof(Vertex));

    // Final values should reinitialize the meshData.
    decoder(*this, meshData, optimizedVertices, optimizedIndices);

    R_INFO("Mesh Builder", "Mesh optimization complete!");
}


ResultCode MeshBuilder::build(Importer* importer, MeshBuilderFlags flags)
{
    ResultCode built = onBuild(importer, flags);
    if (built == RecluseResult_Ok)
    {
        if (flags & Optimize)
        {
            for (auto& meshData : m_data)
            {
                if (meshData.flags & MeshData::Skinned)
                    performOptimize<SkinnedVertex, SkinnedVertexEncoder, SkinnedVertexDecoder>(meshData);
                else
                {
                    performOptimize<RigidVertex, RigidVertexEncoder, RigidVertexDecoder>(meshData);
                }
            }
        }

        if (flags & Quantize)
        {
            for (auto& meshData : m_data)
                performQuantize(meshData);
        }

        if (flags & GenerateMeshlets)
        {
            for (auto& meshData : m_data)
                generateMeshlets(meshData);
        }
    }
    return built;
}


void MeshBuilder::generateMeshlets(MeshData& meshData)
{
    R_INFO("Mesh Builder", "Generating Meshlets...");
    const size_t maxVerticesPerMeshlet      = m_meshletMetadata.maxVerticesPerMeshlet;
    const size_t maxPrimitivesPerMeshlet    = m_meshletMetadata.maxPrimitivesPerMeshlet;
    const F32 coneWeight                    = m_meshletMetadata.coneWeight;

    const size_t maxMeshlets                = meshopt_buildMeshletsBound(meshData.vertexIndices.size(), maxVerticesPerMeshlet, maxPrimitivesPerMeshlet);
    
    std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
    std::vector<U32> meshletVertices(maxMeshlets * maxVerticesPerMeshlet);
    std::vector<U8> meshletPrimitives(maxMeshlets * maxPrimitivesPerMeshlet * 3);
    
    size_t meshletCount = meshopt_buildMeshlets(
        meshlets.data(),
        meshletVertices.data(),
        meshletPrimitives.data(),
        meshData.vertexIndices.data(),
        meshData.vertexIndices.size(),
        &meshData.positions[0].x,
        meshData.positions.size(),
        sizeof(Math::Float3),
        maxVerticesPerMeshlet,
        maxPrimitivesPerMeshlet,
        coneWeight);

    // Trim the meshlets.
    const meshopt_Meshlet& lastMeshlet = meshlets[meshletCount - 1];
    meshletVertices.resize(lastMeshlet.vertex_offset + lastMeshlet.vertex_count);
    meshletPrimitives.resize(lastMeshlet.triangle_offset + ((lastMeshlet.triangle_count * 3 + 3) & ~3));
    meshlets.resize(meshletCount);
    
    MeshletData meshletData{};

    for (meshopt_Meshlet& meshlet : meshlets)
    {
        meshopt_optimizeMeshlet(&meshletVertices[meshlet.vertex_offset], &meshletPrimitives[meshlet.triangle_offset], meshlet.triangle_count, meshlet.vertex_count);
        meshopt_Bounds meshletBounds = meshopt_computeMeshletBounds(&meshletVertices[meshlet.vertex_offset], 
            &meshletPrimitives[meshlet.triangle_offset], 
            meshlet.triangle_count, 
            &meshData.positions[0].x, 
            meshData.positions.size(), 
            sizeof(Math::Float3));

        Meshlet meshletOutput{};

        // Load the meshlet information.
        meshletOutput.primOffset    = meshlet.triangle_offset;
        meshletOutput.primCount     = meshlet.triangle_count;

        meshletOutput.vertexOffset  = meshlet.vertex_offset;
        meshletOutput.vertexCount   = meshlet.vertex_count;

        // Cluster cone culling info for backface culling.
        meshletOutput.coneApex      = Math::Float3(meshletBounds.cone_apex[0], meshletBounds.cone_apex[1], meshletBounds.cone_apex[2]);
        meshletOutput.coneAxis      = Math::Float3(meshletBounds.cone_axis[0], meshletBounds.cone_axis[1], meshletBounds.cone_axis[2]);
        meshletOutput.coneCutoff    = meshletBounds.cone_cutoff;

        // Bounds for frustum culling.
        meshletOutput.bounds        = Math::BoundsSphere{ Math::Float3(meshletBounds.center[0], meshletBounds.center[1], meshletBounds.center[2]), meshletBounds.radius };

        // We pack our triangle indices into a U32 bit value.
        const U32 triangleOffset = static_cast<U32>(meshletData.primitives.size());
        for (U32 i = 0; i < meshlet.triangle_count; ++i)
        {
            U32 i0 = 3 * i + 0 + meshlet.triangle_offset;
            U32 i1 = 3 * i + 1 + meshlet.triangle_offset;
            U32 i2 = 3 * i + 2 + meshlet.triangle_offset;
            U8 vIdx0 = meshletPrimitives[i0];
            U8 vIdx1 = meshletPrimitives[i1];
            U8 vIdx2 = meshletPrimitives[i2];
            U32 packed =    ((static_cast<U32>(vIdx0) & 0xFF) << 0) |
                            ((static_cast<U32>(vIdx1) & 0xFF) << 8) |
                            ((static_cast<U32>(vIdx2) & 0xFF) << 16);
            meshletData.primitives.push_back(packed);
        }

        // Update the primitive offset of the meshlet, as it is now packing 3 triangle indices into one U32.
        meshletOutput.primOffset = triangleOffset;

        meshletData.meshlets.push_back(meshletOutput);
    }

    // Store the meshlet information.
    m_meshletMap[meshData.guid] = meshletData;
    R_INFO("Mesh Builder", "Meshlet Generation Complete!");
}
} // Builder
} // Pipeline
} // Recluse