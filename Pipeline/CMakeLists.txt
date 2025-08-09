# 
cmake_minimum_required(VERSION 3.5)
project("ReclusePipeline")

set ( RECLUSE_PIPELINE_NAME "ReclusePipeline" )
set ( RECLUSE_PIPELINE_MINOR 0 )
set ( RECLUSE_PIPELINE_MAJOR 0 )
set ( RECLUSE_PIPELINE_BUILD 0 )

set ( RECLUSE_VERSION_STRING "v${RECLUSE_PIPELINE_MAJOR}.${RECLUSE_PIPELINE_MINOR}.${RECLUSE_PIPELINE_BUILD}" )

# Use this variable to include to the source library build.
set ( RECLUSE_PIPELINE_COMPILE_FILES )
set ( RECLUSE_PIPELINE_INCLUDE Include )
set ( RECLUSE_PIPELINE_SOURCE Source )
set ( RECLUSE_PIPELINE_INCLUDE_FILES ${RECLUSE_PIPELINE_INCLUDE} )
set ( RECLUSE_PIPELINE_LINK_LIBRARIES )
set ( RECLUSE_PIPELINE_THIRD_PARTY ${RECLUSE_THIRDPARTY_DIR} )

include ( ${CMAKE_SOURCE_DIR}/Systems/CMake/GenerateExports.cmake )

find_package(FBX)

if (FBX_FOUND)
	message("Fbx Sdk found.")
	SET (RECLUSE_PIPELINE_INCLUDE_FILES ${RECLUSE_PIPELINE_INCLUDE_FILES} ${FBX_INCLUDE_DIR})
	set (RECLUSE_PIPELINE_LINK_LIBRARIES_RELEASE optimized ${RECLUSE_PIPELINE_LINK_LIBRARIES} optimized ${FBX_LIBRARY} optimized ${FBX_ZLIB_LIBRARY} optimized ${FBX_XML2_LIBRARY})
	set (RECLUSE_PIPELINE_LINK_LIBRARIES_DEBUG debug ${RECLUSE_PIPELINE_LINK_LIBRARIES} debug ${FBX_LIBRARY_DEBUG} debug ${FBX_ZLIB_LIBRARY_DEBUG} debug ${FBX_XML2_LIBRARY_DEBUG})
	
	SET(RECLUSE_PIPELINE_LINK_LIBRARIES ${RECLUSE_PIPELINE_LINK_LIBRARIES_RELEASE} ${RECLUSE_PIPELINE_LINK_LIBRARIES_DEBUG})
else()
	message(WARNING "$ENV{FBX_DIR} Fbx Sdk not found. Be sure to set up the ENV path FBX_DIR for it. ${FBX_FOUND}")
endif()

set ( RECLUSE_ENGINE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Engine/Include )

include ( CMake/Pipeline.cmake )

add_library (               ${RECLUSE_PIPELINE_NAME} SHARED    ${RECLUSE_PIPELINE_COMPILE_FILES} ) 
target_include_directories( ${RECLUSE_PIPELINE_NAME} PUBLIC    ${RECLUSE_INCLUDE_FILES} ${RECLUSE_FRAMEWORK_INCLUDE_DIR} ${RECLUSE_ENGINE_INCLUDE_DIR} ${RECLUSE_PIPELINE_INCLUDE_FILES} ${RECLUSE_PIPELINE_SOURCE} ${RECLUSE_PIPELINE_THIRD_PARTY})
target_link_libraries (     ${RECLUSE_PIPELINE_NAME}           RecluseFramework ${RECLUSE_PIPELINE_LINK_LIBRARIES} RecluseEngine )
add_dependencies(${RECLUSE_PIPELINE_NAME} RecluseFramework)
add_dependencies(${RECLUSE_PIPELINE_NAME} RecluseEngine)

ExportThisLibrary(${RECLUSE_PIPELINE_NAME})

ImportThisLibrary(${RECLUSE_PIPELINE_NAME} "RecluseFramework")
ImportThisLibrary(${RECLUSE_PIPELINE_NAME} "RecluseEngine")

# Doing some stuff for organization.
if (MSVC)
  foreach(source IN LISTS RECLUSE_PIPELINE_COMPILE_FILES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
  endforeach()
endif()

install(TARGETS ${RECLUSE_PIPELINE_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_PIPELINE_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_PIPELINE_NAME}
        CONFIGURATIONS Debug
        ARCHIVE DESTINATION Lib)
install(TARGETS ${RECLUSE_PIPELINE_NAME}
        CONFIGURATIONS Release
        ARCHIVE DESTINATION Lib)