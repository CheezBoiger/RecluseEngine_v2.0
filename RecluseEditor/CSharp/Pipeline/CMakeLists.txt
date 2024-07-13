cmake_minimum_required( VERSION 3.0 )
project ("RecluseCSharpPipelineModuleInterop")

set ( RECLUSE_CSHARP_NAME "RecluseCSharpPipelineInterop" )
set ( RECLUSE_CSHARP_INCLUDE_DIR Include )
set ( RECLUSE_CSHARP_SOURCE_DIR Source )

set ( RECLUSE_CSHARP_PIPELINE )

include ( CMake/Main.cmake )

include_directories(${CMAKE_SOURCE_DIR}/Framework/Include Include 
	${CMAKE_SOURCE_DIR}/Pipeline/Include 
	${CMAKE_SOURCE_DIR}/RecluseEditor)

add_library( ${RECLUSE_CSHARP_NAME} SHARED
	${RECLUSE_CSHARP_PIPELINE})
	
# Doing some stuff for organization.
if (MSVC)
  foreach(source IN LISTS RECLUSE_CSHARP_PIPELINE)
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

add_dependencies(${RECLUSE_CSHARP_NAME}  RecluseFramework ReclusePipeline)
target_link_libraries(${RECLUSE_CSHARP_NAME} RecluseFramework ReclusePipeline)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " /sdl-") 
set_property(TARGET ${RECLUSE_CSHARP_NAME} PROPERTY COMMON_LANGUAGE_RUNTIME "")
string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
if (MSVC)
	target_compile_options(${RECLUSE_CSHARP_NAME} PRIVATE $<$<BOOL:${MSVC}>:/AI${CMAKE_SOURCE_DIR}/Recluse/bin>)
endif()