//
#include "VulkanPipelineState.hpp"
#include "VulkanDevice.hpp"
#include "VulkanObjects.hpp"
#include "VulkanCommons.hpp"

#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"

#include "VulkanShaderCache.hpp"

namespace Recluse {


static VkPrimitiveTopology getNativeTopology(PrimitiveTopology topology)
{
    switch ( topology ) {
        case PRIMITIVE_TOPOLOGY_LINE_STRIP: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PRIMITIVE_TOPOLOGY_LINE_LIST: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PRIMITIVE_TOPOLOGY_POINT_LIST: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}


static VkVertexInputRate getNativeVertexInputRate(InputRate rate)
{
    switch (rate) {
        case INPUT_RATE_PER_INSTANCE: return VK_VERTEX_INPUT_RATE_INSTANCE;
        case INPUT_RATE_PER_VERTEX: 
        default: return VK_VERTEX_INPUT_RATE_VERTEX;
    }
}


static VkCullModeFlags getNativeCullMode(CullMode mode)
{
    switch (mode) {
        case CULL_MODE_BACK: return VK_CULL_MODE_BACK_BIT;
        case CULL_MODE_FRONT: return VK_CULL_MODE_FRONT_BIT;
        case CULL_MODE_FRONT_AND_BACK: 
        default: return VK_CULL_MODE_FRONT_AND_BACK;
    }
}

static VkCompareOp getNativeCompareOp(CompareOp op)
{
    switch (op) {
        case COMPARE_OP_ALWAYS: return VK_COMPARE_OP_ALWAYS;
        case COMPARE_OP_EQUAL: return VK_COMPARE_OP_EQUAL;
        case COMPARE_OP_GREATER: return VK_COMPARE_OP_GREATER;
        case COMPARE_OP_GREATER_OR_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case COMPARE_OP_LESS: return VK_COMPARE_OP_LESS;
        case COMPARE_OP_LESS_OR_EQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case COMPARE_OP_NOT_EQUAL: return VK_COMPARE_OP_NOT_EQUAL;
        case COMPARE_OP_NEVER: 
        default: return VK_COMPARE_OP_NEVER;
    }
}

static VkPolygonMode getNativePolygonMode(PolygonMode polygonMode)
{
    switch (polygonMode) {
        case POLYGON_MODE_LINE: return VK_POLYGON_MODE_LINE;
        case POLYGON_MODE_POINT: return VK_POLYGON_MODE_POINT;
        case POLYGON_MODE_FILL:
        default: return VK_POLYGON_MODE_FILL;
    }
}


static VkStencilOp getNativeStencilOp(StencilOp op) 
{
    switch (op) {
        case STENCIL_OP_DECREMENT_AND_CLAMP: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case STENCIL_OP_DECREMENT_AND_WRAP: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case STENCIL_OP_INCREMENT_AND_CLAMP: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case STENCIL_OP_INCREMENT_AND_WRAP: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case STENCIL_OP_INVERT: return VK_STENCIL_OP_INVERT;
        case STENCIL_OP_KEEP: return VK_STENCIL_OP_KEEP;
        case STENCIL_OP_REPLACE: return VK_STENCIL_OP_REPLACE;
        case STENCIL_OP_ZERO:
        default: return VK_STENCIL_OP_ZERO;
    }
}


static VkFrontFace getNativeFrontFace(FrontFace face)
{
    switch (face) {
        case FRONT_FACE_CLOCKWISE: return VK_FRONT_FACE_CLOCKWISE;
        case FRONT_FACE_COUNTER_CLOCKWISE:
        default: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}

static VkPipelineRasterizationStateCreateInfo getRasterInfo(const GraphicsPipelineStateDesc::RasterState& rs)
{
    VkPipelineRasterizationStateCreateInfo info = { };
    info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.cullMode                   = getNativeCullMode(rs.cullMode);
    info.depthBiasClamp             = rs.depthBiasClamp;
    info.depthBiasConstantFactor    = rs.depthBiasConstantFactor;
    info.depthBiasEnable            = rs.depthBiasEnable;
    info.depthBiasSlopeFactor       = rs.depthBiasSlopFactor;
    info.depthClampEnable           = rs.depthClampEnable;
    info.frontFace                  = getNativeFrontFace(rs.frontFace);
    info.lineWidth                  = rs.lineWidth;
    info.polygonMode                = getNativePolygonMode(rs.polygonMode);
    info.rasterizerDiscardEnable    = VK_FALSE;
    return info;
}


static VkPipelineInputAssemblyStateCreateInfo getAssemblyInfo(PrimitiveTopology topology)
{
    VkPipelineInputAssemblyStateCreateInfo info = { };
    info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.primitiveRestartEnable     = VK_FALSE;
    info.topology                   = getNativeTopology(topology);
    return info;
}


static VkPipelineVertexInputStateCreateInfo getVertexInputInfo(const GraphicsPipelineStateDesc::VertexInput& vi,
    std::vector<VkVertexInputAttributeDescription>& attributes, std::vector<VkVertexInputBindingDescription>& bindings)
{
    VkPipelineVertexInputStateCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    for (U32 i = 0; i < vi.numVertexBindings; ++i) {
    
        VertexBinding& vertexBind   = vi.pVertexBindings[i];
        U32 binding                 = vertexBind.binding;
        {
            U32 elementStride                                   = vertexBind.stride;
            VkVertexInputBindingDescription bindingDescription  = { };
            bindingDescription.binding                          = binding;
            bindingDescription.stride                           = elementStride;
            bindingDescription.inputRate                        = getNativeVertexInputRate(vertexBind.inputRate);
            bindings.push_back(bindingDescription);
        }

        for (U32 attribIdx = 0; attribIdx < vertexBind.numVertexAttributes; ++attribIdx) {

            VertexAttribute& attrib                     = vertexBind.pVertexAttributes[attribIdx];
            VkVertexInputAttributeDescription attribute = { };
            attribute.binding                           = binding;
            attribute.format                            = Vulkan::getVulkanFormat(attrib.format);
            attribute.location                          = attrib.loc;
            attribute.offset                            = attrib.offset;
            attributes.push_back(attribute);
            
        }
    }

    info.vertexAttributeDescriptionCount    = (U32)attributes.size();
    info.vertexBindingDescriptionCount      = (U32)bindings.size();
    info.pVertexAttributeDescriptions       = attributes.data();
    info.pVertexBindingDescriptions         = bindings.data();

    return info;
}


static VkPipelineDepthStencilStateCreateInfo getDepthStencilInfo(const GraphicsPipelineStateDesc::DepthStencil& ds)
{
    VkPipelineDepthStencilStateCreateInfo info  = { };
    info.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.depthBoundsTestEnable                  = (VkBool32)ds.depthBoundsTestEnable;
    info.depthCompareOp;
    info.depthTestEnable                        = (VkBool32)ds.depthTestEnable;
    info.depthWriteEnable                       = (VkBool32)ds.depthWriteEnable;
    info.front;
    info.maxDepthBounds                         = ds.maxDepthBounds;
    info.minDepthBounds                         = ds.minDepthBounds;
    info.stencilTestEnable                      = (VkBool32)ds.stencilTestEnable;
    info.back;
    return info;
}


static VkPipelineDynamicStateCreateInfo getDynamicStates()
{
    static VkDynamicState dynamicStates[]   = { VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VIEWPORT };
    VkPipelineDynamicStateCreateInfo info   = { };
    info.dynamicStateCount                  = 2;
    info.pDynamicStates                     = dynamicStates;
    info.sType                              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    return info;
}


static VkPipelineColorBlendStateCreateInfo getBlendInfo(const GraphicsPipelineStateDesc::BlendState& state,
    std::vector<VkPipelineColorBlendAttachmentState>& blendAttachments)
{
    VkPipelineColorBlendStateCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.logicOp;
    info.logicOpEnable;
    info.blendConstants;
    info.attachmentCount;
    info.pAttachments;
    return info;
}


static VkPipelineViewportStateCreateInfo getViewportInfo() {
    VkPipelineViewportStateCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    return info;
}


void VulkanPipelineState::destroy(VulkanDevice* pDevice)
{   
    if (m_pipelineLayout) {
    
        vkDestroyPipelineLayout(pDevice->get(), m_pipelineLayout, nullptr);
        m_pipelineLayout = nullptr;
    
    }
    
    if (m_pipeline) {

        vkDestroyPipeline(pDevice->get(), m_pipeline, nullptr);
        m_pipeline = nullptr;    

    }
}


ErrType VulkanGraphicsPipelineState::initialize(VulkanDevice* pDevice, const GraphicsPipelineStateDesc& desc)
{
    VkDevice device                 = pDevice->get();
    VkGraphicsPipelineCreateInfo ci = { };
    VkPipelineLayoutCreateInfo pli  = { };
    VulkanRenderPass* pVr           = static_cast<VulkanRenderPass*>(desc.pRenderPass);
    VkResult result                 = VK_SUCCESS;
    VkPipelineShaderStageCreateInfo shaderStages[16];

    std::vector<VkVertexInputAttributeDescription> attributes;
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;

    VkPipelineRasterizationStateCreateInfo rasterState          = getRasterInfo(desc.raster);
    VkPipelineDepthStencilStateCreateInfo depthStencilState     = getDepthStencilInfo(desc.ds);
    VkPipelineViewportStateCreateInfo viewportState             = getViewportInfo();
    VkPipelineColorBlendStateCreateInfo blendState              = getBlendInfo(desc.blend, blendAttachments);
    VkPipelineVertexInputStateCreateInfo vertInputState         = getVertexInputInfo(desc.vi, attributes, bindings);
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState   = getAssemblyInfo(desc.primitiveTopology);
    VkPipelineDynamicStateCreateInfo dynamicState               = getDynamicStates();
    
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(desc.numDescriptorSetLayouts);

    for (U32 i = 0; i < desc.numDescriptorSetLayouts; ++i) {

        VulkanDescriptorSetLayout* pSetLayout = static_cast<VulkanDescriptorSetLayout*>(desc.ppDescriptorLayouts[i]);
        descriptorSetLayouts[i] = pSetLayout->get();
    
    }

    // TODO: Maybe we can cache the pipeline layout?
    pli.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pli.setLayoutCount = desc.numDescriptorSetLayouts;
    pli.pSetLayouts = descriptorSetLayouts.data();

    result = vkCreatePipelineLayout(device, &pli, nullptr, &m_pipelineLayout);
    
    if (result != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to create pipeline state layout.");

        destroy(pDevice);

        return REC_RESULT_FAILED;
    
    }

    ci.sType                = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    ci.renderPass           = pVr->get();
    ci.pRasterizationState  = &rasterState;
    ci.pColorBlendState     = &blendState;
    ci.pDepthStencilState   = &depthStencilState;
    ci.pInputAssemblyState  = &inputAssemblyState;
    ci.pVertexInputState    = &vertInputState;
    ci.pViewportState       = &viewportState;
    ci.pDynamicState        = &dynamicState;
    ci.stageCount           = 0;
    
    if (desc.pVS) {
        shaderStages[ci.stageCount].module  = ShaderCache::getCachedShaderModule(pDevice, desc.pVS);
        shaderStages[ci.stageCount].stage   = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[ci.stageCount].pName   = "main";
        shaderStages[ci.stageCount].sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ci.stageCount += 1;
    }

    if (desc.pPS) {
        shaderStages[ci.stageCount].module  = ShaderCache::getCachedShaderModule(pDevice, desc.pPS);
        shaderStages[ci.stageCount].stage   = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[ci.stageCount].pName   = "main";
        shaderStages[ci.stageCount].sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ci.stageCount += 1;
    }

    ci.layout               = m_pipelineLayout;
    ci.pStages              = shaderStages;

    result = vkCreateGraphicsPipelines(device, nullptr, 1, &ci, nullptr, &m_pipeline);
   
    if (result != VK_SUCCESS) {
    
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan pipeline state.");
        
        destroy(pDevice);
        
        return REC_RESULT_FAILED;
    
    }

    return REC_RESULT_OK;
}


ErrType VulkanComputePipelineState::initialize(VulkanDevice* pDevice, const ComputePipelineStateDesc& desc)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}
} // Recluse