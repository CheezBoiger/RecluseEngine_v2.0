//
#include "D3D12Commons.hpp"

namespace Dxgi {


DXGI_FORMAT getNativeFormat(Recluse::ResourceFormat format) 
{
    switch (format) 
    {
        case Recluse::ResourceFormat_R8G8B8A8_Unorm:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case Recluse::ResourceFormat_B8G8R8A8_Unorm:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case Recluse::ResourceFormat_B8G8R8A8_Srgb:
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


D3D12_RESOURCE_STATES getNativeResourceState(Recluse::ResourceState state)
{
    switch (state) 
    {
        case Recluse::ResourceState_Common:
        default:
            return D3D12_RESOURCE_STATE_COMMON;
        case Recluse::ResourceState_CopyDestination:
            return D3D12_RESOURCE_STATE_COPY_DEST;
        case Recluse::ResourceState_CopySource:
            return D3D12_RESOURCE_STATE_COPY_SOURCE;
        case Recluse::ResourceState_DepthStencilReadOnly:
            return D3D12_RESOURCE_STATE_DEPTH_READ;
        case Recluse::ResourceState_DepthStencilWrite:
            return D3D12_RESOURCE_STATE_DEPTH_WRITE;
        case Recluse::ResourceState_IndexBuffer:
            return D3D12_RESOURCE_STATE_INDEX_BUFFER;
        case Recluse::ResourceState_Present:
            return D3D12_RESOURCE_STATE_PRESENT;
        case Recluse::ResourceState_RenderTarget:
            return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case Recluse::ResourceState_ShaderResource:
                return (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        case Recluse::ResourceState_UnorderedAccess:
            return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        case Recluse::ResourceState_VertexBuffer:
        case Recluse::ResourceState_ConstantBuffer:
            return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    }

    return D3D12_RESOURCE_STATE_COMMON;
}
} // Dxgi