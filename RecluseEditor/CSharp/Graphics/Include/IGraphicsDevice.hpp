//
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/System/Window.hpp"
#include <memory>
#include <vector>
#include <vcclr.h>
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

// Graphics device.
public ref class IGraphicsDevice
{
public:
    IGraphicsDevice(CSharp::GraphicsApi graphicsApi, System::String^ appName, System::String^ engineName);
    ~IGraphicsDevice();
      
    System::String^ GetString();


    GraphicsDevice* GetNative() { return m_device; }
    GraphicsDevice* operator()() { return GetNative(); }

private:
    GraphicsInstance* m_instance;
    GraphicsAdapter* m_adapter;
    GraphicsDevice* m_device;
};


// Resource object.
public ref class IResource
{
public:
    IResource(IGraphicsDevice^ Device, const ResourceCreateInformation^ CreateInfo, ResourceState InitialState);
    IResource(GraphicsResource* Resource);

    System::UIntPtr Map(System::UInt64 ReadOffsetBytes, System::UInt64 ReadSizeBytes);
    System::UInt32 Unmap(System::UIntPtr Ptr, System::UInt64 WriteOffsetBytes, System::UInt64 WriteSizeBytes);
    void CopyFrom(array<System::Byte>^ Memory);
    System::UIntPtr AsView(CSharp::ResourceViewType ViewType, CSharp::ResourceViewDimension Dim, CSharp::ResourceFormat Format, System::UInt16 BaseLayer, System::UInt16 BaseMip, System::UInt16 Layers, System::UInt16 Mips);

    GraphicsResource* operator ()() { return Resource; }
private:
    GraphicsResource* Resource;
};


/// <summary> 
/// Graphics Context.
/// </summary>
public ref class IGraphicsContext 
{
public:
    IGraphicsContext(IGraphicsDevice^ device, System::IntPtr windowHandle, ResourceFormat format, System::Int32 width, System::Int32 height, System::UInt32 numFrames, FrameBuffering frameBuffering);
    ~IGraphicsContext();

    void SetContextFrame(System::Int32 frames);
    void Begin();
    void BindRenderTargets(array<System::UIntPtr>^ RenderTargetViews, System::UIntPtr DepthStencil);
    void ClearRenderTarget(System::UInt32 RenderTargetIndex, array<System::Single>^ Color, Rect^ RectArea);
    void Transition(IResource^ resource, CSharp::ResourceState toState);
    void End();
    void Present();
    void ResizeSwapchain(System::Int32 Width, System::Int32 Height);
    
    IResource^ GetCurrentFrame();

private:

    void CreateSwapchain(GraphicsDevice* DeviceRef, void* WindowPtr, const SwapchainCreateDescription& Description);
    void QuerySwapchainFrames();
    
    GraphicsContext* Context;
    GraphicsSwapchain* Swapchain;
    GraphicsDevice* DeviceRef;

    // Array of managed swapchain frames. These only hold onto the managed allocated frames. Native frames are destroyed
    // when swapchain is destroyed.
    System::Collections::ArrayList^ SwapchainFrames; 
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