//
#pragma once

#include "Recluse/Arch.hpp"
#include "Recluse/Graphics/GraphicsInstance.hpp"

#include "RecluseVulkan_exports.hpp"

extern "C" {
RecluseVulkan_PUBLIC_API Recluse::GraphicsInstance* createInstance();
RecluseVulkan_PUBLIC_API Recluse::ResultCode destroyInstance(Recluse::GraphicsInstance* instance);
} // extern C