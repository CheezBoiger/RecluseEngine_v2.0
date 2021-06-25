//
#pragma once

#include "Core/Types.hpp"

namespace Recluse {


class GraphicsAdapter;
class GraphicsResource;
class GraphicsResourceView;
class GraphicsPipeline;
class GraphicsSwapchain;
class GraphicsContext;

struct SwapchainCreateDescription {
    U32 desiredFrames;
    U32 renderWidth;
    U32 renderHeight;
};

class GraphicsDevice {
public:
    R_EXPORT virtual ErrType createResource() { return 0; }
    R_EXPORT virtual ErrType createCommandList() { return 0; }
    R_EXPORT virtual ErrType createCommandQueue() { return 0; }
    R_EXPORT virtual ErrType createPipeline() { return 0; }

    R_EXPORT virtual ErrType createSwapchain(GraphicsSwapchain** swapchain, GraphicsContext* pContext,
        const SwapchainCreateDescription* pDescription) { return 0; }

    R_EXPORT virtual ErrType destroySwapchain(GraphicsContext* pContext, GraphicsSwapchain* pSwapchain) { return 0; }
private:
};


class GraphicsSwapchain {
public:
    R_EXPORT virtual ~GraphicsSwapchain() { }

    // Rebuild the swapchain if need be. Pass in NULL to rebuild the swapchain as is.
    // Be sure to update any new frame info and handles that are managed by the front engine!
    R_EXPORT virtual ErrType rebuild(const GraphicsContext* pContext, const GraphicsDevice* pDevice,
        const SwapchainCreateDescription* pDesc) { return REC_RESULT_NOT_IMPLEMENTED; }

    virtual GraphicsResource* getFrame(U32 idx) = 0;
    virtual GraphicsResourceView* getFrameView(U32 idx) = 0;
};
} // Recluse