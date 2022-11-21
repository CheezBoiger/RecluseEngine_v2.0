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
    CommandList_Reset,
    CommandList_Recording,
    CommandList_Ready
};

class VulkanPrimaryCommandList : public GraphicsCommandList 
{
public:
    VulkanPrimaryCommandList()
        : m_boundRenderPass(VK_NULL_HANDLE)
        , m_boundPipelineState(VK_NULL_HANDLE)
        , m_contextRef(nullptr)
        , m_currentCmdBuffer(VK_NULL_HANDLE)
        , m_currentIdx(0)
        , m_status(CommandList_Reset) { }


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
    void release(VulkanContext* pContext);

    void reset() override;

    //! Get the native handle of this command buffer. Vulkan specific.
    VkCommandBuffer get() const;

    //! Begin the command list. This will usually query the 
    //! current buffer available to the device on CPU.
    void begin() override;

    //! Ends recording of this command buffer. Be sure to 
    //! call this function after begin(), and after all commands 
    //! you wish to record.
    void end() override;

    void setRenderPass(RenderPass* pRenderPass) override;
    void clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect) override;
    void clearDepthStencil(F32 clearDepth, U8 clearStencil, const Rect& rect) override;
    void setPipelineState(PipelineState* pPipelineState, BindType bindType) override;
    void bindDescriptorSets(U32 count, DescriptorSet** pSets, BindType bindType) override;

    void drawInstanced(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance) override;
    void bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* pOffsets) override;
    void bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type) override;
    void drawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 firstIndex, U32 vertexOffset, U32 firstInstance) override;

    void drawInstancedIndirect(GraphicsResource* pParams, U32 offset, U32 drawCount, U32 stride) override;
    void drawIndexedInstancedIndirect(GraphicsResource* pParams, U32 offset, U32 drawCount, U32 stride) override;
    void dispatchIndirect(GraphicsResource* pParams, U64 offset) override;

    void copyResource(GraphicsResource* dst, GraphicsResource* src) override;
    void setViewports(U32 numViewports, Viewport* pViewports) override;
    void setScissors(U32 numScissors, Rect* pRects) override;
    void dispatch(U32 x, U32 y, U32 z) override;
    
    void transition(GraphicsResource* pResource, ResourceState dstState) override;

    CommandListStatus getStatus() const { return m_status; }

    void setStatus(CommandListStatus status) { m_status = status; }

private:

    inline void endRenderPass(VkCommandBuffer buffer);
    void resetBinds();

    void beginCommandList(U32 idx);
    void endCommandList(U32 idx);

    // Flushes barrier transitions if we need to. This must be called on any resources that will be accessed by 
    // specific api calls that require the state of the resource to be the desired target at the time.
    void flushBarrierTransitions(VkCommandBuffer cmdBuffer);

    VulkanRenderPass*                   m_boundRenderPass;
    VulkanPipelineState*                m_boundPipelineState;
    std::vector<VkCommandBuffer>        m_buffers; 
    std::vector<VkCommandPool>          m_pools;
    std::vector<VkBufferMemoryBarrier>  m_bufferMemoryBarriers;
    std::vector<VkImageMemoryBarrier>   m_imageMemoryBarriers;
    VulkanContext*                      m_contextRef;
    VkCommandBuffer                     m_currentCmdBuffer;
    U32                                 m_currentIdx;
    CommandListStatus                   m_status;
    
};
} // Recluse