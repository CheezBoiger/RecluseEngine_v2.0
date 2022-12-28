//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"
#include "Recluse/Messaging.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/Resource.hpp"

namespace Recluse {

class GraphicsResource;
class GraphicsResourceView;
class RenderPass;
class PipelineState;


class R_PUBLIC_API GraphicsCommandList 
{
public:
    virtual ~GraphicsCommandList() { }

    virtual void reset() { }
    //! Begin the command list. This will usually query the 
    //! current buffer available to the device on CPU.
    virtual void begin() { }
    
    //! Ends recording of this command buffer. Be sure to 
    //! call this function after begin(), and after all commands 
    //! you wish to record.
    virtual void end() { }

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
    virtual void clearDepthStencil(F32 clearDepth, U8 clearStencil, const Rect& rect) { }

    virtual void copyResource(GraphicsResource* dst, GraphicsResource* src) { }
    virtual void copyBufferRegions(CopyBufferRegion* pRegions, U32 numRegions) { }

    // Transitiion a resource to a given state. This is a modern API specific feature,
    // which is handled by managed drivers for older APIs, yet we reinforce this for older 
    // API anyways, in order to ensure newer APIs will still conform!
    virtual void transition(GraphicsResource* pResource, ResourceState) { }

    virtual Bool supportsAsyncCompute() { return false; }

    virtual void dispatchAsync(U32 x, U32 y, U32 z) { R_ASSERT(supportsAsyncCompute()); }

    virtual void dispatchIndirect(GraphicsResource* pParams, U64 offset) { R_ASSERT(supportsAsyncCompute()); }

    virtual void bindShaderResources(U32 offset, U32 count, GraphicsResourceView** ppResources) { }
    virtual void bindUnorderedAccessViews(U32 offset, U32 count, GraphicsResourceView** ppResources) { }
    virtual void bindConstantBuffers(U32 offset, U32 count, GraphicsResourceView** ppResources) { }
    virtual void bindRenderTargets(U32 count, GraphicsResourceView** ppResources, GraphicsResourceView* pDepthStencil) { }
    virtual void bindSamplers(U32 count, GraphicsSampler** ppSampler) { }
    virtual void bindRasterizerState(const RasterState& state) { }
    virtual void bindBlendState(const BlendState& state) { }
    virtual void setTopology(PrimitiveTopology topology) { }
    virtual void setShader(ShaderStage type, Hash64 shader, U32 permutation) { }
    virtual void setPipelineType(BindType type) { }
private:
    
};
} // Recluse