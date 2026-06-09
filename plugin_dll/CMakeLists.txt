cmake_minimum_required(VERSION 3.10)
project(plugin_dll CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(plugin SHARED
    plugin.cpp
)

if(MSVC)
    target_compile_definitions(plugin PRIVATE _WIN32_WINNT=0x0601)
    target_compile_options(plugin PRIVATE /W3)
    target_compile_options(plugin PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()
