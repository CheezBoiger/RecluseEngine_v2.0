# CMake Entry point build.
cmake_minimum_required ( VERSION 3.0 )
project ( "RecluseProject 2.0v" )

# Mostly configurations are set here.

add_subdirectory ( Engine/CMake )
add_subdirectory ( Game/CMake )