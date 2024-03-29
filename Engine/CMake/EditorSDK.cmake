set ( RECLUSE_EDITOR_SDK_INCLUDE_DIR ${RECLUSE_ENGINE_INCLUDE_DIR}/Recluse/EditorSDK )

add_compile_definitions( RECLUSE_EDITOR_SDK_ENABLE=1 )

set (RECLUSE_ENGINE_COMPILE_FILES
	${RECLUSE_ENGINE_COMPILE_FILES}
	${RECLUSE_EDITOR_SDK_INCLUDE_DIR}/EditorDeclarations.hpp
)