//
#include "Recluse/Pipeline/Texture.hpp"
#include "Recluse/Math/MathCommons.hpp"

#include "Recluse/Math/Vector3.hpp"

namespace Recluse {
namespace Pipeline {


struct Rgb565
{
    U16 r : 5;
    U16 g : 6;
    U16 b : 5;
};

struct BlockBC1
{
    Rgb565 color0;
    Rgb565 color1;
    U32    indices;
};


Texture compress(const Texture& texture, CompressionFormat compressionFormat, CompressFlags compressFlags)
{
    Texture compressTexture;
    
    return compressTexture;
}
} // Pipeline
} // Recluse