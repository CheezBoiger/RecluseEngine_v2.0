//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"

#include "RecluseD3D12_exports.hpp"

extern "C" {
RecluseD3D12_PUBLIC_API Recluse::GraphicsInstance* createInstance();
RecluseD3D12_PUBLIC_API Recluse::ResultCode destroyInstance(Recluse::GraphicsInstance* instance);
} // extern C