//
#include "Recluse/Serialization/Hasher.hpp"
#include "VulkanPipelineState.hpp"
#include "VulkanDevice.hpp"
#include "VulkanObjects.hpp"
#include "VulkanCommons.hpp"
#include "VulkanAdapter.hpp"

#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Filesystem/Filesystem.hpp"

#include "Graphics/LifetimeCache.hpp"

#include "VulkanShaderCache.hpp"

namespace Recluse {
namespace Pipelines {

LifetimeCache<PipelineId, PipelineState>                                            g_pipelineMap;
std::unordered_map<VkDescriptorSetLayout, SharedReferenceObject<VkPipelineLayout>>  g_pipelineLayoutMap;
R_DECLARE_GLOBAL_BOOLEAN(g_allowPipelineCaching, false, "Vulkan.EnablePipelineCache");
R_DECLARE_GLOBAL_STRING(g_pipelineCacheDir, "VulkanCache", "Vulkan.PipelineCacheDir");
R_DECLARE_GLOBAL_U32(g_vulkanPipelineMaxAge, 256, "Vulkan.PipelineMaxAge");


namespace PipelineCache {

U32 pipelineCacheHeaderVersion = 0;
} // PipelineCache

static VkPrimitiveTopology getNativeTopology(PrimitiveTopology topology)
{
    switch ( topology ) 
    {
        case PrimitiveTopology_LineStrip:       return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology_LineList:        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology_PointList:       return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology_TriangleStrip:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology_TriangleList:    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default:                                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}


static VkVertexInputRate getNativeVertexInputRate(InputRate rate)
{
    switch (rate) 
    {
        case InputRate_PerInstance: return VK_VERTEX_INPUT_RATE_INSTANCE;
        case InputRate_PerVertex: 
        default:                    return VK_VERTEX_INPUT_RATE_VERTEX;
    }
}


static VkCullModeFlags getNativeCullMode(CullMode mode)
{
    switch (mode) 
    {
        case CullMode_Back:         return VK_CULL_MODE_BACK_BIT;
        case CullMode_Front:        return VK_CULL_MODE_FRONT_BIT;
        case CullMode_FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
        case CullMode_None:
        default:                    return VK_CULL_MODE_NONE;
    }
}


static VkPolygonMode getNativePolygonMode(PolygonMode polygonMode)
{
    switch (polygonMode) 
    {
        case PolygonMode_Line:      return VK_POLYGON_MODE_LINE;
        case PolygonMode_Point:     return VK_POLYGON_MODE_POINT;
        case PolygonMode_Fill:
        default:                    return VK_POLYGON_MODE_FILL;
    }
}


static VkStencilOp getNativeStencilOp(StencilOp op) 
{
    switch (op) 
    {
        case StencilOp_DecrementAndClamp:   return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOp_DecrementAndWrap:    return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case StencilOp_IncrementAndClamp:   return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOp_IncrementAndWrap:    return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOp_Invert:              return VK_STENCIL_OP_INVERT;
        case StencilOp_Keep:                return VK_STENCIL_OP_KEEP;
        case StencilOp_Replace:             return VK_STENCIL_OP_REPLACE;
        case StencilOp_Zero:
        default:                            return VK_STENCIL_OP_ZERO;
    }
}


static VkFrontFace getNativeFrontFace(FrontFace face)
{
    switch (face) 
    {
        case FrontFace_Clockwise:           return VK_FRONT_FACE_CLOCKWISE;
        case FrontFace_CounterClockwise:
        default:                            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}


static VkLogicOp getLogicOp(LogicOp op)
{
    switch (op) 
    {
        case LogicOp_Clear:         return VK_LOGIC_OP_CLEAR;
        case LogicOp_And:           return VK_LOGIC_OP_AND;
        case LogicOp_AndReverse:    return VK_LOGIC_OP_AND_REVERSE;
        case LogicOp_Copy:          return VK_LOGIC_OP_COPY;
        case LogicOp_AndInverted:   return VK_LOGIC_OP_AND_INVERTED;
        case LogicOp_NoOp:          return VK_LOGIC_OP_NO_OP;
        case LogicOp_Xor:           return VK_LOGIC_OP_XOR;
        case LogicOp_Or:            return VK_LOGIC_OP_OR;
        case LogicOp_Nor:           return VK_LOGIC_OP_NOR;
        case LogicOp_Equivalent:    return VK_LOGIC_OP_EQUIVALENT;
        case LogicOp_Invert:        return VK_LOGIC_OP_INVERT;
        case LogicOp_OrReverse:     return VK_LOGIC_OP_OR_REVERSE;
        case LogicOp_CopyInverted:  return VK_LOGIC_OP_COPY_INVERTED;
        case LogicOp_OrInverted:    return VK_LOGIC_OP_OR_INVERTED;
        case LogicOp_Nand:          return VK_LOGIC_OP_NAND;
        case LogicOp_Set:           return VK_LOGIC_OP_SET;
        default:                    return VK_LOGIC_OP_NO_OP;
    }
}


static VkBlendFactor getBlendFactor(BlendFactor op)
{
    switch (op) 
    {
        case BlendFactor_Zero:                      return VK_BLEND_FACTOR_ZERO;
        case BlendFactor_One:                       return VK_BLEND_FACTOR_ONE;
        case BlendFactor_SourceColor:               return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor_OneMinusSourceColor:       return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor_DestinationColor:          return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor_OneMinusDestinationColor:  return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor_SourceAlpha:               return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor_OneMinusSourceAlpha:       return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor_DestinationAlpha:          return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor_OneMinusDestinationAlpha:  return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor_ConstantColor:             return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendFactor_OneMinusConstantColor:     return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor_ConstantAlpha:             return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendFactor_OneMinusConstantAlpha:     return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case BlendFactor_SourceAlphaSaturate:       return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BlendFactor_SourceOneColor:            return VK_BLEND_FACTOR_SRC1_COLOR;
        case BlendFactor_OneMinusSourceOneColor:    return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BlendFactor_SourceOneAlpha:            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case BlendFactor_OneMinusSourceOneAlpha:    return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        default:                                    return VK_BLEND_FACTOR_ZERO;
    }
}

static VkBlendOp getBlendOp(BlendOp op)
{
    switch (op) 
    {
        case BlendOp_Add:               return VK_BLEND_OP_ADD;
        case BlendOp_Subtract:          return VK_BLEND_OP_SUBTRACT;
        case BlendOp_ReverseSubtract:   return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp_Min:               return VK_BLEND_OP_MIN;
        case BlendOp_Max:               return VK_BLEND_OP_MAX;
        default:                        return VK_BLEND_OP_ADD;
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


struct PipelineCacheHeader
{
    // Device id.
    U32 deviceId;
    // Vendor Id.
    U32 vendorId;
    // Header Length in bytes.
    U32 headerLength;
    // Header version.
    U32 headerVersion;
    U8  pipelineCacheUUID[VK_UUID_SIZE];
};

// TODO(Garcia): We might want to store all pipeline cache data into one file,
//               and then read from there instead. Otherwise we are going to have a 
//               crap ton of separate tiny files...
R_INTERNAL
VkPipelineCache createEmptyCache(VkDevice device)
{
        VkResult result = VK_SUCCESS;
        VkPipelineCache pipelineCache = VK_NULL_HANDLE;
 
        if (g_allowPipelineCaching)
        {
            VkPipelineCacheCreateInfo pipelineCacheCreate = { };
            pipelineCacheCreate.flags = 0;
            pipelineCacheCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            pipelineCacheCreate.initialDataSize = 0;
            pipelineCacheCreate.pInitialData = nullptr;
            result = vkCreatePipelineCache(device, &pipelineCacheCreate, nullptr, &pipelineCache);

            if (result != VK_SUCCESS)
            {
                return VK_NULL_HANDLE;
            }
        }
        return pipelineCache;
}


R_INTERNAL
std::string makeCacheDirectory(const std::string& filename)
{
    std::string dirPath = Filesystem::getCurrentDir() + "/" + g_pipelineCacheDir;
    std::string filePath = dirPath + "/" + filename;
    Filesystem::createDirectory(dirPath);
    return filePath;
}


R_INTERNAL
Bool verifyFileHeaderIntegrity(VulkanDevice* pDevice, const PipelineCacheHeader& header)
{
    const VkPhysicalDeviceProperties& properties = pDevice->getAdapter()->getProperties();
    // Verify if the cache header version is the same.
    if (header.headerVersion != PipelineCache::pipelineCacheHeaderVersion)
        return false;
    if (header.deviceId != properties.deviceID)
        return false;
    if (header.vendorId != properties.vendorID);
        return false;

    for (U32 i = 0; i < VK_UUID_SIZE; ++i)
    {
        if (header.pipelineCacheUUID[i] != properties.pipelineCacheUUID[i])
            return false;
    }    

    return true;
}


R_INTERNAL
Bool findPipelineCache(VulkanDevice* pDevice, PipelineId pipelineId, VkPipelineCache& pipelineCache)
{
    size_t sizeBytes                = 0;
    Bool found                      = false;
    VkDevice device                 = pDevice->get();

    if (g_allowPipelineCaching)
    {
        std::string filename            = makeCacheDirectory("Pipeline-" + std::to_string(pipelineId) + ".txt");
        // TODO: We need to find our file and query for the binary information.
        FileBufferData fileData = { };
        ResultCode r = File::readFrom(&fileData, filename);
        found = (r == RecluseResult_Ok);
    
        if (found)
        {
            VkResult result = VK_SUCCESS;
            PipelineCacheHeader header = { };
            VkPipelineCacheCreateInfo info = { };
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            info.flags = 0;

            memcpy(&header, fileData.data(), sizeof(PipelineCacheHeader));

            if (verifyFileHeaderIntegrity(pDevice, header))
            {
                size_t binarySizeBytes = fileData.size() - sizeof(PipelineCacheHeader);
                void* binaryBasePtr = fileData.data() + sizeof(PipelineCacheHeader);

                info.initialDataSize = binarySizeBytes;
                info.pInitialData = binaryBasePtr;
                result = vkCreatePipelineCache(device, &info, nullptr, &pipelineCache);

                if (result != VK_SUCCESS)
                {
                    R_WARN(R_CHANNEL_VULKAN, "Failed to create found pipeline cache!!");
                    pipelineCache = VK_NULL_HANDLE;
                }
            }
            else
            {
                pipelineCache = VK_NULL_HANDLE;
            }
        }
        else
        {
            pipelineCache = createEmptyCache(device);
        }
    }
    return found;
}


R_INTERNAL
void cachePipeline(VulkanDevice* pDevice, PipelineId pipelineId, VkPipelineCache pipelineCache)
{
    if (g_allowPipelineCaching)
    {
        VkDevice device = pDevice->get();
        if (pipelineCache)
        {
            VkResult result = VK_SUCCESS;
            size_t sizeBytes = 0;
            void* buffer = nullptr;
            const VkPhysicalDeviceProperties& properties = pDevice->getAdapter()->getProperties();
 
            R_VERBOSE(R_CHANNEL_VULKAN, "Caching pipeline");
        
            result = vkGetPipelineCacheData(device, pipelineCache, &sizeBytes, buffer); 
            buffer = malloc(sizeBytes);
            result = vkGetPipelineCacheData(device, pipelineCache, &sizeBytes, buffer);

            // TODO: We need to be able to store this binary information on disk.
            PipelineCacheHeader header  = { };
            header.headerLength         = sizeof(PipelineCacheHeader);
            header.deviceId             = properties.deviceID;
            header.vendorId             = properties.vendorID;
            header.headerVersion        = PipelineCache::pipelineCacheHeaderVersion;

            for (U32 i = 0; i < VK_UUID_SIZE; ++i)
                header.pipelineCacheUUID[i] = properties.pipelineCacheUUID[i];

            std::string filename = makeCacheDirectory("Pipeline-" + std::to_string(pipelineId) + ".txt");
            File fileToDisk;
            fileToDisk.open(filename, "w");
            if (fileToDisk.isOpen())
            {
                fileToDisk.write(&header, sizeof(PipelineCacheHeader));
                fileToDisk.write(buffer, sizeBytes);
                fileToDisk.close();
            }
        
            free(buffer);
        }
    }
}


static VkPipelineRasterizationStateCreateInfo getRasterInfo(VulkanDevice* device, const RasterState& rs)
{
    const VkPhysicalDeviceFeatures& enabledFeatures = device->getEnabledFeatures();
    VkPipelineRasterizationStateCreateInfo info     = { };
    info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.cullMode                   = getNativeCullMode(rs.cullMode);
    info.depthBiasClamp             = rs.depthBiasClamp;
    info.depthBiasConstantFactor    = rs.depthBiasConstantFactor;
    info.depthBiasEnable            = rs.depthBiasEnable;
    info.depthBiasSlopeFactor       = rs.depthBiasSlopFactor;
    info.depthClampEnable           = rs.depthClampEnable;
    info.frontFace                  = getNativeFrontFace(rs.frontFace);
    info.lineWidth                  = enabledFeatures.wideLines ? rs.lineWidth : 1.0f;
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


static VkStencilOpState fillStencilStateOp(const StencilOpState& opState, U8 stencilReadMask, U8 stencilWriteMask, U8 ref)
{
    VkStencilOpState state = { };
    state.compareOp     = Vulkan::getNativeCompareOp(opState.compareOp);
    state.depthFailOp   = Vulkan::getNativeStencilOp(opState.depthFailOp);
    state.failOp        = Vulkan::getNativeStencilOp(opState.failOp);
    state.passOp        = Vulkan::getNativeStencilOp(opState.passOp);
    state.compareMask   = stencilReadMask;
    state.reference     = ref;
    state.writeMask     = stencilWriteMask;
    return state;
} 


static VkPipelineDepthStencilStateCreateInfo getDepthStencilInfo(const DepthStencil& ds)
{
    VkPipelineDepthStencilStateCreateInfo info  = { };
    info.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.depthBoundsTestEnable                  = (VkBool32)ds.depthBoundsTestEnable;
    info.depthCompareOp                         = Vulkan::getNativeCompareOp(ds.depthCompareOp);
    info.depthTestEnable                        = (VkBool32)ds.depthTestEnable;
    info.depthWriteEnable                       = (VkBool32)ds.depthWriteEnable;
    info.maxDepthBounds                         = ds.maxDepthBounds;
    info.minDepthBounds                         = ds.minDepthBounds;
    info.stencilTestEnable                      = (VkBool32)ds.stencilTestEnable;
    info.back                                   = fillStencilStateOp(ds.back, ds.stencilReadMask, ds.stencilWriteMask, ds.stencilReference);
    info.front                                  = fillStencilStateOp(ds.front, ds.stencilReadMask, ds.stencilWriteMask, ds.stencilReference);
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
    std::vector<VkPipelineColorBlendAttachmentState>& blendAttachments, U32 numRenderTargets)
{
    VkPipelineColorBlendStateCreateInfo info = { };
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.logicOp            = getLogicOp(state.logicOp);
    info.logicOpEnable      = state.logicOpEnable;
    info.blendConstants[0]  = state.blendConstants[0];
    info.blendConstants[1]  = state.blendConstants[1];
    info.blendConstants[2]  = state.blendConstants[2];
    info.blendConstants[3]  = state.blendConstants[3];
    info.attachmentCount    = numRenderTargets;

    blendAttachments.resize(numRenderTargets);

    for (U32 i = 0; i < numRenderTargets; ++i) 
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
    R_ASSERT(vi.numVertexBindings < VertexInputLayout::VertexInputLayout_BindingCount);
    VulkanVertexLayout layout = { };
    
    for (U32 i = 0; i < vi.numVertexBindings; ++i) 
    {
        U32 offsetBytes                     = 0;
        const VertexBinding& vertexBind     = vi.vertexBindings[i];
        U32 binding                         = vertexBind.binding;
        U32 elementStride                                   = 0;
        VkVertexInputBindingDescription bindingDescription  = { };
        bindingDescription.binding                          = binding;
        bindingDescription.stride                           = elementStride;
        bindingDescription.inputRate                        = getNativeVertexInputRate(vertexBind.inputRate);

        for (U32 attribIdx = 0; attribIdx < vertexBind.numVertexAttributes; ++attribIdx) 
        {
            VertexAttribute& attrib                     = vertexBind.pVertexAttributes[attribIdx];
            VkVertexInputAttributeDescription attribute = { };
            attribute.binding                           = binding;
            attribute.format                            = Vulkan::getVulkanFormat(attrib.format);
            attribute.location                          = attrib.location;

            U32 formatSizeBytes                         = Vulkan::getFormatSizeBytes(attribute.format);

            if (attrib.offsetBytes != VertexAttribute::OffsetAppend)
            {
                offsetBytes         = attrib.offsetBytes + formatSizeBytes;
                attribute.offset    = attrib.offsetBytes;
            }
            else
            {
                attribute.offset = offsetBytes;
                offsetBytes += formatSizeBytes;
            }
            layout.descriptions.push_back(attribute);   
            elementStride = offsetBytes;
        }
        bindingDescription.stride = vertexBind.stride == 0 ? elementStride : vertexBind.stride;
        layout.bindings.push_back(bindingDescription);
    }

    return layout;
}

ResultCode make(VertexInputLayoutId id, const VertexInputLayout& vl)
{
    auto iter = g_vertexLayoutMap.find(id);
    if (iter != g_vertexLayoutMap.end())
        return RecluseResult_AlreadyExists;
    VulkanVertexLayout layout = createVertexInput(vl);
    g_vertexLayoutMap[id] = layout;
    return RecluseResult_Ok;
}


ResultCode unloadLayout(VertexInputLayoutId id)
{
    auto iter = g_vertexLayoutMap.find(id);
    if (iter != g_vertexLayoutMap.end())
    {
        g_vertexLayoutMap.erase(iter);
        return RecluseResult_Ok;
    }

    return RecluseResult_NotFound;
}


const VulkanVertexLayout* obtain(VertexInputLayoutId inputLayoutId)
{
    auto iter = g_vertexLayoutMap.find(inputLayoutId);
    if (iter != g_vertexLayoutMap.end())
        return &iter->second;
    R_ASSERT_FORMAT(false, "No Vertex Input found for the LayoutId(%d)", inputLayoutId);
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
        layout = *iter->second;
    }
    return layout;
}

VkPipeline createGraphicsPipeline(VulkanDevice* pDevice, VkPipelineCache pipelineCache, const Structure& structure, const ShaderPrograms::VulkanShaderProgram* program)
{
    VkPipeline pipeline                             = VK_NULL_HANDLE;
    VkDevice device                                 = pDevice->get();
    VkGraphicsPipelineCreateInfo ci                 = { };
    const VkRenderPass renderPass                   = structure.state.graphics.renderPass;
    VkResult result                                 = VK_SUCCESS;
    VkPipelineShaderStageCreateInfo shaderStages[16];

    if (!program)
    {
        R_FATAL_ERROR(R_CHANNEL_VULKAN, "Failed to obtain native vulkan program. ShaderProgramId=%llu, Permutation=%llu", structure.state.shaderProgramId, structure.state.shaderPermutation);
        return VK_NULL_HANDLE;
    }

    std::vector<VkVertexInputAttributeDescription> attributes;
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;

    VkPipelineRasterizationStateCreateInfo rasterState          = getRasterInfo(pDevice, structure.state.graphics.raster);
    VkPipelineDepthStencilStateCreateInfo depthStencilState     = getDepthStencilInfo(structure.state.graphics.depthStencil);
    VkPipelineViewportStateCreateInfo viewportState             = getViewportInfo();
    VkPipelineColorBlendStateCreateInfo blendState              = getBlendInfo(structure.state.graphics.blendState, blendAttachments, structure.state.graphics.numRenderTargets);
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
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create pipeline state layout.");

        destroyPipelineLayout(pDevice, pipelineLayout);

        return VK_NULL_HANDLE;
    }

    ci.sType                = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    ci.renderPass           = renderPass;
    ci.pRasterizationState  = &rasterState;
    ci.pColorBlendState     = &blendState;
    ci.pDepthStencilState   = &depthStencilState;
    // We don't need vertex input descriptions if the pipeline is using mesh shaders.
    ci.pInputAssemblyState  = program->graphics.usesMeshShaders ? nullptr : &inputAssemblyState; 
    ci.pVertexInputState    = program->graphics.usesMeshShaders ? nullptr : &vertInputState;
    ci.pViewportState       = &viewportState;
    ci.pDynamicState        = &dynamicState;
    ci.pMultisampleState    = &multisampleState;
    ci.stageCount           = 0;
    
    if (program->graphics.usesMeshShaders)
    {
        if (program->graphics.as)
        {
            shaderStages[ci.stageCount]         = { };
            shaderStages[ci.stageCount].module  = program->graphics.as;
            shaderStages[ci.stageCount].stage   = VK_SHADER_STAGE_TASK_BIT_NV;
            shaderStages[ci.stageCount].pName   = program->graphics.asEntry;
            shaderStages[ci.stageCount].sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            ci.stageCount += 1;
        }
        if (program->graphics.ms)
        {
            shaderStages[ci.stageCount]         = { };
            shaderStages[ci.stageCount].module  = program->graphics.ms;
            shaderStages[ci.stageCount].stage   = VK_SHADER_STAGE_MESH_BIT_NV;
            shaderStages[ci.stageCount].pName   = program->graphics.msEntry;
            shaderStages[ci.stageCount].sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            ci.stageCount += 1;
        }
    }
    else
    {
        if (program->graphics.vs) 
        {
            shaderStages[ci.stageCount]         = { };
            shaderStages[ci.stageCount].module  = program->graphics.vs;
            shaderStages[ci.stageCount].stage   = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[ci.stageCount].pName   = program->graphics.vsEntry;
            shaderStages[ci.stageCount].sType   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            ci.stageCount += 1;
        }

        if (program->graphics.gs)
        {
            shaderStages[ci.stageCount] = { };
            shaderStages[ci.stageCount].module = program->graphics.gs;
            shaderStages[ci.stageCount].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            shaderStages[ci.stageCount].pName = program->graphics.gsEntry;
            shaderStages[ci.stageCount].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
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

    ci.layout               = pipelineLayout;
    ci.pStages              = shaderStages;

    result = vkCreateGraphicsPipelines(device, pipelineCache, 1, &ci, nullptr, &pipeline);
   
    if (result != VK_SUCCESS) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create vulkan pipeline state.");
        
        destroyPipeline(pDevice, pipeline);
        
        return VK_NULL_HANDLE;
    }

    return pipeline;
}


VkPipeline createComputePipeline(VulkanDevice* pDevice, VkPipelineCache pipelineCache, const Structure& structure, const ShaderPrograms::VulkanShaderProgram* program)
{
    VkComputePipelineCreateInfo createInfo  = { };
    VkDevice device                         = pDevice->get();
    VkResult result                         = VK_SUCCESS;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout         = makeLayout(pDevice, structure.state.descriptorLayout);

    if (result != VK_SUCCESS) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create pipeline layout for Compute pipelinestate...");
        
        destroyPipelineLayout(pDevice, pipelineLayout);
        
        return VK_NULL_HANDLE;
    }

    createInfo.sType        = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.layout       = pipelineLayout;
    
    createInfo.stage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage.module = program->compute.cs;
    createInfo.stage.pName  = program->compute.csEntry;
    createInfo.stage.stage  = VK_SHADER_STAGE_COMPUTE_BIT;

    result = vkCreateComputePipelines(device, pipelineCache, 1, &createInfo, nullptr, &pipeline);

    if (result != VK_SUCCESS) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create compute pipeline state!");

        destroyPipeline(pDevice, pipeline);

        return nullptr;
    }

    return pipeline;
}


PipelineId makePipelineId(const Structure& structure)
{
    return recluseHashFast(&structure, sizeof(Structure));
}


PipelineState makePipeline(VulkanDevice* pDevice, const Structure& structure)
{
    return makePipeline(pDevice, structure, makePipelineId(structure));
}


static VkPipeline createRayTracingPipeline(VulkanDevice* pDevice, VkPipelineCache cache, const Structure& structure, ShaderPrograms::VulkanShaderProgram* program)
{
    VkPipeline pipeline = VK_NULL_HANDLE;
#if defined(RECLUSE_RAYTRACING_HEADER)
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineLayout pipelineLayout = makeLayout(pDevice, structure.state.descriptorLayout);
    VkRayTracingPipelineCreateInfoKHR rayTracingInfo    = { };
    rayTracingInfo.sType                                = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rayTracingInfo.maxPipelineRayRecursionDepth         = structure.state.raytrace.rayRecursionDepth;
    rayTracingInfo.layout                               = pipelineLayout;
    rayTracingInfo.stageCount                           = 0;

    VkPipelineShaderStageCreateInfo shaderInfo = { };
    shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    if (program->raytrace.rayAnyHit)
    {
        shaderInfo.pName    = program->raytrace.rayAnyHitEntry;
        shaderInfo.stage    = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        shaderInfo.module   = program->raytrace.rayAnyHit;
        shaderStages.push_back(shaderInfo);
    }

    if (program->raytrace.rayClosest)
    {
        shaderInfo.pName    = program->raytrace.rayClosestEntry;
        shaderInfo.stage    = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        shaderInfo.module   = program->raytrace.rayClosest;
        shaderStages.push_back(shaderInfo);
    }

    if (program->raytrace.rayGen)
    {
        shaderInfo.pName    = program->raytrace.rayGenEntry;
        shaderInfo.stage    = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        shaderInfo.module   = program->raytrace.rayGen;
        shaderStages.push_back(shaderInfo);
    }

    if (program->raytrace.rayIntersect)
    {
        shaderInfo.pName    = program->raytrace.rayIntersectEntry;
        shaderInfo.stage    = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        shaderInfo.module   = program->raytrace.rayIntersect;
        shaderStages.push_back(shaderInfo);
    }

    if (program->raytrace.rayMiss)
    {
        shaderInfo.pName    = program->raytrace.rayMissEntry;
        shaderInfo.stage    = VK_SHADER_STAGE_MISS_BIT_KHR;
        shaderInfo.module   = program->raytrace.rayMiss;
        shaderStages.push_back(shaderInfo);
    }

    VkDeferredOperationKHR deferredOperation            = VK_NULL_HANDLE;
    vkCreateRayTracingPipelinesKHR(pDevice->get(), deferredOperation, cache, 1, &rayTracingInfo, nullptr, &pipeline);
#endif
    return pipeline;
}


PipelineState makePipeline(VulkanDevice* pDevice, const Structure& structure, PipelineId id)
{
    PipelineState pipeline          = { };
    pipeline.lastUsed               = 0;
    if (!g_pipelineMap.inCache(id))
    {
        ShaderPrograms::VulkanShaderProgram* program = ShaderPrograms::obtainShaderProgram(structure.state.shaderProgramId, structure.state.shaderPermutation);
        Bool pipelineCacheHit = findPipelineCache(pDevice, id, pipeline.pipelineCache);

        R_ASSERT_FORMAT
            (
                program, 
                "No shader program (id=%d) with the following permutation (id=%d) exists! This will cause a crash!", 
                structure.state.shaderProgramId, structure.state.shaderPermutation
            );

        pipeline.bindPoint = program->bindPoint;
        switch (program->bindPoint)
        {
        case VK_PIPELINE_BIND_POINT_GRAPHICS:
            pipeline.pipeline = createGraphicsPipeline(pDevice, pipeline.pipelineCache, structure, program);
            break;
        case VK_PIPELINE_BIND_POINT_COMPUTE:
            pipeline.pipeline = createComputePipeline(pDevice, pipeline.pipelineCache, structure, program);
            break;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
            R_ERROR(R_CHANNEL_VULKAN, "Ray tracing pipelines are not supported currently!");
            pipeline.pipeline = createRayTracingPipeline(pDevice, pipeline.pipelineCache, structure, program);
            break;
        }

        if (pipeline.pipeline)
        {
            if (!pipelineCacheHit)
                cachePipeline(pDevice, id, pipeline.pipelineCache);
            g_pipelineMap.insert(id, std::move(pipeline));
        }
    }
    else
    {
        // Increment the count every time we use this pipeline.
        pipeline = *g_pipelineMap.refer(id);
    }
    return pipeline;
}


R_INTERNAL
void destroyPipelineCache(VulkanDevice* pDevice, VkPipelineCache pipelineCache)
{
    VkDevice device = pDevice->get();
    if (pipelineCache)
    {
        vkDestroyPipelineCache(device, pipelineCache, nullptr);
    }
}


ResultCode clearPipelineCache(VulkanDevice* pDevice)
{
    for (auto pipelineLayoutIt : g_pipelineLayoutMap)
    {
        destroyPipelineLayout(pDevice, *pipelineLayoutIt.second);
    }

    g_pipelineMap.forEach([pDevice] (PipelineState& state) -> void
        {
            R_DEBUG(R_CHANNEL_VULKAN, "Destroying pipeline.");
            destroyPipeline(pDevice, state.pipeline);
            destroyPipelineCache(pDevice, state.pipelineCache);
        });

    g_pipelineLayoutMap.clear();
    g_pipelineMap.clear();

    return RecluseResult_Ok;
}


ResultCode releasePipeline(VulkanDevice* pDevice, PipelineId pipelineId)
{
    
    return RecluseResult_Ok;
}


void Structure::nullify()
{
    memset(&state, 0, sizeof(state));
}


void update()
{
    g_pipelineMap.updateTick();
}


void clean(VulkanDevice* device)
{
    g_pipelineMap.check
        (
            g_vulkanPipelineMaxAge, [device] (PipelineState& state) -> void 
            {
                R_DEBUG(R_CHANNEL_VULKAN, "Destroying pipeline.");
                destroyPipeline(device, state.pipeline);
                destroyPipelineCache(device, state.pipelineCache);
            }
        ); 
}
} // Pipelines
} // Recluse