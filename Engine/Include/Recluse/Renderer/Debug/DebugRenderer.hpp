//
#pragma once
#include "Recluse/Types.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"
#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Math/Bounds3D.hpp"
#include "Recluse/Math/Bounds2D.hpp"
#include "Recluse/Math/Matrix44.hpp"

#include <vector>

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
    virtual ~DebugRenderer() { }
    DebugRenderer() : renderer(nullptr) { }

    // Get the debug renderer from the engine module. This is the current debug renderer.
    static DebugRenderer* get();

    void                setRenderer(Renderer* pRenderer) { renderer = pRenderer; }
    TemporaryBuffer     createTempBuffer(const TemporaryBufferDescription& description);
    GraphicsContext*    getContext();

    virtual void        drawText(U32 x, U32 y, F32 scale, const char* text, const Math::Color4& color) = 0;
    virtual void        drawText(const Math::Float3& position, const char* text, F32 scale, const Math::Color4& color) = 0;

    virtual void        drawBox(const Math::Bounds3d& bounds, const Math::Matrix44& transform, const Math::Color4& color, Bool solid = true) = 0;
    virtual void        drawPoint(const Math::Float3& position, F32 scale, const Math::Color4& color) = 0;
    virtual void        drawSphere(const Math::Float3& position, F32 radius, const Math::Color4& color) = 0;
    virtual void        drawLine(const Math::Float3& start, const Math::Float3& end, F32 scale, const Math::Color4& color) = 0;
    virtual void        drawRay(const Math::Float3& origin, const Math::Float3& direction, const Math::Color4& color) = 0;

    virtual void        drawMesh(const std::vector<Math::Float3>& vertices, const std::vector<U16>& indices) = 0;
private:
    Renderer* renderer;
};
} // Engine
} // Recluse