//
#pragma once

#include "Core/Arch.hpp"

#if defined (RECLUSE_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#include <vulkan/vulkan.h>

#define R_CHANNEL_VULKAN "Vulkan"