cmake_minimum_required(VERSION 3.10)
project(plugin_dll CXX)

set(CMAKE_CXX_STANDARD 11)

add_library(plugin SHARED
    plugin.cpp
)

if(MSVC)
    target_compile_options(plugin PRIVATE /W3)
endif()
