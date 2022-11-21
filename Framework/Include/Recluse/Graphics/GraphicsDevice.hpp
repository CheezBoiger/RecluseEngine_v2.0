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
class GraphicsDevice;
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
    //< Memory resource amount, in bytes, per usage index.
    U64 bufferPools[ResourceMemoryUsage_Count];
    //< Memory resoure amount, in bytes, for textures on the gpu.
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


class R_PUBLIC_API GraphicsContext : public ICastableObject
{
public:
    virtual ~GraphicsContext() { }

    virtual void begin() { }
    virtual void end() { }

    // Obtain the graphics command list for the context.
    virtual GraphicsCommandList* getCommandList() { return nullptr; }
    virtual GraphicsDevice* getDevice() { return nullptr; }

        // Not recommended, but submits a copy to this queue, and waits until the command has 
    // completed.
    virtual ErrType copyResource(GraphicsResource* dst, GraphicsResource* src) 
        { return RecluseResult_NoImpl; }

    // Submits copy of regions from src resource to dst resource. Generally the caller thread will
    // be blocked until this function returns, so be sure to use when needed.
    virtual ErrType copyBufferRegions
                        (
                            GraphicsResource* dst, 
                            GraphicsResource* src, 
                            CopyBufferRegion* pRegions, 
                            U32 numRegions
                        )
        { return RecluseResult_NoImpl; }

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
        { return RecluseResult_NoImpl; }

    virtual ErrType wait() { return RecluseResult_NoImpl; }
};


class R_PUBLIC_API GraphicsDevice : public ICastableObject
{
public:
    // Reserve memory to be used for graphics resources.
    virtual ErrType reserveMemory(const MemoryReserveDesc& desc) { return RecluseResult_NoImpl; }

    //< Create graphics resource.
    //<
    virtual ErrType createResource(GraphicsResource** ppResource, GraphicsResourceDescription& pDesc, ResourceState initState) 
        { return RecluseResult_NoImpl; }

    virtual ErrType createDescriptorSetLayout(DescriptorSetLayout** ppLayout, const DescriptorSetLayoutDesc& desc)
        { return RecluseResult_NoImpl; }

    virtual ErrType createGraphicsPipelineState(PipelineState** pPipelineState, const GraphicsPipelineStateDesc& desc) 
        { return RecluseResult_NoImpl; }

    virtual ErrType createComputePipelineState(PipelineState** ppPipelineState, const ComputePipelineStateDesc& desc) 
        { return RecluseResult_NoImpl; }

    virtual ErrType createRaytracingPipelineState() { return RecluseResult_NoImpl; }

    virtual ErrType createResourceView(GraphicsResourceView** ppView, const ResourceViewDesc& desc) 
        { return RecluseResult_NoImpl; }

    virtual ErrType createRenderPass(RenderPass** ppRenderPass, const RenderPassDesc& desc) 
        { return RecluseResult_NoImpl; }

    virtual ErrType createDescriptorSet(DescriptorSet** ppLayout, DescriptorSetLayout* pSetLayout) 
        { return RecluseResult_NoImpl; }

    virtual ErrType createSampler(GraphicsSampler** ppSampler, const SamplerCreateDesc& desc) 
        { return RecluseResult_NoImpl; }

    virtual ErrType destroySampler(GraphicsSampler* pSampler) { return RecluseResult_NoImpl; }
    virtual ErrType destroyResource(GraphicsResource* pResource) { return RecluseResult_NoImpl; }
    virtual ErrType destroyResourceView(GraphicsResourceView* pResourceView) { return RecluseResult_NoImpl; }
    virtual ErrType destroyRenderPass(RenderPass* pRenderPass) { return RecluseResult_NoImpl; }
    virtual ErrType destroyPipelineState(PipelineState* pPipelineState) { return RecluseResult_NoImpl; }
    virtual ErrType destroyDescriptorSet(DescriptorSet* pSet) { return RecluseResult_NoImpl; }
    virtual ErrType destroyDescriptorSetLayout(DescriptorSetLayout* pSetLayout) { return RecluseResult_NoImpl; }

    virtual GraphicsContext* getContext() { return nullptr; }
    
    virtual GraphicsSwapchain* getSwapchain() { return nullptr; }

private:
};


class GraphicsSwapchain 
{
public:

    enum PresentConfig
    {
        // Perform normal presentation.
        PresentConfig_Present  = (0),
        // Skip the presentation of the frame, which will not present the frame.
        PresentConfig_SkipPresent   = (1<<0),
        // Delay the presentation of the frame. This will effectly not bump the frame index,
        // instead will skip the whole presenting process. Beware, this will require having to rerecord your 
        // command list, as it will not be submitted to the graphics queue.
        PresentConfig_DelayPresent  = (1 << 1)
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
    R_PUBLIC_API virtual ErrType present(PresentConfig config = PresentConfig_Present) { return RecluseResult_NoImpl; }

    // Get the current frame index, updates after every present call.
    R_PUBLIC_API virtual U32 getCurrentFrameIndex() { return RecluseResult_NoImpl; }

    virtual GraphicsResource* getFrame(U32 idx) = 0;
    virtual GraphicsResourceView* getFrameView(U32 idx) = 0;

    const SwapchainCreateDescription& getDesc() const { return m_desc; }

private:

    virtual ErrType onRebuild() { return RecluseResult_NoImpl; }

    SwapchainCreateDescription m_desc;
};
} // Recluse