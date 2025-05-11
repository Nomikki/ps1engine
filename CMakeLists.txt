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
# Add position independent code flag for the library, if not already covered by other flags
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Linker directories (if you have local .lib or .so files in a 'lib' directory)
link_directories(lib)

# SFML libraries
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# --- PS1Engine Library --- 
set(ENGINE_SOURCE_FILES
    ps1engine/src/engine.cpp
    ps1engine/src/utility.cpp
    ps1engine/src/mesh.cpp
    ps1engine/src/meshManager.cpp
    ps1engine/src/component.cpp
    ps1engine/src/componentManager.cpp
    ps1engine/src/transform.cpp
    ps1engine/src/camera.cpp
    ps1engine/src/trig_tables.cpp
)

add_library(PS1EngineLib ${ENGINE_SOURCE_FILES})

target_include_directories(PS1EngineLib PUBLIC ps1engine/include)
target_link_libraries(PS1EngineLib PUBLIC sfml-graphics sfml-window sfml-system)

# --- ModelViewer Example --- 
set(MODEL_VIEWER_SOURCE_FILES
    examples/model_viewer/main.cpp
)

add_executable(ModelViewerExample ${MODEL_VIEWER_SOURCE_FILES})

# Link example against the engine library
# The engine library already makes its include directories and SFML linkage PUBLIC,
# so ModelViewerExample should inherit them.
target_link_libraries(ModelViewerExample PRIVATE PS1EngineLib)
# Explicitly link SFML if main.cpp directly uses SFML APIs not covered by engine headers
# target_link_libraries(ModelViewerExample PRIVATE sfml-graphics sfml-window sfml-system)

# Set output directory for the executable
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
# Set output directory for the library (optional, typically goes into CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
set(ARCHIVE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib) # Store .a or .lib files in build/lib
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ARCHIVE_OUTPUT_PATH})

# Custom clean command (optional, CMake handles most cleaning)
add_custom_target(clean_build
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR} # Recreate build dir
    COMMENT "Cleaning build directory"
)

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}") 