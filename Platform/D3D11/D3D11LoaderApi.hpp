//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"

#include "RecluseD3D11_exports.hpp"

extern "C" {
RecluseD3D11_PUBLIC_API Recluse::GraphicsInstance* createInstance();
RecluseD3D11_PUBLIC_API Recluse::ResultCode destroyInstance(Recluse::GraphicsInstance* instance);
} // extern C