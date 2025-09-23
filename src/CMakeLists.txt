#******************************************************************************
# Main CMake project file
#
# Copyright (c) 2025 by Welzel-Online
#******************************************************************************
cmake_minimum_required( VERSION 3.10 )

# Include the versioning script
include( "version.cmake" )


#SET(CMAKE_CXX_FLAGS "-O3")
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections" )
# Linker flags for static linking and dead code elimination
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -Wl,-Bstatic,--gc-sections -Wl,--start-group -lstdc++ -Wl,--end-group" )


add_executable( track0 track0.cpp version.rc )

target_link_libraries( track0 )

add_custom_command( TARGET track0
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E echo "Copy the compiled binary to the ${CMAKE_BUILD_TYPE} folder."
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/${CMAKE_BUILD_TYPE}
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:track0> ${CMAKE_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/.
)
