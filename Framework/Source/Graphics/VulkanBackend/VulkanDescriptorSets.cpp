//
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanCommons.hpp"
#include "VulkanResource.hpp"
#include "VulkanDescriptorManager.hpp"
#include "VulkanViews.hpp"

#include "Recluse/Messaging.hpp"

namespace Recluse {


static VkDescriptorType getDescriptorType(DescriptorBindType bindType)
{
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    switch (bindType) 
    {
        case DESCRIPTOR_CONSTANT_BUFFER:        type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
        case DESCRIPTOR_SAMPLER:                type = VK_DESCRIPTOR_TYPE_SAMPLER; break;
        case DESCRIPTOR_SHADER_RESOURCE_VIEW:   type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
        case DESCRIPTOR_STORAGE_BUFFER:         type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
        case DESCRIPTOR_STORAGE_IMAGE:          type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
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

    for (U32 i = 0; i < desc.numDescriptorBinds; ++i) 
    {
        const DescriptorBindDesc& bindDesc = desc.pDescriptorBinds[i];

        bindings[i].binding             = bindDesc.binding;
        bindings[i].descriptorCount     = bindDesc.numDescriptors;
        bindings[i].descriptorType      = getDescriptorType(bindDesc.bindType);
        bindings[i].stageFlags          = Vulkan::getShaderStages(bindDesc.shaderStages);
        bindings[i].pImmutableSamplers  = nullptr;  // We will eventually...
    }

    ci.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ci.bindingCount     = desc.numDescriptorBinds;
    ci.pBindings        = bindings.data();

    result = vkCreateDescriptorSetLayout(device, &ci, nullptr, &m_layout);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create Vulkan descriptor set layout!");
        return REC_RESULT_FAILED;
    }

    return REC_RESULT_OK;
}


ErrType VulkanDescriptorSetLayout::destroy(VulkanDevice* pDevice)
{
    if (m_layout) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying Vulkan Descriptor Set Layout...");
        vkDestroyDescriptorSetLayout(pDevice->get(), m_layout, nullptr);
        m_layout = VK_NULL_HANDLE;
    }

    return REC_RESULT_OK;
}


ErrType VulkanDescriptorSet::initialize(VulkanDevice* pDevice, VulkanDescriptorSetLayout* pLayout)
{
    VkDescriptorSetLayout layout    = pLayout->get();
    VkResult result                 = VK_SUCCESS;
    VkDevice device                 = pDevice->get();
    VkDescriptorPool pool           = pDevice->getDescriptorHeap()->get();

    VkDescriptorSetAllocateInfo allocIf = { };
    allocIf.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocIf.descriptorSetCount          = 1;
    allocIf.pSetLayouts                 = &layout;
    allocIf.descriptorPool              = pool;

    result = vkAllocateDescriptorSets(device, &allocIf, &m_set);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate vulkan descriptor set!!");
        return REC_RESULT_FAILED;
    }

    m_pDevice = pDevice;    

    return REC_RESULT_OK;
}


ErrType VulkanDescriptorSet::destroy()
{
    VkDevice device         = m_pDevice->get();
    VkDescriptorPool pool   = m_pDevice->getDescriptorHeap()->get();
    VkResult result         = VK_SUCCESS;

    if (m_set) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "freeing vulkan descriptor set...");
    
        result = vkFreeDescriptorSets(device, pool, 1, &m_set);
        
        if (result != VK_SUCCESS) 
        {
            R_ERR(R_CHANNEL_VULKAN, "Failed to free vulkan native descriptor set!");
        } 
        else 
        {
            m_set = VK_NULL_HANDLE;   
        }

    }
    
    return REC_RESULT_OK;
}


ErrType VulkanDescriptorSet::update(DescriptorSetBind* pBinds, U32 bindCount)
{
    VkDevice device = m_pDevice->get();

    std::vector<VkWriteDescriptorSet> writeSet(bindCount);

    // TODO: This needs to support multiple descriptor Counts!!

    VkDescriptorImageInfo imgInfos[16];
    VkDescriptorBufferInfo bufInfos[16];

    U32 bufCount = 0;
    U32 imgCount = 0;

    for (U32 i = 0; i < bindCount; ++i) 
    {
        DescriptorSetBind& bind = pBinds[i];
        writeSet[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSet[i].descriptorCount = bind.descriptorCount;
        writeSet[i].descriptorType  = getDescriptorType(bind.bindType);
        writeSet[i].dstBinding      = bind.binding;
        writeSet[i].dstSet          = m_set;
        writeSet[i].dstArrayElement = 0; // TODO

        // We don't have support for handling multiple descriptors.
        R_ASSERT(bind.descriptorCount == 1);
    
        switch (writeSet[i].descriptorType) 
        {
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            {
                VulkanResourceView* pView       = static_cast<VulkanResourceView*>(bind.srv.pView);
                imgInfos[imgCount].sampler      = VK_NULL_HANDLE;
                imgInfos[imgCount].imageView    = pView->get();
                imgInfos[imgCount].imageLayout  = pView->getExpectedLayout();   
                writeSet[i].pImageInfo          = &imgInfos[imgCount++];
            } break;

            case VK_DESCRIPTOR_TYPE_SAMPLER: 
            {
                R_ASSERT(bind.sampler.pSampler != NULL);

                VkSampler sampler           = static_cast<VulkanSampler*>(bind.sampler.pSampler)->get();
                imgInfos[imgCount].sampler  = sampler;
                writeSet[i].pImageInfo      = &imgInfos[imgCount++];
            } break;

            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            {
                VulkanBuffer* pBufferResource   = static_cast<VulkanBuffer*>(bind.cb.buffer);
                bufInfos[bufCount].buffer       = pBufferResource->get();
                bufInfos[bufCount].offset       = bind.cb.offset;
                bufInfos[bufCount].range        = bind.cb.sizeBytes;
                writeSet[i].pBufferInfo         = &bufInfos[bufCount++];
            } break;
            default: 
                break;
        }
    }

    U32 writeCount = (U32)writeSet.size();
    
    vkUpdateDescriptorSets(device, writeCount, writeSet.data(), 0, nullptr);

    return REC_RESULT_OK;
}
} // Recluse