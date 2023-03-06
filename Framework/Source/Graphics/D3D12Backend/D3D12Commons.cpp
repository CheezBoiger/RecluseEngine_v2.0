//
#include "D3D12Commons.hpp"

namespace Recluse {

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


D3D12_RTV_DIMENSION getRtvDimension(Recluse::ResourceViewDimension dimension)
{
    switch (dimension)
    {
        case ResourceViewDimension_1d:
            return D3D12_RTV_DIMENSION_TEXTURE1D;
        case ResourceViewDimension_1dArray:
            return D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
        case ResourceViewDimension_2d:
            return D3D12_RTV_DIMENSION_TEXTURE2D;
        case ResourceViewDimension_2dArray:
            return D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
        case ResourceViewDimension_2dMultisample:
            return D3D12_RTV_DIMENSION_TEXTURE2DMS;
        case ResourceViewDimension_2dMultisampleArray:
            return D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
        case ResourceViewDimension_3d:
            return D3D12_RTV_DIMENSION_TEXTURE3D;
        case ResourceViewDimension_Buffer:
            return D3D12_RTV_DIMENSION_BUFFER;
        default:
            return D3D12_RTV_DIMENSION_UNKNOWN;
    }
}


D3D12_DSV_DIMENSION getDsvDimension(Recluse::ResourceViewDimension dimension)
{
    switch (dimension)
    {
        case ResourceViewDimension_1d:
            return D3D12_DSV_DIMENSION_TEXTURE1D;
        case ResourceViewDimension_1dArray:
            return D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
        case ResourceViewDimension_2d:
            return D3D12_DSV_DIMENSION_TEXTURE2D;
        case ResourceViewDimension_2dArray:
            return D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
        case ResourceViewDimension_2dMultisample:
            return D3D12_DSV_DIMENSION_TEXTURE2DMS;
        default:
            return D3D12_DSV_DIMENSION_UNKNOWN;
    }
}


D3D12_UAV_DIMENSION getUavDimension(Recluse::ResourceViewDimension dimension)
{
    switch (dimension)
    {
        case ResourceViewDimension_1d:
            return D3D12_UAV_DIMENSION_TEXTURE1D;
        case ResourceViewDimension_1dArray:
            return D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
        case ResourceViewDimension_2d:
            return D3D12_UAV_DIMENSION_TEXTURE2D;
        case ResourceViewDimension_2dArray:
            return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        case ResourceViewDimension_3d:
            return D3D12_UAV_DIMENSION_TEXTURE3D;
        case ResourceViewDimension_Buffer:
            return D3D12_UAV_DIMENSION_BUFFER;
        default:
            return D3D12_UAV_DIMENSION_UNKNOWN;
    }
}


D3D12_SRV_DIMENSION getSrvDimension(Recluse::ResourceViewDimension dimension)
{
    switch (dimension)
    {
        case ResourceViewDimension_1d:
            return D3D12_SRV_DIMENSION_TEXTURE1D;
        case ResourceViewDimension_1dArray:
            return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        case ResourceViewDimension_2d:
            return D3D12_SRV_DIMENSION_TEXTURE2D;
        case ResourceViewDimension_2dArray:
            return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        case ResourceViewDimension_2dMultisample:
            return D3D12_SRV_DIMENSION_TEXTURE2DMS;
        case ResourceViewDimension_2dMultisampleArray:
            return D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
        case ResourceViewDimension_3d:
            return D3D12_SRV_DIMENSION_TEXTURE3D;
        case ResourceViewDimension_Cube:
            return D3D12_SRV_DIMENSION_TEXTURECUBE;
        case ResourceViewDimension_CubeArray:
            return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        case ResourceViewDimension_Buffer:
            return D3D12_SRV_DIMENSION_BUFFER;
        case ResourceViewDimension_RayTraceAccelerationStructure:
            return D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
        default:
            return D3D12_SRV_DIMENSION_UNKNOWN;
    }
}
} // Recluse


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
} // Dxgi