set ( RECLUSE_PIPE_INCLUDE ${RECLUSE_PIPELINE_INCLUDE}/Recluse/Pipeline )
set ( RECLUSE_PIPE_SOURCE ${RECLUSE_PIPELINE_SOURCE})


set (RECLUSE_PIPELINE_INCLUDE_FILES ${RECLUSE_PIPELINE_INCLUDE_FILES}
									${RECLUSE_PIPE_INCLUDE}/MeshBuilder.hpp
									${RECLUSE_PIPE_INCLUDE}/Texture.hpp
									${RECLUSE_PIPE_INCLUDE}/Material.hpp
									${RECLUSE_PIPE_INCLUDE}/ShaderProgramBuilder.hpp)
									
set(RECLUSE_PIPELINE_COMPILE_FILES ${RECLUSE_PIPELINE_COMPILE_FILES}
								  ${RECLUSE_PIPELINE_INCLUDE_FILES}
								  ${RECLUSE_PIPE_SOURCE}/MeshBuilder.cpp
								  ${RECLUSE_PIPE_SOURCE}/TextureCompressor.cpp
								  ${RECLUSE_PIPE_SOURCE}/Texture.cpp
								  ${RECLUSE_PIPE_SOURCE}/SceneImporter/GLTFImporter.cpp
								  ${RECLUSE_PIPE_SOURCE}/SceneImporter/FBXImporter.cpp
								  ${RECLUSE_PIPE_SOURCE}/SceneImporter/Importer.hpp
								  ${RECLUSE_PIPE_SOURCE}/ShaderProgramBuilder.cpp)