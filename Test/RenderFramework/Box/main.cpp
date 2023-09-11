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
#include "Recluse/Filesystem/Filesystem.hpp"

#include "Recluse/Math/Matrix33.hpp"
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Vector3.hpp"
#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Time.hpp"

#include <array>

using namespace Recluse;

GraphicsDevice* device = nullptr;
GraphicsContext* context = nullptr;
GraphicsSwapchain* swapchain = nullptr;
GraphicsInstance* instance = nullptr;
static const U32 g_textureWidth = 64;
static const U32 g_textureHeight = 64;
typedef Math::Float4 Vector4;


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

std::array<Vector4, 36> positions = {
  // front
  Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
  Vector4( 1.0f, -1.0f, 1.0f, 1.0f),
  Vector4( 1.0f,  1.0f, 1.0f, 1.0f),
  Vector4( 1.0f,  1.0f, 1.0f, 1.0f),
  Vector4(-1.0f,  1.0f, 1.0f, 1.0f),
  Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
  // Back
  Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
  Vector4(-1.0f,  1.0f, -1.0f, 1.0f),
  Vector4( 1.0f,  1.0f, -1.0f, 1.0f),
  Vector4( 1.0f,  1.0f, -1.0f, 1.0f),
  Vector4( 1.0f, -1.0f, -1.0f, 1.0f),
  Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
  // up
  Vector4( 1.0f,  1.0f,  1.0f, 1.0f),
  Vector4( 1.0f,  1.0f, -1.0f, 1.0f),
  Vector4(-1.0f,  1.0f, -1.0f, 1.0f),
  Vector4(-1.0f,  1.0f, -1.0f, 1.0f),
  Vector4(-1.0f,  1.0f,  1.0f, 1.0f),
  Vector4( 1.0f,  1.0f,  1.0f, 1.0f),
  // Down
  Vector4( 1.0f, -1.0f,  1.0f, 1.0f),
  Vector4(-1.0f, -1.0f,  1.0f, 1.0f),
  Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
  Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
  Vector4( 1.0f, -1.0f, -1.0f, 1.0f),
  Vector4( 1.0f, -1.0f,  1.0f, 1.0f),
  // right
  Vector4( 1.0f, -1.0f,  1.0f, 1.0f),
  Vector4( 1.0f, -1.0f, -1.0f, 1.0f),
  Vector4( 1.0f,  1.0f, -1.0f, 1.0f),
  Vector4( 1.0f,  1.0f, -1.0f, 1.0f),
  Vector4( 1.0f,  1.0f,  1.0f, 1.0f),
  Vector4( 1.0f, -1.0f,  1.0f, 1.0f),
  // Left
  Vector4(-1.0f, -1.0f,  1.0f, 1.0f),
  Vector4(-1.0f,  1.0f,  1.0f, 1.0f),
  Vector4(-1.0f,  1.0f, -1.0f, 1.0f),
  Vector4(-1.0f,  1.0f, -1.0f, 1.0f),
  Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
  Vector4(-1.0f, -1.0f,  1.0f, 1.0f),
};


std::array<Vector4, 36> normals = {
  // front 
  Vector4(0.0f, 0.0f, 1.0f, 0.0f),
  Vector4(0.0f, 0.0f, 1.0f, 0.0f),
  Vector4(0.0f, 0.0f, 1.0f, 0.0f),
  Vector4(0.0f, 0.0f, 1.0f, 0.0f),
  Vector4(0.0f, 0.0f, 1.0f, 0.0f),
  Vector4(0.0f, 0.0f, 1.0f, 0.0f),
  // Back
  Vector4(0.0f, 0.0f, -1.0f, 0.0f),
  Vector4(0.0f, 0.0f, -1.0f, 0.0f),
  Vector4(0.0f, 0.0f, -1.0f, 0.0f),
  Vector4(0.0f, 0.0f, -1.0f, 0.0f),
  Vector4(0.0f, 0.0f, -1.0f, 0.0f),
  Vector4(0.0f, 0.0f, -1.0f, 0.0f),
  // up
  Vector4(0.0f, 1.0f, 0.0f, 0.0f),
  Vector4(0.0f, 1.0f, 0.0f, 0.0f),
  Vector4(0.0f, 1.0f, 0.0f, 0.0f),
  Vector4(0.0f, 1.0f, 0.0f, 0.0f),
  Vector4(0.0f, 1.0f, 0.0f, 0.0f),
  Vector4(0.0f, 1.0f, 0.0f, 0.0f),
  // Down
  Vector4(0.0f, -1.0f, 0.0f, 0.0f),
  Vector4(0.0f, -1.0f, 0.0f, 0.0f),
  Vector4(0.0f, -1.0f, 0.0f, 0.0f),
  Vector4(0.0f, -1.0f, 0.0f, 0.0f),
  Vector4(0.0f, -1.0f, 0.0f, 0.0f),
  Vector4(0.0f, -1.0f, 0.0f, 0.0f),
  // right
  Vector4(1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 0.0f, 0.0f),
  // Left
  Vector4(-1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(-1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(-1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(-1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(-1.0f, 0.0f, 0.0f, 0.0f),
  Vector4(-1.0f, 0.0f, 0.0f, 0.0f)
};


std::array<Vector4, 36> texcoords = {
  // front
  Vector4(0.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 1.0f, 0.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(0.0f, 1.0f, 0.0f, 1.0f),
  Vector4(0.0f, 0.0f, 0.0f, 0.0f),

  Vector4(0.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 1.0f, 0.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(0.0f, 1.0f, 0.0f, 1.0f),
  Vector4(0.0f, 0.0f, 0.0f, 0.0f),

  Vector4(0.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 1.0f, 0.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(0.0f, 1.0f, 0.0f, 1.0f),
  Vector4(0.0f, 0.0f, 0.0f, 0.0f),

  Vector4(0.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 1.0f, 0.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(0.0f, 1.0f, 0.0f, 1.0f),
  Vector4(0.0f, 0.0f, 0.0f, 0.0f),

  // right
  Vector4(0.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 1.0f, 0.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(0.0f, 1.0f, 0.0f, 1.0f),
  Vector4(0.0f, 0.0f, 0.0f, 0.0f),

  // Left
  Vector4(0.0f, 0.0f, 0.0f, 0.0f),
  Vector4(1.0f, 0.0f, 1.0f, 0.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(1.0f, 1.0f, 1.0f, 1.0f),
  Vector4(0.0f, 1.0f, 0.0f, 1.0f),
  Vector4(0.0f, 0.0f, 0.0f, 0.0f),
};


std::array<Vector4, 36> colors = {
  Vector4(0.0f, 1.0f, 1.0f, 1.0f),
  Vector4(0.0f, 1.0f, 1.0f, 1.0f),
  Vector4(0.0f, 1.0f, 1.0f, 1.0f),
  Vector4(0.0f, 1.0f, 1.0f, 1.0f),
  Vector4(0.0f, 1.0f, 1.0f, 1.0f),
  Vector4(0.0f, 1.0f, 1.0f, 1.0f),

  Vector4(1.0f, 0.0f, 1.0f, 1.0f),
  Vector4(1.0f, 0.0f, 1.0f, 1.0f),
  Vector4(1.0f, 0.0f, 1.0f, 1.0f),
  Vector4(1.0f, 0.0f, 1.0f, 1.0f),
  Vector4(1.0f, 0.0f, 1.0f, 1.0f),
  Vector4(1.0f, 0.0f, 1.0f, 1.0f),

  Vector4(1.0f, 1.0f, 0.0f, 1.0f),
  Vector4(1.0f, 1.0f, 0.0f, 1.0f),
  Vector4(1.0f, 1.0f, 0.0f, 1.0f),
  Vector4(1.0f, 1.0f, 0.0f, 1.0f),
  Vector4(1.0f, 1.0f, 0.0f, 1.0f),
  Vector4(1.0f, 1.0f, 0.0f, 1.0f),

  Vector4(1.0f, 0.0f, 0.0f, 1.0f),
  Vector4(1.0f, 0.0f, 0.0f, 1.0f),
  Vector4(1.0f, 0.0f, 0.0f, 1.0f),
  Vector4(1.0f, 0.0f, 0.0f, 1.0f),
  Vector4(1.0f, 0.0f, 0.0f, 1.0f),
  Vector4(1.0f, 0.0f, 0.0f, 1.0f),

  Vector4(0.0f, 1.0f, 0.0f, 1.0f),
  Vector4(0.0f, 1.0f, 0.0f, 1.0f),
  Vector4(0.0f, 1.0f, 0.0f, 1.0f),
  Vector4(0.0f, 1.0f, 0.0f, 1.0f),
  Vector4(0.0f, 1.0f, 0.0f, 1.0f),
  Vector4(0.0f, 1.0f, 0.0f, 1.0f),

  Vector4(0.0f, 0.0f, 1.0f, 1.0f),
  Vector4(0.0f, 0.0f, 1.0f, 1.0f),
  Vector4(0.0f, 0.0f, 1.0f, 1.0f),
  Vector4(0.0f, 0.0f, 1.0f, 1.0f),
  Vector4(0.0f, 0.0f, 1.0f, 1.0f),
  Vector4(0.0f, 0.0f, 1.0f, 1.0f),
};


std::array<U32, 36> indices = {
  0, 1, 2,
  3, 4, 5,
  6, 7, 8,
  9, 10, 11,
  12, 13, 14,
  15, 16, 17,
  18, 19, 20,
  21, 22, 23,
  24, 25, 26,
  27, 28, 29,
  30, 31, 32,
  33, 34, 35
};


std::vector<Vertex> createCubeInstance(F32 scale)
{
  std::vector<Vertex> cube(36);
  for (size_t i = 0; i < cube.size(); ++i) {
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
  for (size_t i = 0; i < indices.size(); ++i) {
    cubeIs[i] = indices[i];
  }

  return cubeIs;
}


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


void updateConstBuffer(GraphicsResource* resource, U32 width, U32 height)
{
    static F32 t = 0;
    t += 0.01f;
    void* dat = nullptr;
    resource->map(&dat, nullptr);
    Math::Matrix44 T = Math::translate(Math::Matrix44::identity(), Math::Float3(0, 0, 6));
    Math::Matrix44 R = Math::rotate(Math::Matrix44::identity(), Math::Float3(0.0f, 1.0f, 0.0f), Math::deg2Rad(45.0f));
    Math::Matrix44 R2 = Math::rotate(Math::Matrix44::identity(), Math::Float3(0.0f, 0.0f, 1.0f), Math::deg2Rad(t));
    Math::Matrix44 model = R2 * R * T;
    Math::Matrix44 view = Math::translate(Math::Matrix44::identity(), Math::Float3(0, 0, 0));
    Math::Matrix44 proj = Math::perspectiveLH_Aspect(Math::deg2Rad(45.0f), (F32)width / (F32)height, 0.001f, 1000.0f);
    if (instance->getApi() == GraphicsApi_Vulkan)
        proj[5] *= -1;
    Math::Matrix44 mvp = model * view * proj;
    ConstBuffer buf = { };
    buf.modelViewProjection = mvp;
    buf.normal = Math::Matrix44::identity();
    buf.useTexturing = 0;
    memcpy(dat, &buf, sizeof(ConstBuffer));
    resource->unmap(nullptr);
}


void createShaderProgram(GraphicsDevice* device)
{
    if (instance->getApi() == GraphicsApi_Direct3D12)
        GlobalCommands::setValue("ShaderBuilder.NameId", "dxc");
    ShaderProgramDatabase database          = ShaderProgramDatabase("PipelineInitialization.D3D12.Database");
    std::string currDir = Filesystem::getDirectoryFromPath(__FILE__);
    std::string vsSource = currDir + "/" + "vertex.hlsl";
    std::string fsSource = currDir + "/" + "pixel.hlsl";
    Pipeline::Builder::ShaderProgramDescription description;
    description.pipelineType = BindType_Graphics;
    description.language = ShaderLang_Hlsl;
    description.graphics.vs = vsSource.c_str();
    description.graphics.vsName = "Main";
    description.graphics.ps = fsSource.c_str();
    description.graphics.psName = "psMain";
    Pipeline::Builder::buildShaderProgramDefinitions(database, description, ShaderProgram_Box, instance->getApi() == GraphicsApi_Direct3D12 ? ShaderIntermediateCode_Dxil : ShaderIntermediateCode_Spirv);
    Runtime::buildShaderProgram(device, database, ShaderProgram_Box);
    database.clearShaderProgramDefinitions();
}


int main(char* argv[], int c)
{
    Log::initializeLoggingSystem();
    enableLogTypes(LogType_Debug);
    RealtimeTick::initializeWatch(1ull, 0);
    instance  = GraphicsInstance::createInstance(GraphicsApi_Vulkan);
    GraphicsAdapter* adapter    = nullptr;

    Window* window = Window::create("Box", 0, 0, 1024, 1024, ScreenMode_Windowed);
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
        LayerFeatureFlags flags = 0;// LayerFeatureFlag_DebugValidation;
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

    buildVertexLayouts(device);
    createShaderProgram(device);

    GraphicsResource* vertexbuffer = buildVertexBuffer();
    GraphicsResource* indexBuffer = buildIndexBuffer();
    GraphicsResource* constantBuffer = buildConstantBuffer(device);

    GraphicsResource* depthBuffer = buildDepthBuffer(window->getWidth(), window->getHeight());    

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
                updateConstBuffer(constantBuffer, window->getWidth(), window->getHeight());
                GraphicsResource* swapchainImage = swapchain->getFrame(swapchain->getCurrentFrameIndex());
                context->transition(textureResource, ResourceState_CopySource);
                context->transition(swapchainImage, ResourceState_CopyDestination);
                context->copyResource(swapchainImage, textureResource);
                context->transition(swapchainImage, ResourceState_RenderTarget);
                context->transition(vertexbuffer, ResourceState_VertexBuffer);
                context->transition(indexBuffer, ResourceState_IndexBuffer);
                context->transition(depthBuffer, ResourceState_DepthStencilWrite);
                ResourceViewDescription viewDescription = { };
                viewDescription.type = ResourceViewType_RenderTarget;
                viewDescription.format = swapchain->getDesc().format;
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
                Viewport viewport = { 0, 0, swapchain->getDesc().renderWidth, swapchain->getDesc().renderHeight, 1, 0 };
                Rect scissor = { 0, 0, swapchain->getDesc().renderWidth, swapchain->getDesc().renderHeight };
                Math::Float4 clearColor = { 0, 0, 0, 1.0f };
                context->bindRenderTargets(1, &viewId, depthId);
                context->clearDepthStencil(ClearFlag_Depth, 0.f, 0, scissor);
                context->clearRenderTarget(0, &clearColor.x, scissor);
                context->setInputVertexLayout(VertexLayout_PositionNormalTexCoordColor);
                context->setColorWriteMask(0, Color_Rgba);
                context->setShaderProgram(ShaderProgram_Box);
                U64 offset[] = { 0 };
                context->enableDepth(true);
                context->enableDepthWrite(true);
                context->bindVertexBuffers(1, &vertexbuffer, offset);
                context->bindIndexBuffer(indexBuffer, 0, IndexType_Unsigned32);
                context->setDepthCompareOp(CompareOp_GreaterOrEqual);
                context->bindConstantBuffer(ShaderStage_Vertex | ShaderStage_Pixel, 0, constantBuffer, 0, sizeof(ConstBuffer));
                context->setTopology(PrimitiveTopology_TriangleList);
                context->setViewports(1, &viewport);
                context->setScissors(1, &scissor);
                context->drawIndexedInstanced(36, 1, 0, 0, 0);
                context->transition(swapchain->getFrame(swapchain->getCurrentFrameIndex()), ResourceState_Present);
            context->end();
            swapchain->present(context);
        }
        pollEvents();
        
    }
        
    context->wait();

    device->destroySwapchain(swapchain);
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