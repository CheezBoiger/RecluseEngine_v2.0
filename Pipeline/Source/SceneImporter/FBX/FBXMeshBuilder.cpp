//
#include "FBXImporter.hpp"
#include "FBXMeshBuilder.hpp"
#include "Recluse/Messaging.hpp"

#include "fbxsdk.h"

namespace Recluse {
namespace Pipeline {
namespace Builder {
namespace FBX {


Builder::MeshBuilder* FbxMeshBuilder::create()
{
    return new FbxMeshBuilder();
}


ResultCode FbxMeshBuilder::destroy(Builder::MeshBuilder* meshBuilder)
{
    if (!meshBuilder)
        return RecluseResult_NullPtrExcept;
    
    delete meshBuilder;

    return RecluseResult_Ok;
}


ResultCode FbxMeshBuilder::onBuild(Builder::Importer* importer, MeshBuilderFlags flags)
{
    R_ASSERT_FORMAT(importer, "Importer is nullptr!");
    R_ASSERT_FORMAT(importer->getFormat() == Builder::FileFormat_FBX, "Importer is not using Fbx format handle!!!");

    FbxImport* fbximporter = static_cast<FbxImport*>(importer);
    
    FbxImport::Process processes;

    // TODO: This needs to handle polygons with more than 3 edges, this means
    // Find a way to triangulate!!
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

        MeshData meshData   = { };
        meshData.name       = meshNode->GetName();
        meshData.guid       = generateRGUID();

        meshData.resizeAttributes(meshNode->GetControlPointsCount());
        meshData.resizeIndices(meshNode->GetPolygonVertexCount());

        // Make it indexed.
        if (meshNode->GetPolygonVertexCount() > 0)
            meshData.flags |= MeshData::Indexed;

        // Insert this node into the nodeGuids structure.
        nodeGuids.insert(std::make_pair(node, meshData.guid));

        // Extract our materials.
        extractMaterials(meshData, node);

        // Load in the vertices to the positions struct first.
        for (uint vertId = 0; vertId < meshNode->GetControlPointsCount(); ++vertId)
        {
            const FbxVector4& coordinates   = controlPoints[vertId];
            Math::Float3 positionCoords     = Math::Float3(coordinates[0], coordinates[1], coordinates[2]);
            meshData.positions[vertId]      = positionCoords;
        }

        // Iterate through each polygon now.
        for (i32 polygonIdx = 0; polygonIdx < numPolygons; ++polygonIdx)
        {
            for (i32 i = 0 ; i < meshNode->GetElementPolygonGroupCount(); ++i)
            {
            }
            
            const i32 polygonSize       = meshNode->GetPolygonSize(polygonIdx);
            const i32 startVertexIndex  = meshNode->GetPolygonVertexIndex(polygonIdx);

            for (i32 polygonSizeIdx = 0; polygonSizeIdx < polygonSize; ++polygonSizeIdx)
            {
                i32 controlPointIdx = meshNode->GetPolygonVertex(polygonIdx, polygonSizeIdx);

                if (controlPointIdx < 0)
                {
                    R_WARN(FbxImport::FbxChannel, "Invalid control point index found for this mesh. Skipping...");
                    continue;
                }

                // Obtain the vertex coordinate, position.
                const i32 vertexIndex               = vertexIndices[startVertexIndex + polygonSizeIdx];       
                meshData.vertexIndices[vertexId]    = vertexIndex;

                for (uint l = 0; l < meshNode->GetElementUVCount(); ++l)
                {
                    FbxGeometryElementUV* elementUv = meshNode->GetElementUV(l);
                    switch (elementUv->GetMappingMode())
                    {
                        case FbxGeometryElement::eByControlPoint:
                            {
                                switch (elementUv->GetReferenceMode())
                                {
                                    case FbxGeometryElement::eDirect:
                                        {
                                            const FbxVector2& UV = elementUv->GetDirectArray().GetAt(controlPointIdx);
                                            Math::Float2 uv = Math::Float2( UV[0], UV[1] );
                                            meshData.uvs[controlPointIdx] = uv;
                                            break;
                                        }
                                    case FbxGeometryElement::eIndexToDirect:
                                        {
                                            i32 id = elementUv->GetIndexArray().GetAt(controlPointIdx);
                                            const FbxVector2& UV = elementUv->GetDirectArray().GetAt(id);
                                            Math::Float2 uv = Math::Float2( UV[0], UV[1] );
                                            meshData.uvs[controlPointIdx] = uv;
                                            break;
                                        }
                                    default:
                                        break;
                                }
                                break;
                            }

                        case FbxGeometryElement::eByPolygonVertex:
                            {
                                i32 uvIdx = meshNode->GetTextureUVIndex(polygonIdx, polygonSizeIdx);
                                switch (elementUv->GetReferenceMode())
                                {
                                    case FbxGeometryElement::eDirect:
                                    case FbxGeometryElement::eIndexToDirect:
                                        {
                                            const FbxVector2& UV = elementUv->GetDirectArray().GetAt(uvIdx);
                                            Math::Float2 uv = Math::Float2(UV[0], UV[1]);
                                            meshData.uvs[controlPointIdx] = uv;
                                            break;
                                        }
                                    default:
                                        break;
                                }
                                break;
                            }
                        default:
                            break;
                    }
                }

                for (uint l = 0; l < meshNode->GetElementNormalCount(); ++l)
                {
                    FbxGeometryElementNormal* elementNormal = meshNode->GetElementNormal(l);
                    if (elementNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                    {
                        switch (elementNormal->GetReferenceMode())
                        {
                            case FbxGeometryElement::eDirect:
                                {
                                    const FbxVector4& n = elementNormal->GetDirectArray().GetAt(vertexIndex);
                                    Math::Float3 normal = { n[0], n[1], n[2] };
                                    meshData.normals[vertexIndex] = normal;
                                    break;
                                }
                            case FbxGeometryElement::eIndexToDirect:
                                {
                                    i32 id = elementNormal->GetIndexArray().GetAt(vertexId);
                                    const FbxVector4& n = elementNormal->GetDirectArray().GetAt(id);
                                    Math::Float3 normal = { n[0], n[1], n[2] };
                                    meshData.normals[id] = normal;
                                    break;
                                }
                            default:
                                break;
                        }
                    }
                }


                // Increment the vertex counter.
                ++vertexId;
            }

            if (flags & Triangulate)
            {
                // Triangulate the polygon that we just stored.
                // TODO: Try Bowyer-Watson Algorithm? https://en.wikipedia.org/wiki/Bowyer%E2%80%93Watson_algorithm
                //
                R_NO_IMPL();
            }
            else
                R_ASSERT_FORMAT(polygonSize == 3, "Polygon size is not 3! This indicates it is not a triangle, which will need to be triangulated!");
        }

        // Find the parent, if one exists.
        auto parentIt = nodeGuids.find(node->GetParent());
        if (parentIt != nodeGuids.end())
            meshData.parent = parentIt->second;

        // Push the meshdata back.
        m_data.push_back(meshData);

        return RecluseResult_Ok;
    }});
    // Traverse the scene, and build our mesh nodes.
    ResultCode result = fbximporter->traverseScene(fbximporter->getRootNode(), processes);

    return result;
}


ResultCode FbxMeshBuilder::extractMaterials(MeshBuilder::MeshData& meshData, FbxNode* node)
{
    
    return RecluseResult_Ok;
}


ResultCode FbxMeshBuilder::serialize(Archive* archive) const 
{
    return RecluseResult_NoImpl;
}


ResultCode FbxMeshBuilder::deserialize(Archive* archive) 
{
    return RecluseResult_NoImpl;
}


std::vector<Math::Float3> FbxMeshBuilder::triangulatePolygon(const std::vector<Math::Float3>& pointList)
{
    return { };
}
} // FBX
} // Builder
} // Pipeline
} // Recluse