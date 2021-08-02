//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

namespace Recluse {


class GraphicsDevice;
class GraphicsCommandList;
class GraphicsFence;
class GraphicsResource;

enum GraphicsQueueType {
    QUEUE_TYPE_PRESENT      = (1 << 0),
    QUEUE_TYPE_GRAPHICS     = (1 << 1),
    QUEUE_TYPE_COMPUTE      = (1 << 2),
    QUEUE_TYPE_COPY         = (1 << 3)
};

typedef U32 GraphicsQueueTypeFlags;

struct QueueSubmit {
    GraphicsCommandList**   pCommandLists;
    U32                     numCommandLists;
};


// Graphics queue interface.
class R_EXPORT GraphicsQueue {
public:
    GraphicsQueue(GraphicsQueueTypeFlags type) : m_type(type) { }

    virtual ~GraphicsQueue() { }

    GraphicsQueueTypeFlags getType() { return m_type; }

    // Not recommended, but submits a copy to this queue, and waits until the command has 
    // completed.
    virtual ErrType copyResource(GraphicsResource* dst, GraphicsResource* src) 
        { return REC_RESULT_NOT_IMPLEMENTED; }

    // Submits copy of regions from src resource to dst resource. Generally the caller thread will
    // be blocked until this function returns, so be sure to use when needed.
    virtual ErrType copyBufferRegions(GraphicsResource* dst, GraphicsResource* src, 
        CopyBufferRegion* pRegions, U32 numRegions)
        { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual ErrType submit(const QueueSubmit* payload) { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual void wait() { }

private:
    GraphicsQueueTypeFlags m_type;
};
} // Recluse 