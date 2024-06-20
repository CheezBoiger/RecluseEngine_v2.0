//
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include <memory>
#include <vcclr.h>
#using <mscorlib.dll>


namespace Recluse {
namespace CSharp {
#pragma managed


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


public ref class IResource
{
public:
private:
    GraphicsResource* resource;
};

// Graphics device.
public ref class IGraphicsDevice
{
public:
    IGraphicsDevice(CSharp::GraphicsApi graphicsApi, System::String^ appName, System::String^ engineName);
    ~IGraphicsDevice();
      
    System::String^ GetString();


    GraphicsDevice* GetNative() { return m_device; }

private:
    GraphicsInstance* m_instance;
    GraphicsAdapter* m_adapter;
    GraphicsDevice* m_device;
};


/// <summary> 
/// Graphics Context.
/// </summary>
public ref class IGraphicsContext 
{
public:
    IGraphicsContext(IGraphicsDevice^ device, System::IntPtr windowHandle, System::Int32 width, System::Int32 height, FrameBuffering frameBuffering);
    ~IGraphicsContext();

    void SetContextFrame(System::Int32 frames);
    void Begin();
    void Transition(CSharp::ResourceState toState);
    void End();
    void Present();
    
    IResource^ GetCurrentFrame();

private:
    GraphicsContext* context;
    GraphicsSwapchain* swapchain;
    GraphicsDevice* deviceRef;
};
} // CSharp
} // Recluse