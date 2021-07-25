//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Vector4.hpp"

#define RECLUSE_DECLARE_SCENE_BUFFER(structName) struct structName {
#define RECLUSE_END_SCENE_BUFFER() }

#define RECLUSE_SCENE_PARAMETER(name, type) type name

namespace Recluse {

//< Recluse per scene view.
//< Buffer should be used as global scene information, in regards to multiple
// uses in render scene.
RECLUSE_DECLARE_SCENE_BUFFER(RecluseSceneView)
    RECLUSE_SCENE_PARAMETER(view,               Matrix44);
    RECLUSE_SCENE_PARAMETER(clip,               Matrix44);
    RECLUSE_SCENE_PARAMETER(viewToClip,         Matrix44);
    RECLUSE_SCENE_PARAMETER(clipToView,         Matrix44);
    RECLUSE_SCENE_PARAMETER(engineTime,         F32);
    RECLUSE_SCENE_PARAMETER(fixedTime,          F32);
    RECLUSE_SCENE_PARAMETER(cameraJitter,       Float2);
    RECLUSE_SCENE_PARAMETER(cameraPosition,     Float3);
    RECLUSE_SCENE_PARAMETER(screenWidth,        F32);
    RECLUSE_SCENE_PARAMETER(screenHeight,       F32);
    RECLUSE_SCENE_PARAMETER(exposure,           F32);
    RECLUSE_SCENE_PARAMETER(gamma,              F32);
    RECLUSE_SCENE_PARAMETER(enableBump,         F32);
    RECLUSE_SCENE_PARAMETER(sunPosition,        Float4);
    RECLUSE_SCENE_PARAMETER(near,               F32);
    RECLUSE_SCENE_PARAMETER(far,                F32);
    RECLUSE_SCENE_PARAMETER()
RECLUSE_END_SCENE_BUFFER();


// Per mesh information.
struct PerMesh {
    Matrix44 world;
    Matrix44 worldToViewClip;
    Matrix44 n;
};
} // Recluse