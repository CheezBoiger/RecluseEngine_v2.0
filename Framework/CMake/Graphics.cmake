# 
set ( RECLUSE_GRAPHICS_INCLUDE ${RECLUSE_FRAMEWORK_INCLUDE}/Recluse/Graphics )
set ( RECLUSE_GRAPHICS_SOURCE ${RECLUSE_FRAMEWORK_SOURCE}/Graphics )

set ( RECLUSE_GRAPHICS_BUILD
    ${RECLUSE_GRAPHICS_INCLUDE}/GraphicsInstance.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/CommandList.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/CommandHandler.hpp
    #${RECLUSE_GRAPHICS_INCLUDE}/CommandQueue.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/DescriptorSet.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/Format.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/GraphicsDevice.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/GraphicsAdapter.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/PipelineState.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/RenderPass.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/RenderPassMap.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/Resource.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/ResourceView.hpp
	${RECLUSE_GRAPHICS_INCLUDE}/ShaderProgram.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/Shader.hpp
    ${RECLUSE_GRAPHICS_SOURCE}/Shader.cpp
    ${RECLUSE_GRAPHICS_SOURCE}/ShaderMap.hpp
    ${RECLUSE_GRAPHICS_SOURCE}/ShaderMap.cpp
    ${RECLUSE_GRAPHICS_INCLUDE}/VideoMemoryPool.hpp
    ${RECLUSE_GRAPHICS_SOURCE}/DeviceFactory.cpp
    ${RECLUSE_GRAPHICS_SOURCE}/RenderPass.cpp
    ${RECLUSE_GRAPHICS_SOURCE}/RenderPassMap.cpp
	${RECLUSE_GRAPHICS_SOURCE}/ShaderProgram.cpp
	${RECLUSE_GRAPHICS_SOURCE}/LifetimeCache.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/GraphicsCommon.hpp
)



if ( RCL_VULKAN )
    add_definitions( -DRCL_VULKAN=1 )
    find_package( Vulkan )
    if (Vulkan_FOUND)
		message("Vulkan version: ${Vulkan_VERSION}")
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
            ${RECLUSE_VULKAN_DIR}/VulkanInstance.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanInstance.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanDevice.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanDevice.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanFrameResources.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanFrameResources.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanDescriptorManager.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanDescriptorManager.hpp
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
            ${RECLUSE_VULKAN_DIR}/VulkanRenderPass.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanPipelineState.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanPipelineState.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanDescriptorSets.cpp
            ${RECLUSE_VULKAN_DIR}/VulkanShaderCache.hpp
            ${RECLUSE_VULKAN_DIR}/VulkanShaderCache.cpp
        )
    else()
        message(FATAL_ERROR "Failed to find Vulkan library!")
    endif(Vulkan_FOUND)
endif()

if ( RCL_DX11 OR RCL_DX12 )
    set ( RECLUSE_FRAMEWORK_LINK_BINARIES ${RECLUSE_FRAMEWORK_LINK_BINARIES} dxgi.lib )
endif()

if ( RCL_DX11 )
	add_definitions( -DRCL_DX11=1 )
	set ( RECLUSE_FRAMEWORK_LINK_BINARIES ${RECLUSE_FRAMEWORK_LINK_BINARIES} d3d11.lib )
	set ( RECLUSE_D3D11_DIR ${RECLUSE_GRAPHICS_SOURCE}/D3D11Backend )
	set ( RECLUSE_GRAPHICS_BUILD 
		${RECLUSE_GRAPHICS_BUILD}
		${RECLUSE_D3D11_DIR}/D3D11Commons.hpp
		${RECLUSE_D3D11_DIR}/D3D11Device.hpp
		${RECLUSE_D3D11_DIR}/D3D11Device.cpp
		${RECLUSE_D3D11_DIR}/D3D11Instance.hpp
		${RECLUSE_D3D11_DIR}/D3D11Instance.cpp
	)
endif()

if ( RCL_DX12 )
    add_definitions( -DRCL_DX12=1 )
    set ( RECLUSE_FRAMEWORK_LINK_BINARIES ${RECLUSE_FRAMEWORK_LINK_BINARIES} d3d12.lib )
    set ( RECLUSE_D3D12_DIR ${RECLUSE_GRAPHICS_SOURCE}/D3D12Backend )
    set ( RECLUSE_GRAPHICS_BUILD 
        ${RECLUSE_GRAPHICS_BUILD}
        ${RECLUSE_D3D12_DIR}/D3D12Device.hpp
        ${RECLUSE_D3D12_DIR}/D3D12Instance.hpp
        ${RECLUSE_D3D12_DIR}/D3D12Instance.cpp
        ${RECLUSE_D3D12_DIR}/D3D12Adapter.hpp
        ${RECLUSE_D3D12_DIR}/D3D12Device.cpp
        ${RECLUSE_D3D12_DIR}/D3D12Adapter.cpp
        ${RECLUSE_D3D12_DIR}/D3D12Commons.hpp
		${RECLUSE_D3D12_DIR}/D3D12Commons.cpp
        ${RECLUSE_D3D12_DIR}/D3D12Swapchain.hpp
        ${RECLUSE_D3D12_DIR}/D3D12Swapchain.cpp
        ${RECLUSE_D3D12_DIR}/D3D12Queue.hpp
        ${RECLUSE_D3D12_DIR}/D3D12Queue.cpp
        ${RECLUSE_D3D12_DIR}/D3D12CommandList.hpp
        ${RECLUSE_D3D12_DIR}/D3D12CommandList.cpp
        ${RECLUSE_D3D12_DIR}/D3D12Allocator.hpp
        ${RECLUSE_D3D12_DIR}/D3D12Allocator.cpp
        ${RECLUSE_D3D12_DIR}/D3D12RenderPass.hpp
        ${RECLUSE_D3D12_DIR}/D3D12RenderPass.cpp
        ${RECLUSE_D3D12_DIR}/D3D12DescriptorTableManager.hpp
        ${RECLUSE_D3D12_DIR}/D3D12DescriptorTableManager.cpp
        ${RECLUSE_D3D12_DIR}/D3D12Resource.hpp
        ${RECLUSE_D3D12_DIR}/D3D12Resource.cpp
        ${RECLUSE_D3D12_DIR}/D3D12ResourceView.hpp
        ${RECLUSE_D3D12_DIR}/D3D12ResourceView.cpp
		${RECLUSE_D3D12_DIR}/D3D12PipelineState.hpp
		${RECLUSE_D3D12_DIR}/D3D12PipelineState.cpp
		${RECLUSE_D3D12_DIR}/D3D12ShaderCache.hpp
		${RECLUSE_D3D12_DIR}/D3D12ShaderCache.cpp
    )
endif()

set ( RECLUSE_FRAMEWORK_COMPILE_FILES
    ${RECLUSE_FRAMEWORK_COMPILE_FILES}
    ${RECLUSE_GRAPHICS_BUILD}
)