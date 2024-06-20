//
#include "IGraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include <vector>

namespace Recluse {
namespace CSharp {
    

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
}



System::String^ IGraphicsDevice::GetString()
{
    return "I am a graphics device. Don't mind me. I'm managed!";
}


IGraphicsContext::IGraphicsContext(IGraphicsDevice^ device, System::IntPtr windowHandle, System::Int32 width, System::Int32 height, FrameBuffering framebuffering)
    : context(nullptr)
    , swapchain(nullptr)
    , deviceRef(nullptr)
{
    if (device != nullptr)
    {
        deviceRef = device->GetNative();
    }

    // Create the context.
    context = deviceRef->createContext();

    SwapchainCreateDescription swapchainDescription = { };
    swapchainDescription.desiredFrames = 2;
    swapchainDescription.format = ResourceFormat_R8G8B8A8_Unorm;
    swapchainDescription.renderWidth = width;
    swapchainDescription.renderHeight = height;
    Recluse::FrameBuffering nativeFrameBuffering = Recluse::FrameBuffering_Single;

    switch (framebuffering)
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

    void* windowPtr = (void*)windowHandle;
    swapchain = deviceRef->createSwapchain(swapchainDescription, windowPtr);
    R_ASSERT(swapchain);
}


IGraphicsContext::~IGraphicsContext()
{
    if (context)
    {
        deviceRef->releaseContext(context);
    }

    if (swapchain)
    {
        deviceRef->destroySwapchain(swapchain);
    }
}


void IGraphicsContext::Begin()
{
    R_ASSERT(context != nullptr);
    context->begin();
}


void IGraphicsContext::End()
{
    R_ASSERT(context != nullptr);
    context->end();
}


void IGraphicsContext::Present()
{
    R_ASSERT(swapchain != nullptr);
    swapchain->present(context);
}


void IGraphicsContext::SetContextFrame(System::Int32 frames)
{
    R_ASSERT(context != nullptr);
    context->setFrames(frames);
}


IResource^ IGraphicsContext::GetCurrentFrame()
{
    return nullptr;
}


void IGraphicsContext::Transition(CSharp::ResourceState toState)
{
}
} // CSharp
} // Recluse