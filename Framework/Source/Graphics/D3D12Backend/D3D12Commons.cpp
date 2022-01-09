//
#include "D3D12Commons.hpp"

namespace Dxgi {


DXGI_FORMAT getNativeFormat(Recluse::ResourceFormat format) 
{
    switch (format) 
    {
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


SIZE_T getNativeFormatSize(DXGI_FORMAT format)
{
    switch (format) 
    {
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return 4ull;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return 8ull;
    }

    // Return no bytes for unknown formats.
    return 0ull;
}


D3D12_RESOURCE_STATES getNativeResourceState(Recluse::ResourceState state, bool isPixelShader)
{
    switch (state) 
    {
        case Recluse::RESOURCE_STATE_GENERAL:
        default:
            return D3D12_RESOURCE_STATE_COMMON;
        case Recluse::RESOURCE_STATE_COPY_DST:
            return D3D12_RESOURCE_STATE_COPY_DEST;
        case Recluse::RESOURCE_STATE_COPY_SRC:
            return D3D12_RESOURCE_STATE_COPY_SOURCE;
        case Recluse::RESOURCE_STATE_DEPTH_STENCIL_READONLY:
            return D3D12_RESOURCE_STATE_DEPTH_READ;
        case Recluse::RESOURCE_STATE_DEPTH_STENCIL_WRITE:
            return D3D12_RESOURCE_STATE_DEPTH_WRITE;
        case Recluse::RESOURCE_STATE_INDEX_BUFFER:
            return D3D12_RESOURCE_STATE_INDEX_BUFFER;
        case Recluse::RESOURCE_STATE_PRESENT:
            return D3D12_RESOURCE_STATE_PRESENT;
        case Recluse::RESOURCE_STATE_RENDER_TARGET:
            return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case Recluse::RESOURCE_STATE_SHADER_RESOURCE:
            if (isPixelShader)
                return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            else 
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        case Recluse::RESOURCE_STATE_STORAGE:
            return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        case Recluse::RESOURCE_STATE_VERTEX_AND_CONST_BUFFER:
            return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    }

    return D3D12_RESOURCE_STATE_COMMON;
}
} // Dxgi