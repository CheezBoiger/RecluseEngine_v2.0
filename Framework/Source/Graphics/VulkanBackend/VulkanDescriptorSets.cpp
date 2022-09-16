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
        return R_RESULT_FAILED;
    }

    return R_RESULT_OK;
}


ErrType VulkanDescriptorSetLayout::destroy(VulkanDevice* pDevice)
{
    if (m_layout) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "Destroying Vulkan Descriptor Set Layout...");
        vkDestroyDescriptorSetLayout(pDevice->get(), m_layout, nullptr);
        m_layout = VK_NULL_HANDLE;
    }

    return R_RESULT_OK;
}


ErrType VulkanDescriptorSet::initialize(VulkanDevice* pDevice, VulkanDescriptorSetLayout* pLayout)
{
    VkDescriptorSetLayout layout        = pLayout->get();
    VkDevice device                     = pDevice->get();
    VulkanDescriptorManager* pManager   = pDevice->getDescriptorHeap();

    m_allocation = pManager->allocate(&m_set, 1, &layout);

    if (!m_allocation.isValid()) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate vulkan descriptor set!!");
        return R_RESULT_FAILED;
    }

    m_pDevice = pDevice;    

    return R_RESULT_OK;
}


ErrType VulkanDescriptorSet::destroy()
{
    VkDevice device                     = m_pDevice->get();
    VulkanDescriptorManager* pManager   = m_pDevice->getDescriptorHeap();
    ErrType result                      = R_RESULT_OK;

    if (m_set) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "freeing vulkan descriptor set...");
    
        result = pManager->free(m_allocation, &m_set, 1);
        
        if (result != R_RESULT_OK) 
        {
            R_ERR(R_CHANNEL_VULKAN, "Failed to free vulkan native descriptor set!");
        } 
        else 
        {
            m_set = VK_NULL_HANDLE;   
            m_allocation.invalidate();
        }

    }
    
    return result;
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

    return R_RESULT_OK;
}
} // Recluse