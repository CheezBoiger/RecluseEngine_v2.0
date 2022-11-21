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
    switch ( topology ) 
    {
        case PrimitiveTopology_LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology_LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology_PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology_TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology_TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}


static VkVertexInputRate getNativeVertexInputRate(InputRate rate)
{
    switch (rate) 
    {
        case InputRate_PerInstance: return VK_VERTEX_INPUT_RATE_INSTANCE;
        case InputRate_PerVertex: 
        default: return VK_VERTEX_INPUT_RATE_VERTEX;
    }
}


static VkCullModeFlags getNativeCullMode(CullMode mode)
{
    switch (mode) 
    {
        case CullMode_Back: return VK_CULL_MODE_BACK_BIT;
        case CullMode_Front: return VK_CULL_MODE_FRONT_BIT;
        case CullMode_FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
        case CullMode_None:
        default: return VK_CULL_MODE_NONE;
    }
}


static VkPolygonMode getNativePolygonMode(PolygonMode polygonMode)
{
    switch (polygonMode) 
    {
        case PolygonMode_Line: return VK_POLYGON_MODE_LINE;
        case PolygonMode_Point: return VK_POLYGON_MODE_POINT;
        case PolygonMode_Fill:
        default: return VK_POLYGON_MODE_FILL;
    }
}


static VkStencilOp getNativeStencilOp(StencilOp op) 
{
    switch (op) 
    {
        case StencilOp_DecrementAndClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOp_DecrementAndWrap: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case StencilOp_IncrementAndClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOp_IncrementAndWrap: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOp_Invert: return VK_STENCIL_OP_INVERT;
        case StencilOp_Keep: return VK_STENCIL_OP_KEEP;
        case StencilOp_Replace: return VK_STENCIL_OP_REPLACE;
        case StencilOp_Zero:
        default: return VK_STENCIL_OP_ZERO;
    }
}


static VkFrontFace getNativeFrontFace(FrontFace face)
{
    switch (face) 
    {
        case FrontFace_Clockwise: return VK_FRONT_FACE_CLOCKWISE;
        case FrontFace_CounterClockwise:
        default: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}


static VkLogicOp getLogicOp(LogicOp op)
{
    switch (op) 
    {
        case LogicOp_Clear: return VK_LOGIC_OP_CLEAR;
        case LogicOp_And: return VK_LOGIC_OP_AND;
        case LogicOp_AndReverse: return VK_LOGIC_OP_AND_REVERSE;
        case LogicOp_Copy: return VK_LOGIC_OP_COPY;
        case LogicOp_AndInverted: return VK_LOGIC_OP_AND_INVERTED;
        case LogicOp_NoOp: return VK_LOGIC_OP_NO_OP;
        case LogicOp_Xor: return VK_LOGIC_OP_XOR;
        case LogicOp_Or: return VK_LOGIC_OP_OR;
        case LogicOp_Nor: return VK_LOGIC_OP_NOR;
        case LogicOp_Equivalent: return VK_LOGIC_OP_EQUIVALENT;
        case LogicOp_Invert: return VK_LOGIC_OP_INVERT;
        case LogicOp_OrReverse: return VK_LOGIC_OP_OR_REVERSE;
        case LogicOp_CopyInverted: return VK_LOGIC_OP_COPY_INVERTED;
        case LogicOp_OrInverted: return VK_LOGIC_OP_OR_INVERTED;
        case LogicOp_Nand: return VK_LOGIC_OP_NAND;
        case LogicOp_Set: return VK_LOGIC_OP_SET;
        default: return VK_LOGIC_OP_NO_OP;
    }
}


static VkBlendFactor getBlendFactor(BlendFactor op)
{
    switch (op) 
    {
        case BlendFactor_Zero: return VK_BLEND_FACTOR_ZERO;
        case BlendFactor_One: return VK_BLEND_FACTOR_ONE;
        case BlendFactor_SourceColor: return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor_OneMinusSourceColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor_DestinationColor: return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor_OneMinusDestinationColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor_SourceAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor_OneMinusSourceAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor_DestinationAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor_OneMinusDestinationAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor_ConstantColor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendFactor_OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor_ConstantAlpha: return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendFactor_OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case BlendFactor_SourceAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BlendFactor_SourceOneColor: return VK_BLEND_FACTOR_SRC1_COLOR;
        case BlendFactor_OneMinusSourceOneColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BlendFactor_SourceOneAlpha: return VK_BLEND_FACTOR_SRC1_ALPHA;
        case BlendFactor_OneMinusSourceOneAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        default: return VK_BLEND_FACTOR_ZERO;
    }
}

static VkBlendOp getBlendOp(BlendOp op)
{
    switch (op) 
    {
        case BlendOp_Add: return VK_BLEND_OP_ADD;
        case BlendOp_Subtract: return VK_BLEND_OP_SUBTRACT;
        case BlendOp_ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp_Min: return VK_BLEND_OP_MIN;
        case BlendOp_Max: return VK_BLEND_OP_MAX;
        default: return VK_BLEND_OP_ADD;
    }
}


static VkColorComponentFlags getColorComponents(ColorComponentMaskFlags flags)
{
    VkColorComponentFlags components = 0;
    if (flags & Color_R) components |= VK_COLOR_COMPONENT_R_BIT;
    if (flags & Color_G) components |= VK_COLOR_COMPONENT_G_BIT;
    if (flags & Color_B) components |= VK_COLOR_COMPONENT_B_BIT;
    if (flags & Color_A) components |= VK_COLOR_COMPONENT_A_BIT;
    return components;
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


static VkPipelineVertexInputStateCreateInfo getVertexInputInfo
    (
        const GraphicsPipelineStateDesc::VertexInput& vi,
        std::vector<VkVertexInputAttributeDescription>& attributes, 
        std::vector<VkVertexInputBindingDescription>& bindings
    )
{
    VkPipelineVertexInputStateCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    for (U32 i = 0; i < vi.numVertexBindings; ++i) 
    {
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

        for (U32 attribIdx = 0; attribIdx < vertexBind.numVertexAttributes; ++attribIdx) 
        {
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
    info.depthCompareOp                         = Vulkan::getNativeCompareOp(ds.depthCompareOp);
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
    info.logicOp            = getLogicOp(state.logicOp);
    info.logicOpEnable      = state.logicOpEnable;
    info.blendConstants[0]  = state.blendConstants[0];
    info.blendConstants[1]  = state.blendConstants[1];
    info.blendConstants[2]  = state.blendConstants[2];
    info.blendConstants[3]  = state.blendConstants[3];
    info.attachmentCount    = state.numAttachments;

    blendAttachments.resize(state.numAttachments);

    for (U32 i = 0; i < state.numAttachments; ++i) 
    {
        RenderTargetBlendState& blendState      = state.attachments[i];

        blendAttachments[i].blendEnable         = blendState.blendEnable;
        blendAttachments[i].alphaBlendOp        = getBlendOp(blendState.alphaBlendOp);
        blendAttachments[i].dstAlphaBlendFactor = getBlendFactor(blendState.dstAlphaBlendFactor);
        blendAttachments[i].srcAlphaBlendFactor = getBlendFactor(blendState.srcAlphaBlendFactor);
        blendAttachments[i].dstColorBlendFactor = getBlendFactor(blendState.dstColorBlendFactor);
        blendAttachments[i].srcColorBlendFactor = getBlendFactor(blendState.srcColorBlendFactor);
        blendAttachments[i].colorWriteMask      = getColorComponents(blendState.colorWriteMask);
        blendAttachments[i].colorBlendOp        = getBlendOp(blendState.colorBlendOp);

    }

    info.pAttachments = blendAttachments.data();

    return info;
}


static VkPipelineTessellationStateCreateInfo getTessellationStateInfo(const GraphicsPipelineStateDesc::TessellationState& tess)
{
    VkPipelineTessellationStateCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    info.patchControlPoints = tess.numControlPoints;
    return info;
}


static VkPipelineViewportStateCreateInfo getViewportInfo() {
    // We are making our pipelines dynamic for viewport and scissors,
    // so we don't need to bother statically setting this state.
    VkPipelineViewportStateCreateInfo info  = { };
    info.sType                              = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    info.scissorCount                       = 1;
    info.viewportCount                      = 1;
    info.pViewports                         = nullptr;
    info.pScissors                          = nullptr;
    return info;
}


static VkPipelineMultisampleStateCreateInfo getMultisampleStateInfo()
{
    VkPipelineMultisampleStateCreateInfo info   = { };

    info.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pSampleMask                            = nullptr;
    info.rasterizationSamples                   = VK_SAMPLE_COUNT_1_BIT;
    info.sampleShadingEnable                    = VK_FALSE;
    info.alphaToCoverageEnable                  = VK_FALSE;
    info.alphaToOneEnable                       = VK_FALSE;
    info.minSampleShading                       = 0.f;

    return info;
}


VkResult VulkanPipelineState::createLayout(VulkanDevice* pDevice, const PipelineStateDesc& desc)
{
    VkDevice device                 = pDevice->get();
    VkPipelineLayoutCreateInfo pli  = { };
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(desc.numDescriptorSetLayouts);

    for (U32 i = 0; i < desc.numDescriptorSetLayouts; ++i) 
    {
        VulkanDescriptorSetLayout* pSetLayout = static_cast<VulkanDescriptorSetLayout*>(desc.ppDescriptorLayouts[i]);
        descriptorSetLayouts[i] = pSetLayout->get();
    }

    // TODO: Maybe we can cache the pipeline layout?
    pli.sType           = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pli.setLayoutCount  = desc.numDescriptorSetLayouts;
    pli.pSetLayouts     = descriptorSetLayouts.data();

    return vkCreatePipelineLayout(device, &pli, nullptr, &m_pipelineLayout);
}


void VulkanPipelineState::destroy(VulkanDevice* pDevice)
{   
    R_DEBUG(R_CHANNEL_VULKAN, "Destroying vulkan pipeline state...");

    if (m_pipelineLayout) 
    {
        vkDestroyPipelineLayout(pDevice->get(), m_pipelineLayout, nullptr);
        m_pipelineLayout = nullptr;
    }
    
    if (m_pipeline) 
    {
        vkDestroyPipeline(pDevice->get(), m_pipeline, nullptr);
        m_pipeline = nullptr;    
    }
}


ErrType VulkanGraphicsPipelineState::initialize(VulkanDevice* pDevice, const GraphicsPipelineStateDesc& desc)
{
    VkDevice device                 = pDevice->get();
    VkGraphicsPipelineCreateInfo ci = { };
    VulkanRenderPass* pVr           = static_cast<VulkanRenderPass*>(desc.pRenderPass);
    VkResult result                 = VK_SUCCESS;
    ShaderCache* pShaderCache       = pDevice->getShaderCache();
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
    VkPipelineTessellationStateCreateInfo tessState             = getTessellationStateInfo(desc.tess);
    VkPipelineMultisampleStateCreateInfo multisampleState       = getMultisampleStateInfo();

    result = createLayout(pDevice, desc);
    
    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create pipeline state layout.");

        destroy(pDevice);

        return RecluseResult_Failed;
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
    ci.pMultisampleState    = &multisampleState;
    ci.stageCount           = 0;
    
    if (desc.pVS) 
    {
        shaderStages[ci.stageCount]         = { };
        shaderStages[ci.stageCount].module  = pShaderCache->getCachedShaderModule(pDevice, desc.pVS);
        shaderStages[ci.stageCount].stage   = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[ci.stageCount].pName   = "main";
        shaderStages[ci.stageCount].sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ci.stageCount += 1;
    }

    if (desc.pPS) 
    {
        shaderStages[ci.stageCount]         = { };
        shaderStages[ci.stageCount].module  = pShaderCache->getCachedShaderModule(pDevice, desc.pPS);
        shaderStages[ci.stageCount].stage   = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[ci.stageCount].pName   = "main";
        shaderStages[ci.stageCount].sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ci.stageCount += 1;
    }

    ci.layout               = m_pipelineLayout;
    ci.pStages              = shaderStages;

    result = vkCreateGraphicsPipelines(device, nullptr, 1, &ci, nullptr, &m_pipeline);
   
    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan pipeline state.");
        
        destroy(pDevice);
        
        return RecluseResult_Failed;
    }

    return RecluseResult_Ok;
}


ErrType VulkanComputePipelineState::initialize(VulkanDevice* pDevice, const ComputePipelineStateDesc& desc)
{
    VkComputePipelineCreateInfo createInfo  = { };
    VkDevice device                         = pDevice->get();
    VkResult result                         = VK_SUCCESS;
    ShaderCache* pShaderCache               = pDevice->getShaderCache();
    
    result = createLayout(pDevice, desc);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create pipeline layout for Compute pipelinestate...");
        
        destroy(pDevice);
        
        return RecluseResult_Failed;
    }

    createInfo.sType        = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.layout       = m_pipelineLayout;
    
    createInfo.stage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage.module = pShaderCache->getCachedShaderModule(pDevice, desc.pCS);
    createInfo.stage.pName  = "main";
    createInfo.stage.stage  = VK_SHADER_STAGE_COMPUTE_BIT;

    result = vkCreateComputePipelines(device, nullptr, 1, &createInfo, nullptr, &m_pipeline);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create compute pipeline state!");

        destroy(pDevice);

        return RecluseResult_Failed;
    }

    return RecluseResult_Ok;
}
} // Recluse