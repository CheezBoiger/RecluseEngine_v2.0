//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Format.hpp"

#include "Recluse/Renderer/Texture.hpp"

namespace Recluse {
namespace Pipeline {

enum CompressionFormat
{
    CompressionFormat_BC1,
    CompressionFormat_BC2,
    CompressionFormat_BC3,
    CompressionFormat_BC4,
    CompressionFormat_BC5,
    CompressionFormat_BC7
};


enum TextureExt
{
    TextureExt_PNG,
    TextureExt_JPG,
    TextureExt_DDS,
    TextureExt_TGA
};

enum TextureLoadFlag
{   
    TextureLoad_None            = (0),
    TextureLoad_GenerateMips    = (1 << 0),
};
typedef U32 TextureLoadFlags;


enum CompressFlag
{
    CompressFlag_None = (0),
    CompressFlag_HighQuality = (1 << 0),
};
typedef U32 CompressFlags;

R_PUBLIC_API Engine::Texture compress(const Engine::Texture& texture, CompressionFormat compressionFormat, CompressFlags compressFlags);
} // Pipeline
} // Recluse