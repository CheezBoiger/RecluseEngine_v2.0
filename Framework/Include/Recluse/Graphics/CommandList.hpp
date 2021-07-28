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


class R_EXPORT GraphicsCommandList {
public:
    virtual ~GraphicsCommandList() { }

    virtual void reset() { } 
    virtual void begin() { }
    virtual void end() { }

    virtual void bindVertexBuffers(U32 numBuffers, GraphicsResource** ppVertexBuffers, U64* pOffsets) { }
    virtual void bindIndexBuffer() { }

    virtual void drawIndexedInstanced() { }
    virtual void drawInstanced(U32 vertexCount, U32 instanceCount, U32 firstVertex, U32 firstInstance) { }

    virtual void drawInstancedIndirect() { }
    virtual void drawIndexedInstancedIndirect() { }

    virtual void setScissors(U32 numScissors, Rect* pRects) { }
    virtual void setViewports(U32 numViewports, Viewport* pViewports) { }

    virtual void dispatch(U32 x, U32 y, U32 z) { }
    virtual void dispatchRays(U32 x, U32 y, U32 z) { }
    
    virtual void setPipelineState(PipelineState* pPipelineState, BindType bindType) { }

    virtual void setRenderPass(RenderPass* pRenderPass) { }

    virtual void bindDescriptorSets(U32 count, DescriptorSet** pSets, BindType bindType) { }
    
    virtual void clearRenderTarget(U32 idx, F32* clearColor, const Rect& rect) { }
    virtual void clearDepthStencil() { }

    virtual void copyResource(GraphicsResource* dst, GraphicsResource* src) { }
    virtual void copyResourceRegion(GraphicsResource* dst) { }

    virtual void transition(GraphicsResourceView** ppTargets, U32 targetCounts) { }
    
private:
    
};
} // Recluse