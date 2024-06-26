//
#include "IGraphicsDevice.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include <vector>

namespace Recluse {
namespace CSharp {


Recluse::ResourceViewDimension CSharpToDimension(CSharp::ResourceViewDimension Dim)
{
    switch (Dim)
    {
        case ResourceViewDimension::Buffer:
            return ResourceViewDimension_Buffer;
        case ResourceViewDimension::Cube:
            return ResourceViewDimension_Cube;
        case ResourceViewDimension::CubeArray:
            return ResourceViewDimension_CubeArray;
        case ResourceViewDimension::Dim1d:
            return ResourceViewDimension_1d;
        case ResourceViewDimension::Dim1dArray:
            return ResourceViewDimension_1dArray;
        case ResourceViewDimension::Dim2d:
            return ResourceViewDimension_2d;
        case ResourceViewDimension::Dim2dArray:
            return ResourceViewDimension_2dArray;
        case ResourceViewDimension::Dim2dMultisample:
            return ResourceViewDimension_2dMultisample;
        case ResourceViewDimension::Dim2dMultisampleArray:
            return ResourceViewDimension_2dMultisampleArray;
        case ResourceViewDimension::Dim3d:
            return ResourceViewDimension_3d;
        case ResourceViewDimension::None:
        default:
            return ResourceViewDimension_None;
    }
}


Recluse::ResourceViewType CSharpToViewType(CSharp::ResourceViewType Type)
{
    switch (Type)
    {
        case ResourceViewType::RenderTarget:
            return Recluse::ResourceViewType_RenderTarget;
        case ResourceViewType::UnorderedAccess:
            return Recluse::ResourceViewType_UnorderedAccess;
        case ResourceViewType::DepthStencil:
            return Recluse::ResourceViewType_DepthStencil;
        case ResourceViewType::ShaderResource:
        default:
            return Recluse::ResourceViewType_ShaderResource;
    }
}


Recluse::ResourceFormat CSharpToNativeFormat(CSharp::ResourceFormat format)
{
    switch (format)
    {
        case ResourceFormat::R32_Uint:
            return ResourceFormat_R32_Uint;
        case ResourceFormat::R24_Unorm_X8_Typeless:
            return ResourceFormat_R24_Unorm_X8_Typeless;
        case ResourceFormat::R32G32_Uint:
            return ResourceFormat_R32G32_Uint;
        case ResourceFormat::R32G32_Float:
            return ResourceFormat_R32G32_Float;
        case ResourceFormat::R32G32B32A32_Uint:
            return ResourceFormat_R32G32B32A32_Uint;
        case ResourceFormat::R32G32B32A32_Float:
            return ResourceFormat_R32G32B32A32_Float;
        case ResourceFormat::R8_Uint:
            return ResourceFormat_R8_Uint;
        case ResourceFormat::R16_Float:
            return ResourceFormat_R16_Float;
        case ResourceFormat::R32_Float:
            return ResourceFormat_R32_Float;
        case ResourceFormat::R16G16_Float:
            return ResourceFormat_R16G16_Float;
        case ResourceFormat::R16_Uint:
            return ResourceFormat_R16_Uint;
        case ResourceFormat::D24_Unorm_S8_Uint:
            return ResourceFormat_D24_Unorm_S8_Uint;
        case ResourceFormat::D16_Unorm:
            return ResourceFormat_D16_Unorm;
        case ResourceFormat::BC7_Unorm:
            return ResourceFormat_BC7_Unorm;
        case ResourceFormat::BC5_Unorm:
            return ResourceFormat_BC5_Unorm;
        case ResourceFormat::BC4_Unorm:
            return ResourceFormat_BC4_Unorm;
        case ResourceFormat::BC3_Unorm:
            return ResourceFormat_BC3_Unorm;
        case ResourceFormat::BC2_Unorm:
            return ResourceFormat_BC2_Unorm;
        case ResourceFormat::BC1_Unorm:
            return ResourceFormat_BC1_Unorm;
        case ResourceFormat::B8G8R8A8_Srgb:
            return ResourceFormat_B8G8R8A8_Srgb;
        case ResourceFormat::R8G8B8A8_Unorm:
            return ResourceFormat_R8G8B8A8_Unorm;
        case ResourceFormat::B8G8R8A8_Unorm:
            return ResourceFormat_B8G8R8A8_Unorm;
        case ResourceFormat::Unknown:
        default:
            return ResourceFormat_Unknown;
    }
};


Recluse::ResourceState CSharpToNativeState(CSharp::ResourceState state)
{
    switch (state)
    {
        case ResourceState::ShaderResource:
            return Recluse::ResourceState_ShaderResource;
        case ResourceState::UnorderedAccess:
            return Recluse::ResourceState_UnorderedAccess;
        case ResourceState::RenderTarget:
            return Recluse::ResourceState_RenderTarget;
        case ResourceState::Present:
            return Recluse::ResourceState_Present;
        case ResourceState::IndirectArgs:
            return Recluse::ResourceState_IndirectArgs;
        case ResourceState::IndexBuffer:
            return Recluse::ResourceState_IndexBuffer;
        case ResourceState::DepthStencilWrite:
            return Recluse::ResourceState_DepthStencilWrite;
        case ResourceState::DepthStencilReadyOnly:
            return Recluse::ResourceState_DepthStencilReadOnly;
        case ResourceState::CopySource:
            return Recluse::ResourceState_CopySource;
        case ResourceState::CopyDestination:
            return Recluse::ResourceState_CopyDestination;
        case ResourceState::AccelerationStructure:
            return Recluse::ResourceState_AccelerationStructure;
        case ResourceState::Common:
        default:
            return Recluse::ResourceState_Common;
    }
}


Recluse::ResourceMemoryUsage CSharpToNativeMemoryUsage(CSharp::ResourceMemoryUsage Usage)
{
    switch (Usage)
    {
        case ResourceMemoryUsage::CpuVisible:
            return ResourceMemoryUsage_CpuVisible;
        case ResourceMemoryUsage::CpuToGpu:
            return ResourceMemoryUsage_CpuToGpu;
        case ResourceMemoryUsage::GpuToCpu:
            return ResourceMemoryUsage_GpuToCpu;
        case ResourceMemoryUsage::GpuOnly:
        default:
            return ResourceMemoryUsage_GpuOnly;
    }
}


Recluse::ResourceDimension CSharpToNativeResourceDimension(CSharp::ResourceDimension Dim)
{
    switch (Dim)
    {
        case ResourceDimension::Dim1d:
            return ResourceDimension_1d;
        case ResourceDimension::Dim2d:
            return ResourceDimension_2d;
        case ResourceDimension::Dim3d:
            return ResourceDimension_3d;
        case ResourceDimension::Buffer:
        default:
            return ResourceDimension_Buffer;
    }
}


IResource::IResource(GraphicsResource* Resource)
    : Resource(Resource)
{
}


IResource::IResource(IGraphicsDevice^ Device, const ResourceCreateInformation^ CreateInfo, ResourceState InitialState)
{
    Recluse::GraphicsResourceDescription description = { };
    description.width = CreateInfo->Width;
    description.height = CreateInfo->Height;
    description.depthOrArraySize = CreateInfo->DepthOrArraySize;
    description.format = CSharpToNativeFormat(CreateInfo->Format);
    description.usage = (U32)CreateInfo->Usage;
    description.memoryUsage = CSharpToNativeMemoryUsage(CreateInfo->MemoryUsage);
    description.dimension = CSharpToNativeResourceDimension(CreateInfo->Dimension);
    description.mipLevels = (U32)CreateInfo->MipLevels;
    description.samples = (U32)CreateInfo->Samples;
    description.name;

    GraphicsDevice* pDevice = Device->GetNative();
    GraphicsResource* res = nullptr;
    ResultCode result = pDevice->createResource(&res, description, CSharpToNativeState(InitialState));
    Resource = res;
}


System::UIntPtr IResource::Map(System::UInt64 ReadOffsetBytes, System::UInt64 ReadSizeBytes)
{
    if (ReadSizeBytes == 0 && ReadOffsetBytes == 0)
    {
        void* ptr = nullptr;
        Resource->map(&ptr, nullptr);
        return (System::UIntPtr)ptr;
    }
    return (System::UIntPtr)0ull;
}


System::UInt32 IResource::Unmap(System::UIntPtr Ptr, System::UInt64 WriteOffsetBytes, System::UInt64 WriteSizeBytes)
{
    void* ptr = (void*)Ptr;
    MapRange range = { };
    if (WriteOffsetBytes == 0 && WriteSizeBytes == 0)
    {
        Resource->unmap(nullptr);
    }
    return (System::UInt32)0;
}


System::UIntPtr IResource::AsView(CSharp::ResourceViewType ViewType, CSharp::ResourceViewDimension Dim, CSharp::ResourceFormat Format, System::UInt16 BaseLayer, System::UInt16 BaseMip, System::UInt16 Layers, System::UInt16 Mips)
{
    ResourceViewDescription ViewDesc = { };
    ViewDesc.baseArrayLayer = (U32)BaseLayer;
    ViewDesc.baseMipLevel = (U32)BaseMip;
    ViewDesc.type = CSharpToViewType(ViewType);
    ViewDesc.format = CSharpToNativeFormat(Format);
    ViewDesc.dimension = CSharpToDimension(Dim);
    ViewDesc.layerCount = (U32)Layers;
    ViewDesc.mipLevelCount = (U32)Mips;
    ResourceViewId id = Resource->asView(ViewDesc);
    return (UIntPtr)id;
}

    

IGraphicsDevice::IGraphicsDevice(GraphicsApi graphicsApi, System::String^ appName, System::String^ engineName)
    : m_instance(nullptr)
    , m_device(nullptr)
    , m_adapter(nullptr)
{
    switch (graphicsApi)
    {
        case CSharp::GraphicsApi::Direct3D12:
            m_instance = GraphicsInstance::create(GraphicsApi_Direct3D12);
            break;
        case CSharp::GraphicsApi::Vulkan:
            m_instance = GraphicsInstance::create(GraphicsApi_Vulkan);
        default:
            break;
    }
    R_ASSERT(m_instance);
    ApplicationInfo appInfo = { };
    appInfo.engineName = "";
    appInfo.appName = "";
    ResultCode result = m_instance->initialize(appInfo, 0);
    std::vector<GraphicsAdapter*>& adapters = m_instance->getGraphicsAdapters();
    R_ASSERT(!adapters.empty());
    m_adapter = adapters[0];
    R_ASSERT(m_adapter);
    DeviceCreateInfo deviceCreateInfo = { };
    GraphicsDevice* device = nullptr;
    result = m_adapter->createDevice(deviceCreateInfo, &device);
    m_device = device;
}


IGraphicsDevice::~IGraphicsDevice()
{
    R_ASSERT(m_instance);
    R_ASSERT(m_adapter);
    R_ASSERT(m_device);
    m_adapter->destroyDevice(m_device);
    GraphicsInstance::destroyInstance(m_instance);
}



System::String^ IGraphicsDevice::GetString()
{
    return "I am a graphics device. Don't mind me. I'm managed!";
}


IGraphicsContext::IGraphicsContext(IGraphicsDevice^ device)
    : Context(nullptr)
    , DeviceRef(nullptr)
{
    if (device != nullptr)
    {
        DeviceRef = device->GetNative();
    }
    
    // Create the context.
    Context = DeviceRef->createContext();
}

ISwapchain::ISwapchain(IGraphicsDevice^ Device, System::IntPtr WindowHandle,  ResourceFormat format, System::Int32 width, System::Int32 height, System::UInt32 numFrames, FrameBuffering frameBuffering)
    : DeviceRef(nullptr)
    , Swapchain(nullptr)
{
    DeviceRef = Device->GetNative();
    SwapchainCreateDescription swapchainDescription = { };
    swapchainDescription.desiredFrames = static_cast<U32>(numFrames);
    swapchainDescription.format = CSharpToNativeFormat(format);
    swapchainDescription.renderWidth = width;
    swapchainDescription.renderHeight = height;
    Recluse::FrameBuffering nativeFrameBuffering = Recluse::FrameBuffering_Single;

    switch (frameBuffering)
    {
        case CSharp::FrameBuffering::Double:
            swapchainDescription.buffering = FrameBuffering_Double;
            break;
        case CSharp::FrameBuffering::Triple:
            swapchainDescription.buffering = FrameBuffering_Triple;
            break;
        case CSharp::FrameBuffering::Single:
        default:
            swapchainDescription.buffering = FrameBuffering_Single;
            break;
    }

    void* windowPtr = (void*)WindowHandle;
    CreateSwapchain(DeviceRef, windowPtr, swapchainDescription);
    R_ASSERT(Swapchain);
}


ISwapchain::~ISwapchain()
{
    if (Swapchain)
    {
        DeviceRef->destroySwapchain(Swapchain);
    }
}


IGraphicsContext::~IGraphicsContext()
{
    if (Context)
    {
        DeviceRef->releaseContext(Context);
    }
}


void IGraphicsContext::Begin()
{
    Context->begin();
}


void IGraphicsContext::End()
{
    R_ASSERT(Context != nullptr);
    Context->end();
}


void ISwapchain::Prepare(IGraphicsContext^ Context)
{
    R_ASSERT(Context != nullptr);
    Swapchain->prepare(Context->GetContextHandle());
}


void ISwapchain::Present(IGraphicsContext^ Context)
{
    R_ASSERT(Swapchain != nullptr);
    GraphicsContext* NativeContext = Context->GetContextHandle();
    ResultCode result = Swapchain->present(NativeContext);
    if (result == RecluseResult_NeedsUpdate)
    {
        NativeContext->wait();
        Swapchain->rebuild(Swapchain->getDesc());
        QuerySwapchainFrames();
    }
}


void IGraphicsContext::SetContextFrame(System::Int32 frames)
{
    R_ASSERT(Context != nullptr);
    Context->setFrames(frames);
}


IResource^ ISwapchain::GetCurrentFrame()
{
    U32 currFrameIndex = Swapchain->getCurrentFrameIndex();
    System::Object^ SwapchainFrame = SwapchainFrames[currFrameIndex];
    return dynamic_cast<IResource^>(SwapchainFrame);
}


void IGraphicsContext::Transition(IResource^ resource, CSharp::ResourceState toState)
{
    R_ASSERT(Context != nullptr);
    Recluse::ResourceState resourceState = CSharpToNativeState(toState);
    Context->transition(resource(), resourceState);
}


void IGraphicsContext::BindRenderTargets(array<System::UIntPtr>^ RenderTargetViews, System::UIntPtr DepthStencil)
{
    U32 NumRenderTargets = RenderTargetViews->Length;
    std::vector<ResourceViewId> ids(RenderTargetViews->Length);
    for (U32 i = 0; i < RenderTargetViews->Length; ++i)
    {
        ResourceViewId id = static_cast<ResourceViewId>(RenderTargetViews[i]);
        ids[i] = id;
    }
    ResourceViewId depthStencilId = static_cast<ResourceViewId>(DepthStencil);
    Context->bindRenderTargets(NumRenderTargets, ids.data(), depthStencilId);
}


void ISwapchain::QuerySwapchainFrames()
{
    SwapchainFrames = gcnew System::Collections::ArrayList();
    const SwapchainCreateDescription& actualDesc = Swapchain->getDesc();
    for (U32 i = 0; i < actualDesc.desiredFrames; ++i)
    {
        GraphicsResource* NativeSwapchainResource = Swapchain->getFrame(i);
        IResource^ SwapchainResource = gcnew IResource(NativeSwapchainResource);
        SwapchainFrames->Add(SwapchainResource);
    }
}


void IGraphicsContext::ClearRenderTarget(System::UInt32 RenderTargetIndex, array<System::Single>^ Color, Rect^ RectArea)
{
    F32 colors[4];
    Recluse::Rect rect = { };
    colors[0]   = static_cast<F32>(Color[0]);
    colors[1]   = static_cast<F32>(Color[1]);
    colors[2]   = static_cast<F32>(Color[2]);
    colors[3]   = static_cast<F32>(Color[3]);
    rect.x      = static_cast<F32>(RectArea->X);
    rect.y      = static_cast<F32>(RectArea->Y);
    rect.width  = static_cast<F32>(RectArea->Width);
    rect.height = static_cast<F32>(RectArea->Height);
    Context->clearRenderTarget((U32)RenderTargetIndex, colors, rect);
}


void ISwapchain::CreateSwapchain(GraphicsDevice* DeviceRef, void* WindowPtr, const SwapchainCreateDescription& Description)
{
    Swapchain = DeviceRef->createSwapchain(Description, WindowPtr);
    R_ASSERT(Swapchain);
    QuerySwapchainFrames();
}


Rect::Rect(System::Single X, System::Single Y, System::Single Width, System::Single Height)
    : X(X)
    , Y(Y)
    , Width(Width)
    , Height(Height)
{
}


GraphicsHost::GraphicsHost(const String^ HostName)
    : HostName(HostName)
{
}


void ISwapchain::ResizeSwapchain(System::Int32 Width, System::Int32 Height)
{
    SwapchainCreateDescription newDescription = Swapchain->getDesc();
    newDescription.renderWidth = static_cast<U32>(Width);
    newDescription.renderHeight = static_cast<U32>(Height);
    Swapchain->rebuild(newDescription);
    QuerySwapchainFrames();
}


void IGraphicsContext::ClearDepthStencil(CSharp::ClearFlags Flags, System::Single ClearDepth, System::Byte ClearStencil, Rect^ RectArea)
{
    Recluse::ClearFlags flags = Recluse::ClearFlags(Flags);
    Recluse::Rect rect = { };
    rect.x = static_cast<F32>(RectArea->X);
    rect.y = static_cast<F32>(RectArea->Y);
    rect.width = static_cast<F32>(RectArea->Width);
    rect.height = static_cast<F32>(RectArea->Height);
    Context->clearDepthStencil(flags, (F32)ClearDepth, (U8)ClearStencil, rect);
}


void IGraphicsContext::Wait()
{
    Context->wait();
}


void IResource::CopyFrom(array<System::Byte>^ Memory)
{
    UIntPtr BufferMemPtr = Map(0, 0);
    Marshal::Copy(Memory, 0, (IntPtr)BufferMemPtr.ToPointer(), Memory->Length);
    Unmap(BufferMemPtr, 0, 0);
}


void IGraphicsContext::CopyResource(IResource^ Dst, IResource^ Src)
{
    Context->copyResource(Dst(), Src());
}


void IGraphicsContext::CopyBufferRegions(IResource^ Dst, IResource^ Src, array<CSharp::CopyBufferRegion^>^ Regions)
{
    std::vector<Recluse::CopyBufferRegion> regions(Regions->Length);
    for (U32 i = 0; i < regions.size(); ++i)
    {
        CSharp::CopyBufferRegion^ Region = Regions[i];
        regions[i].dstOffsetBytes = Region->DstOffsetBytes;
        regions[i].srcOffsetBytes = Region->SrcOffsetBytes;
        regions[i].szBytes = Region->SizeBytes;
    }
    Context->copyBufferRegions(Dst(), Src(), regions.data(), regions.size());
}


void IGraphicsDevice::CopyResource(IResource^ Dst, IResource^ Src)
{
    m_device->copyResource(Dst(), Src());
}


void IGraphicsDevice::CopyBufferRegions(IResource^ Dst, IResource^ Src, array<CSharp::CopyBufferRegion^>^ Regions)
{
    std::vector<Recluse::CopyBufferRegion> regions(Regions->Length);
    for (U32 i = 0; i < regions.size(); ++i)
    {
        CSharp::CopyBufferRegion^ Region = Regions[i];
        regions[i].dstOffsetBytes = Region->DstOffsetBytes;
        regions[i].srcOffsetBytes = Region->SrcOffsetBytes;
        regions[i].szBytes = Region->SizeBytes;
    }
    m_device->copyBufferRegions(Dst(), Src(), regions.data(), regions.size());
}
} // CSharp
} // Recluse