# CMake third party projects to include in the recluse engine foundation and engine components.
# Most of these projects are to be added via the git submodule command, so be sure to initiate before actually building the engine.
# Use the git submodule update --init --recursive command to bring the proper module prerequisities.

set ( RECLUSE_THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)

if ( RCL_DX11 OR RCL_DX12)
	# TODO: Disable for now, until we can figure out what we need to do to properly compile this, and not have to use an existing binary :D.
	# add_subdirectory ( ${RECLUSE_THIRDPARTY_DIR}/DirectXShaderCompiler )
	#if ( RCL_DX12 )
	#	add_subdirectory( ${RECLUSE_THIRDPARTY_DIR}/DirectX-Headers )
	#endif()
endif()
add_subdirectory ( ${RECLUSE_THIRDPARTY_DIR}/wxWidgets )
#add_subdirectory ( ${RECLUSE_THIRDPARTY_DIR}/zlib )
add_subdirectory ( ${RECLUSE_THIRDPARTY_DIR}/googletest )

# Enable Shared libs on meshoptimizer.
set(MESHOPT_BUILD_SHARED_LIBS ON)
add_subdirectory( ${RECLUSE_THIRDPARTY_DIR}/meshoptimizer )

# Reflection for stuff
if ( RCL_VULKAN ) 
	if ( RCL_GLSLANG OR R_GLSLANG_LEGACY_API )
		set ( SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "Build the reflection project as a lib instead!" )
		set ( SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "Do not build as an executable." )
		add_subdirectory( ${RECLUSE_THIRDPARTY_DIR}/SPIRV-Reflect )
	endif()
endif()