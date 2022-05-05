cmake_minimum_required( VERSION 3.0 )
project("RecluseEditor")

set ( RECLUSE_EDITOR_NAME "RecluseEditor" )
set ( RECLUSE_EDITOR_VERSION_MINOR 0)
set ( RECLUSE_EDITOR_VERSION_MAJOR 1)
set ( RECLUSE_EDITOR_VERSION_PATCH 0)

set ( RECLUSE_EDITOR_INCLUDE_DIR Include )
set ( RECLUSE_EDITOR_SOURCE_DIR Source )
set ( RECLUSE_FRAMEWORK_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Framework/Include )
set ( RECLUSE_ENGINE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Engine/Include )
set ( RECLUSE_THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/ThirdParty )

set ( RECLUSE_EDITOR_INCLUDE_FILES )
set ( RECLUSE_EDITOR_COMPILE_FILES )
set ( RECLUSE_EDITOR_LINK_BINARIES )

# include our build files here.
include ( CMake/Main.cmake )

add_executable( ${RECLUSE_EDITOR_NAME} ${RECLUSE_EDITOR_COMPILE_FILES} )
target_include_directories(${RECLUSE_EDITOR_NAME} PUBLIC ${RECLUSE_ENGINE_INCLUDE_DIR} ${RECLUSE_FRAMEWORK_INCLUDE_DIR} ${RECLUSE_EDITOR_INCLUDE_FILES})
target_link_libraries( ${RECLUSE_EDITOR_NAME} RecluseFramework RecluseEngine ${RECLUSE_EDITOR_LINK_BINARIES})
add_dependencies(${RECLUSE_EDITOR_NAME} RecluseFramework RecluseEngine)

# Doing some stuff for organization.
if (MSVC)
  foreach(source IN LISTS RECLUSE_EDITOR_COMPILE_FILES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
  endforeach()
endif()

# Visual Studio organization. For Good purposes.
if (WIN32)
  set_property(TARGET ${RECLUSE_EDITOR_NAME} 
    PROPERTY 
    FOLDER ${RECLUSE_EDITOR_NAME})
endif()

set( CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/Recluse CACHE PATH "..." FORCE)

install(TARGETS ${RECLUSE_EDITOR_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION Debug/Bin)
install(TARGETS ${RECLUSE_EDITOR_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION Release/Bin)
install(TARGETS ${RECLUSE_EDITOR_NAME}
        CONFIGURATIONS Debug
        ARCHIVE DESTINATION Debug/Lib)
install(TARGETS ${RECLUSE_EDITOR_NAME}
        CONFIGURATIONS Release
        ARCHIVE DESTINATION Release/Lib)