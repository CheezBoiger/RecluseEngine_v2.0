//
#pragma once

#include "Core/Types.hpp"

namespace Recluse {


class GraphicsDevice;
class GraphicsCommandList;
class GraphicsFence;
class GraphicsSemaphore;

enum GraphicsQueueType {
    QUEUE_TYPE_GRAPHICS     = (1 << 0),
    QUEUE_TYPE_COMPUTE      = (1 << 1),
    QUEUE_TYPE_COPY         = (1 << 2)
};

typedef U32 GraphicsQueueTypeFlags;

struct QueueSubmit {
    GraphicsCommandList**   pCommandLists;
    U32                     numCommandLists;
    GraphicsSemaphore**     pWaitSemaphores;
    U32                     numWaitSemaphores;
    GraphicsSemaphore**     pSignalSemaphores;
    U32                     numSignalSemaphores;
};

class R_EXPORT GraphicsQueue {
public:
    GraphicsQueue(GraphicsQueueType type) : m_type(type) { }

    virtual ~GraphicsQueue() { }

    GraphicsQueueTypeFlags getType() { return m_type; }

    virtual ErrType submit(const QueueSubmit* payload) { return 0; }

    virtual void wait() { }

private:
    GraphicsQueueTypeFlags m_type;
};
} // Recluse 