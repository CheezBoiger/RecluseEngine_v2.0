//
#pragma once

#include "Core/Types.hpp"

namespace Recluse {


class GraphicsDevice;
class GraphicsCommandList;
class GraphicsFence;
class GraphicsSemaphore;

enum GraphicsQueueType {
    QUEUE_TYPE_GRAPHICS,
    QUEUE_TYPE_COMPUTE,
    QUEUE_TYPE_COPY
};

struct QueueSubmit {
    GraphicsCommandList**   pCommandLists;
    U32                     numCommandLists;
    GraphicsSemaphore**     pWaitSemaphores;
    U32                     numWaitSemaphores;
    GraphicsSemaphore**     pSignalSemaphores;
    U32                     numSignalSemaphores;
};

class GraphicsQueue {
public:
    R_EXPORT GraphicsQueue(GraphicsQueueType type) : m_type(type) { }

    R_EXPORT virtual ~GraphicsQueue() { }

    R_EXPORT GraphicsQueueType getType() { return m_type; }

    R_EXPORT virtual ErrType submit(const QueueSubmit* payload) { return 0; }

    R_EXPORT virtual void wait() { }

private:
    GraphicsQueueType m_type;
};
} // Recluse 