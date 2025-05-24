//
#include "Recluse/Pipeline/MeshBuilder.hpp"

#include "SceneImporter/FBX/FBXMeshBuilder.hpp"

namespace Recluse {
namespace Pipeline {
namespace Builder {


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


void MeshBuilder::performSimplify()
{
}


void MeshBuilder::performOptimize()
{
}
} // Builder
} // Pipeline
} // Recluse