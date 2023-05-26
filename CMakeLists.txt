cmake_minimum_required(VERSION 3.9)

project(kd-tree LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

if(MSVC)
    # default startup project for Visual Studio
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT test)
    add_compile_options(/MP /wd4819 /wd4996)
    add_definitions(-DUNICODE -D_UNICODE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(test)