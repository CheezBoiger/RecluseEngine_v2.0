//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Utility.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/PipelineState.hpp"
#include "Recluse/Graphics/ShaderProgramBuilder.hpp"

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
    U32                 depthOrArraySize;
    U32                 mipLevels;
    ResourceDimension   dimension;
    ResourceFormat      format;
    U32                 samples;
    ResourceMemoryUsage memoryUsage;
    ResourceUsageFlags  usage;
    ResourceMiscFlags   miscFlags;
    const char*         name; // for debug purposes.
};


struct ResourceViewDescription 
{
    ResourceViewType        type;
    ResourceFormat          format;
    ResourceViewDimension   dimension;
    U32                     baseMipLevel;
    U32                     mipLevelCount;
    U32                     baseArrayLayer;
    U32                     layerCount;
};


enum ContextFlag
{
    // Context push will create a fresh context state.
    ContextFlag_None = 0,
    // Context push will create a context state, and inherit from the parent pipeline.
    ContextFlag_InheritPipelineState = (1 << 0)
};

typedef U32 ContextFlags;


class R_PUBLIC_API GraphicsContext : public ICastableObject
{
public:
    virtual ~GraphicsContext() { }

    // Begin the rendering context recording. This must be called before you conduct drawcalls.
    // Once finished, be sure to call end().
    virtual void begin() { }

    // End the rendering context recording. This must be called after you call begin(). 
    // Must also be called before present can be made.
    virtual void end() { }

    // Get the current device associated with this context.
    virtual GraphicsDevice* getDevice() { return nullptr; }
    
    // Sets up the number of buffers to use for rendering. These buffers are the resources that 
    // essentially are used by the gpu per frame. Each buffer will usually correspond to each frame
    // in order to avoid the cpu from waiting to record and utilize resources when ready. We can essentially
    // have the cpu working on buffers that are ready to be used, while buffers that are pending are being used
    // and displayed by the gpu. 
    // Note: If there are less buffers then there are frames, you might incur a gpu stall on the cpu side, which might 
    //       not be as prevalent on lower-end hardware than higher-end. This is because the cpu side will be busy working on 
    //       resources, while the gpu is waiting for the next batch. More buffers require more memory, but will lower the possibility of 
    //       gpu stalls due to cpu usage, but too many buffers may potentially lead to stall as well! You need a sweet spot, which is usually
    //       around 2-3 buffers. 
    virtual ResultCode setBuffers(U32 newBufferCount) { return RecluseResult_NoImpl; }
    virtual U32 obtainBufferCount() const { return 0; }

    // Obtain the current buffer index from this context.
    virtual U32 obtainCurrentBufferIndex() const { return 0; }

    // Not recommended, but submits a copy to this queue.
    virtual void copyResource(GraphicsResource* dst, GraphicsResource* src) 
        { }

    // Submits copy of regions from src resource to dst resource.
    virtual void  copyBufferRegions
                        (
                            GraphicsResource* dst, 
                            GraphicsResource* src, 
                            const CopyBufferRegion* pRegions, 
                            U32 numRegions
                        )
        { }

    // A more asyncronous alternative, which will utilize a separate, one time only command list
    // and will ultimately need to be checked when finished. 
    // This ONLY WORKS IF asyncronous queue is supported, otherwise, will return with fail.
    virtual void copyBufferRegionsAsync
                        (
                            GraphicsResource* dst,
                            GraphicsResource* src,
                            const CopyBufferRegion* pRegions,
                            U32 numRegions,
                            Bool* isFinished
                        ) 
        { }

    // Requests a host-side wait on the device. This is useful if there is mutual exclusivity of resources that the device is 
    // in the middle of using. For instance, trying to clean up resources at the end of an application, would require waiting until
    // the device has finished its current use of the resources, before performing any clean up.
    // Other instances can include minimizing the window, or when the window is out of focus...
    virtual ResultCode wait() { return RecluseResult_NoImpl; }

    virtual void bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* pOffsets) { }
    virtual void bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type) { }

    virtual void drawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 firstIndex, U32 vertexOffset, U32 firstInstance) { }
    virtual void drawInstanced(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance) { }

    virtual void drawInstancedIndirect(GraphicsResource* pParams, U32 offset, U32 drawCount, U32 stride) { }
    virtual void drawIndexedInstancedIndirect(GraphicsResource* pParams, U32 offset, U32 drawCount, U32 stride) { }

    virtual void setScissors(U32 numScissors, Rect* pRects) { }
    virtual void setViewports(U32 numViewports, Viewport* pViewports) { }

    virtual void dispatch(U32 x, U32 y, U32 z) { }
    virtual void dispatchRays(U32 x, U32 y, U32 z) { }

    virtual void clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect) { }
    virtual void clearDepthStencil(ClearFlags clearFlags, F32 clearDepth, U8 clearStencil, const Rect& rect) { }

    virtual void copyBufferRegions(CopyBufferRegion* pRegions, U32 numRegions) { }

    // Transitiion a resource to a given state. This is a modern API specific feature,
    // which is handled by managed drivers for older APIs, yet we reinforce this for older 
    // API anyways, in order to ensure newer APIs will still conform!
    virtual void transition(GraphicsResource* pResource, ResourceState) { }
    virtual void transitionResources(GraphicsResource** resources, ResourceState* states, U32 resourceCount) { }

    virtual Bool supportsAsyncCompute() { return false; }

    virtual void dispatchAsync(U32 x, U32 y, U32 z) { R_ASSERT(supportsAsyncCompute()); }

    virtual void dispatchIndirect(GraphicsResource* pParams, U64 offset) { R_ASSERT(supportsAsyncCompute()); }

    virtual void setCullMode(CullMode cullmode) { }
    virtual void setFrontFace(FrontFace frontFace) { }
    virtual void setLineWidth(F32 width) { }
    virtual void setDepthCompareOp(CompareOp compareOp) { }
    virtual void setPolygonMode(PolygonMode polygonMode) { }
    virtual void bindShaderResources(ShaderType type, U32 offset, U32 count, ResourceViewId* ppResources) { }
    virtual void bindUnorderedAccessViews(ShaderType type, U32 offset, U32 count, ResourceViewId* ppResources) { }
    virtual void bindConstantBuffer(ShaderType type, U32 slot, GraphicsResource* pResource, U64 offsetBytes, U64 sizeBytes) { }
    virtual void bindRenderTargets(U32 count, ResourceViewId* ppResources, ResourceViewId pDepthStencil = 0) { }
    virtual void bindSamplers(ShaderType type, U32 count, GraphicsSampler** ppSampler) { }
    virtual void bindRasterizerState(const RasterState& state) { }
    virtual void bindBlendState(const BlendState& state) { }
    virtual void releaseBindingResources() { }
    virtual void setTopology(PrimitiveTopology topology) { }
    virtual void setShaderProgram(ShaderProgramId program, U32 permutation = 0u) { }
    virtual void enableDepth(Bool enable) { }
    virtual void enableStencil(Bool enable) { }
    virtual void setInputVertexLayout(VertexInputLayoutId inputLayout) { }
    virtual void setDepthClampEnable(Bool enable) { }
    virtual void setDepthBiasEnable(Bool enable) { }
    virtual void setDepthBiasClamp(F32 value) { }

    // Blend operations to be done on a rendertarget. Index matches the rendertarget index, and must be
    // less than the max rendertargets bindable.
    virtual void setBlendEnable(U32 rtIndex, Bool enable) { }
    virtual void setBlendLogicOpEnable(Bool enable) { }
    virtual void setBlendLogicOp(LogicOp logicOp) { }
    virtual void setBlendConstants(F32 blendConstants[4]) { }
    virtual void setBlend
                    (
                        U32 rtIndex, 
                        BlendFactor srcColorFactor, BlendFactor dstColorFactor, BlendOp colorBlendOp, 
                        BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor, BlendOp alphaOp
                    ) { }
    virtual void setColorWriteMask(U32 rtIndex, ColorComponentMaskFlags writeMask) { }

    // Pop the current pipeline state, resorting to previous pipeline state.
    virtual void popState() { }
    // Push the current pipeline state. Allows inheriting from previous state.
    virtual void pushState(ContextFlags flags = ContextFlag_None) { }

    // Creates bundles to be used for multithreaded rendering. These will only inherit the current pipeline,
    // along with allow for binding resources and calling draw commands. In no way shall bundles be 
    // allowed to change the pipeline state, or render targets.
    virtual GraphicsContext** makeBundles(U32 requestedCount) { return nullptr; }

    // Submits the bundles that will be ran through the context. Keep in mind that all bundles are cleared 
    // when end() is called. This indicates that the next batch of bundles will be reset to be used for the next
    // frame.
    virtual void submitBundles(GraphicsContext** ppBundles, U32 count) { }
};


class R_PUBLIC_API GraphicsDevice : public ICastableObject
{
public:
    static const U32 kMaxGraphicsContexts = 4;

    virtual ~GraphicsDevice() { }

    // Reserve memory to be used for graphics resources.
    virtual ResultCode              reserveMemory(const MemoryReserveDescription& desc) { return RecluseResult_NoImpl; }

    //< Create graphics resource.
    //<
    virtual ResultCode              createResource(GraphicsResource** ppResource, GraphicsResourceDescription& pDesc, ResourceState initState) 
        { return RecluseResult_NoImpl; }

    virtual ResultCode              createSampler(GraphicsSampler** ppSampler, const SamplerDescription& desc) 
        { return RecluseResult_NoImpl; }

    virtual ResultCode              destroySampler(GraphicsSampler* pSampler) { return RecluseResult_NoImpl; }
    virtual ResultCode              destroyResource(GraphicsResource* pResource) { return RecluseResult_NoImpl; }

    // Makes a vertex layout, will return a layout id if one already exists.
    virtual Bool                    makeVertexLayout(VertexInputLayoutId id, const VertexInputLayout& layout) { return false; }
    virtual Bool                    destroyVertexLayout(VertexInputLayoutId id) { return false; }

    virtual GraphicsContext*        createContext() { return nullptr; }
    virtual ResultCode              releaseContext(GraphicsContext* pContext) { return RecluseResult_NoImpl; }
    
    virtual GraphicsSwapchain*      getSwapchain() { return nullptr; }

    // Load up shader programs to be created on the native api.
    virtual ResultCode              loadShaderProgram(ShaderProgramId program, ShaderProgramPermutation permutation, const Builder::ShaderProgramDefinition& definition) { return RecluseResult_NoImpl; }
    
    // Unload a shader program, as well as it's permutations, created by this device.
    virtual ResultCode              unloadShaderProgram(ShaderProgramId program) { return RecluseResult_NoImpl; }
    // Unload all shader programs, as well as their permutations.
    virtual void                    unloadAllShaderPrograms() { return; }

    Bool                            hasFeaturesSupport(LayerFeatureFlags features) { return (m_supportedFeatures & features); }

    // Not recommended, but submits a copy to this queue.
    virtual void                    copyResource(GraphicsResource* dst, GraphicsResource* src) 
        { }

    // Submits copy of regions from src resource to dst resource.
    virtual void                    copyBufferRegions
                                        (
                                            GraphicsResource* dst, 
                                            GraphicsResource* src, 
                                            const CopyBufferRegion* pRegions, 
                                            U32 numRegions
                                        )
        { }

protected:
    // Implementation should set this flag in order to be queried by users. This checks if the device is capable of 
    // supporting features requested.
    void                            setSupportedFeatures(LayerFeatureFlags flags) { m_supportedFeatures = flags; }
private:
    LayerFeatureFlags               m_supportedFeatures;
};


class GraphicsSwapchain : public ICastableObject
{
public:

    enum PresentConfig
    {
        // Perform normal presentation.
        PresentConfig_Present  = (0),
        // Skip the presentation of the frame, which will not present the frame.
        PresentConfig_SkipPresent   = (1<<0),
        // Delay the presentation of the frame. This will effectly not bump the frame index,
        // instead will skip the whole presenting process.
        PresentConfig_DelayPresent  = (1 << 1)
    };

    GraphicsSwapchain(const SwapchainCreateDescription& desc)
        : m_desc(desc) { }

    R_PUBLIC_API virtual ~GraphicsSwapchain() { }

    // Rebuild the swapchain if need be. Pass in NULL to rebuild the swapchain as is.
    // Be sure to update any new frame info and handles that are managed by the front engine!
    R_PUBLIC_API ResultCode                 rebuild(const SwapchainCreateDescription& desc) 
    { 
        m_desc = desc;
        return onRebuild(); 
    }

    // Present the current image.
    R_PUBLIC_API virtual ResultCode         present(PresentConfig config = PresentConfig_Present) { return RecluseResult_NoImpl; }

    // Get the current frame index, updates after every present call.
    R_PUBLIC_API virtual U32                getCurrentFrameIndex() { return RecluseResult_NoImpl; }

    virtual GraphicsResource*               getFrame(U32 idx) = 0;

    const SwapchainCreateDescription&       getDesc() const { return m_desc; }

private:
    virtual ResultCode                      onRebuild() { return RecluseResult_NoImpl; }

    SwapchainCreateDescription              m_desc;
};
} // Recluse