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
)