//
#pragma once

#include "Recluse/Math/Bounds3D.hpp"
#include "Recluse/Math/Matrix44.hpp"

#include "Recluse/Math/Frustum.hpp"

#include "Recluse/Renderer/SceneView.hpp"

namespace Recluse {
namespace Engine {


// Camera is the abstract transformation that is used to view the scene.
// It also serves to perform visibility culling.
class Camera
{
public:


    // Update the camera transformations.
    void update();

    // Obtain the camera view frustum.
    const Frustum& getFrustum() const { return m_frustum; }

private:
    RecluseSceneView m_sceneView;
    Frustum          m_frustum;

    // View-Projection transform.
    Matrix44         m_ViewProjection;
    // Projection transform.
    Matrix44         m_Projection;

    Matrix44         m_InverseViewProjection;
    Matrix44         m_InverseProjection;
    Matrix44         m_WorldTransform;
};

} // Engine
} // Recluse