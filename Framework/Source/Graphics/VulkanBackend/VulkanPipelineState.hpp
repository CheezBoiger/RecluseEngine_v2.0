//
#pragma once

#include "VulkanCommons.hpp"
#include "Recluse/Graphics/PipelineState.hpp"

namespace Recluse {


class VulkanDevice;


class VulkanPipelineState : public PipelineState {
public:
    VulkanPipelineState()
        : m_pipeline(VK_NULL_HANDLE)
        , m_pipelineLayout(VK_NULL_HANDLE) { }

    virtual ~VulkanPipelineState() { }

    void destroy(VulkanDevice* pDevice);

    VkPipeline get() const { return m_pipeline; }
    VkPipelineLayout getLayout() const { return m_pipelineLayout; }

protected:
    VkPipeline          m_pipeline;
    VkPipelineLayout    m_pipelineLayout;
};


class VulkanGraphicsPipelineState : public VulkanPipelineState {
public:

    ErrType initialize(VulkanDevice* pDevice, const GraphicsPipelineStateDesc& desc);

};


class VulkanComputePipelineState : public VulkanPipelineState {
public:
    
    ErrType initialize(VulkanDevice* pDevice, const ComputePipelineStateDesc& desc);

};
} // Recluse