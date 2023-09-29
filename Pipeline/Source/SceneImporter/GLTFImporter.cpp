

#include "SceneImporter/Importer.hpp"
#include "Recluse/Messaging.hpp"

#define STB_IMAGE_IMPLEMENTATION 1
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#define TINYGLTF_IMPLEMENTATION 1
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"


namespace Recluse {
namespace Importer {
namespace GLTF {

} // GLTF
} // Importer
} // Recluse