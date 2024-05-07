//
#include "VulkanObjects.hpp"
#include "VulkanDevice.hpp"
#include "VulkanCommons.hpp"
#include "VulkanResource.hpp"
#include "VulkanDescriptorManager.hpp"
#include "VulkanAdapter.hpp"
#include "VulkanViews.hpp"
#include "Recluse/Math/MathCommons.hpp"

#include "Recluse/Messaging.hpp"

#include <unordered_map>
#include <array>

#define R_MAX_WRITE_BUFFER_INFO_COUNT 64
#define R_MAX_WRITE_IMAGE_INFO_COUNT 64
#define R_MAX_WRITE_INFO_COUNT (R_MAX_WRITE_BUFFER_INFO_COUNT + R_MAX_WRITE_IMAGE_INFO_COUNT)

#define R_MAX_EXPECTED_ACCELERATION_STRUCTURE_COUNT 8

namespace Recluse {
namespace Vulkan {
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
#if defined(RECLUSE_RAYTRACING_HEADER)
            else if (dimension == ResourceViewDimension_RayTraceAccelerationStructure)
                type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
#endif
            else
                type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; 
            break;
        case DescriptorBindType_UnorderedAccess:        
            if (dimension == ResourceViewDimension_Buffer)
                type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
#if defined(RECLUSE_RAYTRACING_HEADER)
            else if (dimension == ResourceViewDimension_RayTraceAccelerationStructure)
                type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
#endif
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

    for (U32 i = 0; i < structure.key.value.constantBuffers; ++i)
    {
        VulkanBuffer* pBuffer = structure.ppConstantBuffers[i].buffer;
        
        // No pBuffer means no slot occupied.
        if (!pBuffer)
            continue;

        bindings[binding].binding               = structure.ppConstantBuffers[i].binding;
        bindings[binding].descriptorCount       = 1;
        bindings[binding].descriptorType        = getDescriptorType(ResourceViewDimension_Buffer, DescriptorBindType_ConstantBuffer);
        bindings[binding].stageFlags            = Vulkan::getShaderStages(pContext->obtainConstantBufferShaderFlags(pBuffer->getId()));
        bindings[binding].pImmutableSamplers    = nullptr;
        binding++;
    }
    
    for (U32 i = 0; i < structure.key.value.srvs; ++i) 
    {
        ShaderResourceBind<VulkanResourceView>& resBind = structure.ppShaderResources[i];
        VulkanResourceView* pView                       = resBind.pResourceView;

        if (!pView)
            continue;

        const ResourceViewDescription& description  = pView->getDesc();

        bindings[binding].binding             = resBind.binding;
        bindings[binding].descriptorCount     = 1;
        bindings[binding].descriptorType      = getDescriptorType(description.dimension, DescriptorBindType_ShaderResource);
        bindings[binding].stageFlags          = Vulkan::getShaderStages(pContext->obtainResourceViewShaderFlags(pView->getId()));
        bindings[binding].pImmutableSamplers  = nullptr;  // We will eventually...
        binding++;
    }

    for (U32 i = 0; i < structure.key.value.uavs; ++i)
    {
        ShaderResourceBind<VulkanResourceView>& resBind = structure.ppUnorderedAccesses[i];
        VulkanResourceView* pView = resBind.pResourceView;

        if (!pView)
            continue;

        const ResourceViewDescription& description = pView->getDesc();

        bindings[binding].binding             = resBind.binding;
        bindings[binding].descriptorCount     = 1;
        bindings[binding].descriptorType      = getDescriptorType(description.dimension, DescriptorBindType_UnorderedAccess);
        bindings[binding].stageFlags          = Vulkan::getShaderStages(pContext->obtainResourceViewShaderFlags(pView->getId()));
        bindings[binding].pImmutableSamplers  = nullptr;
        binding++;
    }

    for (U32 i = 0; i < structure.key.value.samplers; ++i)
    {
        ShaderResourceBind<VulkanSampler>& resBind = structure.ppSamplers[i];
        VulkanSampler* pSampler = resBind.pResourceView;
        bindings[binding].binding           = resBind.binding;
        bindings[binding].descriptorCount   = 1;
        bindings[binding].descriptorType    = VK_DESCRIPTOR_TYPE_SAMPLER;
        bindings[binding].stageFlags        = Vulkan::getShaderStages(pContext->obtainSamplerShaderFlags(pSampler->getId()));
        binding++;
    }

    R_ASSERT(binding <= bindings.size());
    ci.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ci.bindingCount     = binding;
    ci.pBindings        = bindings.data();

    result = vkCreateDescriptorSetLayout(device, &ci, nullptr, &layout);

    if (result != VK_SUCCESS) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to create Vulkan descriptor set layout!");
        return VK_NULL_HANDLE;
    }

    return layout;
}


static VulkanDescriptorAllocation allocateDescriptorSet(VulkanContext* pContext, VkDescriptorSetLayout layout)
{
    DescriptorAllocatorInstance* instance   = pContext->currentDescriptorAllocator();
    VulkanDescriptorAllocation allocation   = instance->allocate(1, &layout);
    if (!allocation.isValid()) 
    {
        R_ERROR(R_CHANNEL_VULKAN, "Failed to allocate vulkan descriptor set!!");
    }
    return allocation;
}


static ResultCode freeDescriptorSet(VulkanContext* pContext, const VulkanDescriptorAllocation& allocation)
{
    DescriptorAllocatorInstance* pManager   = pContext->currentDescriptorAllocator();
    ResultCode result                          = RecluseResult_Ok;

    if (allocation.isValid()) 
    {
        R_DEBUG(R_CHANNEL_VULKAN, "freeing vulkan descriptor set...");
    
        result = pManager->free(allocation);
        
        if (result != RecluseResult_Ok) 
        {
            R_ERROR(R_CHANNEL_VULKAN, "Failed to free vulkan native descriptor set!");
        }

    }
    
    return result;
}


static VkDescriptorBufferInfo makeDescriptorBufferInfo(VulkanBuffer* pBuffer, VkDeviceSize offsetBytes, VkDeviceSize sizeBytes)
{
    VkDescriptorBufferInfo info = { };
    // The actual size of the requested buffer needs to be used, not the actual allocation size, which is usually a 
    // sized aligned to gpu alignment size.
    info.buffer                 = pBuffer->get();
    info.offset                 = offsetBytes;
    info.range                  = sizeBytes;
    return info;
}


static VkDescriptorImageInfo makeDescriptorImageInfo(VulkanImageView* pView)
{
    VkDescriptorImageInfo info = {};
    info.imageLayout = pView->getExpectedLayout();
    info.imageView = pView->get();
    return info;
}


// Vulkan descriptor writer, handles holding onto the record of resource views, buffers, and samplers.
template<U32 BufferExpectedCount, U32 ImageExpectedCount, U32 AccelerationStructureExpected = 0u>
class VulkanDescriptorWriter
{
public:
    VulkanDescriptorWriter()
        : bufferCount(0)
        , imageCount(0)
        , asCount(0)
    { }

    // Record the vulkan view. 
    void recordView(VulkanResourceView* pView, DescriptorBindType bindType, VkDescriptorSet set, U32 binding)
    {
        const ResourceViewDescription& description  = pView->getDesc();
        VkWriteDescriptorSet writeSet = { };
        writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSet.descriptorType = getDescriptorType(description.dimension, bindType);
        writeSet.descriptorCount = 1;
        writeSet.dstBinding = binding;
        writeSet.dstSet = set;
        
        if (description.dimension != ResourceViewDimension_RayTraceAccelerationStructure)
        {
            if (pView->isBufferView())
            {
                VulkanResource* pResource   = pView->getResource()->castTo<VulkanResource>();
                VulkanBuffer* buffer        = pResource->castTo<VulkanBuffer>();
                // Number of elements, times the byte stride.
                U32 sizeBytes = description.numElements * description.byteStride;
                U32 offsetBytes = description.firstElement * description.byteStride;
                R_ASSERT(buffer->getBufferSizeBytes() >= sizeBytes);
                sizeBytes = Math::clamp(sizeBytes, (U32)0, buffer->getBufferSizeBytes());
                VkDescriptorBufferInfo info = makeDescriptorBufferInfo(buffer, offsetBytes, sizeBytes);
                bufferInfo[bufferCount] = info;
                writeSet.pBufferInfo = &bufferInfo[bufferCount++];
            }
            else
            {
                VulkanImageView* pImageView = pView->castTo<VulkanImageView>();
                VkDescriptorImageInfo info = makeDescriptorImageInfo(pImageView);
                imageInfo[imageCount] = info;
                writeSet.pImageInfo = &imageInfo[imageCount++];
            }
        }
        else
#if defined(RECLUSE_RAYTRACING_HEADER)
        {
            VkWriteDescriptorSetAccelerationStructureKHR asWrite = { };
            asWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
            // TODO: Still need to create the acceleration structure!

            asInfo[asCount] = asWrite;
            writeSet.pNext = &asInfo[asCount++];
        }
#else
        {
            R_ASSERT_FORMAT(description.dimension != ResourceViewDimension_RayTraceAccelerationStructure, "Hardware raytracing is not enabled!");
        }
#endif
        writeSets.push_back(writeSet);
    }

    // Record the constant buffer.
    void recordConstantBuffer(VulkanBuffer* buffer, U32 offsetBytes, U32 sizeBytes, VkDescriptorSet set, U32 binding)    
    {
        VkWriteDescriptorSet writeSet = { };
        writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSet.descriptorType = getDescriptorType(ResourceViewDimension_Buffer, DescriptorBindType_ConstantBuffer);
        writeSet.descriptorCount = 1;
        writeSet.dstSet = set;
        writeSet.dstBinding = binding;
        
        VkDescriptorBufferInfo info = makeDescriptorBufferInfo(buffer, offsetBytes, sizeBytes);
        bufferInfo[bufferCount] = info;
        writeSet.pBufferInfo = &bufferInfo[bufferCount++];
        writeSets.push_back(writeSet);
    }

    // Record the sampler.
    void recordSampler(VulkanSampler* sampler, VkDescriptorSet set, U32 binding)
    {
        VkWriteDescriptorSet writeSet = { };
        writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSet.descriptorType = getDescriptorType(ResourceViewDimension_None, DescriptorBindType_Sampler);
        writeSet.dstSet = set;
        writeSet.dstBinding = binding;
        writeSet.descriptorCount = 1;

        VkDescriptorImageInfo info  = { };
        info.imageLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
        info.imageView              = nullptr;
        info.sampler                = sampler->get();

        imageInfo[imageCount] = info;
        writeSet.pImageInfo = &imageInfo[imageCount++];
        writeSets.push_back(writeSet);
    }

    // Do the write. Requires the device that is responsible for owning the write operation.
    RecluseResult write(VkDevice device)
    {
        const U32 sz = static_cast<U32>(writeSets.size());
        vkUpdateDescriptorSets(device, sz, writeSets.data(), 0, nullptr);
        return RecluseResult_Ok;
    }

private:
    std::array<VkDescriptorBufferInfo, BufferExpectedCount> bufferInfo;
    std::array<VkDescriptorImageInfo, ImageExpectedCount> imageInfo;
    std::array<VkWriteDescriptorSetAccelerationStructureKHR, AccelerationStructureExpected> asInfo;

    std::vector<VkWriteDescriptorSet> writeSets;

    U32 bufferCount;
    U32 imageCount;
    U32 asCount;
};


// Call only needs to really be done once, but if there is a need to update, this can be called again.
static ResultCode updateDescriptorSet(VulkanContext* pContext, VkDescriptorSet set, const Structure& structure)
{
    VkDevice device     = pContext->getDevice()->castTo<VulkanDevice>()->get();
    U32 bindingCount    = structure.key.value.constantBuffers 
                        + structure.key.value.samplers 
                        + structure.key.value.srvs 
                        + structure.key.value.uavs;

    U32 binding = 0;
    std::vector<VkWriteDescriptorSet> writeSet;

    VulkanDescriptorWriter<R_MAX_WRITE_BUFFER_INFO_COUNT, R_MAX_WRITE_IMAGE_INFO_COUNT, R_MAX_EXPECTED_ACCELERATION_STRUCTURE_COUNT> writer;

    // NOTE(): This algorithm needs to be aligned with the makeDescriptorSetLayout function!
    //         Since most descriptor sets will likely need to be created with the same format.

    // TODO: This needs to support multiple descriptor Counts!!
    for (U32 i = 0; i < structure.key.value.constantBuffers; ++i)
    {
        VulkanBuffer* pBuffer           = structure.ppConstantBuffers[i].buffer;

        // No constant buffer means the slot is unoccupied.
        if (!pBuffer)
            continue;
        R_ASSERT_FORMAT(pBuffer->isInResourceState(ResourceState_ConstantBuffer), "Resource must be in constant buffer state!");

        VkDeviceSize minUBOAlignOffsetBytes = VulkanAdapter::obtainMinUniformBufferOffsetAlignment(pContext->getDevice()->castTo<VulkanDevice>());
        VkDeviceSize alignedMemoryOffset    = align(structure.ppConstantBuffers[i].offset, minUBOAlignOffsetBytes);
        writer.recordConstantBuffer(pBuffer, alignedMemoryOffset, structure.ppConstantBuffers[i].sizeBytes, set, structure.ppConstantBuffers[i].binding);
    }

    for (U32 i = 0; i < structure.key.value.srvs; ++i)
    {
        ShaderResourceBind<VulkanResourceView>& resBind = structure.ppShaderResources[i];
        VulkanResourceView* pView = resBind.pResourceView;

        // null view means not occupied!
        if (!pView)
            continue;
        R_ASSERT_FORMAT(pView->getResource()->isInResourceState(ResourceState_ShaderResource), "Resource must be in shader resoure state!");

        writer.recordView(pView, DescriptorBindType_ShaderResource, set, resBind.binding);
    }

    for (U32 i = 0; i < structure.key.value.uavs; ++i)
    {
        VkWriteDescriptorSet write                  = { };
        ShaderResourceBind<VulkanResourceView>& resBind = structure.ppUnorderedAccesses[i];
        VulkanResourceView* pView                   = resBind.pResourceView;

        if (!pView)
            continue;
        R_ASSERT_FORMAT(pView->getResource()->isInResourceState(ResourceState_UnorderedAccess), "Resource must be in unordered access state!");

        writer.recordView(pView, DescriptorBindType_UnorderedAccess, set, resBind.binding);
    }

    for (U32 i = 0; i < structure.key.value.samplers; ++i)
    {
        VkWriteDescriptorSet write = { };
        ShaderResourceBind<VulkanSampler>& resBind = structure.ppSamplers[i];
        VulkanSampler* pSampler = resBind.pResourceView;
        writer.recordSampler(pSampler, set, resBind.binding);
    }

    return writer.write(device);
}


typedef Hash64 DescriptorSetId;
typedef Hash64 DescriptorLayoutId;

class DescriptorSetKeyHasher
{
public:
    size_t operator()(const Structure& key) const
    {
        Hash64 hh   = recluseHashFast(&key.key.hash, sizeof(U64) * 2u);
        hh          ^= recluseHashFast(key.ppShaderResources, sizeof(VulkanResourceView*) * key.key.value.srvs);
        hh          ^= recluseHashFast(key.ppConstantBuffers, sizeof(BufferView*) * key.key.value.constantBuffers);
        hh          ^= recluseHashFast(key.ppUnorderedAccesses, sizeof(VulkanResourceView*) * key.key.value.uavs);
        hh          ^= recluseHashFast(key.ppSamplers, sizeof(VulkanSampler*) * key.key.value.samplers);
        return staticCast<size_t>(hh);
    }
};


class DescriptorSetLayoutKeyHasher
{
public:
    Hash64 operator()(const Structure& key) const
    {
        return recluseHashFast(&key.key.hash, sizeof(key.key));
    }
};

std::unordered_map<DescriptorLayoutId, VkDescriptorSetLayout> g_layoutMap;
std::unordered_map<DescriptorSetId, VulkanDescriptorAllocation> g_descriptorSetMap;

VkDescriptorSetLayout makeLayout(VulkanContext* pContext, const Structure& structure)
{
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    DescriptorLayoutId id = DescriptorSetLayoutKeyHasher()(structure);
    auto& iter = g_layoutMap.find(id);
    if (iter == g_layoutMap.end())
    {
        layout = createDescriptorSetLayout(pContext, structure);
        g_layoutMap.insert(std::make_pair(id, layout));
    }
    else
    {
        layout = iter->second;
    }

    return layout;
}


const VulkanDescriptorAllocation& makeDescriptorSet(VulkanContext* pContext, const Structure& structure)
{
    DescriptorSetId id      = DescriptorSetKeyHasher()(structure);
    // TODO: Need to create a customary hash.
    auto& iter              = g_descriptorSetMap.find(id);
    if (iter == g_descriptorSetMap.end())
    {
        VulkanDescriptorAllocation allocation   = allocateDescriptorSet(pContext, makeLayout(pContext, structure));
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


static ResultCode releaseDescriptorSetHelper(VulkanContext* pContext, const VulkanDescriptorAllocation& allocation)
{
    DescriptorAllocatorInstance* pDescriptorManager = pContext->currentDescriptorAllocator();
    return pDescriptorManager->free(allocation);
}


ResultCode releaseDescriptorSet(VulkanContext* pContext, const Structure& structure)
{
    DescriptorSetId id = DescriptorSetKeyHasher()(structure);
    auto& iter = g_descriptorSetMap.find(id);
    if (iter != g_descriptorSetMap.end())
    {
        VulkanDevice* pDevice                           = pContext->getDevice()->castTo<VulkanDevice>();
        VulkanDescriptorAllocation allocation           = iter->second;
        ResultCode error = releaseDescriptorSetHelper(pContext, allocation);
        
        if (error == RecluseResult_Ok)
        { 
            g_descriptorSetMap.erase(iter);
        }
        
        return error;
    }

    return RecluseResult_NotFound;
}

ResultCode releaseLayout(VulkanContext* pContext, const Structure& structure)
{
    DescriptorLayoutId layoutId = DescriptorSetLayoutKeyHasher()(structure);
    auto& iter = g_layoutMap.find(layoutId);
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


void clearDescriptorSetCache(VulkanContext* pContext, ClearCacheFlag flag)
{
    if (flag == ClearCacheFlag_IndividualDescriptorSetClear)
    {
        ResultCode err = RecluseResult_Ok;
        for (auto& descriptor : g_descriptorSetMap)
        {
            err = releaseDescriptorSetHelper(pContext, descriptor.second);
            if (err != RecluseResult_Ok)
            {
                continue;
            }
        }
    }
    else
    {
        // Perform a simple fast clear for the descriptor pool instance.
        // We won't need to individually release each descriptor set, since they will all be released
        // in an instant, but will require re-allocating for the next available frame.
        DescriptorAllocatorInstance* instance   = pContext->currentDescriptorAllocator();
        instance->resetPools();
    }

    g_descriptorSetMap.clear();
}


void clearDescriptorLayoutCache(VulkanDevice* pDevice)
{
    VkDevice device = pDevice->get();
    for (auto it : g_layoutMap)
    {
        vkDestroyDescriptorSetLayout(device, it.second, nullptr);
    }
    g_layoutMap.clear();
}
} // DescriptorSet
} // Vulkan
} // Recluse