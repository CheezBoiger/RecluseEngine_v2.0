set ( RECLUSE_PIPE_INCLUDE ${RECLUSE_PIPELINE_INCLUDE}/Recluse/Pipeline )
set ( RECLUSE_PIPE_SOURCE ${RECLUSE_PIPELINE_SOURCE})


set (RECLUSE_PIPELINE_INCLUDE_FILES ${RECLUSE_PIPELINE_INCLUDE_FILES}
									${RECLUSE_PIPE_INCLUDE}/MeshBuilder.hpp
									${RECLUSE_PIPE_INCLUDE}/Texture.hpp
									${RECLUSE_PIPE_INCLUDE}/Material.hpp
									${RECLUSE_PIPE_INCLUDE}/ShaderProgramBuilder.hpp
									${RECLUSE_PIPE_INCLUDE}/Graphics/ShaderBuilder.hpp)
									
set(RECLUSE_PIPELINE_COMPILE_FILES ${RECLUSE_PIPELINE_COMPILE_FILES}
								  ${RECLUSE_PIPELINE_INCLUDE_FILES}
								  ${RECLUSE_PIPE_SOURCE}/MeshBuilder.cpp
								  ${RECLUSE_PIPE_SOURCE}/TextureCompressor.cpp
								  ${RECLUSE_PIPE_SOURCE}/Texture.cpp
								  ${RECLUSE_PIPE_SOURCE}/SceneImporter/GLTFImporter.cpp
								  ${RECLUSE_PIPE_SOURCE}/SceneImporter/FBXImporter.cpp
								  ${RECLUSE_PIPE_SOURCE}/SceneImporter/Importer.hpp
								  ${RECLUSE_PIPE_SOURCE}/ShaderProgramBuilder.cpp
								  ${RECLUSE_PIPE_SOURCE}/Graphics/ShaderBuilderCommon.cpp
								  ${RECLUSE_PIPE_SOURCE}/Graphics/ShaderBuilderCommon.hpp
								  ${RECLUSE_PIPE_SOURCE}/Graphics/DXCShaderBuilder.cpp
								  ${RECLUSE_PIPE_SOURCE}/Graphics/GlslangShaderBuilder.cpp)
								  
if ( RCL_VULKAN )
	find_package( Vulkan )
	if ( Vulkan_FOUND )
		if ( RCL_GLSLANG OR R_GLSLANG_LEGACY_API )
			add_definitions ( -DRCL_GLSLANG=1 )
			if ( R_GLSLANG_LEGACY_API )
				add_definitions( -DR_GLSLANG_LEGACY_API=1 )
			else()
				add_definitions( -DR_GLSLANG_LEGACY_API=0 )
			endif()
			set ( RECLUSE_PIPELINE_LINK_LIBRARIES ${RECLUSE_PIPELINE_LINK_LIBRARIES} ${Vulkan_LIBRARIES} )
			set ( RECLUSE_PIPELINE_INCLUDE_FILES ${RECLUSE_PIPELINE_INCLUDE_FILES} ${Vulkan_INCLUDE_DIRS} )
			set ( VULKAN_GLSLANG_LIBRARY_RELEASE optimized $ENV{VULKAN_SDK}/Lib/glslang.lib 
									 optimized $ENV{VULKAN_SDK}/Lib/shaderc.lib
									 optimized $ENV{VULKAN_SDK}/Lib/shaderc_util.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/SPIRV.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/HLSL.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/OGLCompiler.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/OSDependent.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/SPIRV-Tools.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/SPIRV-Tools-link.lib
                                     optimized $ENV{VULKAN_SDK}/Lib/SPIRV-Tools-opt.lib)
			message ( WARNING "Windows: you will need to also Download Vulkan SDK DebugShaderLibs in order to use glslang debug libs")
			set ( VULKAN_GLSLANG_LIBRARY_DEBUG debug $ENV{VULKAN_SDK}/Lib/glslangd.lib 
                                     debug $ENV{VULKAN_SDK}/Lib/shadercd.lib
									 debug $ENV{VULKAN_SDK}/Lib/shaderc_utild.lib
									 debug $ENV{VULKAN_SDK}/Lib/GenericCodeGend.lib
									 debug $ENV{VULKAN_SDK}/Lib/SPIRVd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/HLSLd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/OGLCompilerd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/OSDependentd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/SPIRV-Toolsd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/SPIRV-Tools-linkd.lib
                                     debug $ENV{VULKAN_SDK}/Lib/SPIRV-Tools-optd.lib)
			if ( NOT R_GLSLANG_LEGACY_API )
				set ( VULKAN_GLSLANG_LIBRARY_DEBUG ${VULKAN_GLSLANG_LIBRARY_DEBUG} debug $ENV{VULKAN_SDK}/Lib/MachineIndependentd.lib)
				set ( VULKAN_GLSLANG_LIBRARY_RELEASE ${VULKAN_GLSLANG_LIBRARY_RELEASE} 
						optimized $ENV{VULKAN_SDK}/Lib/MachineIndependent.lib 
						optimized $ENV{VULKAN_SDK}/Lib/GenericCodeGen.lib)
			endif()
			set ( VULKAN_GLSLANG_LIBRARIES ${VULKAN_GLSLANG_LIBRARY_RELEASE} ${VULKAN_GLSLANG_LIBRARY_DEBUG} )
			set ( RECLUSE_PIPELINE_LINK_LIBRARIES ${RECLUSE_PIPELINE_LINK_LIBRARIES} ${VULKAN_GLSLANG_LIBRARIES} )
		endif()
	endif()
endif()

if ( RCL_DX11 OR RCL_DX12 )
    set ( RECLUSE_PIPELINE_LINK_LIBRARIES ${RECLUSE_PIPELINE_LINK_LIBRARIES} dxgi.lib )
    set ( RECLUSE_PIPELINE_LINK_LIBRARIES ${RECLUSE_PIPELINE_LINK_LIBRARIES} D3DCompiler.lib )
	if ( RCL_DXC )
		set ( RECLUSE_PIPELINE_LINK_LIBRARIES ${RECLUSE_PIPELINE_LINK_LIBRARIES} dxcompiler.lib )
        add_definitions( -DRCL_DXC=1 )
        message(WARNING "d3dcompiler.dll and dxil.dll needed with executable, since we are now including dxc...")
    endif()
endif()
