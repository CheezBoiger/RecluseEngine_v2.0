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
	${RECLUSE_GRAPHICS_INCLUDE}/ShaderProgramBuilder.hpp
    ${RECLUSE_GRAPHICS_INCLUDE}/Shader.hpp
    ${RECLUSE_GRAPHICS_SOURCE}/Shader.cpp
    ${RECLUSE_GRAPHICS_SOURCE}/ShaderMap.hpp
    ${RECLUSE_GRAPHICS_SOURCE}/ShaderMap.cpp
    ${RECLUSE_GRAPHICS_INCLUDE}/VideoMemoryPool.hpp
    ${RECLUSE_GRAPHICS_SOURCE}/DeviceFactory.cpp
    ${RECLUSE_GRAPHICS_SOURCE}/RenderPass.cpp
    ${RECLUSE_GRAPHICS_SOURCE}/RenderPassMap.cpp
	${RECLUSE_GRAPHICS_SOURCE}/ShaderProgram.hpp
	${RECLUSE_GRAPHICS_SOURCE}/ShaderProgram.cpp
    ${RECLUSE_GRAPHICS_INCLUDE}/ShaderBuilder.hpp
    ${RECLUSE_GRAPHICS_SOURCE}/DXCShaderBuilder.cpp
    ${RECLUSE_GRAPHICS_SOURCE}/GlslangShaderBuilder.cpp
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
		if ( RCL_GLSLANG OR R_GLSLANG_LEGACY_API )
			add_definitions ( -DRCL_GLSLANG=1 )
			if ( R_GLSLANG_LEGACY_API )
				add_definitions( -DR_GLSLANG_LEGACY_API=1 )
			else()
				add_definitions( -DR_GLSLANG_LEGACY_API=0 )
			endif()
			set ( VULKAN_GLSLANG_LIBRARY_RELEASE optimized $ENV{VULKAN_SDK}/Lib/glslang.lib 
									 optimized $ENV{VULKAN_SDK}/Lib/shaderc.lib
									 optimized $ENV{VULKAN_SDK}/Lib/shaderc_util.lib
									 optimized $ENV{VULKAN_SDK}/Lib/GenericCodeGen.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/SPIRV.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/HLSL.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/OGLCompiler.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/OSDependent.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/SPIRV-Tools.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/SPIRV-Tools-link.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/SPIRV-Tools-opt.lib)
			message ( WARNING "Windows: you will need to also Download Vulkan SDK DebugShaderLibs in order to use glslang debug libs")
			set ( VULKAN_GLSLANG_LIBRARY_DEBUG debug $ENV{VULKAN_SDK}/Lib/glslangd.lib 
                                     debug $ENV{VULKAN_SDK}/Lib/shadercd.lib
									 debug $ENV{VULKAN_SDK}/Lib/shaderc_utild.lib
									 debug $ENV{VULKAN_SDK}/Lib/GenericCodeGend.lib
									 debug $ENV{VULKAN_SDK}/Lib/SPIRVd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/HLSLd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/OGLCompilerd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/OSDependentd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/SPIRV-Toolsd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/SPIRV-Tools-linkd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/SPIRV-Tools-optd.lib)
			if ( NOT R_GLSLANG_LEGACY_API )
				set ( VULKAN_GLSLANG_LIBRARY_DEBUG ${VULKAN_GLSLANG_LIBRARY_DEBUG} debug $ENV{VULKAN_SDK}/Lib/MachineIndependentd.lib)
				set ( VULKAN_GLSLANG_LIBRARY_RELEASE ${VULKAN_GLSLANG_LIBRARY_RELEASE} optimized $ENV{VULKAN_SDK}/Lib/MachineIndependent.lib)
			endif()
			set ( VULKAN_GLSLANG_LIBRARIES ${VULKAN_GLSLANG_LIBRARY_RELEASE} ${VULKAN_GLSLANG_LIBRARY_DEBUG} )
			set ( RECLUSE_FRAMEWORK_LINK_BINARIES ${RECLUSE_FRAMEWORK_LINK_BINARIES} ${VULKAN_GLSLANG_LIBRARIES} )
		endif()
    else()
        message(FATAL_ERROR "Failed to find Vulkan library!")
    endif(Vulkan_FOUND)
endif()

if ( RCL_DX12 )
    add_definitions( -DRCL_DX12=1 )
    set ( RECLUSE_FRAMEWORK_LINK_BINARIES ${RECLUSE_FRAMEWORK_LINK_BINARIES} d3d12.lib )
    set ( RECLUSE_FRAMEWORK_LINK_BINARIES ${RECLUSE_FRAMEWORK_LINK_BINARIES} dxgi.lib )
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
    )
    if ( RCL_DXC )
        set ( RECLUSE_FRAMEWORK_LINK_BINARIES ${RECLUSE_FRAMEWORK_LINK_BINARIES} dxcompiler.lib )
        add_definitions( -DRCL_DXC=1 )
        message(WARNING "dxcompiler.dll and dxil.dll needed with executable, since we are now including dxc...")
        
    endif()
endif()

set ( RECLUSE_FRAMEWORK_COMPILE_FILES
    ${RECLUSE_FRAMEWORK_COMPILE_FILES}
    ${RECLUSE_GRAPHICS_BUILD}
)