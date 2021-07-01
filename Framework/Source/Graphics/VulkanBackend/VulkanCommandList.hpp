// 
#pragma once

#include "Graphics/CommandList.hpp"
#include "Core/Types.hpp"
#include "VulkanCommons.hpp"
#include "Graphics/GraphicsDevice.hpp"

#include <vector>

namespace Recluse {

class VulkanDevice;

class VulkanCommandList : public GraphicsCommandList {
public:

    ErrType initialize(VulkanDevice* pDevice, U32 queueFamilyIndex, 
        VkCommandPool* pools, U32 poolCount);

    void destroy(VulkanDevice* pDevice);

    VkCommandBuffer get(U32 i) { return m_buffers[i]; }

    void reset() { }

    void begin() override;
    void end() override;

private:

    std::vector<VkCommandBuffer> m_buffers; 
    std::vector<VkCommandPool>   m_pools;
    VulkanDevice*                m_pDevice;
};
} // Recluse