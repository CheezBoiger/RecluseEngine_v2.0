//
#pragma once

#include "Recluse/RGUID.hpp"
#include "Recluse/Types.hpp"

namespace Recluse {
namespace Editor {

class MainEditorFrame
{
public:

    void bind();

private:

    // The unique id of the frame.
    RGUID m_guid;
};
} // Editor
} // Recluse