//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Utility.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Threading/Threading.hpp"

namespace Recluse {

class GraphicsResource;
typedef Hash64 ResourceId;
typedef Hash64 ResourceViewId;

enum BindType 
{
    BindType_Graphics,
    BindType_Compute,
    BindType_RayTrace,
};


enum StencilOp 
{
    StencilOp_Keep,
    StencilOp_Zero,
    StencilOp_Replace,
    StencilOp_IncrementAndClamp,
    StencilOp_DecrementAndClamp,
    StencilOp_Invert,
    StencilOp_IncrementAndWrap,
    StencilOp_DecrementAndWrap
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
    ResourceViewDimension_2dMultisampleArray,
    ResourceViewDimension_2dArray,
    ResourceViewDimension_3d,
    ResourceViewDimension_Cube,
    ResourceViewDimension_CubeArray,
    ResourceViewDimension_RayTraceAccelerationStructure
};


enum ResourceMemoryUsage 
{
    // Cpu/host usage mainly. Host write access is fastest, but device read access will be slower. Best for write-once on cpu, read-once on gpu.
    ResourceMemoryUsage_CpuOnly,
    // Gpu/Device only memory usage. Gpu read/writes are fastest, but cpu does not have read/write access. Best if resources are not needing to be updated by host.
    ResourceMemoryUsage_GpuOnly,
    // Cpu to Gpu memory usage. Optimized for dynamic writes into buffers and textures from cpu to gpu.
    ResourceMemoryUsage_CpuToGpu,
    // Gpu to Cpu memory usage. Optimized for writing Gpu data back to allow cpu read access. Is usually done in a smaller region of memory, so don't rely much!
    ResourceMemoryUsage_GpuToCpu,
    ResourceMemoryUsage_Count = (ResourceMemoryUsage_GpuToCpu + 1)
};


enum ResourceUsage 
{
    ResourceUsage_VertexBuffer          = (1 << 0),
    ResourceUsage_IndexBuffer           = (1 << 1),
    ResourceUsage_RenderTarget          = (1 << 2),
    ResourceUsage_ShaderResource        = (1 << 3),
    ResourceUsage_ConstantBuffer        = (1 << 4),
    ResourceUsage_TransferDestination   = (1 << 5),
    ResourceUsage_CopyDestination       = ResourceUsage_TransferDestination,
    ResourceUsage_TransferSource        = (1 << 6),
    ResourceUsage_CopySource            = ResourceUsage_TransferSource,
    ResourceUsage_IndirectBuffer        = (1 << 7),
    ResourceUsage_DepthStencil          = (1 << 8),
    ResourceUsage_UnorderedAccess       = (1 << 9)
};


// Additional miscelleneous flags that might be API specific.
// Some of these flags may not be supported on different APIs, so 
// Use them sparingly.
enum ResourceMiscFlag
{
    ResourceMiscFlag_None               = (0),
    ResourceMiscFlag_StructuredBuffer   = (1 << 0), 
    ResourceMiscFlag_RawBuffer          = (1 << 1)
};


typedef U32 ResourceUsageFlags;
typedef U32 ResourceMiscFlags;


enum ResourceViewType 
{
    ResourceViewType_RenderTarget,
    ResourceViewType_ShaderResource,
    ResourceViewType_UnorderedAccess,
    ResourceViewType_DepthStencil
};


enum Semantic
{
    Semantic_Position,
    Semantic_Normal,
    Semantic_Texcoord,
    Semantic_Binormal,
    Semantic_Tangent,
    Semantic_Color,
    Semantic_TessFactor
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
    // Copy Source state.
    ResourceState_CopySource,
    // Vertex buffer state.
    ResourceState_VertexBuffer,
    // Constant buffer state.
    ResourceState_ConstantBuffer,
    // Index buffer state.
    ResourceState_IndexBuffer,
    // Unordered Access View State.
    ResourceState_UnorderedAccess,
    // Depth-stencil readonly state.
    ResourceState_DepthStencilReadOnly,
    // Depth-stencil write state.
    ResourceState_DepthStencilWrite,
    // Presentation state, used mainly for presenting resources to the display engine.
    ResourceState_Present,
    // Indirect argument buffer state.
    ResourceState_IndirectArgs
};


enum FrameBuffering 
{
    FrameBuffering_Single, // Single immediate run.
    FrameBuffering_Double, // VSync
    FrameBuffering_Triple // Triple buffering.
};


enum R_PUBLIC_API LayerFeatureFlag 
{
    LayerFeatureFlag_None               = 0,
    // Checks if gpu has dedicated raytracing.
    LayerFeatureFlag_Raytracing         = (1 << 0),
    // Checks if gpu has dedicated mesh shading.
    LayerFeatureFlag_MeshShading        = (1 << 1),
    // Enables cpu related validation, validation messages and errors.
    LayerFeatureFlag_DebugValidation    = (1 << 2),
    // 
    LayerFeatureFlag_ApiDump            = (1 << 3),
    // Enable marking debugging. This allows graphics objects to be marked or named.
    LayerFeatureFlag_DebugMarking       = (1 << 4),
    // Enable Gpu related validation, messages and errors.
    LayerFeatureFlag_GpuDebugValidation      = (1 << 5)
};


typedef U32 LayerFeatureFlags;


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

// Rectangle structure.
// x is the offset of the rectangle in x position
// y is the offset of the rectangle in y position
// width is the overall width extent of the rectangle.
// height is the overall height extent of the rectangle.
struct Rect 
{
    F32 x, 
        y, 
        width, 
        height;
};


enum ClearFlag
{
    ClearFlag_None = 0,
    ClearFlag_Color = 0x001,
    ClearFlag_Depth = 0x002,
    ClearFlag_Stencil = 0x004
};


typedef U32 ClearFlags;


struct CopyBufferRegion 
{
    U64               srcOffsetBytes;
    U64               dstOffsetBytes;
    U64               szBytes;
};


struct CopyTextureRegion
{
    union
    {
        struct 
        {
            U32 width;
            U32 height;
            U32 arrayLayerOrDepth;
            U32 mipLevel;
        } texture;
    };
};


struct SwapchainCreateDescription 
{
    FrameBuffering  buffering;
    U32             desiredFrames;
    U32             renderWidth;
    U32             renderHeight;
    ResourceFormat  format;
    // Prefer using High-Dynamic Range when available.
    Bool            preferHDR;
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


enum BorderColor
{
    BorderColor_TransparentBlack,
    BorderColor_OpaqueBlack,
    BorderColor_OpaqueWhite
};


struct SamplerDescription 
{
    SamplerAddressMode  addressModeU;
    SamplerAddressMode  addressModeV;
    SamplerAddressMode  addressModeW;
    F32                 minLod;
    F32                 maxLod;
    Filter              magFilter;
    Filter              minFilter;
    CompareOp           compareOp;
    SamplerMipMapMode   mipMapMode;
    F32                 maxAnisotropy;
    F32                 mipLodBias;
    BorderColor         borderColor;
};


struct MemoryReserveDescription 
{
    //< Memory resource amount, in bytes, per usage index.
    U64 bufferPools[ResourceMemoryUsage_Count];
    //< Memory resoure amount, in bytes, for textures on the gpu.
    U64 texturePoolGPUOnly;
};


enum GraphicsQueueType 
{
    QUEUE_TYPE_PRESENT      = (1 << 0),
    QUEUE_TYPE_GRAPHICS     = (1 << 1),
    QUEUE_TYPE_COMPUTE      = (1 << 2),
    QUEUE_TYPE_COPY         = (1 << 3)
};

typedef U32 GraphicsQueueTypeFlags;


struct GraphicsResourceDebug
{
    U64 maxMemoryGpuSizeBytes;
    U64 usedMemoryGpuSizeBytes;
};

// Common Graphics Object manager.
class IGraphicsObject
{
public:
    IGraphicsObject()
    { }

    virtual ~IGraphicsObject() { }

    virtual GraphicsAPI getApi() const = 0;

    // Get the unique id of this graphics object.
    virtual GraphicsId getId() const = 0;

    // This function is called for everytime an object is created. Ensure this 
    // assigns a unique id to it!
    virtual void generateId() { }

    virtual void setDebugName(const char* name) { }
};
} // Recluse