# 
cmake_minimum_required(VERSION 3.16)
project("RecluseFramework")

set ( RECLUSE_FRAMEWORK_NAME "RecluseFramework" )
set ( RECLUSE_FRAMEWORK_MINOR 0 )
set ( RECLUSE_FRAMEWORK_MAJOR 0 )
set ( RECLUSE_FRAMEWORK_BUILD 0 )

set ( RECLUSE_VERSION_STRING "v${RECLUSE_FRAMEWORK_MAJOR}.${RECLUSE_FRAMEWORK_MINOR}.${RECLUSE_FRAMEWORK_BUILD}" )

# Use this variable to include to the source library build.
set ( RECLUSE_FRAMEWORK_COMPILE_FILES )
set ( RECLUSE_FRAMEWORK_INCLUDE Include )
set ( RECLUSE_FRAMEWORK_SOURCE Source )
set ( RECLUSE_FRAMEWORK_INCLUDE_FILES ${RECLUSE_FRAMEWORK_INCLUDE} )
set ( RECLUSE_FRAMEWORK_LINK_BINARIES )
set ( RECLUSE_FRAMEWORK_THIRD_PARTY ${RECLUSE_THIRDPARTY_DIR} )

include ( CMake/Definitions.cmake )
include ( CMake/Core.cmake )
include ( CMake/Graphics.cmake )
include ( CMake/Animation.cmake )
include ( CMake/Math.cmake )

if (R_DEVELOPER)
	add_definitions(-DRECLUSE_DEVELOPER=1)
endif(R_DEVELOPER)

add_library (               ${RECLUSE_FRAMEWORK_NAME} SHARED    ${RECLUSE_FRAMEWORK_COMPILE_FILES} ) 
target_include_directories( ${RECLUSE_FRAMEWORK_NAME} PUBLIC    ${RECLUSE_FRAMEWORK_INCLUDE_FILES} ${RECLUSE_FRAMEWORK_SOURCE} ${RECLUSE_FRAMEWORK_THIRD_PARTY})
target_link_libraries (     ${RECLUSE_FRAMEWORK_NAME}           ${RECLUSE_FRAMEWORK_LINK_BINARIES} )
# Doing some stuff for organization.
if (MSVC)
  foreach(source IN LISTS RECLUSE_FRAMEWORK_COMPILE_FILES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
  endforeach()
endif()

# Visual Studio organization. For Good purposes.
if (WIN32)
	#target_sources(${RECLUSE_FRAMEWORK_NAME} PRIVATE Resource.rc resource.h)
	set_property(TARGET ${RECLUSE_FRAMEWORK_NAME} 
		PROPERTY 
		FOLDER ${RECLUSE_FRAMEWORK_NAME})
endif()

install(TARGETS ${RECLUSE_FRAMEWORK_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_FRAMEWORK_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_FRAMEWORK_NAME}
        CONFIGURATIONS Debug
        ARCHIVE DESTINATION Lib)
install(TARGETS ${RECLUSE_FRAMEWORK_NAME}
        CONFIGURATIONS Release
        ARCHIVE DESTINATION Lib)
   