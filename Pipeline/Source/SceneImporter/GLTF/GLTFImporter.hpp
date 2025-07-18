//
#pragma once

#include "Recluse/Pipeline/Importer.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "tinygltf/tiny_gltf.h"

namespace Recluse {
namespace Pipeline {
namespace Builder {
namespace GLTF {


class GLTFImporter final : public Importer
{
public:

    ResultCode importFile(const std::string& filePath) override;

private:
    // Model.
    tinygltf::Model m_model;
    // Loader.
    tinygltf::TinyGLTF m_loader;
};
} // GLTF
} // Builder
} // Pipeline
} // Recluse