//
#include "Recluse/Types.hpp"
#include "Recluse/System/Window.hpp"
#include "Recluse/System/Input.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Messaging.hpp"

#include "Recluse/Pipeline/ShaderProgramBuilder.hpp"
#include "Recluse/Renderer/Texture.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"
#include "Recluse/System/KeyboardInput.hpp"
#include "Recluse/Filesystem/Archive.hpp"

#include "Recluse/Math/Matrix33.hpp"
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Vector3.hpp"
#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Time.hpp"
#include "../Shared/Geometry.hpp"

#include <array>

using namespace Recluse;

GraphicsDevice* device = nullptr;
GraphicsContext* context = nullptr;
GraphicsSwapchain* swapchain = nullptr;
GraphicsInstance* instance = nullptr;


// TODO: Need to figure out how to better handle resolution resizing.
// GBuffer info.
GraphicsResource* albedoTexture = nullptr;
GraphicsResource* normalTexture = nullptr;
GraphicsResource* materialTexture = nullptr;
GraphicsResource* depthTexture = nullptr;
GraphicsSampler*  gbufferSampler = nullptr;

GraphicsResource* lightSceneTexture = nullptr;
GraphicsResource* lightBuffer = nullptr;
GraphicsResource* lightViewBuffer = nullptr;

GraphicsResource* sceneBuffer = nullptr;

static const U32 g_textureWidth = 64;
static const U32 g_textureHeight = 64;
typedef Math::Float4 Vector4;

struct ConstBuffer
{
    Math::Mat44 modelViewProjection;
    Math::Mat44 normal;
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


struct MeshDraw
{
    GraphicsResource* vertexBuffer;
    GraphicsResource* indexBuffer;
    GraphicsResource* meshTransform;
    GraphicsResource* albedoTexture;
    ResourceViewId albedoView;
    U32 numIndices;
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
    ShaderProgram_Gbuffer = 1234324,
    ShaderProgram_LightResolve = 875467564,
};

//GraphicsResource* buildDepthBuffer(U32, U32);


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
    memcpy(stagingMem, (void*)texture.getBaseAddress(), texture.getTotalSizeBytes());
    stagingBuffer->unmap(nullptr);
    
    device->copyResource(*textureResource, stagingBuffer);
    device->destroyResource(stagingBuffer);
}


void createShaderProgram(GraphicsDevice* device)
{

    ShaderProgramDatabase database          = ShaderProgramDatabase("PipelineInitialization.D3D12.Database");
#if 1
    if (instance->getApi() == GraphicsApi_Direct3D12)
        GlobalCommands::setValue("ShaderBuilder.NameId", "dxc");
    std::string currDir = Filesystem::getDirectoryFromPath(__FILE__);
    std::string vsSource = currDir + "/" + "gbuffer.vs.hlsl";
    std::string fsSource = currDir + "/" + "gbuffer.ps.hlsl";
    Pipeline::Builder::ShaderProgramDescription description;
    description.pipelineType = BindType_Graphics;
    description.language = ShaderLanguage_Hlsl;
    description.graphics.vs = vsSource.c_str();
    description.graphics.vsName = "Main";
    description.graphics.ps = fsSource.c_str();
    description.graphics.psName = "psMain";

    for (U32 i = 0; i < 2; ++i)
    {
        Pipeline::Builder::ShaderProgramPermutationDefinitionInstance permutation;
        Pipeline::Builder::ShaderProgramPermutationDefinition definition;
        definition.name = "USE_TEXTURE";
        definition.offset = 0;
        definition.size = 1;
        definition.value = i;
        permutation.push_back(definition);
        description.permutationDefinitions.push_back(permutation);
    }

    Pipeline::Builder::buildShaderProgramDefinitions(database, description, ShaderProgram_Gbuffer, instance->getApi() == GraphicsApi_Direct3D12 ? ShaderIntermediateCode_Dxil : ShaderIntermediateCode_Spirv);
    Runtime::buildShaderProgram(device, database, ShaderProgram_Gbuffer);

    vsSource = currDir + "/" + "quad.vs.hlsl";
    fsSource = currDir + "/" + "resolve.ps.hlsl";
    description.pipelineType = BindType_Graphics;
    description.language = ShaderLanguage_Hlsl;
    description.graphics.vs = vsSource.c_str();
    description.graphics.vsName = "Main";
    description.graphics.ps = fsSource.c_str();
    description.graphics.psName = "psMain";

    Pipeline::Builder::buildShaderProgramDefinitions(database, description, ShaderProgram_LightResolve, instance->getApi() == GraphicsApi_Direct3D12 ? ShaderIntermediateCode_Dxil : ShaderIntermediateCode_Spirv);
    Runtime::buildShaderProgram(device, database, ShaderProgram_LightResolve);
    {
        ArchiveWriter writer("dxil.database");
        database.serialize(&writer);
    }
#else
    {
        ArchiveReader reader("dxil.database");
        database.deserialize(&reader);
        Runtime::buildShaderProgram(device, database, ShaderProgram_LightResolve);
        Runtime::buildShaderProgram(device, database, ShaderProgram_Gbuffer);
    }
#endif
    database.clearShaderProgramDefinitions();

}


void buildVertexLayouts(GraphicsDevice* device)
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
    Bool result = device->makeVertexLayout(VertexLayout_PositionNormalTexCoordColor, layout);
    R_ASSERT(result);
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
    samplerDescription.maxLod = 1;
    samplerDescription.minLod = 0;
    samplerDescription.minFilter = Filter_Nearest;
    samplerDescription.mipLodBias = 0.f;
    samplerDescription.mipMapMode = SamplerMipMapMode_Nearest;
    GraphicsSampler* sampler = nullptr;
    device->createSampler(&sampler, samplerDescription);
    return sampler;
}


ResultCode createGBuffer(GraphicsDevice* device, U32 width, U32 height)
{
    GraphicsResourceDescription description = { };
    description.dimension = ResourceDimension_2d;
    description.usage = ResourceUsage_RenderTarget | ResourceUsage_ShaderResource | ResourceUsage_UnorderedAccess;
    description.memoryUsage = ResourceMemoryUsage_GpuOnly;
    description.mipLevels = 1;
    description.depthOrArraySize = 1;
    description.width = width;
    description.height = height;
    description.samples = 1;
    
    // Albedo
    description.format = ResourceFormat_R8G8B8A8_Unorm;
    description.name = "GBufferAlbedo";
    ResultCode code = device->createResource(&albedoTexture, description, ResourceState_Common);
    
    description.format = ResourceFormat_R16G16B16A16_Float;
    description.name = "GBufferNormal";
    code = device->createResource(&normalTexture, description, ResourceState_Common);
    
    description.format = ResourceFormat_D32_Float;
    description.name = "GBufferDepth";
    description.usage = ResourceUsage_ShaderResource | ResourceUsage_DepthStencil;
    code = device->createResource(&depthTexture, description, ResourceState_DepthStencilWrite);
    return code;
}


ResultCode destroyGBuffer(GraphicsDevice* device, Bool immediate = false)
{
    if (albedoTexture)
    {
        device->destroyResource(albedoTexture, immediate);
        albedoTexture = nullptr;
    }
    
    if (normalTexture)
    {
        device->destroyResource(normalTexture, immediate);
        normalTexture = nullptr;
    }

    if (depthTexture)
    {
        device->destroyResource(depthTexture, immediate);
        depthTexture = nullptr;
    }
    return RecluseResult_Ok;
}


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
            destroyGBuffer(device, true);
            createGBuffer(device, width, height);
        }
    }
}


struct Light
{
	I32 	LightType;
	F32 	Position;
	F32 	Direction;
	F32 	Attenuation;
	Math::Float4 Color;
};


void createLightBuffer(GraphicsDevice* device)
{
    GraphicsResourceDescription description = { };
    description.dimension = ResourceDimension_Buffer;
    description.format = ResourceFormat_Unknown;
    description.height = 1;
    description.depthOrArraySize = 1;
    description.width = sizeof(Light) * 1;
    description.usage = ResourceUsage_ShaderResource | ResourceUsage_CopyDestination;
    description.mipLevels = 1;
    description.miscFlags = ResourceMiscFlag_StructuredBuffer;
    description.memoryUsage = ResourceMemoryUsage_GpuOnly;

    ResultCode result = device->createResource(&lightBuffer, description, ResourceState_Common);

    struct LightView
    {
        U32 numLights;
        Math::Int3 pad0;
    };

    description.usage = ResourceUsage_ConstantBuffer;
    description.width = sizeof(LightView);
    description.miscFlags = 0;
    description.memoryUsage = ResourceMemoryUsage_CpuToGpu;
    result = device->createResource(&lightViewBuffer, description, ResourceState_ConstantBuffer);
    R_ASSERT(result == RecluseResult_Ok);
}


void applyGBufferRendering(GraphicsContext* context, const std::vector<MeshDraw>& meshes)
{
    context->transition(albedoTexture, ResourceState_RenderTarget);
    context->transition(normalTexture, ResourceState_RenderTarget);
    context->transition(depthTexture, ResourceState_DepthStencilWrite);
    
    ResourceViewDescription description = { };
    description.baseArrayLayer = 0;
    description.baseMipLevel = 0;
    description.layerCount = 1;
    description.mipLevelCount = 1;
    description.type = ResourceViewType_RenderTarget;
    description.dimension = ResourceViewDimension_2d;

    description.format = ResourceFormat_R8G8B8A8_Unorm;

    ResourceViewId albedoRtv = albedoTexture->asView(description);

    description.format = ResourceFormat_R16G16B16A16_Float;
    ResourceViewId normalRtv = normalTexture->asView(description);

    description.format = ResourceFormat_D32_Float;
    description.type = ResourceViewType_DepthStencil;

    ResourceViewId dsv = depthTexture->asView(description);

    ResourceViewId rtvs[] = { albedoRtv, normalRtv };
    context->pushState();
    context->bindRenderTargets(2, rtvs, dsv);
    context->setTopology(PrimitiveTopology_TriangleList);
    context->enableDepth(true);
    context->enableDepthWrite(true);
    context->setColorWriteMask(0, Color_Rgba);
    context->setColorWriteMask(1, Color_Rgba);
    context->setDepthCompareOp(CompareOp_GreaterOrEqual);

    Viewport viewport = { 0, 0, swapchain->getDesc().renderWidth, swapchain->getDesc().renderHeight, 1, 0 };
    Rect scissor = { 0, 0, swapchain->getDesc().renderWidth, swapchain->getDesc().renderHeight };
    F32 clearColor[4] = { 0, 0, 0, 0 };
    context->clearRenderTarget(0, clearColor, scissor);
    context->clearRenderTarget(1, clearColor, scissor);
    context->clearDepthStencil(ClearFlag_Depth, 0.f, 0, scissor);

    context->setViewports(1, &viewport);
    context->setScissors(1, &scissor);
    context->setInputVertexLayout(VertexLayout_PositionNormalTexCoordColor);
    U32 permutation = 0;
    KeyboardListener listener;
    if (listener.isKeyDown(KeyCode_A))
    {
        permutation = makeBitset32(0, 1, 1);
    }
    IShaderProgramBinder& binder = context->bindShaderProgram(ShaderProgram_Gbuffer, permutation);

    for (U32 i = 0; i < meshes.size(); ++i)
    {
        U64 offset[] = { 0 };
        GraphicsResource* vb = meshes[i].vertexBuffer;
        context->transition(meshes[i].albedoTexture, ResourceState_ShaderResource);
        binder.bindConstantBuffer(ShaderStage_Pixel | ShaderStage_Vertex, 0, meshes[i].meshTransform, 0, sizeof(ConstBuffer))
                .bindShaderResource(ShaderStage_Pixel, 0, meshes[i].albedoView)
                .bindSampler(ShaderStage_Pixel, 0, gbufferSampler);
        context->bindVertexBuffers(1, &vb, offset);
        context->bindIndexBuffer(meshes[i].indexBuffer, 0, IndexType_Unsigned32);
        context->drawIndexedInstanced(meshes[i].numIndices, 1, 0, 0, 0);
    }
    context->popState();
    context->clearResourceBinds();
}


void createSceneBuffer(GraphicsDevice* device)
{
    struct SceneBuffer
    {
        Math::Mat44 viewProjection;
        Math::Mat44 view;
    };

    GraphicsResourceDescription description = { };
    description.dimension = ResourceDimension_Buffer;
    description.format = ResourceFormat_Unknown;
    description.height = 1;
    description.depthOrArraySize = 1;
    description.width = sizeof(SceneBuffer);
    description.usage = ResourceUsage_ConstantBuffer;
    description.mipLevels = 1;
    description.miscFlags = ResourceMiscFlag_StructuredBuffer;
    description.memoryUsage = ResourceMemoryUsage_CpuToGpu;

    ResultCode result = device->createResource(&sceneBuffer, description, ResourceState_ConstantBuffer);
    R_ASSERT(result == RecluseResult_Ok);
}


void resolveLighting(GraphicsContext* context)
{
    ResourceViewDescription description = { };
    description.baseArrayLayer = 0;
    description.baseMipLevel = 0;
    description.dimension = ResourceViewDimension_2d;
    description.layerCount = 1;
    description.mipLevelCount = 1;
    description.type = ResourceViewType_RenderTarget;
    GraphicsResource* finalImage = swapchain->getFrame(swapchain->getCurrentFrameIndex());

    description.format = swapchain->getDesc().format;
    ResourceViewId id = finalImage->asView(description);    
    
    description.format = ResourceFormat_R8G8B8A8_Unorm;
    description.type = ResourceViewType_ShaderResource;
    ResourceViewId albedoView = albedoTexture->asView(description);

    description.format = ResourceFormat_R16G16B16A16_Float;
    ResourceViewId normalView = normalTexture->asView(description);

    description.format = ResourceFormat_R32_Float;
    ResourceViewId depthView = depthTexture->asView(description);

    description.dimension = ResourceViewDimension_Buffer;
    description.firstElement = 0;
    description.numElements = 1;
    description.byteStride = sizeof(Light);
    description.format = ResourceFormat_Unknown;
    ResourceViewId lightBufferView = lightBuffer->asView(description);

    Viewport viewport = { 0, 0, swapchain->getDesc().renderWidth, swapchain->getDesc().renderHeight, 1, 0 };
    Rect scissor = { 0, 0, swapchain->getDesc().renderWidth, swapchain->getDesc().renderHeight };
    F32 clearColor[4] = { 0, 0, 0, 0 };

    struct SceneBuffer
    {
        Math::Mat44 viewProjection;
        Math::Mat44 view;
    };

    struct LightView
    {
        U32 numLights;
        Math::Int3 pad0;
    };

    context->transition(finalImage, ResourceState_RenderTarget);
    context->transition(sceneBuffer, ResourceState_ConstantBuffer);
    context->transition(lightViewBuffer, ResourceState_ConstantBuffer);
    context->transition(albedoTexture, ResourceState_ShaderResource);
    context->transition(normalTexture, ResourceState_ShaderResource);
    context->transition(depthTexture, ResourceState_ShaderResource);
    context->transition(lightBuffer, ResourceState_ShaderResource);
    context->pushState();
        context->bindShaderProgram(ShaderProgram_LightResolve, 0)
            .bindShaderResource(ShaderStage_Pixel, 0, albedoView)
            .bindShaderResource(ShaderStage_Pixel, 1, normalView)
            .bindShaderResource(ShaderStage_Pixel, 4, lightBufferView)
            .bindShaderResource(ShaderStage_Pixel, 3, depthView)
            .bindConstantBuffer(ShaderStage_Pixel, 0, sceneBuffer, 0, sizeof(SceneBuffer))
            .bindConstantBuffer(ShaderStage_Pixel, 1, lightViewBuffer, 0, sizeof(LightView))
            .bindSampler(ShaderStage_Pixel, 0, gbufferSampler);

        context->bindRenderTargets(1, &id);
        context->setTopology(PrimitiveTopology_TriangleList);
        context->setColorWriteMask(0, Color_Rgba);
        context->setInputVertexLayout(VertexInputLayout::VertexLayout_Null);
        context->clearRenderTarget(0, clearColor, scissor);
        context->setViewports(1, &viewport);
        context->setScissors(1, &scissor);
        context->drawInstanced(3, 1, 0, 0);
    context->popState();
}


void createCubes(GraphicsDevice* device, std::vector<MeshDraw>& meshes, U32 width, U32 height)
{
    meshes.resize(1);
    std::vector<Vertex> vertices = createCubeInstance(1.0f);
    std::vector<U32> indices = createCubeIndicesInstance();

    Math::Matrix44 view = Math::translate(Math::Matrix44::identity(), Math::Float3(0, 0, 0));
    Math::Matrix44 proj = Math::perspectiveLH_Aspect(Math::deg2Rad(45.0f), (F32)width / (F32)height, 0.001f, 1000.0f);
    for (auto& it : meshes)
    {
        GraphicsResourceDescription description = { };
        description.depthOrArraySize = 1;
        description.memoryUsage = ResourceMemoryUsage_GpuOnly;
        description.mipLevels = 1;
        description.height = 1;
        description.width = sizeof(Vertex) * vertices.size();
        description.samples = 1;
        description.usage = ResourceUsage_VertexBuffer | ResourceUsage_CopyDestination;
        description.dimension = ResourceDimension_Buffer;
        description.format = ResourceFormat_Unknown;
        device->createResource(&it.vertexBuffer, description, ResourceState_CopyDestination);
        
        description.usage =  ResourceUsage_IndexBuffer | ResourceUsage_CopyDestination;
        description.width = sizeof(U32) * indices.size();
        device->createResource(&it.indexBuffer, description, ResourceState_CopyDestination);

        description.usage = ResourceUsage_ConstantBuffer;
        description.memoryUsage = ResourceMemoryUsage_CpuToGpu;
        description.width = sizeof(ConstBuffer);
        device->createResource(&it.meshTransform, description, ResourceState_ConstantBuffer);

        {
            GraphicsResource* buf = nullptr;
            GraphicsResourceDescription bufDesc = { };
            bufDesc.depthOrArraySize = 1;
            bufDesc.dimension = ResourceDimension_Buffer;
            bufDesc.format = ResourceFormat_Unknown;
            bufDesc.height = 1;
            bufDesc.mipLevels = 1;
            bufDesc.width = sizeof(Vertex) * vertices.size();
            bufDesc.memoryUsage = ResourceMemoryUsage_CpuToGpu;
            bufDesc.usage = ResourceUsage_CopySource;
            device->createResource(&buf, bufDesc, ResourceState_CopySource);
            void* dat = nullptr;
            buf->map(&dat, nullptr);
            memcpy(dat, vertices.data(), sizeof(Vertex) * vertices.size());
            buf->unmap(nullptr);

            CopyBufferRegion region = { };
            region.szBytes = sizeof(Vertex) * vertices.size();
            region.dstOffsetBytes = 0;
            region.srcOffsetBytes = 0;
            device->copyBufferRegions(it.vertexBuffer, buf, &region, 1);
            device->destroyResource(buf);
        }

        {
            GraphicsResource* buf = nullptr;
            GraphicsResourceDescription bufDesc = { };
            bufDesc.depthOrArraySize = 1;
            bufDesc.dimension = ResourceDimension_Buffer;
            bufDesc.format = ResourceFormat_Unknown;
            bufDesc.height = 1;
            bufDesc.mipLevels = 1;
            bufDesc.width = sizeof(U32) * indices.size();
            bufDesc.memoryUsage = ResourceMemoryUsage_CpuToGpu;
            bufDesc.usage = ResourceUsage_CopySource;
            device->createResource(&buf, bufDesc, ResourceState_CopySource);
            void* dat = nullptr;
            buf->map(&dat, nullptr);
            memcpy(dat, indices.data(), sizeof(U32) * vertices.size());
            buf->unmap(nullptr);

            CopyBufferRegion region = { };
            region.szBytes = sizeof(U32) * vertices.size();
            region.dstOffsetBytes = 0;
            region.srcOffsetBytes = 0;
            device->copyBufferRegions(it.indexBuffer, buf, &region, 1);
            device->destroyResource(buf);
        }

        ConstBuffer* buffer = nullptr;
        it.meshTransform->map((void**)&buffer, nullptr);
        F32 t = 20.0f * 0;
        t = fmod(t, 360.0f);
        Math::Mat44 T = Math::translate(Math::Mat44::identity(), Math::Float3(0, 0, 6));
        Math::Mat44 R = Math::rotate(Math::Mat44::identity(), Math::Float3(0.0f, 1.0f, 0.0f), Math::deg2Rad(45.0f));
        Math::Mat44 R2 = Math::rotate(Math::Mat44::identity(), Math::Float3(1.0f, 0.0f, 1.0f), Math::deg2Rad(t));
        Math::Mat44 model = R2 * R * T;
        buffer->modelViewProjection = model * view * proj;
        model(3, 0) = 0.f;
        model(3, 1) = 0.f;
        model(3, 2) = 0.f;
        buffer->normal = Math::transpose(Math::inverse(model));
        it.meshTransform->unmap(nullptr);
        
        it.numIndices = (U32)indices.size();
        createTextureResource(&it.albedoTexture);

        ResourceViewDescription textureDescription = { };
        textureDescription.baseArrayLayer = 0;
        textureDescription.baseMipLevel = 0;
        textureDescription.format = ResourceFormat_R8G8B8A8_Unorm;
        textureDescription.dimension = ResourceViewDimension_2d;
        textureDescription.layerCount = 1;
        textureDescription.mipLevelCount = 4;
        textureDescription.type = ResourceViewType_ShaderResource;
        it.albedoView = it.albedoTexture->asView(textureDescription);
    }
}


void destroyCubes(GraphicsDevice* device, std::vector<MeshDraw>& meshes)
{
    for (auto& it : meshes)
    {
        if (it.indexBuffer)
            device->destroyResource(it.indexBuffer);
        if (it.vertexBuffer)
            device->destroyResource(it.vertexBuffer);
        if (it.meshTransform)
            device->destroyResource(it.meshTransform);
        if (it.albedoTexture)
            device->destroyResource(it.albedoTexture);
        it.indexBuffer = nullptr;
        it.vertexBuffer = nullptr;
        it.meshTransform = nullptr;
    }
}


int main(char* argv[], int c)
{
    Log::initializeLoggingSystem();
    enableLogTypes(LogType_Debug | LogType_Info);
    RealtimeTick::initializeWatch(1ull, 0);
    instance  = GraphicsInstance::create(GraphicsApi_Direct3D12);
    GraphicsAdapter* adapter    = nullptr;
    std::vector<MeshDraw> meshes;

    Window* window = Window::create("Deferred", 0, 0, 1200, 800, ScreenMode_Windowed);
    window->show();
    window->setToCenter();
    window->setOnWindowResize(ResizeFunction);

    {
        ApplicationInfo appInfo = { };
        appInfo.engineName = "";
        appInfo.appName = "Deferred";
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
    gbufferSampler = createSampler(device);

    buildVertexLayouts(device);
    createShaderProgram(device);

    createGBuffer(device, swapchain->getDesc().renderWidth, swapchain->getDesc().renderHeight);
    createLightBuffer(device);
    createSceneBuffer(device);
    createCubes(device, meshes, window->getWidth(), window->getHeight());

    std::array<F32, 10> lastMs;
    U32 frameCount = 0;
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
                R_WARN("DeferredBox", "Fps: %f", 1.0f / total);
            }
            swapchain->prepare(context);

            applyGBufferRendering(context, meshes);
            resolveLighting(context);

            context->transition(swapchain->getFrame(swapchain->getCurrentFrameIndex()), ResourceState_Present);
            context->end();

            if (swapchain->present(context) == RecluseResult_NeedsUpdate)
            {
                context->wait();
                swapchain->rebuild(swapchain->getDesc());
            }

            KeyboardListener listener;
            if (listener.isKeyDown(KeyCode_Escape))
            {
                window->close();
            }
        }
        pollEvents();
    }
        
    context->wait();

    destroyGBuffer(device);
    destroyCubes(device, meshes);
    device->destroySwapchain(swapchain);
    device->destroySampler(gbufferSampler);
    //device->destroyResource(depthBuffer);
    device->destroyResource(lightBuffer);
    device->destroyResource(lightViewBuffer);
    device->destroyResource(sceneBuffer);
    device->destroyResource(textureResource);
    device->releaseContext(context);
    adapter->destroyDevice(device);
    GraphicsInstance::destroyInstance(instance);
    Log::destroyLoggingSystem();
    return 0;
}