#
set ( RECLUSE_ENGINE_RENDERER_INCLUDE ${RECLUSE_ENGINE_INCLUDE_DIR}/Recluse/Renderer)
set ( RECLUSE_ENGINE_RENDERER_SOURCE ${RECLUSE_ENGINE_SOURCE_DIR}/Renderer )


set (RECLUSE_ENGINE_COMPILE_FILES 
    ${RECLUSE_ENGINE_COMPILE_FILES}
    ${RECLUSE_ENGINE_RENDERER_INCLUDE}/Renderer.hpp
    ${RECLUSE_ENGINE_RENDERER_INCLUDE}/RendererResources.hpp
    ${RECLUSE_ENGINE_RENDERER_INCLUDE}/SceneView.hpp
    ${RECLUSE_ENGINE_RENDERER_INCLUDE}/Primitive.hpp
    ${RECLUSE_ENGINE_RENDERER_INCLUDE}/Material.hpp
    ${RECLUSE_ENGINE_RENDERER_INCLUDE}/Mesh.hpp
    ${RECLUSE_ENGINE_RENDERER_INCLUDE}/RenderCommand.hpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/Renderer.cpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/AmbientOcclusionModule.cpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/AmbientOcclusionModule.hpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/AOVRenderModule.cpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/AOVRenderModule.hpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/PreZRenderModule.hpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/PreZRenderModule.cpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/LightClusterModule.hpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/LightClusterModule.cpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/PostProcessRenderModule.cpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/PostProcessRenderModule.hpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/Decals.hpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/Decals.cpp
    ${RECLUSE_ENGINE_RENDERER_SOURCE}/Texture2D.cpp
)