//
#include "Recluse/Types.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Math/Vector3.hpp"
#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Time.hpp"

#include <array>

using namespace Recluse;

GraphicsDevice* device = nullptr;
GraphicsContext* context = nullptr;
GraphicsSwapchain* swapchain = nullptr;
static const U32 g_textureWidth = 64;
static const U32 g_textureHeight = 64;


struct Vertex
{
    Math::Float3 position;
    Math::Float3 normal;    
};


std::vector<Vertex> boxCoordinates = 
{
     { { 0, 0, 0 }, {  0, 0, 0 } },
     { { }, { } }
};


std::vector<I16> boxIndicies =
{

};


enum VertexLayout
{
    VertexLayout_PositionNormalTexCoordColor
};

enum ShaderProgram
{
    ShaderProgram_Box = 12354343
};


void ResizeFunction(U32 x, U32 y, U32 width, U32 height)
{
    SwapchainCreateDescription desc = swapchain->getDesc();
    if (desc.renderHeight != height || desc.renderWidth != width)
    {
        if (width > 0 && height > 0)
        {
            context->wait();
            desc.renderWidth = width;
            desc.renderHeight = height;
            swapchain->rebuild(desc);
        }
    }
}


void createTextureResource(GraphicsResource** textureResource)
{
    ResultCode result = RecluseResult_Ok;
    GraphicsResourceDescription textureDesc = { };
    textureDesc.dimension = ResourceDimension_2d;
    textureDesc.format = ResourceFormat_R8G8B8A8_Unorm;
    textureDesc.width = g_textureWidth;
    textureDesc.height = g_textureHeight;
    textureDesc.depthOrArraySize = 1;
    textureDesc.memoryUsage = ResourceMemoryUsage_GpuOnly;
    textureDesc.mipLevels = 1;
    textureDesc.miscFlags = 0;
    textureDesc.samples = 1;
    textureDesc.usage = ResourceUsage_ShaderResource | ResourceUsage_CopyDestination | ResourceUsage_CopySource;
    result = device->createResource(textureResource, textureDesc, ResourceState_CopyDestination);

    R_ASSERT(result == RecluseResult_Ok);

    Math::UByte4 texture[g_textureWidth][g_textureHeight];
    U64 textureTotalSizeBytes = sizeof(Math::UByte4) * g_textureWidth * g_textureHeight;

    for ( int i = 0; i < g_textureHeight; i++ ) 
    {
        for ( int j = 0; j < g_textureWidth; j++ ) 
        {
            U8 c = (((i & 0x8) == 0) ^ ((j & 0x8)  == 0)) * 255;
            texture[i][j][0]  = c;
            texture[i][j][1]  = c;
            texture[i][j][2]  = c;
            texture[i][j][3]  = 255;
        }
    }
    
    GraphicsResourceDescription staging = { };
    staging.dimension = ResourceDimension_Buffer;
    staging.format = ResourceFormat_Unknown;
    staging.width = g_textureWidth * g_textureHeight * sizeof(Math::UByte4);
    staging.height = 1;
    staging.depthOrArraySize = 1;
    staging.memoryUsage = ResourceMemoryUsage_CpuOnly;
    staging.mipLevels = 1;
    staging.miscFlags = 0;
    staging.samples = 1;
    staging.usage = ResourceUsage_CopySource;

    GraphicsResource* stagingBuffer = nullptr;
    device->createResource(&stagingBuffer, staging, ResourceState_CopySource);
    R_ASSERT(result == RecluseResult_Ok);

    void* stagingMem = nullptr;
    stagingBuffer->map(&stagingMem, nullptr);
    memcpy(stagingMem, texture, textureTotalSizeBytes);
    stagingBuffer->unmap(nullptr);
    
    device->copyResource(*textureResource, stagingBuffer);
    device->destroyResource(stagingBuffer);
}


void buildVertexLayouts(GraphicsDevice* pDevice)
{
    VertexAttribute attribs[4];
    attribs[0].format = ResourceFormat_R32G32B32_Float;
    attribs[0].location = 0;
    attribs[0].offsetBytes = 0;
    attribs[0].semantic = Semantic_Position;
    attribs[0].semanticIndex = 0;

    attribs[1].format = ResourceFormat_R32G32B32_Float;
    attribs[1].location = 1;
    attribs[1].offsetBytes = VertexAttribute::OffsetAppend;
    attribs[1].semantic = Semantic_Texcoord;
    attribs[1].semanticIndex = 0;

    attribs[2].format = ResourceFormat_R32G32_Float;
    attribs[2].location = 2;
    attribs[2].offsetBytes = VertexAttribute::OffsetAppend;
    attribs[2].semantic = Semantic_Texcoord;
    attribs[2].semanticIndex = 1;

    attribs[3].format = ResourceFormat_R32G32B32A32_Float;
    attribs[3].location = 3;
    attribs[3].offsetBytes = VertexAttribute::OffsetAppend;
    attribs[3].semantic = Semantic_Texcoord;
    attribs[3].semanticIndex = 2;
    

    VertexInputLayout layout = { };
    layout.numVertexBindings = 1;
    layout.vertexBindings[0].binding = 0;
    layout.vertexBindings[0].inputRate = InputRate_PerVertex;
    layout.vertexBindings[0].numVertexAttributes = 4;
    layout.vertexBindings[0].pVertexAttributes = attribs;
    Bool result = pDevice->makeVertexLayout(VertexLayout_PositionNormalTexCoordColor, layout);
    R_ASSERT(result);
}

int main(char* argv[], int c)
{
    Log::initializeLoggingSystem();
    RealtimeTick::initializeWatch(1ull, 0);
    GraphicsInstance* instance  = GraphicsInstance::createInstance(GraphicsApi_Direct3D12);
    GraphicsAdapter* adapter    = nullptr;

    Window* window = Window::create("Box", 0, 0, 1024, 1024, ScreenMode_Windowed);
    window->open();
    window->setToCenter();
    window->setOnWindowResize(ResizeFunction);

    {
        ApplicationInfo appInfo = { };
        appInfo.engineName = "";
        appInfo.appName = "Box";
        appInfo.appMinor = 0;
        appInfo.appMajor = 0;
        appInfo.appPatch = 0;
        LayerFeatureFlags flags = LayerFeatureFlag_DebugValidation;
        instance->initialize(appInfo, flags);
    }
    
    adapter = instance->getGraphicsAdapters()[0];
    R_ASSERT(adapter);
    
    {
        DeviceCreateInfo devInfo = { };
        adapter->createDevice(devInfo, &device);
    }

    context = device->createContext();
    R_ASSERT(context);
    context->setFrames(2);

    GraphicsResource* textureResource = nullptr;
    createTextureResource(&textureResource);

    GraphicsResource* constBuff = nullptr;
    {
        GraphicsResourceDescription description = { };
        description.depthOrArraySize = 1;
        description.height = 1;
        description.mipLevels = 1;
        description.samples = 1;
        description.dimension = ResourceDimension_Buffer;
        description.usage = ResourceUsage_ConstantBuffer;
        description.width = 32;
        description.memoryUsage = ResourceMemoryUsage_CpuToGpu;
        device->createResource(&constBuff, description, ResourceState_ConstantBuffer);
    }
    //buildVertexLayouts(device);

    SwapchainCreateDescription swapchainDescription = { };
    swapchainDescription.buffering      = FrameBuffering_Triple;
    swapchainDescription.desiredFrames  = 3;
    swapchainDescription.format         = ResourceFormat_R8G8B8A8_Unorm;
    swapchainDescription.renderWidth    = window->getWidth();
    swapchainDescription.renderHeight   = window->getHeight();
    swapchain = device->createSwapchain(swapchainDescription, window->getNativeHandle()); 
    
    std::array<F32, 10> lastMs;
    U32 frameCount = 0;
    while (!window->shouldClose())
    {   
        frameCount += 1;
        RealtimeTick::updateWatch(1ull, 0);
        RealtimeTick tick   = RealtimeTick::getTick(0);
        lastMs[frameCount % lastMs.size()] = tick.delta();
        
        if ((frameCount % lastMs.size()) == 0)
        {
            F32 total = 0.f;
            for (U32 i = 0; i < lastMs.size(); ++i)
                total += lastMs[i];
            total /= static_cast<F32>(lastMs.size());
            R_WARN("Box", "Fps: %f", 1.0f / total);
        }

        if (!window->isMinimized())
        {
            swapchain->prepare(context);
                //context->transition(device->getSwapchain()->getFrame(device->getSwapchain()->getCurrentFrameIndex()), ResourceState_RenderTarget);
                //context->setShaderProgram(ShaderProgram_Box);
                //context->setInputVertexLayout(VertexLayout_PositionNormalTexCoordColor);
                //context->enableDepth(true);
                //context->setDepthCompareOp(CompareOp_GreaterOrEqual);
                //context->bindIndexBuffer(nullptr, 0, IndexType_Unsigned16);
                //context->drawIndexedInstanced(0, 0, 0, 0, 0);
                GraphicsResource* swapchainImage = swapchain->getFrame(swapchain->getCurrentFrameIndex());
                context->transition(textureResource, ResourceState_CopySource);
                context->transition(swapchainImage, ResourceState_CopyDestination);
                context->copyResource(swapchainImage, textureResource);
                context->transition(swapchain->getFrame(swapchain->getCurrentFrameIndex()), ResourceState_Present);
            context->end();
            swapchain->present(context);
        }
        pollEvents();
        
    }
        
    context->wait();

    device->destroySwapchain(swapchain);
    device->destroyResource(constBuff);
    device->destroyResource(textureResource);
    device->releaseContext(context);
    adapter->destroyDevice(device);
    GraphicsInstance::destroyInstance(instance);
    Log::destroyLoggingSystem();
    return 0;
}