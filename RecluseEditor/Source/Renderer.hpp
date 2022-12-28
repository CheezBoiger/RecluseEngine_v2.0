// 
#pragma once

#include "Recluse/Graphics/GraphicsDevice.hpp"
#include "Recluse/Graphics/GraphicsCommon.hpp"

#include "Recluse/Math/Bounds3D.hpp"

#include "Recluse/System/Window.hpp"

namespace Recluse {
namespace Editor {

struct DrawCommand
{
    Math::Bounds3d bounds;
};


class EditorRenderer
{
public:
    void                initialize();
    void                shutdown();

    bool                loadLiveRenderer();
    bool                shutdownLiveRenderer();

private:
    GraphicsDevice*     m_pDevice;
    GraphicsContext*    m_pContext;
    GraphicsAdapter*    m_pAdapter;


};

} // Editor
} // Recluse