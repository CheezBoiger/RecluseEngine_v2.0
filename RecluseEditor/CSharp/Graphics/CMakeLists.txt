cmake_minimum_required( VERSION 3.0 )
project ("RecluseCSharpGraphicsModule")

set ( RECLUSE_CSHARP_NAME "RecluseCSharpGraphics" )
set ( RECLUSE_CSHARP_INCLUDE_DIR Include )
set ( RECLUSE_CSHARP_SOURCE_DIR Source )

set ( RECLUSE_CSHARP_GRAPHICS )

include ( CMake/Main.cmake )

include_directories(${CMAKE_SOURCE_DIR}/Framework/Include Include )

add_library( ${RECLUSE_CSHARP_NAME} SHARED
	${RECLUSE_CSHARP_GRAPHICS})
	
# Doing some stuff for organization.
if (MSVC)
  foreach(source IN LISTS RECLUSE_CSHARP_GRAPHICS)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
  endforeach()
endif()

# Visual Studio organization. For Good purposes.
if (WIN32)
  set_property(TARGET ${RECLUSE_CSHARP_NAME} 
    PROPERTY 
    FOLDER ${RECLUSE_CSHARP_NAME})
endif()

add_dependencies(${RECLUSE_CSHARP_NAME} RecluseFramework)
target_link_libraries(${RECLUSE_CSHARP_NAME} RecluseFramework )
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " /sdl-") 
set_property(TARGET ${RECLUSE_CSHARP_NAME} PROPERTY COMMON_LANGUAGE_RUNTIME "")
string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})