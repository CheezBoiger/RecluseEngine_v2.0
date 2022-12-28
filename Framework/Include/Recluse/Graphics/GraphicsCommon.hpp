//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Threading/Threading.hpp"

namespace Recluse {

class GraphicsResource;

enum BindType 
{
    BindType_Graphics,
    BindType_Compute,
    BindType_RayTrace,
};


enum ResourceDimension 
{
    ResourceDimension_Buffer,
    ResourceDimension_1d,
    ResourceDimension_2d,
    ResourceDimension_3d
};

enum ResourceViewDimension 
{
    ResourceViewDimension_None,
    ResourceViewDimension_Buffer,
    ResourceViewDimension_1d,
    ResourceViewDimension_1dArray,
    ResourceViewDimension_2d,
    ResourceViewDimension_2dMultisample,
    ResourceViewDimension_2dArray,
    ResourceViewDimension_3d,
    ResourceViewDimension_Cube,
    ResourceViewDimension_CubeArray
};


enum ResourceMemoryUsage 
{
    ResourceMemoryUsage_CpuOnly,
    ResourceMemoryUsage_GpuOnly,
    ResourceMemoryUsage_CpuToGpu,
    ResourceMemoryUsage_GpuToCpu,
    ResourceMemoryUsage_Count = (ResourceMemoryUsage_GpuToCpu + 1)
};


enum ResourceUsage 
{
    ResourceUsage_VertexBuffer            = (1 << 0),
    ResourceUsage_IndexBuffer             = (1 << 1),
    ResourceUsage_RenderTarget            = (1 << 2),
    ResourceUsage_ShaderResource          = (1 << 3),
    ResourceUsage_ConstantBuffer          = (1 << 4),
    ResourceUsage_TransferDestination     = (1 << 5),
    ResourceUsage_CopyDestination         = ResourceUsage_TransferDestination,
    ResourceUsage_TransferSource          = (1 << 6),
    ResourceUsage_CopySource              = ResourceUsage_TransferSource,
    ResourceUsage_IndirectBuffer          = (1 << 7),
    ResourceUsage_DepthStencil            = (1 << 8),
    ResourceUsage_UnorderedAccess         = (1 << 9)
};


typedef U32 ResourceUsageFlags;


enum ResourceViewType 
{
    ResourceViewType_RenderTarget,
    ResourceViewType_ShaderResource,
    ResourceViewType_UnorderedAccess,
    ResourceViewType_DepthStencil
};


enum ResourceState 
{
    ResourceState_Common,
    // Render Target resource state.
    ResourceState_RenderTarget,
    // Shader Resource State.
    ResourceState_ShaderResource,
    // Copy Destination state.
    ResourceState_CopyDestination,
    
    ResourceState_CopySource,
    ResourceState_VertexBuffer,
    ResourceState_ConstantBuffer,
    ResourceState_IndexBuffer,
    // Unordered Access View State.
    ResourceState_UnorderedAccess,
    ResourceState_DepthStencilReadOnly,
    ResourceState_DepthStencilWrite,
    ResourceState_Present,
    ResourceState_IndirectArgs
};


enum FrameBuffering 
{
    FrameBuffering_Single, // Single immediate run.
    FrameBuffering_Double, // VSync
    FrameBuffering_Triple // Triple buffering.
};


enum R_PUBLIC_API LayerFeatures 
{
    LayerFeature_RaytracingBit       = (1 << 0),
    LayerFeature_MeshShadingBit      = (1 << 1),
    LayerFeature_DebugValidationBit  = (1 << 2),
    LayerFeature_ApiDumpBit          = (1 << 3)
};


typedef U32 EnableLayerFlags;


enum GraphicsAPI 
{
    GraphicsApi_Unknown,
    GraphicsApi_SoftwareRasterizer,
    GraphicsApi_SoftwareRaytracer,
    GraphicsApi_Vulkan,
    GraphicsApi_OpenGL,
    GraphicsApi_Direct3D11,
    GraphicsApi_Direct3D12
};


enum DescriptorBindType 
{
    DescriptorBindType_ShaderResource,
    DescriptorBindType_ConstantBuffer,
    DescriptorBindType_UnorderedAccess,
    DescriptorBindType_Sampler
};


enum IndexType 
{
    IndexType_Unsigned16,
    IndexType_Unsigned32
};


struct Viewport 
{
    F32 x;
    F32 y;
    F32 width;
    F32 height;
    F32 minDepth;
    F32 maxDepth;
};


struct Rect 
{
    F32 x, 
        y, 
        width, 
        height;
};


struct CopyBufferRegion 
{
    U64               srcOffsetBytes;
    U16               dstOffsetBytes;
    U64               szBytes;
};


struct SwapchainCreateDescription 
{
    FrameBuffering  buffering;
    U32             desiredFrames;
    U32             renderWidth;
    U32             renderHeight;
    ResourceFormat  format;
};


enum SamplerAddressMode 
{
    SamplerAddressMode_Repeat,
    SamplerAddressMode_MirroredRepeat,
    SamplerAddressMode_ClampToEdge,
    SamplerAddressMode_ClampToBorder,
    SamplerAddressMode_MirrorClampToEdge
};


enum Filter 
{
    Filter_Linear,
    Filter_Nearest,
    Filter_Cubic
};


enum SamplerMipMapMode 
{
    SamplerMipMapMode_Nearest,
    SamplerMipMapMode_Linear
};


enum CompareOp 
{
    CompareOp_Never,
    CompareOp_Less,
    CompareOp_Equal,
    CompareOp_LessOrEqual,
    CompareOp_Greater,
    CompareOp_NotEqual,
    CompareOp_GreaterOrEqual,
    CompareOp_Always
};


struct SamplerCreateDesc 
{
    SamplerAddressMode  addrModeU;
    SamplerAddressMode  addrModeV;
    SamplerAddressMode  addrModeW;
    F32                 minLod;
    F32                 maxLod;
    Filter              magFilter;
    Filter              minFilter;
    CompareOp           compareOp;
    SamplerMipMapMode   mipMapMode;
    Bool                anisotropyEnable;
    Bool                compareEnable;
};


enum GraphicsQueueType 
{
    QUEUE_TYPE_PRESENT      = (1 << 0),
    QUEUE_TYPE_GRAPHICS     = (1 << 1),
    QUEUE_TYPE_COMPUTE      = (1 << 2),
    QUEUE_TYPE_COPY         = (1 << 3)
};

typedef U32 GraphicsQueueTypeFlags;

// Common Graphics Object manager.
class IGraphicsObject
{
public:
    IGraphicsObject()
    { generateId(); }

    virtual ~IGraphicsObject() { }

    virtual GraphicsAPI getApi() const = 0;

    // Get the unique id of this graphics object.
    virtual GraphicsId getId() const = 0;

    // This function is called for everytime an object is created. Ensure this 
    // assigns a unique id to it!
    virtual void generateId() { }
};
} // Recluse