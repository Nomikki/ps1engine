cmake_minimum_required(VERSION 3.10)

project(PS1Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set build type to Release for optimizations, can be overridden by user
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -s -O3 -march=native -ffast-math -funroll-loops -msse")

# Include directories
include_directories(include)

# Linker directories (if you have local .lib or .so files in a 'lib' directory)
link_directories(lib) 

# SFML libraries
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/engine.cpp
    src/utility.cpp
    src/mesh.cpp
    src/meshManager.cpp
    src/component.cpp
    src/componentManager.cpp
    src/transform.cpp
    src/camera.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system)

# Set output directory for the executable
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Custom clean command (optional, CMake handles most cleaning)
add_custom_target(clean_build
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR} # Recreate build dir
    COMMENT "Cleaning build directory"
)

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}") 