//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Utility.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/Format.hpp"
#include "Recluse/Graphics/DescriptorSet.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/PipelineState.hpp"
#include "Recluse/Graphics/ShaderProgram.hpp"

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
    // Depth or Array Size. If the Resource has 3D dimensions, then it implies depth.
    // Otherwise any other dimenions will specify array size.
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
    union
    {
        struct 
        {
            // Base array layer. If the view type is Cube or Cube Array, then this 
            // value must be a multiple of 6.
            U32             baseArrayLayer;
            // Layer count. If the view type is Cube, then this MUST be 6. If the view type is Cube Array, then this MUST be a 
            // multiple of 6.
            U32             layerCount;
        };
        struct
        {
            U32             firstElement;
            U32             numElements;
            U32             byteStride;  
        };
    };
};


enum ContextFlag
{
    // Context push will create a fresh context state.
    ContextFlag_None = 0,
    // Context push will create a context state, and inherit from the parent pipeline.
    ContextFlag_InheritPipelineState = (1 << 0)
};


// Shader program binder stores the current bound shader program that will be used for the upcoming drawcalls.
// It will also check any new binds, and create/manage any descriptor sets accordingly. Usually called when 
// user binds a ShaderProgram during graphics recording.
class IShaderProgramBinder
{
public:
    virtual ~IShaderProgramBinder() { }
    IShaderProgramBinder(ShaderProgramId programId, ShaderPermutationId permutationId)
        : m_programId(programId), m_permutation(permutationId) { }

    // Binds a shader resource to the currently bound shader program. Shader Resource must be a view type.
    // \param type The Shader types that this view will be bound to.
    // \param slot The slot that this shader resource will be bound to. This is dependent on the register value for DirectX, or the 
    //             order written in the shader for Vulkan (independent of the binding value.)
    // \param view The actual View of the shader resource to bind to the shader program.
    // \return The same ShaderProgramBinder instance.
    virtual IShaderProgramBinder& bindShaderResource(ShaderStageFlags type, U32 slot, ResourceViewId view) { return (*this); }

    // Binds an unordered access resource to the currently bound shader program. The Unordered Access must be a view type.
    // \param type The Shader types that this view will be bound to.
    // \param slot The slot that this shader resource will be bound to. This is dependent on the register value for DirectX, or the 
    //             order written in the shader for Vulkan (independent of the binding value.)
    // \param view The actual view of the unordered access resoruce to bind to the shader program.
    // \return The same ShaderProgramBinder instance.
    virtual IShaderProgramBinder& bindUnorderedAccessView(ShaderStageFlags type, U32 slot, ResourceViewId view) { return (*this); }

    // Bind a constant buffer that links to certain shaders in the program. Define the slot in the shader program as well.
    // The pResource is the constant buffer resource to be bound, the offsetBytes is the offset in the pResource, along with the 
    // sizeBytes (the size of the data to read.) The data is optional (must be nullptr,) but programmer may specify local data that 
    // they wish to host-device copy to the pResource (pResource must be host copyable.)
    // \return The same ShaderProgramBinder instance.
    virtual IShaderProgramBinder& bindConstantBuffer(ShaderStageFlags type, U32 slot, GraphicsResource* pResource, U32 offsetBytes, U32 sizeBytes, void* data = nullptr) { return (*this); }
    
    // Binds a sampler resource to the currently bound shader program. Sampler must be a handle.
    // \param type The Shader types that this sampler will be bound to.
    // \param slot The slot that this sampler will be bound to. This is dependent on the register value for DirectX, or the 
    //             order written in the shader for Vulkan (independent of the binding value.)
    // \param pSampler The actual sampler handle used to bind to the shader program.
    // \return The same ShaderProgramBinder instance.
    virtual IShaderProgramBinder& bindSampler(ShaderStageFlags type, U32 slot, GraphicsSampler* pSampler) { return (*this); }

    // Return the currently bound program id.
    ShaderProgramId               getProgramId() const { return m_programId; }

    // Returns the currently bound permutation id.
    ShaderPermutationId           getPermutationId() const { return m_permutation; }

protected:
    // The currently bound permutation id.
    ShaderPermutationId           m_permutation;

    // The currently bound program id.
    ShaderProgramId               m_programId;
};

typedef U32 ContextFlags;
typedef U32 DeviceId;


// The context of the rendering api, this will describe the rendering frame work to be sent to the GPU.
// This consists of drawcalls, dispatches, copies, and other gpu related activities.
class R_PUBLIC_API GraphicsContext : public ICastableObject
{
public:
    virtual ~GraphicsContext() { }

    // Begin the rendering context recording. This must be called before you conduct drawcalls.
    // Once finished, be sure to call end().
    virtual void            begin() { }

    // End the rendering context recording. This must be called after you call begin(). 
    // Must also be called before present can be made.
    virtual void            end() { }

    // Get the current device associated with this context.
    virtual GraphicsDevice* getDevice() { return nullptr; }
    
    // Sets up the number of context frames to use for rendering. These are the resources that 
    // essentially are used by the gpu per iteration. Each context frame is independent of the swapchain frame
    // in order to avoid the cpu from waiting to record and utilize resources when ready. We can essentially
    // have the cpu working on context frames that are ready to be used, while frames that are pending are being used
    // and displayed by the gpu. 
    // Note: If there are less context frames then there are swapchain frames, you might incur a gpu stall on the cpu side, which might 
    //       not be as prevalent on lower-end hardware than higher-end. This is because the cpu side will be busy working on 
    //       resources, while the gpu is waiting for the next batch. More context frames require more memory, but will lower the possibility of 
    //       gpu stalls due to cpu usage. Keep in mind you can not have more context frames than there are swapchain frames! You need a sweet spot, which is usually
    //       around 2-3 context frames. 
    virtual ResultCode      setFrames(U32 newBufferCount) { return RecluseResult_NoImpl; }

    // Obtain the current number of frames for this context.
    virtual U32             obtainFrameCount() const { return 0; }

    // Obtain the current context frame index from this context.
    virtual U32             obtainCurrentFrameIndex() const { return 0; }

    // Submits a copy to this queue.
    // \param dst Destination resource to copy into.
    // \param src The source resource to copy from.
    virtual void            copyResource(GraphicsResource* dst, GraphicsResource* src) 
        { }

    // Submits copy of regions from src resource to dst resource. Source may be a texture.
    virtual void            copyBufferRegions
                                (
                                    GraphicsResource* dst, 
                                    GraphicsResource* src, 
                                    const CopyBufferRegion* pRegions, 
                                    U32 numRegions
                                )
        { }

    // Copies texture regions to the destination texture resource. Source may be a buffer.
    // \param dst The destination texture to copy to.
    // \param src The source resource to copy from. This can be either a texture, or a buffer.
    // \param pRegions An array of regions to specify where to copy into the destination texture.
    // \param numRegions The number of regions specified in pRegions.
    virtual void copyTextureRegions(GraphicsResource* dst, GraphicsResource* src, const CopyTextureRegion* pRegions, U32 numRegions) { }

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

    // Dispatch call to run a bound compute shader.
    // \param x The number of workgroups to dispatch in the x direction.
    // \param y The number of workgroups to dispatch in the y diretion.
    // \param z The number of workgroups to dispatch in the z direction.
    virtual void dispatch(U32 x, U32 y, U32 z) { }

    // Dispatch rays for Ray tracing pipelines. Only use if Ray Tracing is supported!
    // \param x The number of ray workgroups to dispatch in the x direction.
    // \param y The number of ray workgroups to dispatch in the y direction.
    // \param z The number of ray workgroups to dispatch in the z direction.
    virtual void dispatchRays(U32 x, U32 y, U32 z) { }

    // Mesh shader dispatch, only used if mesh shaders are supported!
    virtual void dispatchMesh(U32 x, U32 y, U32 z) { }

    virtual void clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect) { }
    virtual void clearDepthStencil(ClearFlags clearFlags, F32 clearDepth, U8 clearStencil, const Rect& rect) { }

    // Transitiion a resource to a given state. This is a modern API specific feature,
    // which is handled by managed drivers for older APIs, yet we reinforce this for older 
    // API anyways, in order to ensure newer APIs will still conform!
    // If subresource == subresourceCount, then will transition all subresources of the resource.
    virtual void transition(GraphicsResource* pResource, ResourceState newState, U16 baseMip = 0, U16 mipCount = 0, U16 baseLayer = 0, U16 layerCount = 0) { }
    virtual void transitionResources(GraphicsResource** resources, ResourceState* states, U32 resourceCount) { }

    virtual Bool supportsAsyncCompute() { return false; }

    virtual void dispatchAsync(U32 x, U32 y, U32 z) { R_ASSERT(supportsAsyncCompute()); }

    virtual void dispatchIndirect(GraphicsResource* pParams, U64 offset) { R_ASSERT(supportsAsyncCompute()); }

    virtual void setCullMode(CullMode cullmode) { }
    virtual void setFrontFace(FrontFace frontFace) { }
    virtual void setLineWidth(F32 width) { }
    virtual void setDepthCompareOp(CompareOp compareOp) { }
    virtual void setPolygonMode(PolygonMode polygonMode) { }
    virtual void bindBlendState(const BlendState& state) { }
    virtual void releaseBindingResources() { }
    virtual void setTopology(PrimitiveTopology topology) { }

    // Sets the shader program, and provides the program binder to bind the necessary resources.
    virtual IShaderProgramBinder& bindShaderProgram(ShaderProgramId program, U32 permutation = 0u) = 0;    
    virtual void bindRenderTargets(U32 count, ResourceViewId* ppResources, ResourceViewId pDepthStencil = 0) { }

    virtual void enableDepth(Bool enable) { }
    virtual void enableDepthWrite(Bool enable) { }
    virtual void enableStencil(Bool enable) { }
    virtual void setInputVertexLayout(VertexInputLayoutId inputLayout) { }
    virtual void setDepthClampEnable(Bool enable) { }
    virtual void setDepthBiasEnable(Bool enable) { }
    virtual void setDepthBiasClamp(F32 value) { }
    virtual void setStencilReference(U8 stencilRef) { }
    virtual void setStencilWriteMask(U8 mask) { }
    virtual void setStencilReadMask(U8 mask) { }
    virtual void setFrontStencilState(const StencilOpState& state) { }
    virtual void setBackStencilState(const StencilOpState& state) { }

    // Hardware render pass support.
    virtual void beginRenderPass(const RenderPassDescription& renderPassDescription) { }
    virtual void endRenderPass() { }

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

    // Clears all resource binds from the context. Useful to start with a clean slate for the binds.
    virtual void clearResourceBinds() { }

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
    virtual ResultCode              createResource(GraphicsResource** ppResource, const GraphicsResourceDescription& pDesc, ResourceState initState) 
        { return RecluseResult_NoImpl; }

    virtual ResultCode              createSampler(GraphicsSampler** ppSampler, const SamplerDescription& desc) 
        { return RecluseResult_NoImpl; }

    virtual ResultCode              destroySampler(GraphicsSampler* pSampler) { return RecluseResult_NoImpl; }

    // Destroy the resource, this will release memory associated with the resource, back to the memory pool.
    // When calling this function, will actually queue the resource for freeing, which is then picked up the next time 
    // the frame is available for cpu work. If you plan to release immediately, flip the immediate flag to true, but be sure
    // that the resource is not currently being read/written to by the gpu during inflight!!
    virtual ResultCode              destroyResource(GraphicsResource* pResource, Bool immediate = false) { return RecluseResult_NoImpl; }

    // Makes a vertex layout, will return a layout id if one already exists.
    virtual Bool                    makeVertexLayout(VertexInputLayoutId id, const VertexInputLayout& layout) { return false; }
    virtual Bool                    destroyVertexLayout(VertexInputLayoutId id) { return false; }

    virtual GraphicsContext*        createContext() { return nullptr; }
    virtual ResultCode              releaseContext(GraphicsContext* pContext) { return RecluseResult_NoImpl; }
    
    virtual GraphicsSwapchain*      createSwapchain(const SwapchainCreateDescription& description, void* windowHandle) { return nullptr; }
    virtual ResultCode              destroySwapchain(GraphicsSwapchain* pSwapchain) { return RecluseResult_NoImpl; }

    // Load up shader programs to be created on the native api.
    virtual ResultCode              loadShaderProgram(ShaderProgramId program, ShaderProgramPermutation permutation, const ShaderProgramDefinition& definition) { return RecluseResult_NoImpl; }
    
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

    DeviceId getDeviceId() const { return m_deviceId; }

protected:
    // Implementation should set this flag in order to be queried by users. This checks if the device is capable of 
    // supporting features requested.
    void                            setSupportedFeatures(LayerFeatureFlags flags) { m_supportedFeatures = flags; }
    void                            setDeviceId(DeviceId deviceId) { m_deviceId = deviceId; }

private:
    LayerFeatureFlags               m_supportedFeatures;
    DeviceId                        m_deviceId;
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

    // Prepares the next frame for rendering, this also begins the next command recording session.
    R_PUBLIC_API virtual ResultCode         prepare(GraphicsContext* context) { return RecluseResult_NoImpl; }
    // Present the current image.
    R_PUBLIC_API virtual ResultCode         present(GraphicsContext* context) { return RecluseResult_NoImpl; }

    // Get the current frame index, updates after every present call.
    R_PUBLIC_API virtual U32                getCurrentFrameIndex() { return RecluseResult_NoImpl; }

    virtual GraphicsResource*               getFrame(U32 idx) = 0;

    const SwapchainCreateDescription&       getDesc() const { return m_desc; }
protected:
    void requestOverrideResourceFormat(ResourceFormat overridedFormat)
    {
        m_desc.format = overridedFormat;
    }

    void requestOverrideFrameCount(U32 newFrameCount)
    {
        m_desc.desiredFrames = newFrameCount;
    }

    void requestOverrideRenderResolution(U32 width, U32 height)
    {
        m_desc.renderWidth = width;
        m_desc.renderHeight = height;
    }

private:
    virtual ResultCode                      onRebuild() { return RecluseResult_NoImpl; }

    SwapchainCreateDescription              m_desc;
};
} // Recluse