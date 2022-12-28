cmake_minimum_required( VERSION 3.0 )
project("Vector2MathTest")

set(APP_NAME "Vector2MathTest")

set( APP_FILES 
    main.cpp
)

include( ../../include.cmake )


add_executable(${APP_NAME} ${APP_FILES})
initialize_recluse_framework(${APP_NAME})
#initialize_gtest_framework(${APP_NAME})
post_build_dll(${APP_NAME})