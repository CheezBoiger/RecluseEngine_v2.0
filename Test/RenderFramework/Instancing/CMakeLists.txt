cmake_minimum_required( VERSION 3.5 )
project("Instancing")

set(APP_NAME "Instancing")

set( APP_FILES 
    main.cpp
)

include( ../../include.cmake )


add_executable(${APP_NAME} ${APP_FILES})
initialize_recluse_framework(${APP_NAME})
initialize_recluse_pipeline(${APP_NAME})
initialize_recluse_engine(${APP_NAME})
post_build_dll(${APP_NAME})
post_build_pipeline_dll(${APP_NAME})
post_d3d12_dll(${APP_NAME})
if (RCL_VULKAN)
	post_vulkan_dll(${APP_NAME})
endif()