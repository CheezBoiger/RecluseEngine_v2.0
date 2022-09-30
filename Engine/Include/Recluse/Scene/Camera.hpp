//
#pragma once

#include "Recluse/Math/Bounds3D.hpp"
#include "Recluse/Math/Matrix44.hpp"

#include "Recluse/Math/Frustum.hpp"

#include "Recluse/Renderer/SceneView.hpp"
#include "Recluse/Game/Components/Transform.hpp"

namespace Recluse {
namespace Engine {


enum CameraPostProcessFlagBits
{
    CameraPostProcessFlag_None = 0,

    CameraPostProcessFlag_EnableEyeAdaptation           = (1 << 0),
    CameraPostProcessFlag_EnableColorGrading            = (1 << 1),
    CameraPostProcessFlag_EnableBrightness              = (1 << 2),
    CameraPostProcessFlag_EnableGamma                   = (1 << 3),
    CameraPostProcessFlag_EnableLensFlare               = (1 << 4),
    CameraPostProcessFlag_EnableGlare                   = (1 << 5),
    CameraPostProcessFlag_EnableBloom                   = (1 << 6),
    CameraPostProcessFlag_EnableVignette                = (1 << 7),
    CameraPostProcessFlag_EnableChromaticAbberration    = (1 << 8),
    CameraPostProcessFlag_EnableLensWarping             = (1 << 9),

    CameraPostProcessFlag_All = 0xffffffff
};

typedef U32 CameraPostProcessFlags;

// Camera is the abstract transformation that is used to view the scene.
// It also serves to perform visibility culling.
class Camera
{
public:

    // Update the camera transformations. Usually the camera will
    // just need the transform in order to apply it's works.
    void            update(const Transform* transform);

    // Obtain the camera view frustum.
    const Frustum&  getFrustum() const { return m_frustum; }

    // Set the post process flags for this camera.
    void setPostProcessFlags(CameraPostProcessFlags flags)
    {
        m_postProcessFlags = flags;
    }

    // Check if we intersect the camera frustum.
    Bool intersects(const Bounds3d& aabb)
    {
        return Recluse::intersects(m_frustum, aabb);
    }

    // Get the camera post process flags.
    CameraPostProcessFlags getPostProcessFlags() const 
    { 
        return m_postProcessFlags; 
    }

private:
    RecluseSceneView        m_sceneView;
    Frustum                 m_frustum;

    // View-Projection transform.
    Matrix44                m_ViewProjection;
    // Projection transform.
    Matrix44                m_Projection;

    Matrix44                m_InverseViewProjection;
    Matrix44                m_InverseProjection;

    CameraPostProcessFlags  m_postProcessFlags;
};

} // Engine
} // Recluse