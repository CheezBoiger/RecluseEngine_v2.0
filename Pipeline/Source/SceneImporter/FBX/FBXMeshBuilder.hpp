//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Pipeline/Importer.hpp"
#include "Recluse/Pipeline/MeshBuilder.hpp"

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

    FbxMeshBuilder() : MeshBuilder(FileFormat_FBX) { }
    ~FbxMeshBuilder() { }


    ResultCode      build(Builder::Importer* importer, MeshBuilderFlags flags) override;
    ResultCode      serialize(Archive* archive) const override;
    ResultCode      deserialize(Archive* archive) override;

private:

    ResultCode      extractMaterials(MeshBuilder::Data& meshData, FbxNode* node);
};
} // FBX
} // Builder
} // Pipeline
} // Recluse