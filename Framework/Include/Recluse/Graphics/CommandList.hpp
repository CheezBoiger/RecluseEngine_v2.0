//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Arch.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

namespace Recluse {

class GraphicsResource;
class GraphicsResourceView;
class RenderPass;
class PipelineState;
class DescriptorSet;

class GraphicsCommandList {
public:
    virtual ~GraphicsCommandList() { }

    R_EXPORT virtual void reset() { } 
    R_EXPORT virtual void begin() { }
    R_EXPORT virtual void end() { }

    R_EXPORT virtual void drawIndexedInstanced() { }
    R_EXPORT virtual void drawInstanced() { }
    R_EXPORT virtual void dispatch(U32 x, U32 y, U32 z) { }
    R_EXPORT virtual void dispatchRays(U32 x, U32 y, U32 z) { }
    
    R_EXPORT virtual void setPipelineState(PipelineState* pPipelineState, BindType bindType) { }

    R_EXPORT virtual void setRenderPass(RenderPass* pRenderPass) { }

    R_EXPORT virtual void bindDescriptorSets(U32 count, DescriptorSet* pSets) { }

    R_EXPORT virtual void copyResources(GraphicsResource* dst, GraphicsResource* src) { }
    R_EXPORT virtual void copyResourceRegion(GraphicsResource* dst) { }
    
private:
    
};
} // Recluse