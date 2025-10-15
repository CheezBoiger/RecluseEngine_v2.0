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

class RendererModule;



// Debug Renderer system is an object that encapsulates the actual renderer, this would be used to only expose needed portions
// for debug rendering.
class DebugRenderer
{
public:
    enum LitType
    {
        // Mesh is flat lit.
        LitType_Flat,
        // Mesh is shaded lit.
        LitType_Shaded,
        // Mesh is wireframe.
        LitType_Wireframe,
        // Mesh is only drawn as outlines.
        LitType_Outline,
    };

    virtual ~DebugRenderer() { }
    DebugRenderer() : renderer(nullptr) { }

    // Get the debug renderer from the engine module. This is the current debug renderer.
    static DebugRenderer* get();

    void                setRenderer(RendererModule* pRenderer) { renderer = pRenderer; }
    TemporaryBuffer     createTempBuffer(const TemporaryBufferDescription& description);
    GraphicsContext*    getContext();

    // Draw text in screen space.
    virtual void        drawText(U32 x, U32 y, F32 scale, const char* text, const Math::Color4& color) = 0;

    // Draw text in 3d world position. 
    virtual void        drawText(const Math::Float3& position, const char* text, F32 scale, const Math::Color4& color) = 0;

    // Draw a box in the world.
    virtual void        drawBox(const Math::Bounds3d& bounds, const Math::Matrix44& transform, const Math::Color4& color, LitType lit = LitType_Flat) = 0;

    // Draw a point in the world.
    virtual void        drawPoint(const Math::Float3& position, F32 scale, const Math::Color4& color) = 0;

    // Draw a debug sphere.
    // \param position The position of the sphere in worldspace.
    // \param radius The radius of the sphere, this determines its the size.
    // \\param color The color of the sphere.
    virtual void        drawSphere(const Math::Float3& position, F32 radius, const Math::Color4& color, LitType lit = LitType_Flat) = 0;

    // Draw line in the world. 
    // \param start = start position in worldspace coordinates
    // \param end = end position in worldspace coordinates
    // \param scale = the width of the line in scale values.
    // \param color = the color of the line.
    virtual void        drawLine(const Math::Float3& start, const Math::Float3& end, F32 scale, const Math::Color4& color) = 0;

    // Draw a triangle in the world.
    // \param v0 vertex 0 of the triangle
    // \param v1 vertex 1 of the triangle
    // \param v2 vertex 2 of the triangle
    // \param clockwise Whether the triangle should be drawn clockwise order or counter clockwise order.
    virtual void        drawTriangle(const Math::Float3& v0, const Math::Float3& v1, const Math::Float3& v2, bool clockwise) = 0;

    // Draw a ray in the world, with the origin starting position, and carrying on with the direction.
    // \param origin The origin of the ray in worldspace
    // \param direction The direction of the ray in worldspace, where it points to, along with the range.
    // \param color The color of the ray.
    virtual void        drawRay(const Math::Float3& origin, const Math::Float3& direction, const Math::Color4& color) = 0;

    // Draw a debug mesh in the world, with a given topology, world transform and type.
    // \param topology The topology of the mesh.
    // \param worldTransform The world transform for the mesh
    // \param vertices The vertices of the mesh
    // \param indices The indices of the mesh.
    // \param color The color of the mesh
    // \param debugType The Lit type of the mesh.
    virtual void        drawMesh(PrimitiveTopology topology, 
                            const Math::Mat44& worldTransform, 
                            const std::vector<Math::Float3>& vertices, 
                            const std::vector<U16>& indices, 
                            const Math::Color4& color,
                            LitType debugType = LitType_Flat) = 0;

    // Render the given debug information that was passed.
    virtual void        render() = 0;

private:
    RendererModule* renderer;
};
} // Engine
} // Recluse