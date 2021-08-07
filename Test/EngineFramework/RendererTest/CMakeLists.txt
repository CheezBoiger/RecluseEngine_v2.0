cmake_minimum_required( VERSION 3.0 )
project("RendererTest")

set(APP_NAME "RendererTest")

set( APP_FILES 
    main.cpp
)

include( ../../include.cmake )


add_executable(${APP_NAME} ${APP_FILES})
initialize_recluse_framework(${APP_NAME})
initialize_recluse_engine(${APP_NAME})
post_build_dll(${APP_NAME})
post_build_engine_dll(${APP_NAME})