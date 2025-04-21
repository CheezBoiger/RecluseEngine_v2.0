//
#include "FBXImporter.hpp"
#include "FBXMeshBuilder.hpp"
#include "Recluse/Messaging.hpp"

#include "fbxsdk.h"

namespace Recluse {
namespace Pipeline {
namespace Builder {
namespace FBX {

ResultCode FbxMeshBuilder::build(Builder::Importer* importer, MeshBuilderFlags flags)
{
    R_ASSERT_FORMAT(importer, "Importer is nullptr!");
    R_ASSERT_FORMAT(importer->getFormat() == Builder::FileFormat_FBX, "Importer is not using Fbx format handle!!!");

    FbxImport* fbximporter = static_cast<FbxImport*>(importer);
    
    FbxImport::Process processes;

    processes({ { FbxNodeAttribute::eMesh }, [&] (FbxNode* node) -> ResultCode 
    {
        FbxMesh* meshNode = (FbxMesh*) node->GetNodeAttribute();

        const i32 numPolygons = meshNode->GetPolygonCount();

        // Fail if there are no polygons to fetch.
        if (numPolygons == 0)
            return RecluseResult_Failed;

        i32 vertexId = 0;
        // Control points is another name for "vertex", with geometric information
        // of each point in our mesh.
        FbxVector4* controlPoints   = meshNode->GetControlPoints();
        int* vertexIndices          = meshNode->GetPolygonVertices();

        Data meshData       = { };
        meshData.name       = meshNode->GetName();
        meshData.guid       = generateRGUID();

        meshData.positions.resize(meshNode->GetControlPointsCount());
        meshData.vertexIndices.resize(meshNode->GetPolygonVertexCount());
        meshData.submeshes.resize(numPolygons);

        // Extract our materials.
        extractMaterials(meshData, node);

        for (i32 polygonIdx = 0; polygonIdx < numPolygons; ++polygonIdx)
        {
            for (i32 i = 0 ; i < meshNode->GetElementPolygonGroupCount(); ++i)
            {
            }

            const i32 polygonSize       = meshNode->GetPolygonSize(polygonIdx);
            const i32 startVertexIndex  = meshNode->GetPolygonVertexIndex(polygonIdx);

            // We are in one polygon, so one submesh.
            Engine::SubMesh submesh = { };

            submesh.offsetElements = vertexId;          // Starting vertex id.
            submesh.rangeElements = 0;                  // This is the range

            for (i32 polygonSizeIdx = 0; polygonSizeIdx < polygonSize; ++polygonSizeIdx)
            {
                i32 controlPointIdx = meshNode->GetPolygonVertex(polygonIdx, polygonSizeIdx);

                if (controlPointIdx < 0)
                {
                    R_WARN(FbxImport::FbxChannel, "Invalid control point index found for this mesh. Skipping...");
                    continue;
                }

                // Obtain the vertex coordinate, position.
                {
                    const FbxVector4& coordinates       = controlPoints[controlPointIdx];
                    const i32 vertexIndex               = vertexIndices[startVertexIndex + polygonSizeIdx];       

                    Math::Float3 positionCoords         = Math::Float3(coordinates[0], coordinates[1], coordinates[2]);

                    meshData.positions[vertexId]        = positionCoords;
                    meshData.vertexIndices[vertexId]    = vertexIndex;
                }
                // Increment the vertex counter.
                submesh.rangeElements += 1;
                ++vertexId;
            }

            // End of the polygon, we will store this submesh.
            if (submesh.rangeElements != 0)
            {
                meshData.submeshes[polygonIdx] = submesh;
            }
        }

        // Push the meshdata back.
        m_data.push_back(meshData);        

        return RecluseResult_Ok;
    }});
    // Traverse the scene, and build our mesh nodes.
    ResultCode result = fbximporter->traverseScene(fbximporter->getRootNode(), processes);
    return result;
}


ResultCode FbxMeshBuilder::extractMaterials(MeshBuilder::Data& meshData, FbxNode* node)
{
    
    return RecluseResult_Ok;
}
} // FBX
} // Builder
} // Pipeline
} // Recluse