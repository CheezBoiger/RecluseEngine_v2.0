//
#pragma once

#include "Recluse/Game/Component.hpp"
#include "Recluse/Math/Bounds3D.hpp"
#include "Recluse/Math/Matrix44.hpp"

#include "Recluse/Math/Frustum.hpp"

#include "Recluse/Renderer/SceneView.hpp"
#include "Recluse/Game/Components/Transform.hpp"

#include <map>
#include <vector>

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
    CameraPostProcessFlag_EnableTonemap                 = (1 << 10),

    CameraPostProcessFlag_All = 0xffffffff
};


enum CameraProjection
{
    CameraProjection_Perspective,
    CameraProjection_Orthographic
};

typedef U32 CameraPostProcessFlags;

// Camera is the abstract transformation that is used to view the scene.
// It also serves to perform visibility culling.
class R_PUBLIC_API Camera : public ECS::Component
{
public:
    R_COMPONENT_DECLARE(Camera);

    // Update the camera transformations. Usually the camera will
    // just need the transform in order to apply it's works.
    void            update(const Transform* transform);

    F32     getAspect() const { return m_aspect; }
    F32     getFov() const { return m_fov; }
    CameraProjection getProjectionMode() const { return m_projectionMode; }

    // Obtain the camera view frustum.
    const Math::Frustum&  getFrustum() const { return m_frustum; }

    // Set the post process flags for this camera.
    void setPostProcessFlags(CameraPostProcessFlags flags)
    {
        m_postProcessFlags = flags;
        m_updateFlags |= CameraUpdate_PostProcess;
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
        m_updateFlags |= CameraUpdate_Projection;
    }

    void setProjectionMode(CameraProjection projectionMode) 
    { 
        if (m_projectionMode != projectionMode) 
        {
            m_projectionMode = projectionMode;
            m_updateFlags |= CameraUpdate_Projection;
        }
    }

    Math::Matrix44 getProjection() const { return m_Projection; }
    Math::Matrix44 getViewProjection() const { return m_ViewProjection; }
    Math::Matrix44 getInverseProjection() const { return m_InverseProjection; }
    Math::Matrix44 getInverseViewProjection() const { return m_InverseViewProjection; }
    Math::Matrix44 getInverseView() const { return m_InverseView; }

    // Projects screen coordinates to world coordinates.    
    Math::Float3    screenToWorldPoint(U32 screenX, U32 screenY);

private:
    void clearCameraUpdateFlags() { m_updateFlags = 0; }
    void clearPostProcessFlags() { m_postProcessFlags = 0; }

    enum CameraUpdateFlag
    {
        CameraUpdate_View = (1 << 0),
        CameraUpdate_Projection = (1 << 1),
        CameraUpdate_Frustum = (1 << 2),
        CameraUpdate_PostProcess = (1 << 3)
    };

    typedef U32 CameraUpdateFlags;

    RecluseSceneView        m_sceneView;
    Math::Frustum           m_frustum;

    // View-Projection transform.
    Math::Matrix44          m_ViewProjection;
    // Projection transform.
    Math::Matrix44          m_Projection;
    Math::Matrix44          m_View;

    Math::Matrix44          m_InverseViewProjection;
    Math::Matrix44          m_InverseProjection;
    Math::Matrix44          m_InverseView;

    CameraPostProcessFlags  m_postProcessFlags;
    CameraUpdateFlags       m_updateFlags;
    CameraProjection        m_projectionMode;
    F32                     m_fov;
    F32                     m_aspect;
    F32                     m_near;
    F32                     m_far;
};


class CameraRegistry : public ECS::Registry<Camera>
{
public:
    R_COMPONENT_REGISTRY_DECLARE(CameraRegistry);

    // Get the main view camera.
    Camera*             getMain() { return mainCamera; }

    // Set the main view camera.
    void                setMain(Camera* main) { mainCamera = main; }

    // Allocation calls. These must be overridden, as they will be called by external systems,
    // when required. 
    virtual ResultCode  onAllocateComponent(const RGUID& owner) override;

    // Free calls. These must be overridden, as they will be called by external systems when
    // required.
    virtual ResultCode  onFreeComponent(const RGUID& owner) override;
    virtual Camera*     getComponent(const RGUID& entityKey) override;

    virtual std::vector<Camera*> getAllComponents() override { return std::vector<Camera*>(activeCameras); }

private:
    Camera* mainCamera;
    
    std::map<RGUID, U32, RGUID::Less>   cameraMap;
    std::vector<Camera*>                activeCameras;
};
} // Engine
} // Recluse