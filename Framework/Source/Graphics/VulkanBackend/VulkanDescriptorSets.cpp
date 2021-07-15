//
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanCommons.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


static VkDescriptorType getDescriptorType(DescriptorBindType bindType)
{
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    switch (bindType) {
        case DESCRIPTOR_CONSTANT_BUFFER: type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
        case DESCRIPTOR_SAMPLER: type = VK_DESCRIPTOR_TYPE_SAMPLER; break;
        case DESCRIPTOR_SHADER_RESOURCE_VIEW: type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
        case DESCRIPTOR_STORAGE_BUFFER: type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
        case DESCRIPTOR_STORAGE_IMAGE: type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
        default: break;
    }

    return type;
}


ErrType VulkanDescriptorSetLayout::initialize(VulkanDevice* pDevice, const DescriptorSetLayoutDesc& desc)
{
    VkDevice device                     = pDevice->get();
    VkDescriptorSetLayoutCreateInfo ci  = { };
    VkResult result                     = VK_SUCCESS;

    std::vector<VkDescriptorSetLayoutBinding> bindings(desc.numDescriptorBinds);

    for (U32 i = 0; i < desc.numDescriptorBinds; ++i) {
        const DescriptorBind& bind = desc.pDescriptorBinds[i];

        bindings[i].binding             = bind.binding;
        bindings[i].descriptorCount     = bind.numDescriptors;
        bindings[i].descriptorType      = getDescriptorType(bind.bindType);
        bindings[i].stageFlags          = Vulkan::getShaderStages(bind.shaderStages);
        bindings[i].pImmutableSamplers  = nullptr;  // We will eventually...
    
    }

    ci.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ci.bindingCount     = desc.numDescriptorBinds;
    ci.pBindings        = bindings.data();

    result = vkCreateDescriptorSetLayout(device, &ci, nullptr, &m_layout);

    if (result != VK_SUCCESS) {

        R_ERR(R_CHANNEL_VULKAN, "Failed to create Vulkan descriptor set layout!");
        return REC_RESULT_FAILED;

    }

    return REC_RESULT_OK;
}


ErrType VulkanDescriptorSetLayout::destroy(VulkanDevice* pDevice)
{
    if (m_layout) {
    
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying Vulkan Descriptor Set Layout...");

        vkDestroyDescriptorSetLayout(pDevice->get(), m_layout, nullptr);
        m_layout = VK_NULL_HANDLE;
    
    }

    return REC_RESULT_OK;
}
} // Recluse