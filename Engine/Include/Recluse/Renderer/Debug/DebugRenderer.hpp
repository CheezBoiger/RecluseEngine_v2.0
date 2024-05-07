//
#pragma once
#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Math/Vector4.hpp"

namespace Recluse {


class GraphicsContext;
class GraphicsAdapter;
class GraphicsInstance;

namespace Engine {

class Renderer;



// Debug Renderer system is an object that encapsulates the actual renderer, this would be used to only expose needed portions
// for debug rendering.
class DebugRenderer
{
public:
    ~DebugRenderer() { }
    DebugRenderer(Renderer* renderer) : renderer(renderer) { }

    ResultCode          createTempResource(const GraphicsResourceDescription& description);
    GraphicsContext*    getContext();

    void                drawText(U32 x, U32 y, F32 scale, const char* text, const Math::Color4& color);
    void                drawLabel();
private:
    Renderer* renderer;
};
} // Engine
} // Recluse