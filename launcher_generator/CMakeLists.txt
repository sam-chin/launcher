cmake_minimum_required(VERSION 3.10)
project(launcher_generator CXX)

set(CMAKE_CXX_STANDARD 11)

find_package(MFC REQUIRED)

add_executable(launcher_generator WIN32
    main.cpp
    launcher_generator.rc
    ../game_launcher/config.c
)

target_include_directories(launcher_generator PRIVATE
    ../game_launcher
)

target_link_libraries(launcher_generator PRIVATE
    ${MFC_LIBRARIES}
    user32
    gdi32
    shell32
)

if(MSVC)
    target_compile_definitions(launcher_generator PRIVATE 
        _AFXDLL
        _WIN32_WINNT=0x0601
        _UNICODE
        UNICODE
    )
    target_compile_options(launcher_generator PRIVATE /W3 /utf-8)
endif()
