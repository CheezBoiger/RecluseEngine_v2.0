# CMake Entry point build.
cmake_minimum_required ( VERSION 3.5 )

# Mostly third party configurations are set here.

# This cmake file now runs on its own script.
#include ( ThirdParty/ThirdPartyModuleSetup.cmake )

# The actual project should be set here.

set( RECLUSE_DIRECTORY 		${CMAKE_SOURCE_DIR}/Recluse )
set( RECLUSE_LIB_DIRECTORY  ${RECLUSE_DIRECTORY}/lib )
set( RECLUSE_INCLUDE_DIRECTORY ${RECLUSE_DIRECTORY}/include )
set( RECLUSE_BINARY_DIRECTORY  ${RECLUSE_DIRECTORY}/bin )
set( CMAKE_INSTALL_PREFIX 	${RECLUSE_DIRECTORY} CACHE PATH "..." FORCE )

set( RECLUSE_THIRDPARTY_DIR RecluseLibraries CACHE STRING "User specified thirdparty directory.")

# Look for OpenCV, as it is needed for the compressor.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/Systems/CMake)

# Project system.

project ( "Recluse" )

set ( RECLUSE_OUTPUT_DIRECTORY ${RECLUSE_DIRECTORY}/Bin )

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${RECLUSE_OUTPUT_DIRECTORY} )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${RECLUSE_OUTPUT_DIRECTORY} )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${RECLUSE_OUTPUT_DIRECTORY} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

# Framework subdirectory.
add_subdirectory ( Framework )

# Platform subdirectory
add_subdirectory ( Platform )

add_subdirectory ( Pipeline )

# Add shader compiler subdirectory.
add_subdirectory( ShaderCompiler )

# Engine subdirectory.
add_subdirectory ( Engine )

# Editor subdirectory
add_subdirectory ( RecluseEditor )