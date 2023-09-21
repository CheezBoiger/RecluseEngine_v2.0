//
#include "D3D12Commons.hpp"

#include "Recluse/Messaging.hpp"

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
        case Recluse::ResourceState_IndirectArgs:
            return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
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


D3D12_COMPARISON_FUNC getNativeComparisonFunction(Recluse::CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp_Always:
            return D3D12_COMPARISON_FUNC_ALWAYS;
        case CompareOp_Equal:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case CompareOp_Greater:
            return D3D12_COMPARISON_FUNC_GREATER;
        case CompareOp_GreaterOrEqual:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case CompareOp_Less:
            return D3D12_COMPARISON_FUNC_LESS;
        case CompareOp_LessOrEqual:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case CompareOp_NotEqual:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case CompareOp_Never:
        default:
            return D3D12_COMPARISON_FUNC_NEVER;
    }
}


D3D12_INDEX_BUFFER_STRIP_CUT_VALUE getNativeStripCutValue(Recluse::IndexType type)
{
    switch (type)
    {
        case Recluse::IndexType_Unsigned16:     return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
        default:
        case Recluse::IndexType_Unsigned32:     return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    }
}


D3D12_BLEND_OP getBlendOp(Recluse::BlendOp blendOp)
{
    switch (blendOp)
    { 
        case BlendOp_Subtract:          return D3D12_BLEND_OP_SUBTRACT;
        case BlendOp_ReverseSubtract:   return D3D12_BLEND_OP_REV_SUBTRACT;
        case BlendOp_Max:               return D3D12_BLEND_OP_MAX;
        case BlendOp_Min:               return D3D12_BLEND_OP_MIN;
        case BlendOp_Add:               
        default:                        return D3D12_BLEND_OP_ADD;
    }
}


D3D12_BLEND getBlendFactor(Recluse::BlendFactor blendFactor)
{
    switch (blendFactor)
    {
        case BlendFactor_DestinationAlpha:              return D3D12_BLEND_DEST_ALPHA;
        case BlendFactor_DestinationColor:              return D3D12_BLEND_DEST_COLOR;
        case BlendFactor_One:                           return D3D12_BLEND_ONE;
        case BlendFactor_OneMinusSourceAlpha:           return D3D12_BLEND_INV_SRC_ALPHA;
        case BlendFactor_OneMinusSourceColor:           return D3D12_BLEND_INV_SRC_COLOR;
        case BlendFactor_OneMinusSourceOneAlpha:        return D3D12_BLEND_INV_SRC1_ALPHA;
        case BlendFactor_OneMinusSourceOneColor:        return D3D12_BLEND_INV_SRC1_COLOR;
        case BlendFactor_SourceAlpha:                   return D3D12_BLEND_SRC_ALPHA;
        case BlendFactor_SourceAlphaSaturate:           return D3D12_BLEND_SRC_ALPHA_SAT;
        case BlendFactor_SourceColor:                   return D3D12_BLEND_SRC_COLOR;
        case BlendFactor_SourceOneAlpha:                return D3D12_BLEND_SRC1_ALPHA;
        case BlendFactor_SourceOneColor:                return D3D12_BLEND_SRC1_COLOR;
        case BlendFactor_Zero:                          return D3D12_BLEND_ZERO;
        case BlendFactor_ConstantAlpha: 
        case BlendFactor_ConstantColor:
        case BlendFactor_OneMinusConstantAlpha:         
        case BlendFactor_OneMinusConstantColor:
        case BlendFactor_OneMinusDestinationAlpha:
        case BlendFactor_OneMinusDestinationColor:      return D3D12_BLEND_BLEND_FACTOR;
    }
}


D3D12_LOGIC_OP getLogicOp(Recluse::LogicOp logicOp)
{
    switch (logicOp)
    {
        case LogicOp_And:               return D3D12_LOGIC_OP_AND;
        case LogicOp_AndInverted:       return D3D12_LOGIC_OP_AND_INVERTED;
        case LogicOp_AndReverse:        return D3D12_LOGIC_OP_AND_REVERSE;
        case LogicOp_Clear:             return D3D12_LOGIC_OP_CLEAR;
        case LogicOp_Copy:              return D3D12_LOGIC_OP_COPY;
        case LogicOp_CopyInverted:      return D3D12_LOGIC_OP_COPY_INVERTED;
        case LogicOp_Equivalent:        return D3D12_LOGIC_OP_EQUIV;
        case LogicOp_Invert:            return D3D12_LOGIC_OP_INVERT;
        case LogicOp_Nand:              return D3D12_LOGIC_OP_NAND;
        case LogicOp_NoOp:              return D3D12_LOGIC_OP_NOOP;
        case LogicOp_Nor:               return D3D12_LOGIC_OP_NOR;
        case LogicOp_Or:                return D3D12_LOGIC_OP_OR;
        case LogicOp_OrInverted:        return D3D12_LOGIC_OP_OR_INVERTED;
        case LogicOp_OrReverse:         return D3D12_LOGIC_OP_OR_REVERSE;
        case LogicOp_Set:               return D3D12_LOGIC_OP_SET;
        case LogicOp_Xor:               return D3D12_LOGIC_OP_XOR;
    }
}


D3D12_STENCIL_OP getStencilOp(Recluse::StencilOp stencilOp)
{
    switch (stencilOp)
    {
        case StencilOp_DecrementAndClamp:   return D3D12_STENCIL_OP_DECR_SAT;
        case StencilOp_DecrementAndWrap:    return D3D12_STENCIL_OP_DECR;
        case StencilOp_IncrementAndClamp:   return D3D12_STENCIL_OP_INCR_SAT;
        case StencilOp_IncrementAndWrap:    return D3D12_STENCIL_OP_INCR;
        case StencilOp_Invert:              return D3D12_STENCIL_OP_INVERT;
        case StencilOp_Keep:                return D3D12_STENCIL_OP_KEEP;
        case StencilOp_Replace:             return D3D12_STENCIL_OP_REPLACE;
        case StencilOp_Zero:                return D3D12_STENCIL_OP_ZERO;
    }
}


D3D12_PRIMITIVE_TOPOLOGY_TYPE getPrimitiveTopologyType(PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology_LineStrip:       
        case PrimitiveTopology_LineList:        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case PrimitiveTopology_PointList:       return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case PrimitiveTopology_TriangleStrip:
        case PrimitiveTopology_TriangleList:
        default: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    }
}


D3D12_PRIMITIVE_TOPOLOGY getPrimitiveTopology(Recluse::PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology_LineList:        return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case PrimitiveTopology_LineStrip:       return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PrimitiveTopology_PointList:       return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case PrimitiveTopology_TriangleList:    return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PrimitiveTopology_TriangleStrip:   return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        default:                                return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }
}


UINT calculateSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize)
{ 
    return MipSlice + (ArraySlice * MipLevels) + (PlaneSlice * MipLevels * ArraySize); 
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
        case Recluse::ResourceFormat_R32G32B32_Float:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case Recluse::ResourceFormat_R32G32_Float:
            return DXGI_FORMAT_R32G32_FLOAT;
        case Recluse::ResourceFormat_R16_Float:
            return DXGI_FORMAT_R16_FLOAT;
        case Recluse::ResourceFormat_D24_Unorm_S8_Uint:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case Recluse::ResourceFormat_D32_Float:
            return DXGI_FORMAT_D32_FLOAT;
        case Recluse::ResourceFormat_R16G16_Float:
            return DXGI_FORMAT_R16G16_FLOAT;
        case Recluse::ResourceFormat_R11G11B10_Float:
            return DXGI_FORMAT_R11G11B10_FLOAT;
        case Recluse::ResourceFormat_R32_Float:
            return DXGI_FORMAT_R32_FLOAT;
        case Recluse::ResourceFormat_R8_Uint:
            return DXGI_FORMAT_R8_UINT;
        case Recluse::ResourceFormat_R16G16B16A16_Float:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case Recluse::ResourceFormat_R32G32B32A32_Float:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case Recluse::ResourceFormat_R32G32_Uint:
            return DXGI_FORMAT_R32G32_UINT;
        case Recluse::ResourceFormat_R32_Uint:
            return DXGI_FORMAT_R32_UINT;
        case Recluse::ResourceFormat_R32_Int:
            return DXGI_FORMAT_R32_SINT;
        case Recluse::ResourceFormat_BC1_Unorm:
            return DXGI_FORMAT_BC1_UNORM;
        case Recluse::ResourceFormat_BC2_Unorm:
            return DXGI_FORMAT_BC2_UNORM;
        case Recluse::ResourceFormat_BC3_Unorm:
            return DXGI_FORMAT_BC3_UNORM;
        case Recluse::ResourceFormat_BC4_Unorm:
            return DXGI_FORMAT_BC4_UNORM;
        case Recluse::ResourceFormat_BC5_Unorm:
            return DXGI_FORMAT_BC5_UNORM;
        case Recluse::ResourceFormat_BC7_Unorm:
            return DXGI_FORMAT_BC7_UNORM;
        default:
            break;
    }

    return DXGI_FORMAT_UNKNOWN;
}


SIZE_T getNativeFormatSize(DXGI_FORMAT format)
{
    switch (format) 
    {
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_TYPELESS:
            
            return 1ull;

        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UINT:

            return 2ull;

        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:

            return 4ull;

        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G8X24_TYPELESS:

            return 8ull;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:

            return 8ull;

        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_SINT:
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_UINT:

            return 12ull;

        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_UINT:

            return 16ull;

        default:
            R_ASSERT_FORMAT(false, "D3D12: Can not find byte size format for DXGI format %d", format);
    }

    // Return no bytes for unknown formats.
    return 0ull;
}


D3D12_SHADER_VISIBILITY getShaderVisibilityFlags(Recluse::ShaderStageFlags shaderStageFlags)
{
    return D3D12_SHADER_VISIBILITY_ALL;
}
} // Dxgi