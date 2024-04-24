cmake_minimum_required( VERSION 3.0 )
project("CommandListSubmit")

set(APP_NAME "CommandListSubmit")

set( APP_FILES 
    main.cpp
)

include( ../../include.cmake )


add_executable(${APP_NAME} ${APP_FILES})
initialize_recluse_framework(${APP_NAME})
post_build_dll(${APP_NAME})
post_d3d12_dll(${APP_NAME})
post_vulkan_dll(${APP_NAME})