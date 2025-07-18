

#include "Recluse/Pipeline/Importer.hpp"
#include "Recluse/Messaging.hpp"

#include "GLTFImporter.hpp"

#define STB_IMAGE_IMPLEMENTATION 1
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#define TINYGLTF_IMPLEMENTATION 1
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"


namespace Recluse {
namespace Pipeline {
namespace Builder {
namespace GLTF {



ResultCode GLTFImporter::importFile(const std::string& filePath)
{
    std::string err;
    std::string warn;
    std::string ext = File::extension(filePath);

    bool success = false;
    if (ext.compare(".glb") == 0)
        success = m_loader.LoadBinaryFromFile(&m_model, &err, &warn, filePath);
    else
        success = m_loader.LoadASCIIFromFile(&m_model, &err,  &warn, filePath);
    return success ? RecluseResult_Ok : RecluseResult_Failed;
}
} // GLTF
} // Builder
} // Pipeline
} // Recluse