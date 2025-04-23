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
namespace Builder {
namespace FBX {

// FBX Scene import. Handles and destroys the FBX scene tree when importing.
// 
class FbxImport : public Pipeline::Builder::Importer
{
public:
    // The channel for debugging.
    static constexpr const char* FbxChannel = "FBX";

    typedef std::vector<FbxNodeAttribute::EType> ElementTypes;
    typedef std::function<ResultCode(FbxNode* node)> FbxProcessFunction;

    static Pipeline::Builder::Importer*     create();
    static ResultCode                       destroy(Pipeline::Builder::Importer* importer);

    // Process description, is used to describe the process for specific nodes with the given
    // EType list.
    struct ProcessDescription
    {
        ElementTypes        types;
        FbxProcessFunction  func;
    };

    // Process block used while traversing the FBX scene tree.
    // ETypes are specified as to what, or how, a node should be processed.
    // More than one process function can handle a node.
    class Process
    {
    public:
        typedef std::map<FbxNodeAttribute::EType, std::vector<FbxProcessFunction>> ProcessMap;

        Bool contains(FbxNodeAttribute::EType etype) const
        {
            auto it = m_processMap.find(etype);
            return (it != m_processMap.end());
        }

        const std::vector<FbxProcessFunction>& get(FbxNodeAttribute::EType etype) const
        {
            auto it = m_processMap.find(etype);
            if (it == m_processMap.end())
                return { };
            return it->second;
        }

        const std::vector<FbxProcessFunction>& operator[](FbxNodeAttribute::EType etype) const
        {
            return get(etype);
        }

        Process& operator()(const ProcessDescription& processDesc) 
        {
            return add(processDesc);
        }

        Process& add(const ProcessDescription& processDesc)
        {
            for (auto etype : processDesc.types)
                m_processMap[etype].push_back(processDesc.func);
            return *this;
        }
    private:
        ProcessMap m_processMap;
    };

    FbxImport();

    virtual ~FbxImport();

    ResultCode importFile(const std::string& filePath) override;
    ResultCode traverseScene(FbxNode* parentNode, const Process& processes);
    ResultCode processNode(FbxNode* node, const Process& processes);

    FbxNode*   getRootNode() const { return m_scene->GetRootNode(); }

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

    // FBX Scene manager.
    FbxManager*     m_manager;

    // The FBX importer object.
    FbxImporter*    m_importer;

    // The FBX scene itself.
    FbxScene*       m_scene;

    // Not sure I want this.
    std::vector<LightDescription> lights;
};
} // FBX
} // Builder
} // Pipeline
} // Recluse