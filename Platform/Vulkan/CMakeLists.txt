# 
cmake_minimum_required(VERSION 3.16)
project("RecluseVulkan")

set ( RECLUSE_VULKAN_NAME "RecluseVulkan" )
set ( RECLUSE_VULKAN_MINOR 0 )
set ( RECLUSE_VULKAN_MAJOR 0 )
set ( RECLUSE_VULKAN_BUILD 0 )

set ( RECLUSE_VERSION_STRING "v${RECLUSE_VULKAN_MAJOR}.${RECLUSE_VULKAN_MINOR}.${RECLUSE_VULKAN_BUILD}" )

# Use this variable to include to the source library build.
set ( RECLUSE_VULKAN_COMPILE_FILES )
set ( RECLUSE_VULKAN_INCLUDE ${CMAKE_SOURCE_DIR}/Framework/Include )
set ( RECLUSE_VULKAN_SOURCE Source )
set ( RECLUSE_VULKAN_INCLUDE_FILES ${RECLUSE_VULKAN_INCLUDE} )
set ( RECLUSE_VULKAN_LINK_BINARIES )
set ( RECLUSE_VULKAN_THIRD_PARTY ${RECLUSE_THIRDPARTY_DIR} )


#if ( RCL_VULKAN )
    add_definitions( -DRCL_VULKAN=1 )
    find_package( Vulkan )
    if (Vulkan_FOUND)
		message("Vulkan version: ${Vulkan_VERSION}")
        set ( RECLUSE_VULKAN_LINK_BINARIES ${RECLUSE_VULKAN_LINK_BINARIES} ${Vulkan_LIBRARIES} )
        set ( RECLUSE_VULKAN_INCLUDE_FILES ${RECLUSE_VULKAN_INCLUDE_FILES} ${Vulkan_INCLUDE_DIRS} )
        set ( RECLUSE_VULKAN_DIR .)
        set ( RECLUSE_VULKAN_COMPILE_FILES
            ${RECLUSE_VULKAN_COMPILE_FILES}
			${RECLUSE_VULKAN_DIR}/VulkanLoaderApi.hpp
			${RECLUSE_VULKAN_DIR}/VulkanLoaderapi.cpp
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
#endif()


if (R_DEVELOPER)
	add_definitions(-DRECLUSE_DEVELOPER=1)
endif(R_DEVELOPER)

add_library (               ${RECLUSE_VULKAN_NAME} SHARED    ${RECLUSE_VULKAN_COMPILE_FILES} ) 
target_include_directories( ${RECLUSE_VULKAN_NAME} PUBLIC    ${RECLUSE_VULKAN_INCLUDE_FILES} ${RECLUSE_VULKAN_SOURCE} ${RECLUSE_VULKAN_THIRD_PARTY})
target_link_libraries (     ${RECLUSE_VULKAN_NAME}           ${RECLUSE_VULKAN_LINK_BINARIES} RecluseFramework )
add_dependencies( ${RECLUSE_VULKAN_NAME} RecluseFramework )
# Doing some stuff for organization.
if (MSVC)
  foreach(source IN LISTS RECLUSE_VULKAN_COMPILE_FILES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
  endforeach()
endif()

# Visual Studio organization. For Good purposes.
if (WIN32)
	#target_sources(${RECLUSE_VULKAN_NAME} PRIVATE Resource.rc resource.h)
	set_property(TARGET ${RECLUSE_VULKAN_NAME} 
		PROPERTY 
		FOLDER ${RECLUSE_VULKAN_NAME})
endif()

install(TARGETS ${RECLUSE_VULKAN_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_VULKAN_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_VULKAN_NAME}
        CONFIGURATIONS Debug
        ARCHIVE DESTINATION Lib)
install(TARGETS ${RECLUSE_VULKAN_NAME}
        CONFIGURATIONS Release
        ARCHIVE DESTINATION Lib)
   