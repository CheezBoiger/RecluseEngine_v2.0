# CMake third party projects to include in the recluse engine foundation and engine components.
# Most of these projects are to be added via the git submodule command, so be sure to initiate before actually building the engine.
# Use the git submodule update --init --recursive command to bring the proper module prerequisities.

set ( RECLUSE_THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)

# TODO: Disable for now, until we can figure out what we need to do to properly compile this, and not have to use an existing binary :D.
#add_subdirectory ( ${RECLUSE_THIRDPARTY_DIR}/DirectXShaderCompiler )
add_subdirectory ( ${RECLUSE_THIRDPARTY_DIR}/wxWidgets )
add_subdirectory ( ${RECLUSE_THIRDPARTY_DIR}/zlib )
add_subdirectory ( ${RECLUSE_THIRDPARTY_DIR}/googletest )