//
#pragma once

#include "Core/Types.hpp"
#include "Core/Arch.hpp"

namespace Recluse {

class GraphicsResource;
class GraphicsResourceView;

class CommandBuffer {
public:

    R_EXPORT virtual void reset() { } 
    R_EXPORT virtual void begin() { }
    R_EXPORT virtual void end() { }

    R_EXPORT virtual void drawIndexedInstanced() { }
    R_EXPORT virtual void drawInstanced() { }
    R_EXPORT virtual void dispatch(U32 x, U32 y, U32 z) { }
    R_EXPORT virtual void dispatchRays() { }
    
    R_EXPORT virtual void setPipelineState() { }

    R_EXPORT virtual void beginRenderPass() { }
    R_EXPORT virtual void endRenderPass() { }

    R_EXPORT virtual void bindDescriptorSets() { }

    R_EXPORT virtual void copyResources(GraphicsResource* dst, GraphicsResource* src) { }
    R_EXPORT virtual void copyResourceRegion(GraphicsResource* dst) { }
    
private:
    
};
} // Recluse