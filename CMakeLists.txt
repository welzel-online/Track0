#******************************************************************************
# Minimalistic CMake project file
#
# The main CMakeLists.txt file only includes the src subdirectory.
# The actual project CMakeLists.txt is located in the src subdirectory.
#
# Copyright (c) 2025 by Welzel-Online
#******************************************************************************
cmake_minimum_required( VERSION 3.10 )

project( Track0 )

add_subdirectory( src )
