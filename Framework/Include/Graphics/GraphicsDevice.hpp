//
#pragma once

#include "Core/Types.hpp"

namespace Recluse {


class GraphicsAdapter;
class GraphicsResource;
class GraphicsResourceView;
class GraphicsPipeline;
class GraphicsSwapchain;

class GraphicsDevice {
public:
    R_EXPORT virtual ErrType createResource() { return 0; }
    R_EXPORT virtual ErrType createCommandList() { return 0; }
    R_EXPORT virtual ErrType createCommandQueue() { return 0; }
    R_EXPORT virtual ErrType createPipeline() { return 0; }

    R_EXPORT virtual ErrType createSwapchain(GraphicsSwapchain** swapchain, 
        U32 desiredFrames, U32 renderWidth, U32 renderHeight) { return 0; }

    R_EXPORT virtual ErrType destroySwapchain() { return 0; }
private:
};


class GraphicsSwapchain {
public:
    virtual ~GraphicsSwapchain() { }


    virtual GraphicsResource* getFrame(U32 idx) = 0;
    virtual GraphicsResourceView* getFrameView(U32 idx) = 0;
};
} // Recluse