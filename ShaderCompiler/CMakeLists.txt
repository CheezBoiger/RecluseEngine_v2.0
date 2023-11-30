#
cmake_minimum_required ( VERSION 3.0 )
project("RecluseShaderCompiler")

set ( RECLUSE_SC_NAME "RecluseSC")
set ( RECLUSE_SC_VERSION_MINOR )
set ( RECLUSE_SC_VERSION_MAJOR )
set ( RECLUSE_SC_VERSION_PATCH )

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set( RECLUSE_SC_FILES 
    main.cpp
    ShaderCompiler.hpp
    ShaderCompiler.cpp
)

add_executable(${RECLUSE_SC_NAME}
    ${RECLUSE_SC_FILES}
)

set ( RECLUSE_PIPE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Pipeline/Include)

target_include_directories(${RECLUSE_SC_NAME} PUBLIC ${RECLUSE_FRAMEWORK_INCLUDE_DIR} ${RECLUSE_PIPE_INCLUDE_DIR})
target_link_libraries(${RECLUSE_SC_NAME} RecluseFramework ReclusePipeline)
add_dependencies(${RECLUSE_SC_NAME} RecluseFramework ReclusePipeline)

set_target_properties( ${RECLUSE_SC_NAME} PROPERTIES FOLDER "RecluseShaderCompiler")