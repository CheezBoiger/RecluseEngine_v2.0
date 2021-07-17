// 
#pragma once

#include "Recluse/Graphics/CommandList.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Types.hpp"
#include "VulkanCommons.hpp"

#include <vector>

namespace Recluse {

class VulkanDevice;
class VulkanRenderPass;

class VulkanCommandList : public GraphicsCommandList {
public:
    VulkanCommandList()
        : m_boundRenderPass(VK_NULL_HANDLE)
        , m_pDevice(nullptr) { }

    ErrType initialize(VulkanDevice* pDevice, U32 queueFamilyIndex, 
        VkCommandPool* pools, U32 poolCount);

    void destroy(VulkanDevice* pDevice);

    VkCommandBuffer get() const;

    void reset() { }

    void begin() override;
    void end() override;

    void setRenderPass(RenderPass* pRenderPass) override;
    void clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect) override;

private:

    inline void endRenderPass(VkCommandBuffer buffer);
    void resetBinds();

    VulkanRenderPass*            m_boundRenderPass;
    std::vector<VkCommandBuffer> m_buffers; 
    std::vector<VkCommandPool>   m_pools;
    VulkanDevice*                m_pDevice;
};
} // Recluse