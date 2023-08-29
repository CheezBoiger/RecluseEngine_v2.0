cmake_minimum_required( VERSION 3.0 )
project("PipelineInitialization")

set(APP_NAME "PipelineInitialization")

set( APP_FILES 
    main.cpp
)

include( ../../include.cmake )


add_executable(${APP_NAME} ${APP_FILES})
initialize_recluse_framework(${APP_NAME})
initialize_recluse_pipeline(${APP_NAME})
post_build_dll(${APP_NAME})
post_build_pipeline_dll(${APP_NAME})