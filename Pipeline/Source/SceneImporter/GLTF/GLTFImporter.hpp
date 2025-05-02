//
#pragma once

#include "Recluse/Pipeline/Importer.hpp"

#include "tinygltf/tiny_gltf.h"

namespace Recluse {
namespace Pipeline {
namespace Builder {
namespace GLTF {


class GLTFImporter final : public Importer
{
public:

    ResultCode importFile(const std::string& filePath) override
    {
        return RecluseResult_NoImpl;
    }
};
} // GLTF
} // Builder
} // Pipeline
} // Recluse