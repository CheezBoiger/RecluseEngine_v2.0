//
#include "VulkanPipelineState.hpp"
#include "VulkanDevice.hpp"
#include "VulkanObjects.hpp"
#include "VulkanCommons.hpp"

#include "Recluse/Types.hpp"
#include "Recluse/Messaging.hpp"


namespace Recluse {


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

    ci.sType        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    ci.renderPass   = pVr->get();
    ci.layout       = m_pipelineLayout;

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