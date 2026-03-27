set ( RECLUSE_FRAMEWORK_INCLUDE ${CMAKE_SOURCE_DIR}/../Framework/Include )
set ( RECLUSE_GENERATED_INCLUDES ${CMAKE_SOURCE_DIR}/../Recluse/include/ )
set ( RECLUSE_FRAMEWORK_DEBUG_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Lib/RecluseFramework.lib )
set ( RECLUSE_FRAMEWORK_RELEASE_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Lib/RecluseFramework.lib )

set ( RECLUSE_ENGINE_INCLUDE ${CMAKE_SOURCE_DIR}/../Engine/Include )
set( RECLUSE_ENGINE_DEBUG_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Lib/RecluseEngine.lib )
set( RECLUSE_ENGINE_RELEASE_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Lib/RecluseEngine.lib )

set ( RECLUSE_PIPELINE_INCLUDE ${CMAKE_SOURCE_DIR}/../Pipeline/Include )
set( RECLUSE_PIPELINE_DEBUG_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Lib/ReclusePipeline.lib )
set( RECLUSE_PIPELINE_RELEASE_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Lib/ReclusePipeline.lib )

set ( RECLUSE_VULKAN_DEBUG_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Lib/RecluseVulkan.lib )
set ( RECLUSE_VULKAN_RELEASE_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Lib/RecluseVulkan.lib )

set ( RECLUSE_D3D12_DEBUG_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Lib/RecluseD3D12.lib )
set ( RECLUSE_D3D12_RELEASE_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Lib/RecluseD3D12.lib )


set ( RECLUSE_THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/Thirdparty )

function(ImportThisLibrary TARGET_LIBRARY IMPORT_LIBRARY)
	set(RECLUSE_GENERATED_LIBRARY_DIRECTORY ${CMAKE_SOURCE_DIR}/../Recluse/Build/Libraries)
	set(IMPORT_LIBRARY_DIRECTORY ${RECLUSE_GENERATED_LIBRARY_DIRECTORY}/${IMPORT_LIBRARY})
	target_include_directories(${TARGET_LIBRARY} PUBLIC ${IMPORT_LIBRARY_DIRECTORY})
endfunction()

function(initialize_recluse_framework TARGET_NAME )
    message(STATUS "Recluse: Linking ${TARGET_NAME} with Recluse Framework")
    include_directories(${RECLUSE_FRAMEWORK_INCLUDE} ${RECLUSE_GENERATED_INCLUDES})
    target_link_libraries(${TARGET_NAME} debug ${RECLUSE_FRAMEWORK_DEBUG_LIB})
    target_link_libraries(${TARGET_NAME} optimized ${RECLUSE_FRAMEWORK_RELEASE_LIB})
	ImportThisLibrary(${TARGET_NAME} "RecluseFramework")
endfunction()

function( initialize_recluse_engine TARGET_NAME )
    message(STATUS "Recluse: Linking ${TARGET_NAME} with Recluse Engine")
    target_include_directories(${TARGET_NAME} PUBLIC ${RECLUSE_ENGINE_INCLUDE})
    target_link_libraries(${TARGET_NAME} debug ${RECLUSE_ENGINE_DEBUG_LIB})
    target_link_libraries(${TARGET_NAME} optimized ${RECLUSE_ENGINE_RELEASE_LIB})
	ImportThisLibrary(${TARGET_NAME} "RecluseEngine")
endfunction()

function( initialize_recluse_pipeline TARGET_NAME )
    message(STATUS "Recluse: Linking ${TARGET_NAME} with Recluse Pipeline")
    target_include_directories(${TARGET_NAME} PUBLIC ${RECLUSE_PIPELINE_INCLUDE})
    target_link_libraries(${TARGET_NAME} debug ${RECLUSE_PIPELINE_DEBUG_LIB})
    target_link_libraries(${TARGET_NAME} optimized ${RECLUSE_PIPELINE_RELEASE_LIB})
	ImportThisLibrary(${TARGET_NAME} "ReclusePipeline")
endfunction()

function(initialize_gtest_framework TARGET_NAME )
	enable_testing()
	target_link_libraries(${TARGET_NAME} gtest gmock)
endfunction()

# Adds the Recluse Framework DLL to the given app directory.
#macro(post_build_dll TARGET_NAME)
#add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#        ${CMAKE_SOURCE_DIR}/../Recluse/$<CONFIGURATION>/Bin/RecluseFramework.dll
#        $<TARGET_FILE_DIR:${TARGET_NAME}>)
#endmacro()
#
## Extra macro that adds the engine to the given app directory.
#macro(post_build_engine_dll TARGET_NAME)
#add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#        ${CMAKE_SOURCE_DIR}/../Recluse/$<CONFIGURATION>/Bin/RecluseEngine.dll
#        $<TARGET_FILE_DIR:${TARGET_NAME}>)
#endmacro()


macro(post_build_dll TARGET_NAME)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/../Recluse/Bin/RecluseFramework.dll
        $<TARGET_FILE_DIR:${TARGET_NAME}>)
endmacro()

# Extra macro that adds the engine to the given app directory.
macro(post_build_engine_dll TARGET_NAME)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/../Recluse/Bin/RecluseEngine.dll
        $<TARGET_FILE_DIR:${TARGET_NAME}>)
endmacro()

macro(post_build_pipeline_dll TARGET_NAME)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/../Recluse/Bin/ReclusePipeline.dll
        $<TARGET_FILE_DIR:${TARGET_NAME}>)
endmacro()

macro(post_vulkan_dll TARGET_NAME)
	if (NOT EXISTS ${CMAKE_SOURCE_DIR}/../Recluse/Bin/RecluseVulkan.dll)
		add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
				${CMAKE_SOURCE_DIR}/../Recluse/Bin/RecluseVulkan.dll
				$<TARGET_FILE_DIR:${TARGET_NAME}>)
	endif()
endmacro()

macro(post_d3d12_dll TARGET_NAME)
	if (NOT EXISTS ${CMAKE_SOURCE_DIR}/../Recluse/Bin/RecluseD3D12.dll)
		add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
				${CMAKE_SOURCE_DIR}/../Recluse/Bin/RecluseD3D12.dll
				$<TARGET_FILE_DIR:${TARGET_NAME}>)
	endif()
endmacro()