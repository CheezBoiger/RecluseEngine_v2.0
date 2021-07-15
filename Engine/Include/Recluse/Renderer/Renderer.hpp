//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {

class GraphicsDevice;
class GraphicsAdapter;
class GraphicsContext;

namespace Engine {


class R_EXPORT Renderer {
public:
    void initialize();
    void cleanUp();

    void render();
    void present();

private:
    GraphicsContext* m_pContext;
    GraphicsAdapter* m_pAdapter;
    GraphicsDevice* m_pDevice;
};
} // Engine
} // Recluse