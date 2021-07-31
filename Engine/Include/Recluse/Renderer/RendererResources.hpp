//
#pragma once

#include "Recluse/Graphics/Resource.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"


namespace Recluse {
namespace Engine {

class Texture2D;

struct SceneBuffers {
    Texture2D* pSceneAlbedo;
    Texture2D* pSceneAo;
    Texture2D* pSceneMat;
    Texture2D* pSceneNormal;
    Texture2D* pSceneBrightness;
    Texture2D* pSceneHDRTexture;
    Texture2D* pSceneDepth;
};
} // Engine
} // Recluse