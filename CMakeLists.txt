# CMake Entry point build.
cmake_minimum_required ( VERSION 3.0 )
project ( "RecluseProject 2.0v" )

# Mostly configurations are set here.

# Framework subdirectory.
add_subdirectory ( Framework )

# Engine subdirectory.
add_subdirectory ( Engine )