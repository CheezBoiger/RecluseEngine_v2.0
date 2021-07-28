//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/CommandQueue.hpp"
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
class GraphicsQueue;
class RenderPass;


struct SwapchainCreateDescription {
    FrameBuffering buffering;
    U32 desiredFrames;
    U32 renderWidth;
    U32 renderHeight;
    GraphicsQueue* pBackbufferQueue;
};


struct GraphicsResourceDescription {
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


struct MemoryReserveDesc {
    U64 bufferPools[RESOURCE_MEMORY_USAGE_COUNT];
    U64 texturePoolGPUOnly;
};


struct ResourceViewDesc {
    ResourceViewType        type;
    ResourceFormat          format;
    ResourceViewDimension   dimension;
    U32                     baseMipLevel;
    U32                     mipLevelCount;
    U32                     baseArrayLayer;
    U32                     layerCount;
    GraphicsResource*       pResource;
};


struct RenderPassDesc {
    GraphicsResourceView* ppRenderTargetViews[8];
    U32                   numRenderTargets;
    U32                   width;
    U32                   height;
    GraphicsResourceView* pDepthStencil;
};


class R_EXPORT GraphicsDevice {
public:
    // Reserve memory to be used for graphics resources.
    virtual ErrType reserveMemory(const MemoryReserveDesc& desc) { return REC_RESULT_NOT_IMPLEMENTED; }

    //< Create graphics resource.
    //<
    virtual ErrType createResource(GraphicsResource** ppResource, GraphicsResourceDescription& pDesc) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    //< Create a command list.
    //< 
    virtual ErrType createCommandList(GraphicsCommandList** pList, GraphicsQueueTypeFlags flags) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    //< Create a command queue.
    //<
    virtual ErrType createCommandQueue(GraphicsQueue** ppQueue, GraphicsQueueTypeFlags type) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType createDescriptorSetLayout(DescriptorSetLayout** ppLayout, const DescriptorSetLayoutDesc& desc)
        { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType createGraphicsPipelineState(PipelineState** pPipelineState, const GraphicsPipelineStateDesc& desc) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType createComputePipelineState(PipelineState** ppPipelineState, const ComputePipelineStateDesc& desc) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType createRaytracingPipelineState() { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType createResourceView(GraphicsResourceView** ppView, const ResourceViewDesc& desc) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType createRenderPass(RenderPass** ppRenderPass, const RenderPassDesc& desc) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType createDescriptorSet(DescriptorSet** ppLayout, DescriptorSetLayout* pSetLayout) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType createSwapchain(GraphicsSwapchain** swapchain,
        const SwapchainCreateDescription& pDescription) { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType destroySwapchain(GraphicsSwapchain* pSwapchain) { return REC_RESULT_NOT_IMPLEMENTED; }
    virtual ErrType destroyCommandQueue(GraphicsQueue* pQueue) { return REC_RESULT_NOT_IMPLEMENTED; }
    virtual ErrType destroyResource(GraphicsResource* pResource) { return REC_RESULT_NOT_IMPLEMENTED; }
    virtual ErrType destroyResourceView(GraphicsResourceView* pResourceView) { return REC_RESULT_NOT_IMPLEMENTED; }
    virtual ErrType destroyCommandList(GraphicsCommandList* pList) { return REC_RESULT_NOT_IMPLEMENTED; }
    virtual ErrType destroyRenderPass(RenderPass* pRenderPass) { return REC_RESULT_NOT_IMPLEMENTED; }
    virtual ErrType destroyPipelineState(PipelineState* pPipelineState) { return REC_RESULT_NOT_IMPLEMENTED; }
    virtual ErrType destroyDescriptorSet(DescriptorSet* pSet) { return REC_RESULT_NOT_IMPLEMENTED; }
    virtual ErrType destroyDescriptorSetLayout(DescriptorSetLayout* pSetLayout) { return REC_RESULT_NOT_IMPLEMENTED; }

    
private:
};


class GraphicsSwapchain {
public:
    GraphicsSwapchain(const SwapchainCreateDescription& desc)
        : m_desc(desc) { }

    R_EXPORT virtual ~GraphicsSwapchain() { }

    // Rebuild the swapchain if need be. Pass in NULL to rebuild the swapchain as is.
    // Be sure to update any new frame info and handles that are managed by the front engine!
    R_EXPORT ErrType rebuild(const SwapchainCreateDescription& desc) { 
        m_desc = desc;
        return onRebuild(); 
    }

    // Present the current image.
    R_EXPORT virtual ErrType present() { return REC_RESULT_NOT_IMPLEMENTED; }

    // Get the current frame index, updates after every present call.
    R_EXPORT virtual U32 getCurrentFrameIndex() { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual GraphicsResource* getFrame(U32 idx) = 0;
    virtual GraphicsResourceView* getFrameView(U32 idx) = 0;

    const SwapchainCreateDescription& getDesc() const { return m_desc; }

private:

    virtual ErrType onRebuild() { return REC_RESULT_NOT_IMPLEMENTED; }

    SwapchainCreateDescription m_desc;
};
} // Recluse