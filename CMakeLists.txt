cmake_minimum_required(VERSION 3.16)
project(waifufy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Optimize builds
add_compile_options(-O3 -DNDEBUG)

# Library
add_library(waifufy_core STATIC
    src/waifufy_core.cpp
)
target_include_directories(waifufy_core PUBLIC include)

# CLI
add_executable(waifufy
    src/main.cpp
)
target_link_libraries(waifufy waifufy_core)
