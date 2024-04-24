# 
cmake_minimum_required(VERSION 3.16)
project("RecluseD3D12")

set ( RECLUSE_D3D12_NAME "RecluseD3D12" )
set ( RECLUSE_D3D12_MINOR 0 )
set ( RECLUSE_D3D12_MAJOR 0 )
set ( RECLUSE_D3D12_BUILD 0 )

set ( RECLUSE_VERSION_STRING "v${RECLUSE_D3D12_MAJOR}.${RECLUSE_D3D12_MINOR}.${RECLUSE_D3D12_BUILD}" )

# Use this variable to include to the source library build.
set ( RECLUSE_D3D12_COMPILE_FILES )
set ( RECLUSE_D3D12_INCLUDE ${CMAKE_SOURCE_DIR}/Framework/Include ${CMAKE_SOURCE_DIR}/Framework/Source )
set ( RECLUSE_D3D12_SOURCE Source )
set ( RECLUSE_D3D12_INCLUDE_FILES ${RECLUSE_D3D12_INCLUDE} )
set ( RECLUSE_D3D12_LINK_BINARIES )
set ( RECLUSE_D3D12_THIRD_PARTY ${RECLUSE_THIRDPARTY_DIR} )


if (R_DEVELOPER)
	add_definitions(-DRECLUSE_DEVELOPER=1)
endif(R_DEVELOPER)

#if ( RCL_DX12 )
    add_definitions( -DRCL_DX12=1 )
    set ( RECLUSE_D3D12_LINK_BINARIES ${RECLUSE_D3D12_LINK_BINARIES} d3d12.lib dxgi.lib )
	
    set ( RECLUSE_D3D12_DIR .)
    set ( RECLUSE_D3D12_COMPILE_FILES 
        ${RECLUSE_D3D12_COMPILE_FILES}
		${RECLUSE_D3D12_DIR}/D3D12LoaderApi.hpp
		${RECLUSE_D3D12_DIR}/D3D12LoaderApi.cpp
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
#endif()

add_library (               ${RECLUSE_D3D12_NAME} SHARED    ${RECLUSE_D3D12_COMPILE_FILES} ) 
target_include_directories( ${RECLUSE_D3D12_NAME} PUBLIC    ${RECLUSE_D3D12_INCLUDE_FILES} ${RECLUSE_D3D12_SOURCE} ${RECLUSE_D3D12_THIRD_PARTY})
target_link_libraries (     ${RECLUSE_D3D12_NAME}           ${RECLUSE_D3D12_LINK_BINARIES} RecluseFramework)
add_dependencies( ${RECLUSE_D3D12_NAME} RecluseFramework )
# Doing some stuff for organization.
if (MSVC)
  foreach(source IN LISTS RECLUSE_D3D12_COMPILE_FILES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
  endforeach()
endif()

# Visual Studio organization. For Good purposes.
if (WIN32)
	#target_sources(${RECLUSE_D3D12_NAME} PRIVATE Resource.rc resource.h)
	set_property(TARGET ${RECLUSE_D3D12_NAME} 
		PROPERTY 
		FOLDER ${RECLUSE_D3D12_NAME})
endif()

install(TARGETS ${RECLUSE_D3D12_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_D3D12_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_D3D12_NAME}
        CONFIGURATIONS Debug
        ARCHIVE DESTINATION Lib)
install(TARGETS ${RECLUSE_D3D12_NAME}
        CONFIGURATIONS Release
        ARCHIVE DESTINATION Lib)
   