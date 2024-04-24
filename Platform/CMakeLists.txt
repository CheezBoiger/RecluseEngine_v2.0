# CMake Entry point build
cmake_minimum_required( VERSION 3.16 )

set ( RECLUSE_DIRECTORY ${CMAKE_SOURCE_DIR}/Recluse )
set ( CMAKE_INSTALL_PREFIX ${RECLUSE_DIRECTORY} CACHE PATH "..." FORCE )

project ("Platform")

if ( RCL_VULKAN )
	add_subdirectory ( Vulkan )
endif()

if ( RCL_DX12 )
	add_subdirectory ( D3D12 )
endif()

if ( RCL_DX11 )
	add_subdirectory ( D3D11 )
endif()