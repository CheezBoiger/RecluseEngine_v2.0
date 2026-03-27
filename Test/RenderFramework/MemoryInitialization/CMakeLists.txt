cmake_minimum_required( VERSION 3.5 )
project("MemoryInitialization")

set(APP_NAME "MemoryInitialization")

set( APP_FILES 
    main.cpp
)

include( ../../include.cmake )


add_executable(${APP_NAME} ${APP_FILES})
initialize_recluse_framework(${APP_NAME})
post_build_dll(${APP_NAME})

if (RCL_DX12)
	post_d3d12_dll(${APP_NAME})
endif()

if (RCL_VULKAN)
	post_vulkan_dll(${APP_NAME})
endif()