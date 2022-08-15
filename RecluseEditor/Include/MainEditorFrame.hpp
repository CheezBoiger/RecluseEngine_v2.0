//
#pragma once

#include "Recluse/RGUID.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {


class MainEditorFrame
{
public:
    void onInitialize();
    void onExit();

    void startUp();
    void tearDown();

private:

    // The unique id of the frame.
    RGUID m_guid;
};
} // Recluse