// 
#include "Core/Types.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanContext.hpp"
#include "Core/Messaging.hpp"


namespace Recluse {


GraphicsResource* VulkanSwapchain::getFrame(U32 idx)
{
    return nullptr;
}


GraphicsResourceView* VulkanSwapchain::getFrameView(U32 idx)
{
    return nullptr;
}


ErrType VulkanSwapchain::build(VkDevice device, VulkanContext* pContext, const SwapchainCreateDescription* pDesc)
{
    VkSwapchainCreateInfoKHR createInfo = { };
    VkResult result                     = VK_SUCCESS;

    ErrType builtSurface = createSurface(pContext->get(), pDesc->winHandle);

    if (builtSurface != REC_RESULT_OK) {
        R_ERR(R_CHANNEL_VULKAN, "Surface failed to create! Aborting build");
        return builtSurface;
    }
    
    // TODO: We need to pass a struct input for desired configurations.
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.clipped          = VK_TRUE;
    createInfo.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageFormat      = VK_FORMAT_B8G8R8A8_SRGB;
    createInfo.surface          = m_surface;
    createInfo.imageExtent      = { pDesc->renderWidth, pDesc->renderHeight };
    createInfo.oldSwapchain     = (m_swapchain) ? m_swapchain : VK_NULL_HANDLE;

    result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain);

    if (result != VK_SUCCESS) {
        
        R_ERR(R_CHANNEL_VULKAN, "Failed to create swapchain!");

        return REC_RESULT_FAILED;
    }    

    return REC_RESULT_OK;
}


ErrType VulkanSwapchain::createSurface(VkInstance instance, void* handle)
{
    VkResult result             = VK_SUCCESS;
    if (!handle) {

        return REC_RESULT_NULL_PTR_EXCEPTION;
    }

#if defined(RECLUSE_WINDOWS)
    VkWin32SurfaceCreateInfoKHR createInfo = { };

    createInfo.sType            = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance        = GetModuleHandle(NULL);
    createInfo.hwnd             = (HWND)handle;
    createInfo.pNext            = nullptr;
    createInfo.flags            = 0;            // future use, not needed.

    result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &m_surface);
#endif

    return REC_RESULT_OK;    
}


ErrType VulkanSwapchain::rebuild(const GraphicsContext* pContext, const SwapchainCreateDescription* pDesc)
{
    return REC_RESULT_NOT_IMPLEMENTED;
}
} // Recluse