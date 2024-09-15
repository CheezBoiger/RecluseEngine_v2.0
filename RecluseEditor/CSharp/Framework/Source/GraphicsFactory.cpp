//
#include "IGraphicsDevice.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include <vector>
#include <list>

namespace Recluse {
namespace CSharp {


R_INTERNAL Recluse::ResourceViewDimension CSharpToDimension(CSharp::ResourceViewDimension Dim)
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


R_INTERNAL Recluse::ResourceViewType CSharpToViewType(CSharp::ResourceViewType Type)
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


R_INTERNAL Recluse::ResourceFormat CSharpToNativeFormat(CSharp::ResourceFormat format)
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
        case ResourceFormat::D32_Float:
            return ResourceFormat_D32_Float;
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


R_INTERNAL CSharp::ResourceFormat NativeFormatToCSharpFormat(Recluse::ResourceFormat Format)
{
    switch (Format)
    {
        case ResourceFormat_R11G11B10_Float:
            return ResourceFormat::R11G11B10_Float;
        case ResourceFormat_R16G16B16A16_Float:
            return ResourceFormat::R16G16B16A16_Float;
        case ResourceFormat_R16G16_Float:
            return ResourceFormat::R16G16_Float;
        case ResourceFormat_R16_Float:
            return ResourceFormat::R16_Float;
        case ResourceFormat_R16_Uint:
            return ResourceFormat::R16_Uint;
        case ResourceFormat_R24_Unorm_X8_Typeless:
            return ResourceFormat::R24_Unorm_X8_Typeless;
        case ResourceFormat_R32G32B32A32_Float:
            return ResourceFormat::R32G32B32A32_Float;
        case ResourceFormat_R32G32B32A32_Uint:
            return ResourceFormat::R32G32B32A32_Uint;
        case ResourceFormat_R32G32_Float:
            return ResourceFormat::R32G32_Float;
        case ResourceFormat_B8G8R8A8_Srgb:
            return ResourceFormat::B8G8R8A8_Srgb;
        case ResourceFormat_B8G8R8A8_Unorm:
            return ResourceFormat::B8G8R8A8_Unorm;
        case ResourceFormat_BC1_Unorm:
            return ResourceFormat::BC1_Unorm;
        case ResourceFormat_BC2_Unorm:
            return ResourceFormat::BC2_Unorm;
        case ResourceFormat_BC3_Unorm:
            return ResourceFormat::BC3_Unorm;
        case ResourceFormat_BC4_Unorm:
            return ResourceFormat::BC4_Unorm;
        case ResourceFormat_BC5_Unorm:
            return ResourceFormat::BC5_Unorm;
        case ResourceFormat_BC7_Unorm:
            return ResourceFormat::BC7_Unorm;
        case ResourceFormat_D24_Unorm_S8_Uint:
            return ResourceFormat::D24_Unorm_S8_Uint;
        case ResourceFormat_D32_Float:
            return ResourceFormat::D32_Float;
        case ResourceFormat_D32_Float_S8_Uint:
            return ResourceFormat::D32_Float_S8_Uint;
        case ResourceFormat_R8_Uint:
            return ResourceFormat::R8_Uint;
        case ResourceFormat_Unknown:
            return ResourceFormat::Unknown;
    }   
}


R_INTERNAL Recluse::ResourceState CSharpToNativeState(CSharp::ResourceState state)
{
    switch (state)
    {
        case ResourceState::ConstantBuffer:
            return Recluse::ResourceState_ConstantBuffer;
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


R_INTERNAL Recluse::ResourceMemoryUsage CSharpToNativeMemoryUsage(CSharp::ResourceMemoryUsage Usage)
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


R_INTERNAL Recluse::ResourceDimension CSharpToNativeResourceDimension(CSharp::ResourceDimension Dim)
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


R_INTERNAL Recluse::FrontFace CSharpToNativeFrontFace(CSharp::FrontFace Face)
{
    switch (Face)
    {
        case FrontFace::Clockwise:
            return FrontFace_Clockwise;
        case FrontFace::CounterClockwise:
        default:
            return FrontFace_CounterClockwise;
    }
}


R_INTERNAL Recluse::CullMode CSharpToNativeCullMode(CSharp::CullMode Mode)
{
    switch (Mode)
    {
        case CullMode::Front:
            return CullMode_Front;
        case CullMode::Back:
            return CullMode_Back;
        case CullMode::FrontAndBack:
            return CullMode_FrontAndBack;
        case CullMode::None:
        default:
            return CullMode_None;
    }
}


R_INTERNAL Recluse::PrimitiveTopology CSharpToNativePrimitiveTopology(CSharp::PrimitiveTopology Topology)
{
    switch (Topology)
    {
        case PrimitiveTopology::TriangleStrip:
            return PrimitiveTopology_TriangleStrip;
        case PrimitiveTopology::PointList:
            return PrimitiveTopology_PointList;
        case PrimitiveTopology::LineList:
            return PrimitiveTopology_LineList;
        case PrimitiveTopology::LineStrip:
            return PrimitiveTopology_LineStrip;
        case PrimitiveTopology::TriangleList:
        default:
            return PrimitiveTopology_TriangleList;
    }
}


R_INTERNAL Recluse::PolygonMode CSharpToNativePolygonMode(CSharp::PolygonMode Mode)
{
    switch (Mode)
    {
        case PolygonMode::Line:
            return PolygonMode_Line;
        case PolygonMode::Point:
            return PolygonMode_Point;
        case PolygonMode::Fill:
        default:
            return PolygonMode_Fill;
    }
}


R_INTERNAL Recluse::SamplerAddressMode getSamplerAddressMode(CSharp::SamplerAddressMode samplerAddress)
{
    switch (samplerAddress)
    {
        case CSharp::SamplerAddressMode::ClampToBorder:
            return SamplerAddressMode_ClampToBorder;
        case CSharp::SamplerAddressMode::ClampToEdge:
            return SamplerAddressMode_ClampToEdge;
        case CSharp::SamplerAddressMode::MirrorClampToEdge:
            return SamplerAddressMode_MirrorClampToEdge;
        case CSharp::SamplerAddressMode::Repeat:
        default:
            return SamplerAddressMode_Repeat;
    }
}


R_INTERNAL Recluse::BorderColor getBorderColor(CSharp::BorderColor borderColor)
{
    switch (borderColor)
    {
        case CSharp::BorderColor::OpaqueWhite:
            return BorderColor_OpaqueWhite;
        case CSharp::BorderColor::TransparentBlack:
            return BorderColor_TransparentBlack;
        case CSharp::BorderColor::OpaqueBlack:
        default:
            return BorderColor_OpaqueBlack;
    }
}


R_INTERNAL Recluse::Filter getFilter(CSharp::Filter filter)
{
    switch (filter)
    {
        case CSharp::Filter::Cubic:
            return Filter_Cubic;
        case CSharp::Filter::Nearest:
            return Filter_Nearest;
        case CSharp::Filter::Linear:
        default:
            return Filter_Linear;
    }
}


R_INTERNAL Recluse::CompareOp CSharpToNativeCompareOp(CSharp::CompareOp op)
{
    switch (op)
    {
        case CSharp::CompareOp::Always:
            return CompareOp_Always;
        case CSharp::CompareOp::Equal:
            return CompareOp_Equal;
        case CSharp::CompareOp::Greater:
            return CompareOp_Greater;
        case CSharp::CompareOp::GreaterOrEqual:
            return CompareOp_GreaterOrEqual;
        case CSharp::CompareOp::Less:
            return CompareOp_Less;
        case CSharp::CompareOp::LessOrEqual:
            return CompareOp_LessOrEqual;
        case CSharp::CompareOp::NotEqual:
            return CompareOp_NotEqual;
        case CSharp::CompareOp::Never:
        default:
            return CompareOp_Never;
            
    }
}


R_INTERNAL Recluse::SamplerMipMapMode getSamplerMipMode(CSharp::SamplerMipMapMode mipMapMode)
{
    switch (mipMapMode)
    {
        case CSharp::SamplerMipMapMode::Linear:
            return SamplerMipMapMode_Linear;
        case CSharp::SamplerMipMapMode::Nearest:
        default:
            return SamplerMipMapMode_Nearest;
    }
}


R_INTERNAL Recluse::LogicOp CSharpToNativeLogicOp(CSharp::LogicOp Op)
{
    switch (Op)
    {
        case LogicOp::And:
            return LogicOp_And;
        case LogicOp::AndReverse:
            return LogicOp_AndReverse;
        case LogicOp::Copy:
            return LogicOp_Copy;
        case LogicOp::AndInverted:
            return LogicOp_AndInverted;
        case LogicOp::Xor:
            return LogicOp_Xor;
        case LogicOp::Clear:
            return LogicOp_Clear;
        case LogicOp::Or:
            return LogicOp_Or;
        case LogicOp::Nor:
            return LogicOp_Nor;
        case LogicOp::Equivalent:
            return LogicOp_Equivalent;
        case LogicOp::NoOp:
        default:
            return LogicOp_NoOp;
    }
}


R_INTERNAL Recluse::BlendOp CSharpToNativeBlendOp(CSharp::BlendOp Op)
{
    switch (Op)
    {
        case BlendOp::Subtract:
            return BlendOp_Subtract;
        case BlendOp::ReverseSubtract:
            return BlendOp_ReverseSubtract;
        case BlendOp::Min:
            return BlendOp_Min;
        case BlendOp::Max:
            return BlendOp_Max;
        case BlendOp::Add:
        default:
            return BlendOp_Add;
    }
}


R_INTERNAL Recluse::BlendFactor CSharpToNativeBlendFactor(CSharp::BlendFactor Factor)
{
    switch (Factor)
    {
        case BlendFactor::SourceAlpha:
            return BlendFactor_SourceAlpha;
        case BlendFactor::SourceOneAlpha:
            return BlendFactor_SourceOneAlpha;
        case BlendFactor::OneMinusSourceAlpha:
            return BlendFactor_OneMinusSourceAlpha;
        case BlendFactor::DestinationAlpha:
            return BlendFactor_DestinationAlpha;
        case BlendFactor::DestinationColor:
            return BlendFactor_DestinationColor;
        case BlendFactor::OneMinusDestinationColor:
            return BlendFactor_OneMinusDestinationColor;
        case BlendFactor::OneMinusDestinationAlpha:
            return BlendFactor_OneMinusDestinationAlpha;
        case BlendFactor::OneMinusConstantColor:
            return BlendFactor_OneMinusConstantColor;
        case BlendFactor::ConstantAlpha:
            return BlendFactor_ConstantAlpha;
        case BlendFactor::SourceColor:
            return BlendFactor_SourceColor;
        case BlendFactor::OneMinusConstantAlpha:
            return BlendFactor_OneMinusConstantAlpha;
        case BlendFactor::OneMinusSourceColor:
            return BlendFactor_OneMinusSourceColor;
        case BlendFactor::OneMinusSourceOneColor:
            return BlendFactor_OneMinusSourceOneColor;
        case BlendFactor::SourceAlphaSaturate:
            return BlendFactor_SourceAlphaSaturate;
        case BlendFactor::SourceOneColor:
            return BlendFactor_SourceOneColor;
        case BlendFactor::OneMinusSourceOneAlpha:
            return BlendFactor_OneMinusSourceOneAlpha;
        case BlendFactor::One:
            return BlendFactor_One;
        case BlendFactor::Zero:
            return BlendFactor_Zero;
        case BlendFactor::ConstantColor:
        default:
            return BlendFactor_ConstantColor;
    }
}


IResource::IResource(GraphicsResource* Resource)
    : Resource(Resource)
    , DeviceRef(nullptr)
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
    description.miscFlags = 0;
    description.name;

    GraphicsDevice* pDevice = Device->GetNative();
    GraphicsResource* res = nullptr;
    ResultCode result = pDevice->createResource(&res, description, CSharpToNativeState(InitialState));
    Resource = res;
    DeviceRef = pDevice;
}


IResource::~IResource()
{
    if (DeviceRef)
    {
        DeviceRef->destroyResource(Resource, m_releaseImmediately);
    }
}


IResource::!IResource()
{
    if (DeviceRef)
    {
        DeviceRef->destroyResource(Resource, m_releaseImmediately);
    }
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

    

IGraphicsDevice::IGraphicsDevice(GraphicsApi graphicsApi, System::String^ appName, System::String^ engineName, bool EnableDebugLayer)
    : m_instance(nullptr)
    , m_device(nullptr)
    , m_adapter(nullptr)
{
    if (EnableDebugLayer)
    {
        enableLogFile("recluse.log", true);
        Log::initializeLoggingSystem();
    }

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
    std::string EngineName = "";
    std::string AppName = "";
    if (appName->Length != 0)
    {
        AppName.resize(appName->Length);
        for (U32 i = 0; i < appName->Length; ++i)
            AppName[i] = appName[i];
    }

    if (engineName->Length != 0)
    {
        EngineName.resize(engineName->Length);
        for (U32 i = 0; i < engineName->Length; ++i)
            EngineName[i] = engineName[i];
    }

    appInfo.engineName = AppName.c_str();
    appInfo.appName = EngineName.c_str();
    LayerFeatureFlags layerFlags = 0;
    if (EnableDebugLayer)
    {
        layerFlags = LayerFeatureFlag_DebugValidation | LayerFeatureFlag_GpuDebugValidation;
    }
    ResultCode result = m_instance->initialize(appInfo, layerFlags);
    std::vector<GraphicsAdapter*>& adapters = m_instance->getGraphicsAdapters();
    R_ASSERT(!adapters.empty());
    m_adapter = adapters[0];
    R_ASSERT(m_adapter);
    DeviceCreateInfo deviceCreateInfo = { };
    GraphicsDevice* device = nullptr;
    result = m_adapter->createDevice(deviceCreateInfo, &device);
    m_device = device;
    m_api = graphicsApi;
}


IGraphicsDevice::~IGraphicsDevice()
{
    R_ASSERT(m_instance);
    R_ASSERT(m_adapter);
    R_ASSERT(m_device);
    m_adapter->destroyDevice(m_device);
    GraphicsInstance::destroyInstance(m_instance);
    Log::destroyLoggingSystem();
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


IGraphicsDevice::!IGraphicsDevice()
{
    if (m_device)
    {
        m_adapter->destroyDevice(m_device);
        GraphicsInstance::destroyInstance(m_instance);
    }
    Log::destroyLoggingSystem();
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


ISwapchain::!ISwapchain()
{
    if (Swapchain)
    {
        DeviceRef->destroySwapchain(Swapchain);
    }
    Swapchain = nullptr;
    DeviceRef = nullptr;
}


IGraphicsContext::~IGraphicsContext()
{
    if (Context)
    {
        DeviceRef->releaseContext(Context);
    }
}


IGraphicsContext::!IGraphicsContext()
{
    if (Context)
    {
        DeviceRef->releaseContext(Context);
    }

    Context = nullptr;
    DeviceRef = nullptr;
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


void IGraphicsContext::BindIndexBuffer(IResource^ IndexBuffer, System::UInt64 OffsetBytes, CSharp::IndexType Type)
{
    Context->bindIndexBuffer(IndexBuffer(), (U64)OffsetBytes, (Recluse::IndexType)Type);
}


void IGraphicsContext::BindVertexBuffers(array<IResource^>^ VertexBuffers, array<System::UInt64>^ Offsets)
{
    std::vector<GraphicsResource*> Verts(VertexBuffers->Length);
    std::vector<U64> OffsetBytes(Offsets->Length);
    for (U32 i = 0; i < Verts.size(); ++i)
    {
        IResource^ VB = VertexBuffers[0];
        Verts[i] = VB();
    }
    for (U32 i = 0; i < OffsetBytes.size(); ++i)
    {
        OffsetBytes[i] = (U64)Offsets[i];
    }
    Context->bindVertexBuffers(Verts.size(), Verts.data(), OffsetBytes.data());
}


void IGraphicsContext::SetScissors(array<CSharp::Rect^>^ Rects)
{
    std::vector<Recluse::Rect> rects(Rects->Length);

    for (U32 i = 0; i < rects.size(); ++i)
    {
        CSharp::Rect^ RectArea = Rects[i];
        rects[i].x      = static_cast<F32>(RectArea->X);
        rects[i].y      = static_cast<F32>(RectArea->Y);
        rects[i].width  = static_cast<F32>(RectArea->Width);
        rects[i].height = static_cast<F32>(RectArea->Height);
    }
    Context->setScissors(rects.size(), rects.data());
}


void IGraphicsContext::SetViewports(array<CSharp::Viewport^>^ Viewports)
{
    std::vector<Recluse::Viewport> viewports(Viewports->Length);
    for (U32 i = 0; i < viewports.size(); ++i)
    {
        CSharp::Viewport^ Viewport = Viewports[i];
        viewports[i].x          = static_cast<F32>(Viewport->X);
        viewports[i].y          = static_cast<F32>(Viewport->Y);
        viewports[i].width      = static_cast<F32>(Viewport->Width);
        viewports[i].height     = static_cast<F32>(Viewport->Height);
        viewports[i].minDepth   = static_cast<F32>(Viewport->MinDepth);
        viewports[i].maxDepth   = static_cast<F32>(Viewport->MaxDepth);
    }
    Context->setViewports(viewports.size(), viewports.data());
}


void IGraphicsContext::EnableDepth(System::Boolean Enable)
{
    Context->enableDepth((Bool)Enable);
}


void IGraphicsContext::EnableDepthWrite(System::Boolean Enable)
{
    Context->enableDepthWrite((Bool)Enable);
}


void IGraphicsContext::EnableStencil(System::Boolean Enable)
{
    Context->enableStencil((Bool)Enable);
}


void IGraphicsContext::SetDepthClampEnable(System::Boolean Enable)
{
    Context->setDepthClampEnable(Enable);
}


void IGraphicsContext::SetDepthBiasEnable(System::Boolean Enable)
{
    Context->setDepthBiasEnable(Enable);
}


void IGraphicsContext::SetDepthBiasClamp(System::Single Scale)
{
    Context->setDepthBiasClamp(Scale);
}


void IGraphicsContext::SetStencilReference(System::Byte StencilRef)
{
    Context->setStencilReference(StencilRef);
}


void IGraphicsContext::SetStencilReadMask(System::Byte Mask)
{
    Context->setStencilReadMask(Mask);
}


void IGraphicsContext::SetStencilWriteMask(System::Byte Mask)
{
    Context->setStencilWriteMask(Mask);
}


void IGraphicsContext::SetInputVertexLayout(System::UInt64 InputLayout)
{
    Context->setInputVertexLayout((VertexInputLayoutId)InputLayout);
}


void IGraphicsContext::DrawIndexedInstanced(System::Int32 IndexCount, System::Int32 InstanceCount, System::Int32 FirstIndex, System::Int32 VertexOffset, System::Int32 FirstInstance)
{
    Context->drawIndexedInstanced(IndexCount, InstanceCount, FirstIndex, VertexOffset, FirstInstance);
}


void IGraphicsContext::DrawInstanced(System::Int32 VertexCount, System::Int32 InstanceCount, System::Int32 FirstVertex, System::Int32 FirstInstance)
{
    Context->drawInstanced(VertexCount, InstanceCount, FirstVertex, FirstInstance);
}


void IGraphicsContext::Dispatch(System::Int32 X, System::Int32 Y, System::Int32 Z)
{
    Context->dispatch((U32)X, (U32)Y, (U32)Z);
}


void IGraphicsContext::SetCullMode(CullMode Mode)
{
    Context->setCullMode(CSharpToNativeCullMode(Mode));
}


void IGraphicsContext::SetFrontFace(FrontFace Face)
{
    Context->setFrontFace(CSharpToNativeFrontFace(Face));
}


void IGraphicsContext::SetDepthCompareOp(CompareOp CompareOperation)
{
    Context->setDepthCompareOp(CSharpToNativeCompareOp(CompareOperation));
}


void IGraphicsContext::SetPolygonMode(PolygonMode Mode)
{
    Context->setPolygonMode(CSharpToNativePolygonMode(Mode));
}


void IGraphicsContext::SetTopology(PrimitiveTopology Topology)
{
    Context->setTopology(CSharpToNativePrimitiveTopology(Topology));
}


void IGraphicsContext::SetLineWidth(System::Single Width)
{
    Context->setLineWidth((F32)Width);
}


void IGraphicsContext::SetColorWriteMask(System::UInt32 RtIndex, ColorComponent WriteMask)
{
    Context->setColorWriteMask(RtIndex, (ColorComponentMaskFlags)WriteMask);
}


void IGraphicsContext::SetBlendEnable(System::UInt32 RtIndex, System::Boolean Enable)
{
    Context->setBlendEnable(RtIndex, Enable);
}


void IGraphicsContext::SetBlendLogicOpEnable(System::Boolean Enable)
{
    Context->setBlendLogicOpEnable(Enable);
}


void IGraphicsContext::SetBlendLogicOp(LogicOp LogicOperation)
{
    Context->setBlendLogicOp(CSharpToNativeLogicOp(LogicOperation));
}


void IGraphicsContext::SetBlendConstants(array<System::Single^>^ BlendConstants)
{
    F32 blendConstants[4];
    blendConstants[0] = static_cast<F32>(BlendConstants[0]);
    blendConstants[1] = static_cast<F32>(BlendConstants[1]);
    blendConstants[2] = static_cast<F32>(BlendConstants[2]);
    blendConstants[3] = static_cast<F32>(BlendConstants[3]);
    Context->setBlendConstants(blendConstants);
}


void IGraphicsContext::SetBlend(System::UInt32 RtIndex, BlendFactor SrcColorFactor, BlendFactor DstColorFactor, BlendOp ColorBlendOp,
                                          BlendFactor SrcAlphaFactor, BlendFactor DstAlphaFactor, BlendOp AlphaOp)
{
    Context->setBlend(RtIndex, CSharpToNativeBlendFactor(SrcColorFactor), CSharpToNativeBlendFactor(DstColorFactor), CSharpToNativeBlendOp(ColorBlendOp),
                               CSharpToNativeBlendFactor(SrcAlphaFactor), CSharpToNativeBlendFactor(DstAlphaFactor), CSharpToNativeBlendOp(AlphaOp));
}


ShaderProgramBinder^ IGraphicsContext::BindShaderProgram(System::UInt64 ProgramId, System::UInt32 Permutation)
{
    IShaderProgramBinder& binder = Context->bindShaderProgram((ShaderProgramId)ProgramId, (U32)Permutation);
    ShaderProgramBinder^ ProgramBinder = gcnew ShaderProgramBinder(binder);
    return ProgramBinder;
}


ShaderProgramBinder::ShaderProgramBinder(IShaderProgramBinder& binder)
    : ShaderProgram(binder)
{
}


ShaderProgramBinder^ ShaderProgramBinder::BindShaderResource(CSharp::ShaderStage Stage, System::UInt32 Slot, System::UIntPtr View)
{
    ShaderProgram.bindShaderResource((Recluse::ShaderStageFlags)Stage, (U32)Slot, (ResourceViewId)View);
    return this;
}


ShaderProgramBinder^ ShaderProgramBinder::BindUnorderedAccessView(CSharp::ShaderStage Stage, System::UInt32 Slot, System::UIntPtr View)
{
    ShaderProgram.bindUnorderedAccessView((Recluse::ShaderStageFlags)Stage, (U32)Slot, (ResourceViewId)View);
    return this;
}

ShaderProgramBinder^ ShaderProgramBinder::BindConstantBuffer(CSharp::ShaderStage Stage, System::UInt32 Slot, IResource^ Resource, System::UInt32 OffsetBytes, System::UInt32 SizeBytes, array<System::Byte>^ Data)
{
    std::vector<U8> nativeData;
    void* ptr = nullptr;
    if (Data != nullptr)
    {
        nativeData.resize(Data->Length);
        Marshal::Copy(Data, 0, (IntPtr)nativeData.data(), Data->Length);
        ptr = nativeData.data();
    }
    ShaderProgram.bindConstantBuffer((Recluse::ShaderStageFlags)Stage, (U32)Slot, Resource(), (U32)OffsetBytes, (U32)SizeBytes, ptr);
    return this;
}


ShaderProgramBinder^ ShaderProgramBinder::BindSampler(CSharp::ShaderStage Stage, System::UInt32 Slot, ISampler^ Sampler)
{
    ShaderProgram.bindSampler((ShaderStageFlags)Stage, (U32)Slot, Sampler());
    return this;
}


ISampler::ISampler(IGraphicsDevice^ Device, 
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
        CSharp::BorderColor BorderColour)
    : DeviceRef(Device())
    , Sampler(nullptr)
{
    GraphicsSampler* sampler        = nullptr;
    SamplerDescription description  = { };
    description.maxAnisotropy       = MaxAnisotropy;
    description.mipLodBias          = MipLodBias;
    description.minLod              = MinLod;
    description.maxLod              = MaxLod;
    description.addressModeU        = getSamplerAddressMode(AddressModeU);
    description.addressModeV        = getSamplerAddressMode(AddressModeV);
    description.addressModeW        = getSamplerAddressMode(AddressModeW);
    description.borderColor         = getBorderColor(BorderColour);
    description.mipMapMode          = getSamplerMipMode(MipMapMode);
    description.minFilter           = getFilter(MinFilter);
    description.magFilter           = getFilter(MagFilter);
    description.compareOp           = CSharpToNativeCompareOp(CompareOperation);
    ResultCode code = Device()->createSampler(&sampler, description);
    
    Sampler = sampler;
}


ISampler::~ISampler()
{
    if (Sampler)
    {
        DeviceRef->destroySampler(Sampler);
    }
}


ISampler::!ISampler()
{
    if (Sampler)
    {
        DeviceRef->destroySampler(Sampler);
        Sampler = nullptr;
    }
}


R_INTERNAL Recluse::InputRate CSharpToNativeInputRate(CSharp::InputRate input)
{
    switch (input)
    {
        case InputRate::PerInstance:
            return InputRate_PerInstance;
        case InputRate::PerVertex:
        default:
            return InputRate_PerVertex;
    }
}


System::Boolean IGraphicsDevice::MakeVertexLayout(System::UInt64 VertexLayoutId, IVertexInputLayout^ VertexLayout)
{
    R_ASSERT(VertexLayout != nullptr);
    Recluse::VertexInputLayout Layout = { };
    std::list<std::vector<Recluse::VertexAttribute>> attributeList = { };

    Layout.numVertexBindings = VertexLayout->VertexBindings->Count;
    for (U32 i = 0; i < Layout.numVertexBindings; ++i)
    {
        Recluse::VertexBinding& binding = Layout.vertexBindings[i];
        CSharp::IVertexBinding^ Binding = static_cast<CSharp::IVertexBinding^>(VertexLayout->VertexBindings[i]);
        
        binding.binding = Binding->Binding;
        binding.inputRate = CSharpToNativeInputRate(Binding->Rate);
        binding.stride = Binding->Stride;
        binding.numVertexAttributes = Binding->VertexAttributes->Count;
        if (binding.numVertexAttributes != 0)
        {
            attributeList.push_back(std::vector<Recluse::VertexAttribute>());
            std::vector<Recluse::VertexAttribute>& attributes = attributeList.back();
            attributes.resize(binding.numVertexAttributes);
            for (U32 i = 0; i < binding.numVertexAttributes; ++i)
            {
                CSharp::IVertexAttribute^ Attrib = static_cast<CSharp::IVertexAttribute^>(Binding->VertexAttributes[i]);
                attributes[i].format = CSharpToNativeFormat(Attrib->Format);
                attributes[i].location = Attrib->Location;
                attributes[i].offsetBytes = Attrib->OffsetBytes;
                attributes[i].semantic = (Recluse::Semantic)Attrib->SemanticTag;
                attributes[i].semanticIndex = Attrib->SemanticIndex;
            }
            binding.pVertexAttributes = attributes.data();
        }
    }
    return (System::Boolean)m_device->makeVertexLayout((VertexInputLayoutId)VertexLayoutId, Layout);
}


void IGraphicsDevice::UnloadShaderProgram(System::UInt64 ProgramId)
{
    this->m_device->unloadShaderProgram((ShaderProgramId)ProgramId);
}


void IGraphicsDevice::DestroyVertexLayout(System::UInt64 VertexLayoutId)
{
    m_device->destroyVertexLayout((VertexInputLayoutId)VertexLayoutId);
}


CSharp::ResourceFormat ISwapchain::GetFormat()
{
    const SwapchainCreateDescription& desc = Swapchain->getDesc();
    return NativeFormatToCSharpFormat(desc.format);
}


System::Int32 ISwapchain::GetWidth()
{
    const SwapchainCreateDescription& desc = Swapchain->getDesc();
    return desc.renderWidth;
}


System::Int32 ISwapchain::GetHeight()
{
    const SwapchainCreateDescription& desc = Swapchain->getDesc();
    return desc.renderHeight;
}
} // CSharp
} // Recluse