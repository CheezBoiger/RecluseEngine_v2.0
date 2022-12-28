//
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanCommons.hpp"
#include "VulkanResource.hpp"
#include "VulkanDescriptorManager.hpp"
#include "VulkanViews.hpp"

#include "Recluse/Messaging.hpp"

#include <unordered_map>

namespace Recluse {
namespace DescriptorSets {
static VkDescriptorType getDescriptorType(ResourceViewDimension dimension, DescriptorBindType bindType)
{
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    switch (bindType) 
    {
        case DescriptorBindType_ConstantBuffer:         
            type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
            break;
        case DescriptorBindType_Sampler:                
            type = VK_DESCRIPTOR_TYPE_SAMPLER; 
            break;
        case DescriptorBindType_ShaderResource:
            if (dimension == ResourceViewDimension_Buffer)
                type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            else
                type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; 
            break;
        case DescriptorBindType_UnorderedAccess:        
            if (dimension == ResourceViewDimension_Buffer)
                type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            else
                type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            break;
        default: break;
    }

    return type;
}


VkDescriptorSetLayout createDescriptorSetLayout(VulkanContext* pContext, const DescriptorSets::Structure& structure)
{

    VkDevice device                     = pContext->getDevice()->castTo<VulkanDevice>()->get();
    VkDescriptorSetLayout layout        = VK_NULL_HANDLE;
    VkDescriptorSetLayoutCreateInfo ci  = { };
    VkResult result                     = VK_SUCCESS;

    U32 numBindings = structure.key.value.srvs + structure.key.value.uavs + structure.key.value.constantBuffers + structure.key.value.samplers;
    std::vector<VkDescriptorSetLayoutBinding> bindings(numBindings);

    U32 binding = 0;
    
    for (U32 i = 0; i < structure.key.value.srvs; ++i) 
    {
        VulkanResourceView* pView                   = structure.ppShaderResources[i];
        const ResourceViewDescription& description  = pView->getDesc();

        bindings[binding].binding             = binding;
        bindings[binding].descriptorCount     = 1;
        bindings[binding].descriptorType      = getDescriptorType(description.dimension, DescriptorBindType_ShaderResource);
        bindings[binding].stageFlags          = Vulkan::getShaderStages(pContext->obtainResourceViewShaderFlags(pView->getId()));
        bindings[binding].pImmutableSamplers  = nullptr;  // We will eventually...
        binding++;
    }

    for (U32 i = 0; i < structure.key.value.uavs; ++i)
    {
        VulkanResourceView* pView = structure.ppUnorderedAccesses[i];
        const ResourceViewDescription& description = pView->getDesc();

        bindings[binding].binding             = binding;
        bindings[binding].descriptorCount     = 1;
        bindings[binding].descriptorType      = getDescriptorType(description.dimension, DescriptorBindType_UnorderedAccess);
        bindings[binding].stageFlags          = Vulkan::getShaderStages(pContext->obtainResourceViewShaderFlags(pView->getId()));
        bindings[binding].pImmutableSamplers  = nullptr;
        binding++;
    }

    for (U32 i = 0; i < structure.key.value.constantBuffers; ++i)
    {
        VulkanBuffer* pBuffer = structure.ppConstantBuffers[i];

        bindings[binding].binding               = binding;
        bindings[binding].descriptorCount       = 1;
        bindings[binding].descriptorType        = getDescriptorType(ResourceViewDimension_Buffer, DescriptorBindType_ConstantBuffer);
        bindings[binding].stageFlags            = Vulkan::getShaderStages(pContext->obtainConstantBufferShaderFlags(pBuffer->getId()));
        bindings[binding].pImmutableSamplers    = nullptr;
        binding++;
    }

    for (U32 i = 0; i < structure.key.value.samplers; ++i)
    {
        VulkanSampler* pSampler = structure.ppSamplers[i];
        bindings[binding].binding           = binding;
        bindings[binding].descriptorCount   = 1;
        bindings[binding].descriptorType    = VK_DESCRIPTOR_TYPE_SAMPLER;
        bindings[binding].stageFlags        = Vulkan::getShaderStages(pContext->obtainSamplerShaderFlags(pSampler->getId()));
    }

    ci.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ci.bindingCount     = bindings.size();
    ci.pBindings        = bindings.data();

    result = vkCreateDescriptorSetLayout(device, &ci, nullptr, &layout);

    if (result != VK_SUCCESS) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to create Vulkan descriptor set layout!");
        return VK_NULL_HANDLE;
    }

    return layout;
}


static VulkanDescriptorAllocation allocateDescriptorSet(VulkanDevice* pDevice, VkDescriptorSetLayout layout)
{
    VulkanDescriptorManager* pManager       = pDevice->getDescriptorHeap();
    VulkanDescriptorAllocation allocation   = pManager->allocate(1, &layout);
    if (!allocation.isValid()) 
    {
        R_ERR(R_CHANNEL_VULKAN, "Failed to allocate vulkan descriptor set!!");
    }
    return allocation;
}


static ErrType freeDescriptorSet(VulkanDevice* pDevice, const VulkanDescriptorAllocation& allocation)
{
    VkDevice device                     = pDevice->get();
    VulkanDescriptorManager* pManager   = pDevice->getDescriptorHeap();
    ErrType result                      = RecluseResult_Ok;

    if (allocation.isValid()) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "freeing vulkan descriptor set...");
    
        result = pManager->free(allocation);
        
        if (result != RecluseResult_Ok) 
        {
            R_ERR(R_CHANNEL_VULKAN, "Failed to free vulkan native descriptor set!");
        }

    }
    
    return result;
}


static VkDescriptorBufferInfo makeDescriptorBufferInfo(VulkanResource* pResource)
{
    VkDescriptorBufferInfo info = { };
    R_ASSERT(pResource->isBuffer());
    VulkanBuffer* pBuffer       = pResource->castTo<VulkanBuffer>();
    // The actual size of the requested buffer needs to be used, not the actual allocation size, which is usually a 
    // sized aligned to gpu alignment size.
    const GraphicsResourceDescription& desc = pBuffer->getDesc();
    const VulkanMemory& memory  = pBuffer->getMemory();
    info.buffer                 = pBuffer->get();
    info.offset                 = memory.offsetBytes;
    info.range                  = desc.width;
    return info;
}


static VkDescriptorImageInfo makeDescriptorImageInfo(VulkanResourceView* pView)
{
    VkDescriptorImageInfo info = {};
    info.imageLayout = pView->getExpectedLayout();
    info.imageView = pView->get();
    return info;
}


// Call only needs to really be done once, but if there is a need to update, this can be called again.
static ErrType updateDescriptorSet(VulkanContext* pContext, VkDescriptorSet set, const Structure& structure)
{
    VkDevice device     = pContext->getDevice()->castTo<VulkanDevice>()->get();
    U32 bindingCount    = structure.key.value.constantBuffers 
                        + structure.key.value.samplers 
                        + structure.key.value.srvs 
                        + structure.key.value.uavs;

    U32 binding = 0;
    std::vector<VkWriteDescriptorSet> writeSet;
    std::vector<VkDescriptorBufferInfo> bufferInfo;
    std::vector<VkDescriptorImageInfo> imageInfo;

    bufferInfo.reserve(32);
    imageInfo.reserve(64);
    writeSet.reserve(96);

    // NOTE(): This algorithm needs to be aligned with the makeDescriptorSetLayout function!
    //         Since most descriptor sets will likely need to be created with the same format.

    // TODO: This needs to support multiple descriptor Counts!!
    for (U32 i = 0; i < structure.key.value.srvs; ++i)
    {
        VkWriteDescriptorSet write = { };
        VulkanResourceView* pView = structure.ppShaderResources[i];
        const ResourceViewDescription& description  = pView->getDesc();
        VulkanResource* pResource                   = pView->getResource()->castTo<VulkanResource>();
        
        write.sType             = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorCount   = 1;
        write.dstSet            = set;
        write.descriptorType    = getDescriptorType(description.dimension, DescriptorBindType_ShaderResource);
        write.dstBinding        = binding++;

        if (description.dimension == ResourceDimension_Buffer)
        {
            VkDescriptorBufferInfo info = makeDescriptorBufferInfo(pResource);
            bufferInfo.push_back(info);
            write.pBufferInfo = &bufferInfo.back();
        }
        else
        {

            VkDescriptorImageInfo info = makeDescriptorImageInfo(pView);
            imageInfo.push_back(info);
        }

        writeSet.push_back(write);
    }

    for (U32 i = 0; i < structure.key.value.uavs; ++i)
    {
        VkWriteDescriptorSet write = { };
        VulkanResourceView* pView = structure.ppUnorderedAccesses[i];
        const ResourceViewDescription& description = pView->getDesc();
        VulkanResource* pResource = pView->getResource()->castTo<VulkanResource>();

        write.sType             = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorCount   = 1;
        write.dstSet            = set;
        write.descriptorType    = getDescriptorType(description.dimension, DescriptorBindType_UnorderedAccess);
        write.dstBinding        = binding++;

        if (description.dimension == ResourceDimension_Buffer)
        { 
            VkDescriptorBufferInfo info = makeDescriptorBufferInfo(pResource);
            bufferInfo.push_back(info);
            write.pBufferInfo = &bufferInfo.back();
        }
        else
        {
            VkDescriptorImageInfo info = makeDescriptorImageInfo(pView);
            imageInfo.push_back(info);
            write.pImageInfo = &imageInfo.back();
        }

        writeSet.push_back(write);
    }

    for (U32 i = 0; i < structure.key.value.constantBuffers; ++i)
    {
        VkWriteDescriptorSet write      = { };
        VulkanBuffer* pBuffer           = structure.ppConstantBuffers[i];
        VkDescriptorBufferInfo info     = makeDescriptorBufferInfo(pBuffer);

        bufferInfo.push_back(info);

        write.sType             = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType    = getDescriptorType(ResourceViewDimension_Buffer, DescriptorBindType_ConstantBuffer);
        write.descriptorCount   = 1;
        write.dstBinding        = binding++;
        write.dstSet            = set;
        write.pBufferInfo       = &bufferInfo.back();
        writeSet.push_back(write);
    }

    for (U32 i = 0; i < structure.key.value.samplers; ++i)
    {
        VkWriteDescriptorSet write = { };
        VulkanSampler* pSampler = structure.ppSamplers[i];
        VkDescriptorImageInfo info  = { };
        info.imageLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
        info.imageView              = nullptr;
        info.sampler                = pSampler->get();
        imageInfo.push_back(info);

        write.sType             = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType    = getDescriptorType(ResourceViewDimension_None, DescriptorBindType_Sampler);
        write.descriptorCount   = 1;
        write.dstSet            = set;
        write.dstBinding        = binding++;
        write.pImageInfo        = &imageInfo.back();
    }

    const U32 sz = static_cast<U32>(writeSet.size());
    vkUpdateDescriptorSets(device, sz, writeSet.data(), 0, nullptr);

    return RecluseResult_Ok;
}

typedef Hash64 DescriptorSetId;

class DescriptorSetKeyHasher
{
public:
    size_t operator()(const Structure& key) const
    {
        Hash64 hh   = recluseHash((void*)&key.key.hash, sizeof(U64) * 2u);
        hh          ^= recluseHash(key.ppShaderResources, sizeof(VulkanResourceView*) * key.key.value.srvs);
        hh          ^= recluseHash(key.ppConstantBuffers, sizeof(VulkanBuffer*) * key.key.value.constantBuffers);
        hh          ^= recluseHash(key.ppUnorderedAccesses, sizeof(VulkanResourceView*) * key.key.value.uavs);
        hh          ^= recluseHash(key.ppSamplers, sizeof(VulkanSampler*) * key.key.value.samplers);
        return staticCast<size_t>(hh);
    }
};


class DescriptorSetLayoutKeyHasher
{
public:
    size_t operator()(const Structure& key) const
    {
        Hash64 hh = recluseHash((void*)&key.key.hash, sizeof(U64) * 2u);
        // TODO: This hasher needs to be quick. So we need to ensure we don't have too many loops here.

        return staticCast<size_t>(hh);
    }
};

class DescriptorSetLayoutKeyComparer
{
public:
    bool operator()(const Structure& lh, const Structure& rh) const
    {
        return (lh.key.hash.l0 == rh.key.hash.l0) && (lh.key.hash.l1 == rh.key.hash.l1);
    }
};

std::unordered_map<Structure, VkDescriptorSetLayout, DescriptorSetLayoutKeyHasher, DescriptorSetLayoutKeyComparer> g_layoutMap;
std::unordered_map<DescriptorSetId, VulkanDescriptorAllocation> g_descriptorSetMap;


VkDescriptorSetLayout makeLayout(VulkanContext* pContext, const Structure& structure)
{
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    auto& iter = g_layoutMap.find(structure);
    if (iter == g_layoutMap.end())
    {
        layout = createDescriptorSetLayout(pContext, structure);
        g_layoutMap.insert(std::make_pair(structure, layout));
    }
    else
    {
        layout = iter->second;
    }

    return layout;
}

const VulkanDescriptorAllocation& makeDescriptorSet(VulkanContext* pContext, const Structure& structure)
{
    VulkanDevice* pDevice   = pContext->getDevice()->castTo<VulkanDevice>();
    DescriptorSetId id      = DescriptorSetKeyHasher()(structure);
    // TODO: Need to create a customary hash.
    auto& iter              = g_descriptorSetMap.find(id);
    if (iter == g_descriptorSetMap.end())
    {
        VulkanDescriptorAllocation allocation   = allocateDescriptorSet(pDevice, makeLayout(pContext, structure));
        const VulkanDescriptorAllocation::DescriptorSet set = allocation.getDescriptorSet(0);
        updateDescriptorSet(pContext, set.set, structure);
        g_descriptorSetMap.insert(std::make_pair(id, allocation));
        return g_descriptorSetMap[id];
    }
    else
    {
        return iter->second;
    }
}


ErrType releaseDescriptorSet(VulkanContext* pContext, const Structure& structure)
{
    DescriptorSetId id = DescriptorSetKeyHasher()(structure);
    auto& iter = g_descriptorSetMap.find(id);
    if (iter != g_descriptorSetMap.end())
    {
        VulkanDevice* pDevice                           = pContext->getDevice()->castTo<VulkanDevice>();
        VulkanDescriptorManager* pDescriptorManager     = pDevice->getDescriptorHeap();
        VulkanDescriptorAllocation allocation           = iter->second;
        ErrType error = pDescriptorManager->free(allocation);
        
        if (error == RecluseResult_Ok)
        { 
            g_descriptorSetMap.erase(iter);
        }
        
        return error;
    }

    return RecluseResult_NotFound;
}

ErrType releaseLayout(VulkanContext* pContext, const Structure& structure)
{
    auto& iter = g_layoutMap.find(structure);
    if (iter != g_layoutMap.end())
    {
        VkDevice device = pContext->getDevice()->castTo<VulkanDevice>()->get();
        vkDestroyDescriptorSetLayout(device, iter->second, nullptr);
        g_layoutMap.erase(iter);
        return RecluseResult_Ok;
    }

    return RecluseResult_NotFound;
}

DescriptorSetLayoutId obtainDescriptorLayoutKey(const Structure& structure)
{
    return DescriptorSetKeyHasher()(structure);
}
} // DescriptorSet
} // Recluse