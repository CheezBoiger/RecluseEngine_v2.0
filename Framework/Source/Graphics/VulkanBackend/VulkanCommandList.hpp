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

enum CommandListStatus 
{
    COMMAND_LIST_RESET,
    COMMAND_LIST_RECORDING,
    COMMAND_LIST_READY
};

class VulkanCommandList : public GraphicsCommandList 
{
public:
    VulkanCommandList()
        : m_boundRenderPass(VK_NULL_HANDLE)
        , m_boundPipelineState(VK_NULL_HANDLE)
        , m_pDevice(nullptr)
        , m_currentCmdBuffer(VK_NULL_HANDLE)
        , m_currentIdx(0)
        , m_status(COMMAND_LIST_RESET) { }

    ErrType initialize(VulkanDevice* pDevice, U32 queueFamilyIndex, 
        VkCommandPool* pools, U32 poolCount);

    void destroy(VulkanDevice* pDevice);

    VkCommandBuffer get() const;

    void begin() override;
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

    void copyResource(GraphicsResource* dst, GraphicsResource* src) override;
    void setViewports(U32 numViewports, Viewport* pViewports) override;
    void setScissors(U32 numScissors, Rect* pRects) override;
    void dispatch(U32 x, U32 y, U32 z) override;
    
    void transition(ResourceTransition* pTargets, U32 targetCounts) override;

    CommandListStatus getStatus() const { return m_status; }

    void setStatus(CommandListStatus status) { m_status = status; }

private:

    inline void endRenderPass(VkCommandBuffer buffer);
    void resetBinds();

    void beginCommandList(U32 idx);
    void endCommandList(U32 idx);

    VulkanRenderPass*            m_boundRenderPass;
    VulkanPipelineState*         m_boundPipelineState;
    std::vector<VkCommandBuffer> m_buffers; 
    std::vector<VkCommandPool>   m_pools;
    VulkanDevice*                m_pDevice;
    VkCommandBuffer              m_currentCmdBuffer;
    U32                          m_currentIdx;
    CommandListStatus            m_status;
    
};
} // Recluse