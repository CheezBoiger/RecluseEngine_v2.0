//
#pragma once

#include "VulkanCommons.hpp"

#include <vector>


namespace Recluse {


class VulkanDevice;


class VulkanDescriptorManager 
{
public:

    VulkanDescriptorManager() 
        : m_pool(VK_NULL_HANDLE) { }

    void initialize(VulkanDevice* pDevice);
    void destroy(VulkanDevice* pDevice);

    VkDescriptorPool get() const { return m_pool; }

private:
    VkDescriptorPool m_pool;
};
} // Recluse