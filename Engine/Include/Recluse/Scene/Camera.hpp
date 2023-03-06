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
class R_PUBLIC_API Camera
{
public:
    // Get the main view camera.
    static Camera* getMain();

    // Update the camera transformations. Usually the camera will
    // just need the transform in order to apply it's works.
    void            updateView(const Transform* transform);

    // Obtain the camera view frustum.
    const Math::Frustum&  getFrustum() const { return m_frustum; }

    // Set the post process flags for this camera.
    void setPostProcessFlags(CameraPostProcessFlags flags)
    {
        m_postProcessFlags = flags;
        m_needsUpdate = true;
    }

    // Check if we intersect the camera frustum.
    Bool intersects(const Math::Bounds3d& aabb)
    {
        return Math::intersects(m_frustum, aabb);
    }

    // Get the camera post process flags.
    CameraPostProcessFlags getPostProcessFlags() const 
    { 
        return m_postProcessFlags; 
    }

    void setProjection(const Math::Matrix44& projection)
    {
        m_Projection = projection;
        m_needsUpdate = true;
    }

    Math::Matrix44 getProjection() const { return m_Projection; }
    Math::Matrix44 getViewProjection() const { return m_ViewProjection; }

private:
    RecluseSceneView        m_sceneView;
    Math::Frustum           m_frustum;

    // View-Projection transform.
    Math::Matrix44          m_ViewProjection;
    // Projection transform.
    Math::Matrix44          m_Projection;

    Math::Matrix44          m_InverseViewProjection;
    Math::Matrix44          m_InverseProjection;

    CameraPostProcessFlags  m_postProcessFlags;
    Bool                    m_needsUpdate;
    Float3                  m_position;
    Quaternion              m_rotation;
};

} // Engine
} // Recluse