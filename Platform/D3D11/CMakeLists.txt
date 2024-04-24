# 
cmake_minimum_required(VERSION 3.16)
project("RecluseD3D11")

set ( RECLUSE_D3D11_NAME "RecluseD3D11" )
set ( RECLUSE_D3D11_MINOR 0 )
set ( RECLUSE_D3D11_MAJOR 0 )
set ( RECLUSE_D3D11_BUILD 0 )

set ( RECLUSE_VERSION_STRING "v${RECLUSE_D3D11_MAJOR}.${RECLUSE_D3D11_MINOR}.${RECLUSE_D3D11_BUILD}" )

# Use this variable to include to the source library build.
set ( RECLUSE_D3D11_COMPILE_FILES )
set ( RECLUSE_D3D11_INCLUDE ${RECLUSE_FRAMEWORK_INCLUDE} )
set ( RECLUSE_D3D11_SOURCE Source )
set ( RECLUSE_D3D11_INCLUDE_FILES ${RECLUSE_D3D11_INCLUDE} )
set ( RECLUSE_D3D11_LINK_BINARIES )
set ( RECLUSE_D3D11_THIRD_PARTY ${RECLUSE_THIRDPARTY_DIR} )


if (R_DEVELOPER)
	add_definitions(-DRECLUSE_DEVELOPER=1)
endif(R_DEVELOPER)

if ( RCL_DX11 )
	add_definitions( -DRCL_DX11=1 )
	set ( RECLUSE_FRAMEWORK_LINK_BINARIES ${RECLUSE_FRAMEWORK_LINK_BINARIES} d3d11.lib dxgi.lib )
	set ( RECLUSE_D3D11_DIR . )
	set ( RECLUSE_D3D11_COMPILE_FILES 
		${RECLUSE_D3D11_COMPILE_FILES}
		${RECLUSE_D3D11_DIR}/D3D11LoaderApi.hpp
		${RECLUSE_D3D11_DIR}/D3D11LoaderApi.cpp
		${RECLUSE_D3D11_DIR}/D3D11Commons.hpp
		${RECLUSE_D3D11_DIR}/D3D11Device.hpp
		${RECLUSE_D3D11_DIR}/D3D11Device.cpp
		${RECLUSE_D3D11_DIR}/D3D11Instance.hpp
		${RECLUSE_D3D11_DIR}/D3D11Instance.cpp
	)
endif()

add_library (               ${RECLUSE_D3D11_NAME} SHARED    ${RECLUSE_D3D11_COMPILE_FILES} ) 
target_include_directories( ${RECLUSE_D3D11_NAME} PUBLIC    ${RECLUSE_D3D11_INCLUDE_FILES} ${RECLUSE_D3D11_SOURCE} ${RECLUSE_D3D11_THIRD_PARTY})
target_link_libraries (     ${RECLUSE_D3D11_NAME}           ${RECLUSE_D3D11_LINK_BINARIES} RecluseFramework )
add_dependencies( ${RECLUSE_D3D11_NAME} RecluseFramework )
# Doing some stuff for organization.
if (MSVC)
  foreach(source IN LISTS RECLUSE_D3D11_COMPILE_FILES)
    get_filename_component(source_path "${source}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
  endforeach()
endif()

# Visual Studio organization. For Good purposes.
if (WIN32)
	#target_sources(${RECLUSE_D3D11_NAME} PRIVATE Resource.rc resource.h)
	set_property(TARGET ${RECLUSE_D3D11_NAME} 
		PROPERTY 
		FOLDER ${RECLUSE_D3D11_NAME})
endif()

install(TARGETS ${RECLUSE_D3D11_NAME}
        CONFIGURATIONS Debug
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_D3D11_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION Bin)
install(TARGETS ${RECLUSE_D3D11_NAME}
        CONFIGURATIONS Debug
        ARCHIVE DESTINATION Lib)
install(TARGETS ${RECLUSE_D3D11_NAME}
        CONFIGURATIONS Release
        ARCHIVE DESTINATION Lib)
   