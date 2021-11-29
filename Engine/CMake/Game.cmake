#

set ( RECLUSE_GAME_INCLUDE_DIR ${RECLUSE_ENGINE_INCLUDE_DIR}/Recluse/Game )
set ( RECLUSE_GAME_COMPONENTS_INCLUDE_DIR ${RECLUSE_GAME_INCLUDE_DIR}/Components )
set ( RECLUSE_GAME_SOURCE_DIR ${RECLUSE_ENGINE_SOURCE_DIR}/Game )

set ( RECLUSE_ENGINE_COMPILE_FILES
    ${RECLUSE_ENGINE_COMPILE_FILES}
    ${RECLUSE_GAME_INCLUDE_DIR}/Component.hpp
    ${RECLUSE_GAME_INCLUDE_DIR}/GameObject.hpp
    ${RECLUSE_GAME_INCLUDE_DIR}/ObjectSerializer.hpp
    ${RECLUSE_GAME_COMPONENTS_INCLUDE_DIR}/Transform.hpp
    ${RECLUSE_GAME_COMPONENTS_INCLUDE_DIR}/RendererComponent.hpp
	${RECLUSE_ENGINE_INCLUDE_DIR}/Recluse/Application.hpp
	${RECLUSE_ENGINE_INCLUDE_DIR}/Recluse/MessageBus.hpp
	${RECLUSE_ENGINE_SOURCE_DIR}/MessageBus.cpp
	${RECLUSE_ENGINE_SOURCE_DIR}/Application.cpp
	${RECLUSE_ENGINE_INCLUDE_DIR}/Recluse/EngineModule.hpp
)