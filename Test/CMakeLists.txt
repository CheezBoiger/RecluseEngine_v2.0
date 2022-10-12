cmake_minimum_required(VERSION 3.0)
project("TestingFramework")

set( TEST_FRAMEWORK_NAME "TestingFramework" )

set ( RECLUSE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../Recluse/Tests )

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${RECLUSE_OUTPUT_DIRECTORY} )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${RECLUSE_OUTPUT_DIRECTORY} )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${RECLUSE_OUTPUT_DIRECTORY} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

add_subdirectory( CoreFramework )
add_subdirectory( RenderFramework )
add_subdirectory( EngineFramework )