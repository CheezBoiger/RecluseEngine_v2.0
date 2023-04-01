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
class VulkanPipelineState;
class VulkanContext;

enum CommandListStatus 
{
    CommandList_Closed,
    CommandList_Reset,
    CommandList_Recording,
    CommandList_Ready
};

class VulkanPrimaryCommandList : public GraphicsCommandList 
{
public:
    VulkanPrimaryCommandList()
        : m_currentCmdBuffer(VK_NULL_HANDLE)
        , m_currentIdx(0)
        , m_queueFamilyIndex(0)
        , m_status(CommandList_Closed) { }


    //! Initialize the command buffer, using queue family and the command pools 
    //!
    ErrType initialize
                (
                    VulkanContext* pContext, 
                    U32 queueFamilyIndex, 
                    const VkCommandPool* pools, 
                    U32 poolCount
                );

    //! Release the Vulkan handle of this command buffer.
    void                release(VulkanContext* pContext);
    void                reset() override;

    // Shift the primary command list to use a command buffer from the given index.
    void                use(U32 idx) { m_currentIdx = idx; }

    //! Get the native handle of this command buffer. Vulkan specific.
    VkCommandBuffer     get() const;

    void                begin();
    void                end();

    CommandListStatus   getStatus() const { return m_status; }
    void                setStatus(CommandListStatus status) { m_status = status; }

private:
    void                beginCommandList(U32 idx);
    void                endCommandList(U32 idx);

    std::vector<VkCommandBuffer>        m_buffers;
    VkCommandBuffer                     m_currentCmdBuffer;
    U32                                 m_currentIdx;
    CommandListStatus                   m_status;
    U32                                 m_queueFamilyIndex;
};


// Secondary command list for multithreaded rendering.
class VulkanSecondaryCommandList : public GraphicsCommandList
{
public:

};
} // Recluse