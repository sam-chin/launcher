cmake_minimum_required(VERSION 3.10)
project(encrypt_tool C)

set(CMAKE_C_STANDARD 99)

add_executable(encrypt_tool
    main.c
    ../common/crc32/crc32.c
    ../common/encrypt/encrypt.c
    ../common/file_pack/file_pack.c
)

target_include_directories(encrypt_tool PRIVATE
    ../common/crc32
    ../common/encrypt
    ../common/file_pack
)

if(MSVC)
    target_compile_definitions(encrypt_tool PRIVATE _WIN32_WINNT=0x0601)
    target_compile_options(encrypt_tool PRIVATE /W3)
endif()
