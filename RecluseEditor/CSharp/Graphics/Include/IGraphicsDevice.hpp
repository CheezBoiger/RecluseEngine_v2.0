//
#ifndef GRAPHICS_DEVICE_HPP
#define GRAPHICS_DEVICE_HPP
#pragma once
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/System/Window.hpp"
#include <memory>
#include <vector>
#include <vcclr.h>
#include <msclr/marshal.h>

#using <WindowsBase.dll>
#using <PresentationCore.dll>
#using <PresentationFramework.dll>
#using <mscorlib.dll>

#pragma managed

namespace Recluse {
namespace CSharp {

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Interop;
using namespace System::Windows::Input;
using namespace System::Windows::Media;
using namespace System::Runtime::InteropServices;

public enum class GraphicsApi : System::Int32
{
    Direct3D12,
    Vulkan
};


public enum class FrameBuffering : System::Int32
{
    Single,
    Double,
    Triple
};


public enum class ResourceState : System::Int32
{
    Common,
    UnorderedAccess,
    RenderTarget,
    VertexBuffer,
    IndexBuffer,
    CopySource,
    CopyDestination,
    ShaderResource,
    DepthStencilReadyOnly,
    DepthStencilWrite,
    Present, 
    IndirectArgs,
    AccelerationStructure
};


// Should follow the ResourceFormat in Recluse Graphics Framework.
public enum class ResourceFormat : System::Int32
{
    Unknown,
    R8G8B8A8_Unorm,
    R16G16B16A16_Float,
    R11G11B10_Float,
    D32_Float,
    R32_Float,
    D16_Unorm,
    D24_Unorm_S8_Uint,
    D32_Float_S8_Uint,
    R24_Unorm_X8_Typeless,
    X24_Typeless_S8_Uint,
    R16G16_Float,
    B8G8R8A8_Srgb,
    R32G32B32A32_Float,
    R32G32B32A32_Uint,
    R8_Uint,
    R32G32_Float,
    R32G32_Uint,
    R16_Uint,
    R16_Float,
    B8G8R8A8_Unorm,
    R32G32B32_Float,
    R32_Uint,
    R32_Int,
    BC1_Unorm,
    BC2_Unorm,
    BC3_Unorm,
    BC4_Unorm,
    BC5_Unorm,
    BC7_Unorm,
};


public enum class ResourceViewType : System::Int32
{
    RenderTarget,
    ShaderResource,
    UnorderedAccess,
    DepthStencil
};


public enum class ResourceViewDimension : System::Int32
{
    None,
    Buffer,
    Dim1d,
    Dim1dArray,
    Dim2d,
    Dim2dMultisample,
    Dim2dMultisampleArray,
    Dim2dArray,
    Dim3d,
    Cube,
    CubeArray,
    RayTraceAccelerationStructure
};


public enum class ResourceUsage : System::Int32
{
    VertexBuffer    = (1 << 0),
    IndexBuffer     = (1 << 1),
    RenderTarget    = (1 << 2),
    ShaderResource  = (1 << 3),
    ConstantBuffer  = (1 << 4),
    TransferDst     = (1 << 5),
    CopyDst         = TransferDst,
    TransferSrc     = (1 << 6),
    CopySrc         = TransferSrc,
    IndirectBuffer  = (1 << 7),
    DepthStencil    = (1 << 8),
    UnorderedAccess = (1 << 9),
    AccelerationStructure = (1 << 10)
};


public enum class ResourceMemoryUsage : System::Int32
{
    CpuVisible,
    GpuOnly,
    CpuToGpu,
    GpuToCpu
};


public enum class ResourceDimension : System::Int32
{
    Buffer,
    Dim1d,
    Dim2d,
    Dim3d
};


public enum class ClearFlags : System::Int32
{
    None,
    Color = (1 << 0),
    Depth = (1 << 1),
    Stencil = (1 << 2)
};


public enum class ShaderStage : System::Int32
{
    None                    = (ShaderType_None),
    Vertex                  = (1<<ShaderType_Vertex),
    Hull                    = (1<<ShaderType_Hull),
    TessellationControl     = Hull,
    Domain                  = (1<<ShaderType_Domain),
    TessellationEvaluation  = Domain,
    Geometry                = (1<<ShaderType_Geometry),
    Pixel                   = (1<<ShaderType_Pixel),
    Fragment                = Pixel,
    RayGeneration           = (1<<ShaderType_RayGeneration),
    RayClosestHit           = (1<<ShaderType_RayClosestHit),
    RayAnyHit               = (1<<ShaderType_RayAnyHit),
    RayIntersect            = (1<<ShaderType_RayIntersect),
    RayMiss                 = (1<<ShaderType_RayMiss),
    Amplification           = (1<<ShaderType_Amplification),
    Task                    = Amplification,
    Mesh                    = (1<<ShaderType_Mesh),
    Compute                 = (1<<ShaderType_Compute),
    All                     = ShaderStage_All
};


public enum class SamplerAddressMode : System::Int32
{
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge
};


public enum class Filter : System::Int32
{
    // Defines linear filtering
    Linear,
    // Defines nearest neighbor filtering.
    Nearest,
    // Defines cubic filtering.
    Cubic
};


public enum class SamplerMipMapMode : System::Int32
{
    // Nearest neighbor interpolation for mip maps.
    Nearest,
    // Linear interpolation for mip maps.
    Linear
};


public enum class CompareOp : System::Int32
{
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always
};


public enum class PolygonMode : System::Int32
{
    Fill,
    Line,
    Point
};


public enum class CullMode : System::Int32 
{
    None,
    Front,
    Back,
    FrontAndBack
};


public enum class PrimitiveTopology : System::Int32
{
    TriangleList,
    TriangleStrip,
    PointList,
    LineList,
    LineStrip
};


public enum class FrontFace : System::Int32
{
    CounterClockwise,
    Clockwise
};


public enum class BorderColor : System::Int32
{
    TransparentBlack,
    OpaqueBlack,
    OpaqueWhite
};


public enum class InputRate : System::Int32
{
    PerVertex,
    PerInstance
};


public enum class Semantic : System::Int32
{
    Position = Semantic_Position,   
    Normal = Semantic_Normal,
    Texcoord = Semantic_Texcoord,
    Binormal = Semantic_Binormal,
    Tangent = Semantic_Tangent,
    Color = Semantic_Color,
    TessFactor = Semantic_TessFactor
};


public enum class IndexType : System::UInt32
{
    Unsigned16,
    Unsigned32
};


public enum class ColorComponent : System::UInt32
{
    Red = Color_R,
    Green = Color_G,
    Blue = Color_B,
    Alpha = Color_A,
    Rgba = Color_Rgba
};


public enum class BlendOp : System::UInt32
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};


public enum class LogicOp : System::Int32
{
    Clear,
    And,
    AndReverse,
    Copy,
    AndInverted,
    NoOp,
    Xor,
    Or,
    Nor,
    Equivalent,
    Invert,
    OrReverse,
    CopyInverted,
    OrInverted,
    Nand,
    Set
};


public enum class BlendFactor : System::Int32
{
    Zero,
    One,
    SourceColor,
    OneMinusSourceColor,
    DestinationColor,
    OneMinusDestinationColor,
    SourceAlpha,
    OneMinusSourceAlpha,
    DestinationAlpha,
    OneMinusDestinationAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SourceAlphaSaturate,
    SourceOneColor,
    OneMinusSourceOneColor,
    SourceOneAlpha,
    OneMinusSourceOneAlpha
};


public value struct ResourceCreateInformation
{
    System::UInt32 Width;
    System::UInt32 Height;
    System::UInt32 DepthOrArraySize;
    System::UInt32 MipLevels;
    System::UInt32 Samples;
    ResourceFormat Format;
    ResourceUsage Usage;
    ResourceMemoryUsage MemoryUsage;
    ResourceDimension Dimension;
    System::String^ Name;
};


public value struct Rect
{
    System::Single X;
    System::Single Y;
    System::Single Width;
    System::Single Height;

    Rect(System::Single X, System::Single Y, System::Single Width, System::Single Height);
};


public value struct Viewport
{
    System::Single X;
    System::Single Y;
    System::Single Width;
    System::Single Height;
    System::Single MinDepth;
    System::Single MaxDepth;

    Viewport(System::Single X, System::Single Y, System::Single Width, System::Single Height, System::Single MinD, System::Single MaxD)
        : X(X)
        , Y(Y)
        , Width(Width)
        , Height(Height)
        , MinDepth(MinD)
        , MaxDepth(MaxD)
    {
    }
};


public value struct CopyBufferRegion
{
    System::UInt32 DstOffsetBytes;
    System::UInt32 SrcOffsetBytes;
    System::UInt32 SizeBytes;
};


ref class IResource;
ref class IShaderProgramDefinition;
value struct IVertexInputLayout;

// Graphics device.
R_PUBLIC_API public ref class IGraphicsDevice
{
public:
    IGraphicsDevice(CSharp::GraphicsApi graphicsApi, System::String^ appName, System::String^ engineName, bool EnableDebugLayer);
    ~IGraphicsDevice();
    !IGraphicsDevice();

    void CopyResource(IResource^ Dst, IResource^ Src);
    void CopyBufferRegions(IResource^ Dst, IResource^ Src, array<CSharp::CopyBufferRegion^>^ Regions);

    void LoadShaderProgram(System::UInt64 ProgramId, System::UInt64 Permutation, IShaderProgramDefinition^ Definition);
    void MakeVertexLayout(System::UInt64 VertexLayoutId, IVertexInputLayout^ VertexLayout);

    void UnloadShaderProgram(System::UInt64 ProgramId);
    void DestroyVertexLayout(System::UInt64 VertexLayoutId);

    GraphicsApi GetApi() { return m_api; }

    GraphicsDevice* GetNative() { return m_device; }
    GraphicsDevice* operator()() { return GetNative(); }

private:
    GraphicsInstance* m_instance;
    GraphicsAdapter* m_adapter;
    GraphicsDevice* m_device;
    GraphicsApi     m_api;
};


public value struct IVertexAttribute
{
    enum class Helper { OffsetAppend = Recluse::VertexAttribute::OffsetAppend };
    System::UInt32          Location;
    System::UInt32          OffsetBytes;
    CSharp::ResourceFormat  Format;
    CSharp::Semantic        SemanticTag;
    System::UInt32          SemanticIndex;
};


public value struct IVertexBinding
{
    System::UInt32 Binding;
    System::UInt32 Stride;
    InputRate      Rate;
    System::Collections::ArrayList^ VertexAttributes;
};


public value struct IVertexInputLayout
{
public:
    enum class Limits { BindingCount = VertexInputLayout::VertexInputLayout_BindingCount };
    enum class Constants { Null = VertexInputLayout::VertexLayout_Null };
    System::Collections::ArrayList^ VertexBindings;
};


// Resource object.
R_PUBLIC_API public ref class IResource
{
public:
    IResource(IGraphicsDevice^ Device, const ResourceCreateInformation^ CreateInfo, ResourceState InitialState);
    IResource(GraphicsResource* Resource);

    ~IResource();
    !IResource();

    System::UIntPtr Map(System::UInt64 ReadOffsetBytes, System::UInt64 ReadSizeBytes);
    System::UInt32 Unmap(System::UIntPtr Ptr, System::UInt64 WriteOffsetBytes, System::UInt64 WriteSizeBytes);
    void CopyFrom(array<System::Byte>^ Memory);
    System::UIntPtr AsView(CSharp::ResourceViewType ViewType, CSharp::ResourceViewDimension Dim, CSharp::ResourceFormat Format, System::UInt16 BaseLayer, System::UInt16 BaseMip, System::UInt16 Layers, System::UInt16 Mips);

    GraphicsResource* operator ()() { return Resource; }

    void MarkToReleaseImmediately() { m_releaseImmediately = true; }
private:
    GraphicsResource* Resource;
    GraphicsDevice* DeviceRef;
    Bool m_releaseImmediately;
};


public ref class ISampler
{
public:
    ISampler(IGraphicsDevice^ Device, 
        CSharp::SamplerAddressMode AddressModeU, 
        CSharp::SamplerAddressMode AddressModeV, 
        CSharp::SamplerAddressMode AddressModeW,
        System::Single MinLod,
        System::Single MaxLod,
        CSharp::Filter MagFilter,
        CSharp::Filter MinFilter,
        CSharp::CompareOp CompareOperation,
        CSharp::SamplerMipMapMode MipMapMode,
        CSharp::Single MaxAnisotropy,
        CSharp::Single MipLodBias,
        CSharp::BorderColor BorderColour);
    ~ISampler();
    // Finalizer.
    !ISampler();

    GraphicsSampler* operator()() { return Sampler; }
private:
    GraphicsSampler* Sampler;
    GraphicsDevice* DeviceRef;
};


public ref class ShaderProgramBinder
{
public:
    ShaderProgramBinder(IShaderProgramBinder& binder);

    ShaderProgramBinder^ BindShaderResource(CSharp::ShaderStage Stage, System::UInt32 Slot, System::UIntPtr View);
    ShaderProgramBinder^ BindUnorderedAccessView(CSharp::ShaderStage Stage, System::UInt32 Slot, System::UIntPtr View);
    ShaderProgramBinder^ BindConstantBuffer(CSharp::ShaderStage Stage, System::UInt32 Slot, IResource^ Resource, System::UInt32 OffsetBytes, System::UInt32 SizeBytes, array<System::Byte>^ Data);
    ShaderProgramBinder^ BindSampler(CSharp::ShaderStage Stage, System::UInt32 Slot, ISampler^ Sampler);
private:
    IShaderProgramBinder& ShaderProgram;
};


/// <summary> 
/// Graphics Context.
/// </summary>
public ref class IGraphicsContext 
{
public:
    IGraphicsContext(IGraphicsDevice^ device);
    ~IGraphicsContext();
    !IGraphicsContext();

    void SetContextFrame(System::Int32 frames);
    void Begin();
    void BindRenderTargets(array<System::UIntPtr>^ RenderTargetViews, System::UIntPtr DepthStencil);
    void ClearRenderTarget(System::UInt32 RenderTargetIndex, array<System::Single>^ Color, Rect^ RectArea);
    void ClearDepthStencil(ClearFlags Flags, System::Single ClearDepth, System::Byte ClearStencil, Rect^ RectArea);
    void Transition(IResource^ resource, CSharp::ResourceState toState);
    void CopyResource(IResource^ Dst, IResource^ Src);
    void CopyBufferRegions(IResource^ Dst, IResource^ Src, array<CSharp::CopyBufferRegion^>^ Regions);
    void BindVertexBuffers(array<IResource^>^ VertexBuffers, array<System::UInt64>^ Offsets);
    void BindIndexBuffer(IResource^ IndexBuffer, System::UInt64 OffsetBytes, CSharp::IndexType Type);
    void EnableDepth(System::Boolean Enable);
    void EnableDepthWrite(System::Boolean Enable);
    void EnableStencil(System::Boolean Enable);
    void SetInputVertexLayout(System::UInt64 InputLayout);
    void SetCullMode(CullMode Mode);
    void SetFrontFace(FrontFace Face);
    void SetDepthCompareOp(CompareOp CompareOperation);
    void SetDepthClampEnable(System::Boolean Enable);
    void SetDepthBiasEnable(System::Boolean Enable);
    void SetDepthBiasClamp(System::Single Scale);
    void SetStencilReference(System::Byte StencilRef);
    void SetStencilWriteMask(System::Byte Mask);
    void SetStencilReadMask(System::Byte Mask);
    void SetTopology(PrimitiveTopology Topology);
    void SetPolygonMode(PolygonMode Mode);
    void SetLineWidth(System::Single Width);

    void SetColorWriteMask(System::UInt32 RtIndex, ColorComponent WriteMask);
    void SetBlendEnable(System::UInt32 RtIndex, System::Boolean Enable);
    void SetBlendLogicOpEnable(System::Boolean Enable);
    void SetBlendLogicOp(LogicOp LogicOperation);
    void SetBlendConstants(array<System::Single^>^ BlendConstants);
    void SetBlend(System::UInt32 RtIndex, BlendFactor SrcColorFactor, BlendFactor DstColorFactor, BlendOp ColorBlendOp,
                                          BlendFactor SrcAlphaFactor, BlendFactor DstAlphaFactor, BlendOp AlphaOp);
    
    void DrawIndexedInstanced(System::Int32 IndexCount, System::Int32 InstanceCount, System::Int32 FirstIndex, System::Int32 VertexOffset, System::Int32 FirstInstance);
    void DrawInstanced(System::Int32 VertexCount, System::Int32 InstanceCount, System::Int32 FirstVertex, System::Int32 FirstInstance);
    void Dispatch(System::Int32 X, System::Int32 Y, System::Int32 Z);
    ShaderProgramBinder^ BindShaderProgram(System::UInt64 ProgramId, System::UInt32 Permutation);

    void SetScissors(array<CSharp::Rect^>^ Rects);
    void SetViewports(array<CSharp::Viewport^>^ Viewports);
    void End();
    void Wait();
    
    GraphicsContext* GetContextHandle() { return Context; }
    GraphicsContext* operator()() { return GetContextHandle(); }

private:
    
    GraphicsContext* Context;
    GraphicsDevice* DeviceRef;
};


public ref class ISwapchain
{
public:
    ISwapchain(IGraphicsDevice^ Device, System::IntPtr WindowHandle,  ResourceFormat format, System::Int32 width, System::Int32 height, System::UInt32 numFrames, FrameBuffering frameBuffering);
    ~ISwapchain();
    !ISwapchain();

    void ResizeSwapchain(System::Int32 Width, System::Int32 Height);

    void Prepare(IGraphicsContext^ Context);
    void Present(IGraphicsContext^ Context);

    ResourceFormat GetFormat();
    IResource^ GetCurrentFrame();
private:
    void CreateSwapchain(GraphicsDevice* DeviceRef, void* WindowPtr, const SwapchainCreateDescription& Description);
    void QuerySwapchainFrames();

    System::Collections::ArrayList^ SwapchainFrames;
    GraphicsDevice* DeviceRef;
    GraphicsSwapchain* Swapchain;
};


// Handle host creates a window for the graphics system. To be used for 
// this graphics device.
public ref class GraphicsHost : public System::Windows::Interop::HwndHost
{
public:
    GraphicsHost(const String^ HostName);

    virtual HandleRef BuildWindowCore(HandleRef HwndParent) override
    {    
        w = Window::create("GraphicsHost", 0, 0, 300, 200, ScreenMode_WindowBorderless, HwndParent.Handle.ToPointer());
        HWND HostHandle = (HWND)w->getNativeHandle();
        return HandleRef(this, (IntPtr)HostHandle);
    }

    virtual void DestroyWindowCore(HandleRef Hwnd) override
    {
        Window::destroy(w);
    }

private:
    Window* w;
    U32 RenderWidth;
    U32 RenderHeight;
    const String^ HostName;
};
} // CSharp
} // Recluse
#endif // GRAPHICS_DEVICE_HPP