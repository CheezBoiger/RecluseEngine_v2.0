//
#include "D3D12Commons.hpp"

namespace Dxgi {


DXGI_FORMAT getNativeFormat(Recluse::ResourceFormat format) 
{
    switch (format) {
    case Recluse::RESOURCE_FORMAT_R8G8B8A8_UNORM:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case Recluse::RESOURCE_FORMAT_B8G8R8A8_UNORM:
        return DXGI_FORMAT_B8G8R8A8_UNORM;
    case Recluse::RESOURCE_FORMAT_B8G8R8A8_SRGB:
        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    default:
        break;
    }

  return DXGI_FORMAT_UNKNOWN;
}


SIZE_T getNativeFormatSize(Recluse::ResourceFormat format)
{
    switch (format) {
        case Recluse::RESOURCE_FORMAT_R16G16_FLOAT:
        case Recluse::RESOURCE_FORMAT_R8G8B8A8_UNORM:
            return 4ull;
        case Recluse::RESOURCE_FORMAT_R16G16B16A16_FLOAT:
            return 8ull;
    }

    // Return no bytes for unknown formats.
    return 0ull;
}
} // Dxgi