//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Pipeline/Importer.hpp"
#include "Recluse/Pipeline/MeshBuilder.hpp"

#include "Recluse/Math/Vector3.hpp"

#include "fbxsdk.h"

namespace Recluse {
namespace Pipeline {
namespace Builder {
namespace FBX {

class FbxMeshBuilder final : public Builder::MeshBuilder
{
public:

    static Builder::MeshBuilder*    create();
    static ResultCode               destroy(Builder::MeshBuilder* meshBuilder);

    FbxMeshBuilder() : MeshBuilder(FileFormat_FBX, ".fbx") { }
    ~FbxMeshBuilder() { }


    ResultCode                      build(Builder::Importer* importer, MeshBuilderFlags flags) override;
    ResultCode                      serialize(Archive* archive) const override;
    ResultCode                      deserialize(Archive* archive) override;

private:

    ResultCode                      extractMaterials(MeshBuilder::MeshData& meshData, FbxNode* node);
    std::vector<Math::Float3>       triangulatePolygon(const std::vector<Math::Float3>& pointList);

    // Node guids.
    std::map<FbxNode*, RGUID> nodeGuids;
};
} // FBX
} // Builder
} // Pipeline
} // Recluse