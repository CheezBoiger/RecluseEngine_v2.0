//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Pipeline/Importer.hpp"
#include "Recluse/Pipeline/MeshBuilder.hpp"

namespace Recluse {
namespace Pipeline {
namespace Builder {
namespace FBX {

class FbxMeshBuilder final : public Builder::MeshBuilder
{
public:
    FbxMeshBuilder() { }
    ~FbxMeshBuilder() { }
    ResultCode build(Builder::Importer* importer, MeshBuilderFlags flags) override;
    ResultCode serialize(Archive* archive) const override;
    ResultCode deserialize(Archive* archive) override;

    ResultCode extractMaterials(MeshBuilder::Data& meshData, FbxNode* node);
};
} // FBX
} // Builder
} // Pipeline
} // Recluse