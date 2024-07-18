//
#include "Recluse/Types.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Renderer/Texture.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/System/KeyboardInput.hpp"
#include "Recluse/Filesystem/Archive.hpp"

#include "Recluse/Math/Quaternion.hpp"
#include "Recluse/Math/Matrix33.hpp"
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Vector3.hpp"
#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Time.hpp"

#include "../Shared/Geometry.hpp"

#include <array>

#define READ_DATABASE 0
#define COMPILE_SHADER_PROGRAM 1
#define WRITE_DATABASE 1
#define MANUAL_PROGRAMING_UPDATE 0

#if WRITE_DATABASE
#include "Recluse/Pipeline/ShaderProgramBuilder.hpp"
#endif

using namespace Recluse;

GraphicsDevice* device = nullptr;
GraphicsContext* context = nullptr;
GraphicsSwapchain* swapchain = nullptr;
GraphicsInstance* instance = nullptr;
static const U32 g_textureWidth = 64;
static const U32 g_textureHeight = 64;

// TODO: Need to figure out how to better handle resolution resizing.
GraphicsResource* depthBuffer = nullptr;

struct ConstBuffer
{
    Math::Matrix44 modelViewProjection;
    Math::Matrix44 normal;
    U32             useTexturing;
    U32             pad0[3];
     
};


struct Vertex
{
    Math::Float3 position;
    Math::Float3 normal;
    Math::Float2 texCoord0;
    Math::Float4 color;
};


std::vector<Vertex> createCubeInstance(F32 scale)
{
    std::vector<Vertex> cube(36);
    std::array<Math::Float3, 36> positions = Shared::Box::GetPositions();
    std::array<Math::Float3, 36> normals = Shared::Box::GetNormals();
    std::array<Math::Float4, 36> texcoords = Shared::Box::GetTexCoords();
    std::array<Math::Float4, 36> colors = Shared::Box::GetColors(); 
    for (size_t i = 0; i < cube.size(); ++i) 
    {
        Math::Float4 scaledPos = positions[i] * scale;
        cube[i].position = Math::Float3(scaledPos.x, scaledPos.y, scaledPos.z);
        cube[i].normal = Math::Float3(normals[i].x, normals[i].y, normals[i].z);
        cube[i].texCoord0 = Math::Float2(texcoords[i].x, texcoords[i].y);
        cube[i].color = colors[i];
    //null_bones(cube[i]);

    //cube[i].position.y *= -1.0f;
    //cube[i].normal.y *= -1.0f;
    }
  return cube;
}


std::vector<U32> createCubeIndicesInstance()
{
    std::vector<U32> cubeIs(36);
    std::array<U32, 36> indices = Shared::Box::GetIndices();
    for (size_t i = 0; i < indices.size(); ++i) 
    {
        cubeIs[i] = indices[i];
    }
    return cubeIs;
}


enum VertexLayout
{
    VertexLayout_PositionNormalTexCoordColor = 0
};

enum ShaderProgram
{
    ShaderProgram_Box = 12354343
};

GraphicsResource* buildDepthBuffer(U32, U32);

void ResizeFunction(U32 x, U32 y, U32 width, U32 height)
{
    SwapchainCreateDescription desc = swapchain->getDesc();
    //if (desc.renderHeight != height || desc.renderWidth != width)
    {
        if (width > 0 && height > 0)
        {
            context->wait();
            desc.renderWidth = width;
            desc.renderHeight = height;
            swapchain->rebuild(desc);
            device->destroyResource(depthBuffer, true);
            depthBuffer = buildDepthBuffer(width, height);
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
    textureDesc.depthOrArraySize = 16;
    textureDesc.memoryUsage = ResourceMemoryUsage_GpuOnly;
    textureDesc.mipLevels = 4;
    textureDesc.miscFlags = 0;
    textureDesc.samples = 1;
    textureDesc.usage = ResourceUsage_ShaderResource | ResourceUsage_CopyDestination | ResourceUsage_CopySource;
    result = device->createResource(textureResource, textureDesc, ResourceState_CopyDestination);

    R_ASSERT(result == RecluseResult_Ok);
    Engine::Texture texture = Engine::Texture("Name", g_textureWidth, g_textureHeight, 16u, 4u, ResourceFormat_R8G8B8A8_Unorm);
    UPtr baseAddress = texture.getBaseAddress();
    U32 bytesPerPixel = Engine::obtainFormatBytes(texture.getPixelFormat());
    for (U32 layer = 0; layer < texture.getArrayLayers(); ++layer)
    {
        for (U32 mipmap = 0; mipmap < texture.getMipCount(); ++mipmap)
        {
            const Engine::Subresource& subresource = texture.getSubresource(layer, mipmap);
            UPtr subresourceAddress = baseAddress + subresource.getOffsteBytes();
            U8* data = (U8*)subresourceAddress;
            for (int y = 0; y < subresource.getHeight(); y++) 
            {
                for (int x = 0; x < subresource.getWidth(); x++) 
                {
                    U8 c = (((y & 0x8) == 0) ^ ((x & 0x8)  == 0)) * 255;
                    Math::UByte4& output = (Math::UByte4&)data[(x * bytesPerPixel) + texture.getRowPitch() * y];
                    output[0] = c;
                    output[1] = c;
                    output[2] = c;
                    output[3] = 255;
                }
            }
        }
    }
    
    GraphicsResourceDescription staging = { };
    staging.dimension = ResourceDimension_Buffer;
    staging.format = ResourceFormat_Unknown;
    staging.width = texture.getTotalSizeBytes();
    staging.height = 1;
    staging.depthOrArraySize = 1;
    staging.memoryUsage = ResourceMemoryUsage_CpuVisible;
    staging.mipLevels = 1;
    staging.miscFlags = 0;
    staging.samples = 1;
    staging.usage = ResourceUsage_CopySource;

    GraphicsResource* stagingBuffer = nullptr;
    device->createResource(&stagingBuffer, staging, ResourceState_CopySource);
    R_ASSERT(result == RecluseResult_Ok);

    void* stagingMem = nullptr;
    stagingBuffer->map(&stagingMem, nullptr);
    memcpy(stagingMem, (void*)texture.getBaseAddress(), texture.getTotalSizeBytes());
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


GraphicsResource* buildVertexBuffer()
{
    std::vector<Vertex> vertices = createCubeInstance(1.0f);
    GraphicsResource* vertexBuffer = nullptr;
    GraphicsResource* stagingBuffer = nullptr;
    GraphicsResourceDescription desc = { };
    desc.depthOrArraySize = 1;
    desc.memoryUsage = ResourceMemoryUsage_GpuOnly;
    desc.mipLevels = 1;
    desc.height = 1;
    desc.width = sizeof(Vertex) * vertices.size();
    desc.samples = 1;
    desc.usage = ResourceUsage_VertexBuffer | ResourceUsage_CopyDestination;
    desc.dimension = ResourceDimension_Buffer;
    desc.format = ResourceFormat_Unknown;
    device->createResource(&vertexBuffer, desc, ResourceState_CopyDestination);

    desc.memoryUsage = ResourceMemoryUsage_CpuToGpu;
    desc.usage = ResourceUsage_CopySource;
    device->createResource(&stagingBuffer, desc, ResourceState_CopySource);

    void* dat = nullptr;
    stagingBuffer->map(&dat, nullptr);
    memcpy(dat, vertices.data(), sizeof(Vertex) * vertices.size());
    stagingBuffer->unmap(nullptr);

    CopyBufferRegion region = { };
    region.dstOffsetBytes = 0;
    region.srcOffsetBytes = 0;
    region.szBytes = sizeof(Vertex) * vertices.size();
    device->copyBufferRegions(vertexBuffer, stagingBuffer, &region, 1);
    device->destroyResource(stagingBuffer);
    return vertexBuffer;
}


GraphicsResource* buildIndexBuffer()
{
    std::vector<U32> indices = createCubeIndicesInstance();
    GraphicsResource* vertexBuffer = nullptr;
    GraphicsResource* stagingBuffer = nullptr;
    GraphicsResourceDescription desc = { };
    desc.depthOrArraySize = 1;
    desc.memoryUsage = ResourceMemoryUsage_GpuOnly;
    desc.mipLevels = 1;
    desc.height = 1;
    desc.width = sizeof(U32) * indices.size();
    desc.samples = 1;
    desc.usage = ResourceState_IndexBuffer | ResourceUsage_CopyDestination;
    desc.dimension = ResourceDimension_Buffer;
    desc.format = ResourceFormat_Unknown;
    device->createResource(&vertexBuffer, desc, ResourceState_CopyDestination);

    desc.memoryUsage = ResourceMemoryUsage_CpuToGpu;
    desc.usage = ResourceUsage_CopySource;
    desc.format = ResourceFormat_Unknown;
    device->createResource(&stagingBuffer, desc, ResourceState_CopySource);

    void* dat = nullptr;
    stagingBuffer->map(&dat, nullptr);
    memcpy(dat, indices.data(), sizeof(U32) * indices.size());
    stagingBuffer->unmap(nullptr);
    CopyBufferRegion region = { };
    region.dstOffsetBytes = 0;
    region.srcOffsetBytes = 0;
    region.szBytes = sizeof(U32) * indices.size();
    device->copyBufferRegions(vertexBuffer, stagingBuffer, &region, 1);
    device->destroyResource(stagingBuffer);
    return vertexBuffer;
}


GraphicsResource* buildDepthBuffer(U32 width, U32 height)
{
    GraphicsResource* depthBuffer = nullptr;
    GraphicsResourceDescription desc = { };
    desc.depthOrArraySize = 1;
    desc.memoryUsage = ResourceMemoryUsage_GpuOnly;
    desc.mipLevels = 1;
    desc.height = height;
    desc.width = width;
    desc.samples = 1;
    desc.usage = ResourceUsage_DepthStencil;
    desc.dimension = ResourceDimension_2d;
    desc.format = ResourceFormat_D32_Float;
    device->createResource(&depthBuffer, desc, ResourceState_DepthStencilWrite);
    return depthBuffer;
}


GraphicsResource* buildConstantBuffer(GraphicsDevice* device)
{
    GraphicsResource* constBuffer = nullptr;
    GraphicsResourceDescription description = { };
    description.depthOrArraySize = 1;
    description.height = 1;
    description.mipLevels = 1;
    description.samples = 1;
    description.dimension = ResourceDimension_Buffer;
    description.usage = ResourceUsage_ConstantBuffer;
    description.width = sizeof(ConstBuffer);
    description.memoryUsage = ResourceMemoryUsage_CpuToGpu;
    device->createResource(&constBuffer, description, ResourceState_ConstantBuffer);
    return constBuffer;
}


void updateConstBuffer(IShaderProgramBinder& binder, GraphicsResource* resource, U32 width, U32 height, F32 delta)
{
    static F32 t = 0;
    static Bool isTexturing = false;
    t += 20.0f * delta;
    t = fmod(t, 360.0f);
    Math::Quaternion q = Math::angleAxis(Math::normalize(Math::Float3(1.0f, 0.0f, 1.0f)), Math::deg2Rad(t));
    Math::Matrix44 T = Math::translate(Math::Matrix44::identity(), Math::Float3(0, 0, 6));
    Math::Matrix44 R = Math::rotate(Math::Matrix44::identity(), Math::Float3(0.0f, 1.0f, 0.0f), Math::deg2Rad(45.0f));
    //Math::Matrix44 R2 = Math::rotate(Math::Matrix44::identity(), Math::Float3(1.0f, 0.0f, 1.0f), Math::deg2Rad(t));
    Math::Matrix44 R2 = Math::quatToMat44(q);
    Math::Matrix44 model = R2 * R * T;
    Math::Matrix44 view = Math::translate(Math::Matrix44::identity(), Math::Float3(0, 0, 0));
    Math::Matrix44 proj = Math::perspectiveLH_Aspect(Math::deg2Rad(45.0f), (F32)width / (F32)height, 0.001f, 1000.0f);

    if (instance->getApi() == GraphicsApi_Vulkan)
        proj[5] *= -1;
    Math::Matrix44 mvp = model * view * proj;
    ConstBuffer buf = { };
    buf.modelViewProjection = mvp;
    buf.normal = Math::Matrix44::identity();
    KeyboardListener listener;
    if (listener.isKeyDownOnce(KeyCode_A))
    {
        isTexturing = isTexturing ? false : true;
    }
    buf.useTexturing = isTexturing;


#if MANUAL_PROGRAMING_UPDATE 
    void* dat = nullptr;
    resource->map(&dat, nullptr);
    memcpy(dat, &buf, sizeof(ConstBuffer));
    resource->unmap(nullptr);
    binder.bindConstantBuffer(ShaderStage_Vertex | ShaderStage_Pixel, 0, resource, 0, sizeof(ConstBuffer));
#else
    binder.bindConstantBuffer(ShaderStage_Vertex | ShaderStage_Pixel, 0, resource, 0, sizeof(ConstBuffer), &buf);
#endif
}

void createShaderProgram(GraphicsDevice* device)
{
    ShaderProgramDatabase database          = ShaderProgramDatabase("PipelineInitialization.D3D12.Database");
#if COMPILE_SHADER_PROGRAM
    std::string currDir = Filesystem::getDirectoryFromPath(__FILE__);
    std::string vsSource = currDir + "/" + "vertex.hlsl";
    std::string fsSource = currDir + "/" + "pixel.hlsl";

    Pipeline::Builder::ShaderProgramDescription description;
    description.pipelineType = BindType_Graphics;
    description.language = ShaderLanguage_Hlsl;
    description.graphics.vs = vsSource.c_str();
    description.graphics.vsName = "Main";
    description.graphics.ps = fsSource.c_str();
    description.graphics.psName = "psMain";
    
    if (instance->getApi() == GraphicsApi_Direct3D12)
        GlobalCommands::setValue("ShaderBuilder.NameId", "dxc");

    Pipeline::Builder::buildShaderProgram(database, description, ShaderProgram_Box, instance->getApi() == GraphicsApi_Direct3D12 ? ShaderIntermediateCode_Dxil : ShaderIntermediateCode_Spirv);
#if WRITE_DATABASE
    {
        R_VERBOSE("Database", "Writing database.");
        ArchiveWriter writer("database.dxil.recluse");
        database.serialize(&writer);
    }
#endif

#endif
#if READ_DATABASE
    {
        R_VERBOSE("Database", "Reading database.");
        ArchiveReader reader("database.dxil.recluse");
        database.deserialize(&reader);
    }
#endif
    Runtime::buildShaderProgram(device, database, ShaderProgram_Box);
    database.clearShaderProgramDefinitions();
}


GraphicsSampler* createSampler(GraphicsDevice* device)
{
    SamplerDescription samplerDescription = { };
    samplerDescription.addressModeU = SamplerAddressMode_ClampToBorder;
    samplerDescription.addressModeV = SamplerAddressMode_ClampToBorder;
    samplerDescription.addressModeW = SamplerAddressMode_ClampToBorder;
    samplerDescription.borderColor = BorderColor_OpaqueBlack;
    samplerDescription.compareOp = CompareOp_Never;
    samplerDescription.magFilter = Filter_Nearest;
    samplerDescription.maxAnisotropy = 0.0f;
    samplerDescription.maxLod = 16;
    samplerDescription.minLod = 0;
    samplerDescription.minFilter = Filter_Nearest;
    samplerDescription.mipLodBias = 0.f;
    samplerDescription.mipMapMode = SamplerMipMapMode_Nearest;
    GraphicsSampler* sampler = nullptr;
    device->createSampler(&sampler, samplerDescription);
    return sampler;
}


int main(char* argv[], int c)
{
    Log::initializeLoggingSystem();
    enableLogTypes(LogType_Debug | LogType_Info);
    RealtimeTick::initializeWatch(1ull, 0);
    instance  = GraphicsInstance::create(GraphicsApi_Vulkan);
    GraphicsAdapter* adapter    = nullptr;
    GraphicsSampler* sampler    = nullptr;

    Window* window = Window::create("Box", 0, 0, 1200, 800, ScreenMode_Windowed);
    window->show();
    window->setToCenter();
    window->setOnWindowResize(ResizeFunction);

    {
        ApplicationInfo appInfo = { };
        appInfo.engineName = "";
        appInfo.appName = "Box";
        appInfo.appMinor = 0;
        appInfo.appMajor = 0;
        appInfo.appPatch = 0;
        LayerFeatureFlags flags = LayerFeatureFlag_DebugValidation | LayerFeatureFlag_GpuDebugValidation;
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

    SwapchainCreateDescription swapchainDescription = { };
    swapchainDescription.buffering      = FrameBuffering_Triple;
    swapchainDescription.desiredFrames  = 3;
    swapchainDescription.format         = ResourceFormat_R8G8B8A8_Unorm;
    swapchainDescription.renderWidth    = window->getWidth();
    swapchainDescription.renderHeight   = window->getHeight();
    swapchain = device->createSwapchain(swapchainDescription, window->getNativeHandle());

    GraphicsResource* textureResource = nullptr;
    createTextureResource(&textureResource);
    sampler = createSampler(device);

    buildVertexLayouts(device);
    createShaderProgram(device);

    GraphicsResource* vertexbuffer = buildVertexBuffer();
    GraphicsResource* indexBuffer = buildIndexBuffer();
    GraphicsResource* constantBuffer = buildConstantBuffer(device);

    depthBuffer = buildDepthBuffer(window->getWidth(), window->getHeight());    

    std::array<F32, 10> lastMs;
    U32 frameCount = 0;
    GraphicsSwapchain* pSc = swapchain;
    while (!window->shouldClose())
    {

        if (!window->isMinimized())
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
            pSc->prepare(context);
                GraphicsResource* swapchainImage = pSc->getFrame(pSc->getCurrentFrameIndex());
                context->transition(swapchainImage, ResourceState_RenderTarget);
                context->transition(vertexbuffer, ResourceState_VertexBuffer);
                context->transition(indexBuffer, ResourceState_IndexBuffer);
                context->transition(depthBuffer, ResourceState_DepthStencilWrite);
                context->transition(textureResource, ResourceState_ShaderResource);
                ResourceViewDescription viewDescription = { };
                viewDescription.type = ResourceViewType_RenderTarget;
                viewDescription.format = pSc->getDesc().format;
                viewDescription.dimension = ResourceViewDimension_2d;
                viewDescription.baseMipLevel = 0;
                viewDescription.baseArrayLayer = 0;
                viewDescription.layerCount = 1;
                viewDescription.mipLevelCount = 1;
                ResourceViewId viewId = swapchainImage->asView(viewDescription);
                ResourceViewDescription depthDescription = { };
                depthDescription.type = ResourceViewType_DepthStencil;
                depthDescription.format = ResourceFormat_D32_Float;
                depthDescription.dimension = ResourceViewDimension_2d;
                depthDescription.baseMipLevel = 0;
                depthDescription.baseArrayLayer = 0;
                depthDescription.layerCount = 1;
                depthDescription.mipLevelCount = 1;
                ResourceViewId depthId = depthBuffer->asView(depthDescription);
                ResourceViewDescription textureDescription = { };
                textureDescription.baseArrayLayer = 0;
                textureDescription.baseMipLevel = 0;
                textureDescription.format = ResourceFormat_R8G8B8A8_Unorm;
                textureDescription.dimension = ResourceViewDimension_2d;
                textureDescription.layerCount = 1;
                textureDescription.mipLevelCount = 4;
                textureDescription.type = ResourceViewType_ShaderResource;
                ResourceViewId textureView = textureResource->asView(textureDescription);
                Viewport viewport = { 0, 0, pSc->getDesc().renderWidth, pSc->getDesc().renderHeight, 1, 0 };
                Rect scissor = { 0, 0, pSc->getDesc().renderWidth, pSc->getDesc().renderHeight };
                Math::Float4 clearColor = { 0, 0, 0, 1.0f };
                U64 offset[] = { 0 };
                context->bindRenderTargets(1, &viewId, depthId);
                context->clearDepthStencil(ClearFlag_Depth, 0.f, 0, scissor);
                context->clearRenderTarget(0, &clearColor.x, scissor);
                context->setInputVertexLayout(VertexLayout_PositionNormalTexCoordColor);
                context->setColorWriteMask(0, Color_Rgba);
                IShaderProgramBinder& binder = context->bindShaderProgram(ShaderProgram_Box);
                binder.bindShaderResource(ShaderStage_Pixel, 0, textureView);
                binder.bindSampler(ShaderStage_Pixel, 0, sampler);
                updateConstBuffer(binder, constantBuffer, window->getWidth(), window->getHeight(), tick.delta());
                //context->bindConstantBuffer(ShaderStage_Vertex | ShaderStage_Pixel, 0, constantBuffer, 0, sizeof(ConstBuffer));
                context->enableDepth(true);
                context->enableDepthWrite(true);
                context->bindVertexBuffers(1, &vertexbuffer, offset);
                context->bindIndexBuffer(indexBuffer, 0, IndexType_Unsigned32);
                context->setDepthCompareOp(CompareOp_GreaterOrEqual);
                context->setTopology(PrimitiveTopology_TriangleList);
                context->setViewports(1, &viewport);
                context->setScissors(1, &scissor);
                context->drawIndexedInstanced(36, 1, 0, 0, 0);
                context->transition(textureResource, ResourceState_CopySource);
                context->transition(swapchainImage, ResourceState_CopyDestination);
                context->copyResource(swapchainImage, textureResource);
                context->transition(pSc->getFrame(pSc->getCurrentFrameIndex()), ResourceState_Present);
            context->end();
            if (pSc->present(context) == RecluseResult_NeedsUpdate)
            {
                context->wait();
                pSc->rebuild(pSc->getDesc());
            }

            KeyboardListener listener;
            if (listener.isKeyDown(KeyCode_Escape))
            {
                window->close();
            }
            if (listener.isKeyDownOnce(KeyCode_F))
            {
                if (window->getScreenMode() != ScreenMode_FullscreenBorderless)
                    window->setScreenMode(ScreenMode_FullscreenBorderless);
                else
                {
                    window->setScreenMode(ScreenMode_WindowBorderless);
                    window->setScreenSize(800, 600);
                }
                window->update();
                window->setToCenter();
            }
        }

        pollEvents();
    }
        
    context->wait();

    device->destroySwapchain(swapchain);
    device->destroySampler(sampler);
    device->destroyResource(constantBuffer);
    device->destroyResource(depthBuffer);
    device->destroyResource(vertexbuffer);
    device->destroyResource(indexBuffer);
    device->destroyResource(textureResource);
    device->releaseContext(context);
    adapter->destroyDevice(device);
    GraphicsInstance::destroyInstance(instance);
    Log::destroyLoggingSystem();
    return 0;
}