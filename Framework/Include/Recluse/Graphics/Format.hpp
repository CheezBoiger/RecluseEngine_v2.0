//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Serialization/Hasher.hpp"

namespace Recluse {

enum ResourceFormat 
{
    ResourceFormat_Unknown,
    ResourceFormat_R8G8B8A8_Unorm,
    ResourceFormat_R16G16B16A16_Float,
    ResourceFormat_R11G11B10_Float,
    ResourceFormat_D32_Float,
    ResourceFormat_R32_Float,
    ResourceFormat_D24_Unorm_S8_Uint,
    ResourceFormat_D32_Float_S8_Uint,
    ResourceFormat_R16G16_Float,
    ResourceFormat_B8G8R8A8_Srgb,
    ResourceFormat_R32G32B32A32_Float,
    ResourceFormat_R32G32B32A32_Uint,
    ResourceFormat_R8_Uint,
    ResourceFormat_R32G32_Float,
    ResourceFormat_R32G32_Uint,
    ResourceFormat_R16_Uint,
    ResourceFormat_R16_Float,
    ResourceFormat_B8G8R8A8_Unorm,
    ResourceFormat_R32G32B32_Float
};


extern const char* getResourceFormatString(ResourceFormat format);

typedef Hash64 GraphicsId;
} // Recluse