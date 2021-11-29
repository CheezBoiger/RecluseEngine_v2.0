//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

namespace Recluse {

class GraphicsResource;
class GraphicsResourceView;
class RenderPass;
class PipelineState;
class DescriptorSet;


struct ResourceTransition {
    GraphicsResource*   pResource;
    ResourceState       dstState;
    U32                 layers;
    U32                 baseLayer;
    U32                 mips;
    U32                 baseMip;
};

#define MAKE_RESOURCE_TRANSITION(pResource, dstState, baseMip, mips, baseLayer, layers) { pResource, dstState, layers, baseLayer, mips, baseMip }


class R_PUBLIC_API GraphicsCommandList {
public:
    virtual ~GraphicsCommandList() { }

    virtual void begin() { }
    virtual void end() { }

    virtual void bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* pOffsets) { }
    virtual void bindIndexBuffer(GraphicsResource* pIndexBuffer, U64 offsetBytes, IndexType type) { }

    virtual void drawIndexedInstanced(U32 indexCount, U32 instanceCount, U32 firstIndex, U32 vertexOffset, U32 firstInstance) { }
    virtual void drawInstanced(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance) { }

    virtual void drawInstancedIndirect(GraphicsResource* pParams) { }
    virtual void drawIndexedInstancedIndirect(GraphicsResource* pParams) { }

    virtual void setScissors(U32 numScissors, Rect* pRects) { }
    virtual void setViewports(U32 numViewports, Viewport* pViewports) { }

    virtual void dispatch(U32 x, U32 y, U32 z) { }
    virtual void dispatchRays(U32 x, U32 y, U32 z) { }
    
    virtual void setPipelineState(PipelineState* pPipelineState, BindType bindType) { }

    virtual void setRenderPass(RenderPass* pRenderPass) { }

    virtual void bindDescriptorSets(U32 count, DescriptorSet** pSets, BindType bindType) { }
    
    virtual void clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect) { }
    virtual void clearDepthStencil(F32 clearDepth, U8 clearStencil, const Rect& rect) { }

    virtual void copyResource(GraphicsResource* dst, GraphicsResource* src) { }
    virtual void copyBufferRegions(CopyBufferRegion* pRegions, U32 numRegions) { }

    virtual void transition(ResourceTransition* pTargets, U32 targetCounts) { }

    virtual Bool supportsAsyncCompute() { return false; }

    virtual void dispatchAsync(U32 x, U32 y, U32 z) { }

    virtual void dispatchIndirect(GraphicsResource* pParams) { }

private:
    
};
} // Recluse