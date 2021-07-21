#
cmake_minimum_required( VERSION 3.0 )
project("RecluseEngine")

set ( RECLUSE_ENGINE_NAME "RecluseEngine" )
set ( RECLUSE_ENGINE_VERSION_MINOR 1)
set ( RECLUSE_ENGINE_VERSION_MAJOR 1)
set ( RECLUSE_ENGINE_VERSION_PATCH 1)

set ( RECLUSE_ENGINE_INCLUDE_DIR Include )
set ( RECLUSE_ENGINE_SOURCE_DIR Source )
set ( RECLUSE_FRAMEWORK_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Framework/Include )

set ( RECLUSE_ENGINE_INCLUDE_FILES )
set ( RECLUSE_ENGINE_COMPILE_FILES )
set ( RECLUSE_ENGINE_LINK_BINARIES )

# include our build files here.
include ( CMake/Game.cmake )
include ( CMake/Renderer.cmake )
include ( CMake/Scene.cmake )

add_library( ${RECLUSE_ENGINE_NAME} SHARED ${RECLUSE_ENGINE_COMPILE_FILES})
target_include_directories( ${RECLUSE_ENGINE_NAME} PUBLIC ${RECLUSE_INCLUDE_FILES} 
    ${RECLUSE_ENGINE_INCLUDE_DIR} ${RECLUSE_FRAMEWORK_INCLUDE_DIR})
target_link_libraries( ${RECLUSE_ENGINE_NAME} RecluseFramework ${RECLUSE_ENGINE_LINK_BINARIES})
add_dependencies(${RECLUSE_ENGINE_NAME} RecluseFramework)

# Doing some stuff for organization.
if (MSVC)
  foreach(source IN LISTS RECLUSE_ENGINE_COMPILE_FILES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
  endforeach()
endif()

# Visual Studio organization. For Good purposes.
if (WIN32)
  set_property(TARGET ${RECLUSE_ENGINE_NAME} 
    PROPERTY 
    FOLDER ${RECLUSE_ENGINE_NAME})
endif()

set( CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/Recluse CACHE PATH "..." FORCE)

install(TARGETS ${RECLUSE_ENGINE_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION Debug/Bin)
install(TARGETS ${RECLUSE_ENGINE_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION Release/Bin)
install(TARGETS ${RECLUSE_ENGINE_NAME}
        CONFIGURATIONS Debug
        ARCHIVE DESTINATION Debug/Lib)
install(TARGETS ${RECLUSE_ENGINE_NAME}
        CONFIGURATIONS Release
        ARCHIVE DESTINATION Release/Lib)