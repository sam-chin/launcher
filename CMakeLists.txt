cmake_minimum_required(VERSION 3.10)
project(game_launcher_project C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(encrypt_tool)
add_subdirectory(game_launcher)
add_subdirectory(launcher_generator)
add_subdirectory(plugin_dll)
