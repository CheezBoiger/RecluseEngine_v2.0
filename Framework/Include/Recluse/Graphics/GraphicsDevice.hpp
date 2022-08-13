//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/PipelineState.hpp"

namespace Recluse {

class GraphicsCommandList;
class GraphicsAdapter;
class GraphicsResource;
class GraphicsResourceView;
class GraphicsPipeline;
class GraphicsSwapchain;
class GraphicsContext;
class GraphicsSampler;
class RenderPass;


struct GraphicsResourceDescription 
{
    U32                 width;
    U32                 height;
    U32                 depth;
    U32                 arrayLevels;
    U32                 mipLevels;
    ResourceDimension   dimension;
    ResourceFormat      format;
    U32                 samples;
    ResourceMemoryUsage memoryUsage;
    ResourceUsageFlags  usage;
};


struct MemoryReserveDesc 
{
    U64 bufferPools[RESOURCE_MEMORY_USAGE_COUNT];
    U64 texturePoolGPUOnly;
};


struct ResourceViewDesc 
{
    ResourceViewType        type;
    ResourceFormat          format;
    ResourceViewDimension   dimension;
    U32                     baseMipLevel;
    U32                     mipLevelCount;
    U32                     baseArrayLayer;
    U32                     layerCount;
    GraphicsResource*       pResource;
};


struct RenderPassDesc 
{
    GraphicsResourceView* ppRenderTargetViews[8];
    U32                   numRenderTargets;
    U32                   width;
    U32                   height;
    GraphicsResourceView* pDepthStencil;
};


class R_PUBLIC_API GraphicsDevice 
{
public:
    // Reserve memory to be used for graphics resources.
    virtual ErrType reserveMemory(const MemoryReserveDesc& desc) { return R_RESULT_NO_IMPL; }

    //< Create graphics resource.
    //<
    virtual ErrType createResource(GraphicsResource** ppResource, GraphicsResourceDescription& pDesc, ResourceState initState) 
        { return R_RESULT_NO_IMPL; }

    virtual ErrType createDescriptorSetLayout(DescriptorSetLayout** ppLayout, const DescriptorSetLayoutDesc& desc)
        { return R_RESULT_NO_IMPL; }

    virtual ErrType createGraphicsPipelineState(PipelineState** pPipelineState, const GraphicsPipelineStateDesc& desc) 
        { return R_RESULT_NO_IMPL; }

    virtual ErrType createComputePipelineState(PipelineState** ppPipelineState, const ComputePipelineStateDesc& desc) 
        { return R_RESULT_NO_IMPL; }

    virtual ErrType createRaytracingPipelineState() { return R_RESULT_NO_IMPL; }

    virtual ErrType createResourceView(GraphicsResourceView** ppView, const ResourceViewDesc& desc) 
        { return R_RESULT_NO_IMPL; }

    virtual ErrType createRenderPass(RenderPass** ppRenderPass, const RenderPassDesc& desc) 
        { return R_RESULT_NO_IMPL; }

    virtual ErrType createDescriptorSet(DescriptorSet** ppLayout, DescriptorSetLayout* pSetLayout) 
        { return R_RESULT_NO_IMPL; }

    virtual ErrType createSampler(GraphicsSampler** ppSampler, const SamplerCreateDesc& desc) 
        { return R_RESULT_NO_IMPL; }

    virtual ErrType destroySampler(GraphicsSampler* pSampler) { return R_RESULT_NO_IMPL; }
    virtual ErrType destroyResource(GraphicsResource* pResource) { return R_RESULT_NO_IMPL; }
    virtual ErrType destroyResourceView(GraphicsResourceView* pResourceView) { return R_RESULT_NO_IMPL; }
    virtual ErrType destroyRenderPass(RenderPass* pRenderPass) { return R_RESULT_NO_IMPL; }
    virtual ErrType destroyPipelineState(PipelineState* pPipelineState) { return R_RESULT_NO_IMPL; }
    virtual ErrType destroyDescriptorSet(DescriptorSet* pSet) { return R_RESULT_NO_IMPL; }
    virtual ErrType destroyDescriptorSetLayout(DescriptorSetLayout* pSetLayout) { return R_RESULT_NO_IMPL; }

    // Obtain the graphics command list created by the device.
    virtual GraphicsCommandList* getCommandList() { return nullptr; }
    
    virtual GraphicsSwapchain* getSwapchain() { return nullptr; }
    
    virtual ErrType wait() { return R_RESULT_NO_IMPL; }
    // Not recommended, but submits a copy to this queue, and waits until the command has 
    // completed.
    virtual ErrType copyResource(GraphicsResource* dst, GraphicsResource* src) 
        { return R_RESULT_NO_IMPL; }

    // Submits copy of regions from src resource to dst resource. Generally the caller thread will
    // be blocked until this function returns, so be sure to use when needed.
    virtual ErrType copyBufferRegions
                        (
                            GraphicsResource* dst, 
                            GraphicsResource* src, 
                            CopyBufferRegion* pRegions, 
                            U32 numRegions
                        )
        { return R_RESULT_NO_IMPL; }

    // A more asyncronous alternative, which will utilize a separate, one time only command list
    // and will ultimately need to be checked when finished. 
    // This ONLY WORKS IF asyncronous queue is supported, otherwise, will return with fail.
    virtual ErrType copyBufferRegionsAsync
                        (
                            GraphicsResource* dst,
                            GraphicsResource* src,
                            CopyBufferRegion* pRegions,
                            U32 numRegions,
                            Bool* isFinished
                        ) 
        { return R_RESULT_NO_IMPL; }

private:
};


class GraphicsSwapchain 
{
public:

    enum PresentConfig
    {
        // Perform normal presentation.
        NORMAL_PRESENT  = (0),
        // Delay the presentation of the frame, which will not present the frame.
        DELAY_PRESENT   = (1<<0)
    };

    GraphicsSwapchain(const SwapchainCreateDescription& desc)
        : m_desc(desc) { }

    R_PUBLIC_API virtual ~GraphicsSwapchain() { }

    // Rebuild the swapchain if need be. Pass in NULL to rebuild the swapchain as is.
    // Be sure to update any new frame info and handles that are managed by the front engine!
    R_PUBLIC_API ErrType rebuild(const SwapchainCreateDescription& desc) 
    { 
        m_desc = desc;
        return onRebuild(); 
    }

    // Present the current image.
    R_PUBLIC_API virtual ErrType present(PresentConfig config = NORMAL_PRESENT) { return R_RESULT_NO_IMPL; }

    // Get the current frame index, updates after every present call.
    R_PUBLIC_API virtual U32 getCurrentFrameIndex() { return R_RESULT_NO_IMPL; }

    virtual GraphicsResource* getFrame(U32 idx) = 0;
    virtual GraphicsResourceView* getFrameView(U32 idx) = 0;

    const SwapchainCreateDescription& getDesc() const { return m_desc; }

private:

    virtual ErrType onRebuild() { return R_RESULT_NO_IMPL; }

    SwapchainCreateDescription m_desc;
};
} // Recluse