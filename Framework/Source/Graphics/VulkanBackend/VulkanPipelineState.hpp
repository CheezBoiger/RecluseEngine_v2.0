//
#pragma once

#include "VulkanCommons.hpp"
#include "Recluse/Graphics/PipelineState.hpp"
#include "VulkanObjects.hpp"

#include "Graphics/ShaderProgram.hpp"

namespace Recluse {

class VulkanDevice;

typedef Hash64 PipelineId;

namespace Pipelines {
namespace VertexLayout {

struct VulkanVertexLayout
{
    std::vector<VkVertexInputAttributeDescription> descriptions;
    std::vector<VkVertexInputBindingDescription> bindings;
};


ResultCode make(VertexInputLayoutId id, const VertexInputLayout& layout);
ResultCode unloadLayout(VertexInputLayoutId id);
Bool unloadAll();
} // VertexLayout

struct Structure
{
    struct 
    {
        union 
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
                U32 unused0;
            } raytrace;
        };
        VkDescriptorSetLayout       descriptorLayout;
        ShaderProgramPermutation    shaderPermutation;
        ShaderProgramId             shaderProgramId;
    } state;

    void nullify();
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
VkPipelineLayout        makeLayout(VulkanDevice* pDevice, VkDescriptorSetLayout layout);
} // PipelineManager
} // Recluse