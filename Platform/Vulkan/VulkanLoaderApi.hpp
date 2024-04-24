//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"

extern "C" {
R_PUBLIC_API Recluse::GraphicsInstance* createInstance();
R_PUBLIC_API Recluse::ResultCode destroyInstance(Recluse::GraphicsInstance* instance);
} // extern C