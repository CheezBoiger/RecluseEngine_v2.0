# CMake Entry point build.
cmake_minimum_required ( VERSION 3.16 )

# Mostly third party configurations are set here.

include ( ThirdParty/ThirdPartyModuleSetup.cmake )

# The actual project should be set here.

set( RECLUSE_DIRECTORY 		${CMAKE_SOURCE_DIR}/Recluse )
set( CMAKE_INSTALL_PREFIX 	${RECLUSE_DIRECTORY} CACHE PATH "..." FORCE )

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

# Add shader compiler subdirectory.
add_subdirectory( ShaderCompiler )

# Engine subdirectory.
add_subdirectory ( Engine )

# Editor subdirectory
add_subdirectory ( RecluseEditor )