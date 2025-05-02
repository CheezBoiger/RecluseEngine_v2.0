//
#include "Recluse/Pipeline/Importer.hpp"


#include <queue>

#include "fbxsdk.h"
#include "FBXImporter.hpp"

namespace Recluse {
namespace Pipeline {
namespace Builder {
namespace FBX {


Pipeline::Builder::Importer* FbxImport::create()
{
    return new FbxImport();
}


ResultCode FbxImport::destroy(Pipeline::Builder::Importer* importer)
{
    if (!importer)
        return RecluseResult_NullPtrExcept;
    if (importer->getFormat() != FileFormat_FBX)
        return RecluseResult_InvalidArgs;

    delete importer;

    return RecluseResult_Ok;
}


FbxImport::FbxImport()
    : Importer(Pipeline::Builder::FileFormat_FBX, ".fbx")
    , m_manager(nullptr)
    , m_importer(nullptr)
    , m_scene(nullptr)
{
    m_manager = FbxManager::Create();
    R_ASSERT_FORMAT(m_manager, "Fbx Manager failed to create");
    m_importer = FbxImporter::Create(m_manager, "Default Manager");
}


FbxImport::~FbxImport()
{
    if (m_scene) m_scene->Destroy();
    if (m_importer) m_importer->Destroy();
    if (m_manager) m_manager->Destroy();

    m_importer = nullptr;
    m_manager = nullptr;
    m_scene = nullptr;
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

        FbxScene* scene = FbxScene::Create(m_manager, "Default Scene");
        success = m_importer->Import(scene);

        // Use as current scene.
        if (success)
            m_scene = scene;
    }
    return success ? RecluseResult_Ok : RecluseResult_Failed;
}


ResultCode FbxImport::traverseScene(FbxNode* parentNode, const Process& processes)
{
    // We will traverse recursively for now.
    if (!parentNode) return RecluseResult_Ok;

    processNode(parentNode, processes);

    // Traverse recursively
    for (U32 i = 0; i < parentNode->GetChildCount(); ++i)
    {
        FbxNode* child = parentNode->GetChild(i);
        traverseScene(child, processes);
    }

    return RecluseResult_Ok;
}


ResultCode FbxImport::processNode(FbxNode* node, const Process& processes)
{
    R_ASSERT(node);
    ResultCode result = RecluseResult_Failed;
    if (node->GetNodeAttribute())
    {
        FbxNodeAttribute::EType attribType = node->GetNodeAttribute()->GetAttributeType();
        if (processes.contains(attribType))
        {
            const std::vector<FbxProcessFunction>& functions = processes[attribType];
            R_ASSERT_FORMAT(!functions.empty(), "Couldn't find processing functions for etype=%d". (i32)attribType);
            for (auto func : functions)
            {
                result = func(node);
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
} // Builder
} // Pipeline
} // Recluse