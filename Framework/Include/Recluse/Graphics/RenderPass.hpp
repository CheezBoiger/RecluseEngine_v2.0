//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


class GraphicsResourceView;

// Render Pass info to be used for rendering.
class RenderPass {
public:
    virtual ~RenderPass() { }

    virtual GraphicsResourceView* getRenderTarget(U32 idx) = 0;
    virtual GraphicsResourceView* getDepthStencil() = 0;
};
} // Recluse 