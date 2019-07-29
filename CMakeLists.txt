cmake_minimum_required(VERSION 3.14)
project(vxrt_vulkan)

set(CMAKE_CXX_STANDARD 17)

# Set search paths
set(PROJECT_ROOT_PATH ${CMAKE_SOURCE_DIR})
set(DEPS_INCLUDE_PATH "" CACHE PATH "The path to depended includes")
set(DEPS_LIB_PATH "" CACHE PATH "The path to depended libraries")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${DEPS_INCLUDE_PATH} ${DEPS_LIB_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_ROOT_PATH}/cmake)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# Link SDL2
set(DEPS_INCLUDE ${DEPS_INCLUDE} ${SDL2_INCLUDE_DIR})
set(DEPS_LIB ${DEPS_LIB} ${SDL2_LIBRARY}
    Vulkan::Vulkan glslang glslSPIRV glslang-default-resource-limits
    Boost::system
    )

# Build GLSLANG
add_subdirectory(glslang-lib glslang)

# Scan Source
file(GLOB_RECURSE SRC "${CMAKE_SOURCE_DIR}/source/*.*")
add_executable(vxrt_vulkan ${SRC})
target_include_directories(vxrt_vulkan PUBLIC ${DEPS_INCLUDE})
target_link_libraries(vxrt_vulkan ${DEPS_LIB})
