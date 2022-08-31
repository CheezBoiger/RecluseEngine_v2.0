# CMake Entry point build.
cmake_minimum_required ( VERSION 3.16 )
project ( "RecluseProject v2.0" )

# Mostly configurations are set here.

include ( ThirdParty/ThirdPartyIncludes.cmake )

# Framework subdirectory.
add_subdirectory ( Framework )

# Add shader compiler subdirectory.
add_subdirectory( ShaderCompiler )

# Engine subdirectory.
add_subdirectory ( Engine )

# Editor subdirectory
add_subdirectory ( RecluseEditor )