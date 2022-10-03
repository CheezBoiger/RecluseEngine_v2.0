//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Math/Matrix44.hpp"
#include "Recluse/Math/Vector4.hpp"

#define RECLUSE_DECLARE_SCENE_BUFFER(structName) struct structName {
#define RECLUSE_END_SCENE_BUFFER() }

#define RECLUSE_SCENE_PARAMETER(name, type) type name

namespace Recluse {
namespace Engine {

//< Recluse per scene view.
//< Buffer should be used as global scene information, in regards to multiple
// uses in render scene.
RECLUSE_DECLARE_SCENE_BUFFER(RecluseSceneView)
    RECLUSE_SCENE_PARAMETER(View,                   Math::Matrix44);
    RECLUSE_SCENE_PARAMETER(Clip,                   Math::Matrix44);
    RECLUSE_SCENE_PARAMETER(ViewToClip,             Math::Matrix44);
    RECLUSE_SCENE_PARAMETER(ClipToView,             Math::Matrix44);
    RECLUSE_SCENE_PARAMETER(EngineTime,             F32);
    RECLUSE_SCENE_PARAMETER(FixedTime,              F32);
    RECLUSE_SCENE_PARAMETER(CameraJitter,           Math::Float2);
    RECLUSE_SCENE_PARAMETER(CameraPosition,         Math::Float3);
    RECLUSE_SCENE_PARAMETER(DeltaTime,              F32);
    RECLUSE_SCENE_PARAMETER(CameraDirection,        Math::Float3);
    RECLUSE_SCENE_PARAMETER(ScreenWidth,            F32);
    RECLUSE_SCENE_PARAMETER(ScreenHeight,           F32);
    RECLUSE_SCENE_PARAMETER(InverseScreenWidth,     F32);
    RECLUSE_SCENE_PARAMETER(InverseScreenHeight,    F32);
    RECLUSE_SCENE_PARAMETER(Exposure,               F32);
    RECLUSE_SCENE_PARAMETER(Gamma,                  F32);
    RECLUSE_SCENE_PARAMETER(EnableBump,             F32);
    RECLUSE_SCENE_PARAMETER(Near,                   F32);
    RECLUSE_SCENE_PARAMETER(Far,                    F32);
    RECLUSE_SCENE_PARAMETER(SunPosition,            Math::Float4);
RECLUSE_END_SCENE_BUFFER();

using namespace Recluse::Math;

struct StaticVertex 
{
    Float3 vPosition;
    Float3 vNormal;
    Float3 vTangent;
    Float3 vBinormal;
    Float4 vTexCoords;
};


struct SkinnedVertex 
{
    Float3 vPosition;
    Float3 vNormal;
    Float3 vTangent;
    Float3 vBinormal;
    Float4 vTexCoords;
    Float4 vBoneWeights;
    UInt4  vBoneIndices;
};


struct StaticMorph 
{
    Float3 vPosition;
    Float3 vNormal;
};
} // Engine
} // Recluse