cmake_minimum_required( VERSION 3.16 )
project("RecluseEditorBase")

set ( RECLUSE_EDITOR_NAME "RecluseEditorBase" )
set ( RECLUSE_EDITOR_VERSION_MINOR 0)
set ( RECLUSE_EDITOR_VERSION_MAJOR 1)
set ( RECLUSE_EDITOR_VERSION_PATCH 0)

set ( RECLUSE_EDITOR_INCLUDE_DIR Include )
set ( RECLUSE_EDITOR_SOURCE_DIR Source )
set ( RECLUSE_FRAMEWORK_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Framework/Include )
set ( RECLUSE_ENGINE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Engine/Include )
set ( RECLUSE_THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/ThirdParty )

set ( RECLUSE_EDITOR_INCLUDE_FILES )
set ( RECLUSE_EDITOR_COMPILE_FILES )
set ( RECLUSE_EDITOR_LINK_BINARIES )

# include our build files here.
include ( CMake/Main.cmake )

add_executable( ${RECLUSE_EDITOR_NAME} ${RECLUSE_EDITOR_COMPILE_FILES} )
target_include_directories( 
	${RECLUSE_EDITOR_NAME} 
	PUBLIC 
	${RECLUSE_EDITOR_INCLUDE_DIR} ${RECLUSE_ENGINE_INCLUDE_DIR} ${RECLUSE_FRAMEWORK_INCLUDE_DIR} ${RECLUSE_EDITOR_INCLUDE_FILES} )

target_link_libraries( ${RECLUSE_EDITOR_NAME} 
	RecluseFramework RecluseEngine ${RECLUSE_EDITOR_LINK_BINARIES} wx::net wx::core wx::base)
	
add_dependencies(${RECLUSE_EDITOR_NAME} RecluseFramework RecluseEngine)

add_subdirectory(CSharp/Graphics)
add_subdirectory(CSharp/Pipeline)
add_subdirectory(FrontEnd)

# Doing some stuff for organization.
if (MSVC)
  foreach(source IN LISTS RECLUSE_EDITOR_COMPILE_FILES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
  endforeach()
endif()

# Visual Studio organization. For Good purposes.
if (WIN32)
  set_property(TARGET ${RECLUSE_EDITOR_NAME} 
    PROPERTY 
    FOLDER ${RECLUSE_EDITOR_NAME})
endif()

install(TARGETS ${RECLUSE_EDITOR_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_EDITOR_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_EDITOR_NAME}
        CONFIGURATIONS Debug
        ARCHIVE DESTINATION Lib)
install(TARGETS ${RECLUSE_EDITOR_NAME}
        CONFIGURATIONS Release
        ARCHIVE DESTINATION Lib)

# Code to run after the compilation of our thirdparty libs AND our Editor executable.
install(CODE [[
#			file(GET_RUNTIME_DEPENDENCIES 
#				EXECUTABLES $<TARGET_FILE:RecluseEditor>
#				RESOLVED_DEPENDENCIES_VAR R_DEPS
#				UNRESOLVED_DEPENDENCIES_VAR U_DEPS
#				DIRECTORIES ${CMAKE_INSTALL_PREFIX}/lib/vc_x64_dll
#			)
#
#			foreach(_FILE ${R_DEPS})
#				message(WARNING "${_FILE}")
#				file(INSTALL
#					#CONFIGURATIONS Release
#					DESTINATION ${CMAKE_INSTALL_PREFIX}/Release/Bin
#					TYPE SHARED_LIBRARY
#					FOLLOW_SYMLINK_CHAIN
#					FILES "${_FILE}"
#				)
#			endforeach()
#			
#			foreach(_FILE ${R_DEPS})
#				message(WARNING "${_FILE}")
#					file(INSTALL
#						CONFIGURATIONS Debug
#						DESTINATION ${CMAKE_INSTALL_PREFIX}/Debug/Bin
#						TYPE SHARED_LIBRARY
#						FOLLOW_SYMLINK_CHAIN
#						FILES "${_FILE}"
#				)
#			endforeach()
#			
#			foreach(_FILE ${U_DEPS})
#				message(ERROR "Can not resolve ${_FILE}")
#			endforeach()
#				
				file(GLOB_RECURSE WX_DLLS "${CMAKE_INSTALL_PREFIX}/lib/vc_x64_dll/*.dll")
				foreach(_FILE ${WX_DLLS})
					file(INSTALL
						DESTINATION ${CMAKE_INSTALL_PREFIX}/Bin
						TYPE SHARED_LIBRARY
						FOLLOW_SYMLINK_CHAIN
						FILES "${_FILE}"
					)
				endforeach()
#				#message(WARNING "${WX_DLLS}")
		]])