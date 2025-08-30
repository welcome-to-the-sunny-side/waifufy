cmake_minimum_required(VERSION 3.16)
project(waifufy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
