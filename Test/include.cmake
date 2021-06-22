set ( RECLUSE_FRAMEWORK_INCLUDE ${CMAKE_SOURCE_DIR}/../Framework/Include)
set ( RECLUSE_FRAMEWORK_LIB ${CMAKE_SOURCE_DIR}/../Recluse/Debug/Lib/RecluseFramework.lib)

function(initialize_recluse_framework TARGET_NAME )
    message(STATUS "Recluse: Linking ${TARGET_NAME} with Recluse Framework")
    include_directories(${RECLUSE_FRAMEWORK_INCLUDE})
    target_link_libraries(${TARGET_NAME} PUBLIC ${RECLUSE_FRAMEWORK_LIB})
endfunction()

macro(post_build_dll TARGET_NAME)
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/../Recluse/$<CONFIGURATION>/Bin/RecluseFramework.dll
        $<TARGET_FILE_DIR:${TARGET_NAME}>)
endmacro()