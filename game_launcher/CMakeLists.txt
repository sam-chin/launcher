cmake_minimum_required(VERSION 3.10)
project(game_launcher CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(game_launcher WIN32
    main.cpp
    config.c
    dll_loader.c
    game_launcher.rc
    ../common/crc32/crc32.c
    ../common/encrypt/encrypt.c
    ../common/file_pack/file_pack.c
)

target_include_directories(game_launcher PRIVATE
    ../common/crc32
    ../common/encrypt
    ../common/file_pack
)

target_link_libraries(game_launcher PRIVATE
    user32
    gdi32
    comctl32
)

if(MSVC)
    target_compile_definitions(game_launcher PRIVATE 
        _AFXDLL 
        _WIN32_WINNT=0x0601
    )
    target_compile_options(game_launcher PRIVATE /W3)
endif()
