set ( RECLUSE_FRAMEWORK_INCLUDE ${CMAKE_SOURCE_DIR}/../Framework/Include )
set ( RECLUSE_FRAMEWORK_DEBUG_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Debug/Lib/RecluseFramework.lib )
set ( RECLUSE_FRAMEWORK_RELEASE_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Release/Lib/RecluseFramework.lib )

set ( RECLUSE_ENGINE_INCLUDE ${CMAKE_SOURCE_DIR}/../Engine/Include )
set( RECLUSE_ENGINE_DEBUG_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Debug/Lib/RecluseEngine.lib )
set( RECLUSE_ENGINE_RELEASE_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Release/Lib/RecluseEngine.lib )

function(initialize_recluse_framework TARGET_NAME )
    message(STATUS "Recluse: Linking ${TARGET_NAME} with Recluse Framework")
    include_directories(${RECLUSE_FRAMEWORK_INCLUDE})
    target_link_libraries(${TARGET_NAME} debug ${RECLUSE_FRAMEWORK_DEBUG_LIB})
    target_link_libraries(${TARGET_NAME} optimized ${RECLUSE_FRAMEWORK_RELEASE_LIB})
endfunction()

function( initialize_recluse_engine TARGET_NAME )
    message(STATUS "Recluse: Linking ${TARGET_NAME} with Recluse Engine")
    target_include_directories(${TARGET_NAME} PUBLIC ${RECLUSE_ENGINE_INCLUDE})
    target_link_libraries(${TARGET_NAME} debug ${RECLUSE_ENGINE_DEBUG_LIB})
    target_link_libraries(${TARGET_NAME} optimized ${RECLUSE_ENGINE_RELEASE_LIB})
endfunction()

# Adds the Recluse Framework DLL to the given app directory.
macro(post_build_dll TARGET_NAME)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/../Recluse/$<CONFIGURATION>/Bin/RecluseFramework.dll
        $<TARGET_FILE_DIR:${TARGET_NAME}>)
endmacro()

# Extra macro that adds the engine to the given app directory.
macro(post_build_engine_dll TARGET_NAME)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/../Recluse/$<CONFIGURATION>/Bin/RecluseEngine.dll
        $<TARGET_FILE_DIR:${TARGET_NAME}>)
endmacro()