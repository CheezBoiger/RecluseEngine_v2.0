//
#pragma once

#include "VulkanCommons.hpp"
#include "Recluse/Graphics/PipelineState.hpp"
#include "VulkanObjects.hpp"

#include "Recluse/Graphics/ShaderProgram.hpp"

namespace Recluse {
namespace Vulkan {
class VulkanDevice;

typedef Hash64 PipelineId;

namespace Pipelines {
namespace VertexLayout {

struct VulkanVertexLayout
{
    std::vector<VkVertexInputAttributeDescription> descriptions;
    std::vector<VkVertexInputBindingDescription> bindings;


    Hash64 hash() const
    {
        return combineHash64(recluseHashFast(descriptions.data(), descriptions.size()), recluseHashFast(bindings.data(), bindings.size()));
    }
};


ResultCode make(DeviceId deviceId, VertexInputLayoutId id, const VertexInputLayout& layout);
ResultCode unloadLayout(DeviceId deviceId, VertexInputLayoutId id);
Bool unloadAll(DeviceId deviceId);
} // VertexLayout


// TODO: VkRenderPass is causing bad cache keys.
//      We need a better way to identify these structures.
struct Structure
{
    struct State
    {
        union PipelineState
        {
            struct 
            {
                U32                         numRenderTargets;
                RasterState                 raster;
                BlendState                  blendState;
                DepthStencil                depthStencil;
                VertexInputLayoutId         ia;
                TessellationState           tess;
                PrimitiveTopology           primitiveTopology;
                VkRenderPass                renderPass;
            } graphics;
            struct
            {
                U32 rayRecursionDepth;
                
            } raytrace;

            PipelineState() { graphics = { }; }
        } pipeline;
        std::array<VkDescriptorSetLayout, 8>  descriptorSetLayouts;
        ShaderProgramPermutation            shaderPermutation;
        ShaderProgramId                     shaderProgramId;
        U8                                  numSets;

        State() { }
    } state;

    void nullify();
    Hash64 hash() const;
};


struct PipelineState
{
    U32                 lastUsed      = 0;
    VkPipelineBindPoint bindPoint     = VK_PIPELINE_BIND_POINT_GRAPHICS;
    VkPipeline          pipeline      = VK_NULL_HANDLE;
    VkPipelineCache     pipelineCache = VK_NULL_HANDLE;
};

PipelineId              makePipelineId(const Structure& pipelineStructure);

// Creates a pipeline if one does not exist. Otherwise, returns an existing pipeline.
PipelineState           makePipeline(VulkanDevice* pDevice, const Structure& structure);
PipelineState           makePipeline(VulkanDevice* pDevice, const Structure& structure, PipelineId pipelineId);

// Clears the whole pipeline cache.
ResultCode              clearPipelineCache(VulkanDevice* pDevice);
ResultCode              releasePipeline(VulkanDevice* pDevice, PipelineId pipelineId);

// Creates a pipeline layout if one does not exist. Otherwise, returns an existing pipeline layout.
VkPipelineLayout        makeLayout(VulkanDevice* pDevice, const VkDescriptorSetLayout* layouts, U32 layoutCount);
void                    update(DeviceId id);
void                    clean(VulkanDevice* device);
} // PipelineManager
} // Vulkan
} // Recluse