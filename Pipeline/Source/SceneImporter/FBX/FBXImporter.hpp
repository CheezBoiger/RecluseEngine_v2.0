//
#pragma once

#include "Recluse/Pipeline/Importer.hpp"
#include "Recluse/Generated/Common/Common.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"

#include "fbxsdk.h"

#include <vector>
#include <map>

namespace Recluse {
namespace Pipeline {
namespace FBX {

class FbxImport : public Pipeline::Builder::Importer
{
public:
    const char* FbxChannel = "FBX";
    typedef std::vector<FbxNodeAttribute::EType> ElementTypes;
    typedef std::function<ResultCode(FbxNode* node)> FbxProcessFunction;

    FbxImport();

    virtual ~FbxImport();

    ResultCode importFile(const std::string& filePath) override;
    ResultCode traverseScene(FbxNode* parentNode, const ElementTypes& types, FbxProcessFunction& func);
    ResultCode processNode(FbxNode* node, const ElementTypes& types, FbxProcessFunction& func);

    ResultCode processMesh(FbxNode* node)
    {
        FbxMesh* meshNode = (FbxMesh*) node->GetNodeAttribute();

        const i32 numPolygons = meshNode->GetPolygonCount();
        i32 vertexId = 0;
        // Control points is another name for "vertex", with geometric information
        // of each point in our mesh.
        FbxVector4* controlPoints = meshNode->GetControlPoints();

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

                i32 vertexIndex = meshNode->GetPolygonVertices()[startVertexIndex];
                if (controlPointIdx < 0)
                {
                    R_WARN(FbxChannel, "Invalid control point index found for this mesh. Skipping...");
                    continue;
                }

                // Obtain the vertex coordinate, position.
                const FbxVector4& coordinates = controlPoints[controlPointIdx];
                
                // Increment the vertex counter.
                ++vertexId;
            }
        }
        return RecluseResult_NoImpl;
    }

    ResultCode processLight(FbxNode* node)
    {
        FbxLight* light = (FbxLight*) node->GetNodeAttribute();
        FbxLight::EType lightType = light->LightType;
        LightDescription description = { };

        switch (lightType)
        {
            case FbxLight::eArea:
            {   
                description.lightType = LightType_Area;
                break;
            }
            case FbxLight::eDirectional:
            {
                description.lightType = LightType_Directional;
                break;
            }
            case FbxLight::ePoint:
            {
                description.lightType       = LightType_Point;
                FbxVector4 v4               = node->LclTranslation.Get();
                Math::Float3 position       = Math::Float3(v4[0], v4[1], v4[2]);
                description.point.position  = position;
                light->FarAttenuationStart;
                break;
            }
            case FbxLight::eSpot:
            {
                description.lightType       = LightType_Spot;
                FbxDouble innerCone         = light->InnerAngle;
                FbxDouble outerCone         = light->OuterAngle;
                FbxDouble3 pos              = node->LclTranslation;
                FbxDouble3 rot;
                description.spot.position   = Math::Float3(pos[0], pos[1], pos[2]);
                description.spot.outerCone  = outerCone;
                description.spot.innerCone  = innerCone;
            }
            case FbxLight::eVolume:
            default: break;
        }

        const FbxBool hasShadows        = light->CastShadows;
        const FbxBool castLight         = light->CastLight;
        const FbxDouble3 color          = light->Color;
        const FbxDouble intensity       = light->Intensity;

        description.shadowed            = hasShadows;
        description.enable              = castLight;
        description.attenuation         = intensity;
        description.color               = Math::Float3(color[0], color[1], color[2]);
        lights.push_back(description);
        return RecluseResult_NoImpl;
    }

private:
    FbxManager*     m_manager;
    FbxImporter*    m_importer;
    FbxScene*       m_scene;

    std::vector<LightDescription> lights;
};
} // FBX
} // Pipeline
} // Recluse