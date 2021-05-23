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
    R_EXPORT virtual ErrType createSwapchain() { return 0; }

    R_EXPORT virtual ErrType destroySwapchain() { return 0; }
private:
};
} // Recluse