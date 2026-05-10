# Common helpers for libraries linked to targeted builds.
#
function(CopySharedLib PATH_SOURCE TARGET_NAME)
	message("I am doing ${PATH_SOURCE} ${TARGET_NAME}")
	add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		${PATH_SOURCE}
		$<TARGET_FILE_DIR:${TARGET_NAME}>
		COMMENT "Copying ${PATH_SOURCE} to ${TARGET_NAME}")
endfunction()

# Handles copying a list of paths instead of individually
#
function(CopySharedLibList TARGET_NAME PATH_SOURCES)
	list(LENGTH ${PATH_SOURCES} LenPath)
	math(EXPR LAST_INDEX "${LenPath} - 1")
	foreach(i RANGE 0 ${LAST_INDEX})
		list(GET ${PATH_SOURCES} ${i} ITEM1)
		message("Copying ${ITEM1} to ${TARGET_NAME}")
		#get_filename_component(FILENAME ${ITEM1} NAME)
		#set (DESTINATION_PATH "${PATH_DESTINATION}/${FILENAME}")
		#CopySharedLib(${ITEM1} ${DESTINATION_PATH})
		CopySharedLib(${ITEM1} ${TARGET_NAME})
	endforeach()
endfunction()

function(StructureFiles COMPILE_FILES)
	if (MSVC)
	  foreach(source IN LISTS COMPILE_FILES)
		get_filename_component(source_path "${source}" PATH)
		string(REPLACE "/" "\\" source_path_msvc "${source_path}")
		source_group("${source_path_msvc}" FILES "${source}")
	  endforeach()
	endif()
endfunction()