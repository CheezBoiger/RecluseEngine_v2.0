# 
set ( RECLUSE_GRAPHICS_INCLUDE ${RECLUSE_FRAMEWORK_INCLUDE}/Graphics )
set ( RECLUSE_GRAPHICS_SOURCE ${RECLUSE_FRAMEWORK_SOURCE}/Graphics )

set ( RECLUSE_GRAPHICS_BUILD
    ${RECLUSE_GRAPHICS_INCLUDE}/GraphicsContext.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/CommandBuffer.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/CommandHandler.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/CommandQueue.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/DescriptorSet.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/Format.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/GraphicsDevice.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/GraphicsAdapter.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/PipelineState.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/RenderPass.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/RenderPassMap.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/Resource.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/ResourceView.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/Shader.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/ShaderMap.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/VideoMemoryPool.hpp
    ${RECLUSE_GRAPHICS_SOURCE}/DeviceFactory.cpp
    ${RECLUSE_GRAPHICS_SOURCE}/RenderPass.cpp
    ${RECLUSE_GRAPHICS_SOURCE}/RenderPassMap.cpp
    ${RECLUSE_GRAPHICS_SOURCE}/ShaderBuilder.hpp
)



if ( RCL_VULKAN )
    find_package( Vulkan )
    if (Vulkan_FOUND)
        set ( RECLUSE_FRAMEWORK_LINK_BINARIES ${RECLUSE_FRAMEWORK_LINK_BINARIES} ${Vulkan_LIBRARIES} )
        set ( RECLUSE_FRAMEWORK_INCLUDE_FILES ${RECLUSE_FRAMEWORK_INCLUDE_FILES} ${Vulkan_INCLUDE_DIRS} )
        set ( RECLUSE_VULKAN_DIR ${RECLUSE_GRAPHICS_SOURCE}/VulkanBackend )
        set ( RECLUSE_GRAPHICS_BUILD
            ${RECLUSE_GRAPHICS_BUILD}
            ${RECLUSE_VULKAN_DIR}/VulkanAdapter.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanAdapter.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanAllocator.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanAllocator.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanCommons.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanContext.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanContext.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanDevice.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanDevice.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanFrameResources.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanFrameResources.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanLogicalDevice.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanObjects.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanResource.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanResource.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanSwapchain.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanSwapchain.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanQueue.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanQueue.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanCommons.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanViews.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanViews.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanCommandList.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanCommandList.cpp
        )   
    else()
        message(FATAL_ERROR "Failed to find Vulkan library!")
    endif(Vulkan_FOUND)
endif()

set ( RECLUSE_FRAMEWORK_COMPILE_FILES
    ${RECLUSE_FRAMEWORK_COMPILE_FILES}
    ${RECLUSE_GRAPHICS_BUILD}
)