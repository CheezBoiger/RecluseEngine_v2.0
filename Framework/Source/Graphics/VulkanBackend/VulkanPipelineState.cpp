//
#include "Recluse/Serialization/Hasher.hpp"
#include "VulkanPipelineState.hpp"
#include "VulkanDevice.hpp"
#include "VulkanObjects.hpp"
#include "VulkanCommons.hpp"

#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"

#include "VulkanShaderCache.hpp"

namespace Recluse {
namespace Pipelines {

std::unordered_map<PipelineId, PipelineState> g_pipelineMap;
std::unordered_map<VkDescriptorSetLayout, VkPipelineLayout> g_pipelineLayoutMap;

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


static VkPipelineRasterizationStateCreateInfo getRasterInfo(const RasterState& rs)
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


static VkPipelineDepthStencilStateCreateInfo getDepthStencilInfo(const DepthStencil& ds)
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


static VkPipelineColorBlendStateCreateInfo getBlendInfo(const BlendState& state,
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
        const RenderTargetBlendState& blendState      = state.attachments[i];

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


static VkPipelineTessellationStateCreateInfo getTessellationStateInfo(const TessellationState& tess)
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


VkPipelineLayout createPipelineLayout(VulkanDevice* pDevice, const VkDescriptorSetLayout* layouts, U32 layoutCount)
{
    VkDevice device                 = pDevice->get();
    VkPipelineLayoutCreateInfo pli  = { };
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    // TODO: Maybe we can cache the pipeline layout?
    pli.sType           = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pli.setLayoutCount  = layoutCount;
    pli.pSetLayouts     = layouts;

    VkResult result = vkCreatePipelineLayout(device, &pli, nullptr, &pipelineLayout);

    return pipelineLayout;
}


void destroyPipeline(VulkanDevice* pDevice, VkPipeline pipeline)
{   
    R_DEBUG(R_CHANNEL_VULKAN, "Destroying vulkan pipeline state...");
    
    if (pipeline) 
    {
        vkDestroyPipeline(pDevice->get(), pipeline, nullptr);
    }
}

void destroyPipelineLayout(VulkanDevice* pDevice, VkPipelineLayout layout)
{
    R_DEBUG(R_CHANNEL_VULKAN, "Destroying vulkan pipeline layout...");
    if (layout)
    {
        vkDestroyPipelineLayout(pDevice->get(), layout, nullptr);
    }
}

namespace VertexLayout {

std::unordered_map<VertexInputLayoutId, VulkanVertexLayout> g_vertexLayoutMap;

static VulkanVertexLayout createVertexInput(const VertexInputLayout& vi)
{
    R_ASSERT(vi.numVertexBindings < VertexInputLayout::VertexInputLayout_Count);
    VulkanVertexLayout layout = { };
    
    for (U32 i = 0; i < vi.numVertexBindings; ++i) 
    {
        const VertexBinding& vertexBind     = vi.vertexBindings[i];
        U32 binding                         = vertexBind.binding;
        {
            U32 elementStride                                   = vertexBind.stride;
            VkVertexInputBindingDescription bindingDescription  = { };
            bindingDescription.binding                          = binding;
            bindingDescription.stride                           = elementStride;
            bindingDescription.inputRate                        = getNativeVertexInputRate(vertexBind.inputRate);
            layout.bindings.push_back(bindingDescription);
        }

        for (U32 attribIdx = 0; attribIdx < vertexBind.numVertexAttributes; ++attribIdx) 
        {
            VertexAttribute& attrib                     = vertexBind.pVertexAttributes[attribIdx];
            VkVertexInputAttributeDescription attribute = { };
            attribute.binding                           = binding;
            attribute.format                            = Vulkan::getVulkanFormat(attrib.format);
            attribute.location                          = attrib.loc;
            attribute.offset                            = attrib.offset;
            layout.descriptions.push_back(attribute);   
        }
    }

    return layout;
}

Bool make(VertexInputLayoutId id, const VertexInputLayout& vl)
{
    auto iter = g_vertexLayoutMap.find(id);
    if (iter != g_vertexLayoutMap.end())
        return true;
    VulkanVertexLayout layout = createVertexInput(vl);
    g_vertexLayoutMap[id] = layout;
    return true;
}


const VulkanVertexLayout* obtain(VertexInputLayoutId inputLayoutId)
{
    auto iter = g_vertexLayoutMap.find(inputLayoutId);
    if (iter != g_vertexLayoutMap.end())
        return &iter->second;
    return nullptr;
}


Bool unloadAll()
{
    g_vertexLayoutMap.clear();
    return true;
}
} // VertexLayout

VkPipelineLayout makeLayout(VulkanDevice* pDevice, VkDescriptorSetLayout descriptorLayout)
{
    VkPipelineLayout layout = VK_NULL_HANDLE;
    auto iter = g_pipelineLayoutMap.find(descriptorLayout);
    if (iter == g_pipelineLayoutMap.end())
    {
        if (layout = createPipelineLayout(pDevice, &descriptorLayout, 1))
        {
            g_pipelineLayoutMap.insert(std::make_pair(descriptorLayout, layout));
        }
    }
    else
    {
        layout = iter->second;
    }
    return layout;
}

VkPipeline createGraphicsPipeline(VulkanDevice* pDevice, const Structure& structure, const ShaderPrograms::VulkanShaderProgram* program)
{
    VkPipeline pipeline                             = VK_NULL_HANDLE;
    VkDevice device                                 = pDevice->get();
    VkGraphicsPipelineCreateInfo ci                 = { };
    const VkRenderPass renderPass                   = structure.state.graphics.renderPass;
    VkResult result                                 = VK_SUCCESS;
    VkPipelineShaderStageCreateInfo shaderStages[16];

    if (!program)
    {
        R_FATAL_ERR(R_CHANNEL_VULKAN, "Failed to obtain native vulkan program. ShaderProgramId=%llu, Permutation=%llu", structure.state.shaderProgramId, structure.state.shaderPermutation);
        return VK_NULL_HANDLE;
    }

    std::vector<VkVertexInputAttributeDescription> attributes;
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;

    VkPipelineRasterizationStateCreateInfo rasterState          = getRasterInfo(structure.state.graphics.raster);
    VkPipelineDepthStencilStateCreateInfo depthStencilState     = getDepthStencilInfo(structure.state.graphics.depthStencil);
    VkPipelineViewportStateCreateInfo viewportState             = getViewportInfo();
    VkPipelineColorBlendStateCreateInfo blendState              = getBlendInfo(structure.state.graphics.blendState, blendAttachments);
    VkPipelineVertexInputStateCreateInfo vertInputState         = { };
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState   = getAssemblyInfo(structure.state.graphics.primitiveTopology);
    VkPipelineDynamicStateCreateInfo dynamicState               = getDynamicStates();
    VkPipelineTessellationStateCreateInfo tessState             = getTessellationStateInfo(structure.state.graphics.tess);
    VkPipelineMultisampleStateCreateInfo multisampleState       = getMultisampleStateInfo();

    const VertexLayout::VulkanVertexLayout* pLayout = VertexLayout::obtain(structure.state.graphics.ia);

    if (pLayout)
    {
        vertInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertInputState.vertexBindingDescriptionCount = pLayout->bindings.size();
        vertInputState.vertexAttributeDescriptionCount = pLayout->descriptions.size();
        vertInputState.pVertexAttributeDescriptions = pLayout->descriptions.data();
        vertInputState.pVertexBindingDescriptions = pLayout->bindings.data();
    }

    VkPipelineLayout pipelineLayout = makeLayout(pDevice, structure.state.descriptorLayout);
    
    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create pipeline state layout.");

        destroyPipelineLayout(pDevice, pipelineLayout);

        return VK_NULL_HANDLE;
    }

    ci.sType                = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    ci.renderPass           = renderPass;
    ci.pRasterizationState  = &rasterState;
    ci.pColorBlendState     = &blendState;
    ci.pDepthStencilState   = &depthStencilState;
    ci.pInputAssemblyState  = &inputAssemblyState;
    ci.pVertexInputState    = &vertInputState;
    ci.pViewportState       = &viewportState;
    ci.pDynamicState        = &dynamicState;
    ci.pMultisampleState    = &multisampleState;
    ci.stageCount           = 0;
    
    if (program->graphics.vs) 
    {
        shaderStages[ci.stageCount]         = { };
        shaderStages[ci.stageCount].module  = program->graphics.vs;
        shaderStages[ci.stageCount].stage   = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[ci.stageCount].pName   = program->graphics.vsEntry;
        shaderStages[ci.stageCount].sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ci.stageCount += 1;
    }

    if (program->graphics.ps) 
    {
        shaderStages[ci.stageCount]         = { };
        shaderStages[ci.stageCount].module  = program->graphics.ps;
        shaderStages[ci.stageCount].stage   = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[ci.stageCount].pName   = program->graphics.psEntry;
        shaderStages[ci.stageCount].sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ci.stageCount += 1;
    }

    if (program->graphics.ds)
    {
        shaderStages[ci.stageCount] = { };
        shaderStages[ci.stageCount].module = program->graphics.ds;
        shaderStages[ci.stageCount].stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        shaderStages[ci.stageCount].pName = program->graphics.dsEntry;
        shaderStages[ci.stageCount].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ci.stageCount += 1;
    }

    if (program->graphics.hs)
    {
        shaderStages[ci.stageCount] = { };
        shaderStages[ci.stageCount].module = program->graphics.hs;
        shaderStages[ci.stageCount].stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        shaderStages[ci.stageCount].pName = program->graphics.hsEntry;
        shaderStages[ci.stageCount].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        ci.stageCount += 1;
    }

    ci.layout               = pipelineLayout;
    ci.pStages              = shaderStages;

    result = vkCreateGraphicsPipelines(device, nullptr, 1, &ci, nullptr, &pipeline);
   
    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create vulkan pipeline state.");
        
        destroyPipeline(pDevice, pipeline);
        
        return VK_NULL_HANDLE;
    }

    return pipeline;
}


VkPipeline createComputePipeline(VulkanDevice* pDevice, const Structure& structure, const ShaderPrograms::VulkanShaderProgram* program)
{
    VkComputePipelineCreateInfo createInfo  = { };
    VkDevice device                         = pDevice->get();
    VkResult result                         = VK_SUCCESS;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout         = makeLayout(pDevice, structure.state.descriptorLayout);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create pipeline layout for Compute pipelinestate...");
        
        destroyPipelineLayout(pDevice, pipelineLayout);
        
        return VK_NULL_HANDLE;
    }

    createInfo.sType        = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.layout       = pipelineLayout;
    
    createInfo.stage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage.module = program->compute.cs;
    createInfo.stage.pName  = program->compute.csEntry;
    createInfo.stage.stage  = VK_SHADER_STAGE_COMPUTE_BIT;

    result = vkCreateComputePipelines(device, nullptr, 1, &createInfo, nullptr, &pipeline);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create compute pipeline state!");

        destroyPipeline(pDevice, pipeline);

        return nullptr;
    }

    return pipeline;
}


PipelineId makePipelineId(const Structure& structure)
{
    return recluseHash((void*)&structure, sizeof(Structure));
}


PipelineState makePipeline(VulkanDevice* pDevice, const Structure& structure)
{
    PipelineId id = makePipelineId(structure);
    return makePipeline(pDevice, structure, makePipelineId(structure));
}


PipelineState makePipeline(VulkanDevice* pDevice, const Structure& structure, PipelineId id)
{
    PipelineState pipeline = { };
    auto iter = g_pipelineMap.find(id);
    if (iter == g_pipelineMap.end())
    {
        ShaderPrograms::VulkanShaderProgram* program = ShaderPrograms::obtainShaderProgram(structure.state.shaderProgramId, structure.state.shaderPermutation);
        pipeline.bindPoint = program->bindPoint;
        switch (program->bindPoint)
        {
        case VK_PIPELINE_BIND_POINT_GRAPHICS:
            pipeline.pipeline = createGraphicsPipeline(pDevice, structure, program);
            break;
        case VK_PIPELINE_BIND_POINT_COMPUTE:
            pipeline.pipeline = createComputePipeline(pDevice, structure, program);
            break;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
            R_ERR(R_CHANNEL_VULKAN, "Ray tracing pipelines are not supported currently!");
            break;
        }

        if (pipeline.pipeline)
        {
            g_pipelineMap.insert(std::make_pair(id, pipeline));
        }
    }
    else
    {
        pipeline = iter->second;
    }
    return pipeline;
}


ErrType clearPipelineCache(VulkanDevice* pDevice)
{
    for (auto pipelineLayoutIt : g_pipelineLayoutMap)
    {
        destroyPipelineLayout(pDevice, pipelineLayoutIt.second);
    }

    for (auto pipelineIt : g_pipelineMap)
    {
        destroyPipeline(pDevice, pipelineIt.second.pipeline);
    }

    g_pipelineLayoutMap.clear();
    g_pipelineMap.clear();

    return RecluseResult_Ok;
}
} // Pipelines
} // Recluse