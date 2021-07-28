//
#include "VulkanCommons.hpp"

namespace Vulkan {


VkShaderStageFlags getShaderStages(Recluse::ShaderTypeFlags flags)
{
    VkShaderStageFlags vkFlags = 0;

    if (flags == Recluse::SHADER_TYPE_ALL) {
        vkFlags = VK_SHADER_STAGE_ALL;
    } else {
        if (flags & Recluse::SHADER_TYPE_PIXEL) {
            vkFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if (flags & Recluse::SHADER_TYPE_VERTEX) {
            vkFlags |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (flags & Recluse::SHADER_TYPE_HULL) {
            vkFlags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if (flags & Recluse::SHADER_TYPE_DOMAIN) {
            vkFlags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }
        if (flags & Recluse::SHADER_TYPE_COMPUTE) {
            vkFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
    }

    return vkFlags;
}
} // Vulkan