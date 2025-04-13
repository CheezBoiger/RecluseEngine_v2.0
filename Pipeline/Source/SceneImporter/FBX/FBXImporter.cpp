//
#include "Recluse/Pipeline/Importer.hpp"


#include <queue>

#include "fbxsdk.h"
#include "FBXImporter.hpp"

namespace Recluse {
namespace Pipeline {
namespace FBX {


FbxImport::FbxImport()
    : Importer(Pipeline::Builder::FileFormat_FBX)
    , m_manager(nullptr)
    , m_importer(nullptr)
{
    m_manager = FbxManager::Create();
    R_ASSERT_FORMAT(m_manager, "Fbx Manager failed to create");
    m_importer = FbxImporter::Create(m_manager, ""); 
}


FbxImport::~FbxImport()
{
    if (m_importer) m_importer->Destroy();
    if (m_manager) m_manager->Destroy(); 

    m_importer = nullptr;
    m_manager = nullptr;
}


ResultCode FbxImport::importFile(const std::string& filePath)
{
    R_ASSERT(m_importer);
    bool success = m_importer->Initialize(filePath.c_str(), -1, m_manager->GetIOSettings());
    if (success)
    {
        R_DEBUG(FbxChannel, "Imported %s", filePath.c_str());

        if (m_importer->IsFBX())
        {
            i32 minor, major, revision;
            m_importer->GetFileVersion(major, minor, revision);
            R_DEBUG(FbxChannel, "File version: %d.%d.%d", major, minor, revision);
        }

        FbxScene* scene = nullptr;
        success = m_importer->Import(scene);   

        // Use as current scene.
        if (success)
            m_scene = scene;
    }
    return success ? RecluseResult_Ok : RecluseResult_Failed;
}


ResultCode FbxImport::traverseScene(FbxNode* parentNode, const ElementTypes& types, FbxProcessFunction& func)
{
    // We will traverse recursively for now.
    if (!parentNode) return RecluseResult_Ok;

    processNode(parentNode, types, func);

    // Traverse recursively
    for (U32 i = 0; i < parentNode->GetChildCount(); ++i)
    {
        FbxNode* child = parentNode->GetChild(i);
        traverseScene(child, types, func);
    }

    return RecluseResult_Ok;
}


ResultCode FbxImport::processNode(FbxNode* node, const ElementTypes& types, FbxProcessFunction& func)
{
    R_ASSERT(node);
    ResultCode result = RecluseResult_Failed;
    if (node->GetNodeAttribute())
    {
        FbxNodeAttribute::EType attribType = node->GetNodeAttribute()->GetAttributeType();
        for (auto type : types)
        {
            if (type == attribType)
            {
                result = func(node);
                break;
            }
        }
    }
    else
    {
        R_WARN(FbxChannel, "Found a null node attribute! Node Name: %s", node->GetName());
    }
    return result;
}
} // FBX
} // Pipeline
} // Recluse