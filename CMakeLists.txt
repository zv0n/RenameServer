cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

if(APPLE)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif()

find_package(Catch2 3 REQUIRED)

project(RenameServer)

add_executable(RenameServer
    main.cpp
    rename_object.cpp
    functions.cpp
    config/config.cpp
    filesystem/unix/filesystem.cpp
    jwt.cpp
    )
target_link_libraries(RenameServer restbed config++ jwt)
if(UNIX AND NOT APPLE)
    target_link_libraries(RenameServer dl)
endif()

add_library(thetvdb SHARED
    thetvdb/tv_rename.cpp
    thetvdb/functions.cpp
    rename_object.cpp
    functions.cpp
    filesystem/unix/filesystem.cpp
    network/unix/network.cpp)

target_link_libraries(thetvdb curl)

add_library(simple SHARED
    simple_rename/simple.cpp
    rename_object.cpp
    functions.cpp
    filesystem/unix/filesystem.cpp)

add_library(moviedb SHARED
    themoviedb/moviedb.cpp
    themoviedb/functions.cpp
    rename_object.cpp
    functions.cpp
    filesystem/unix/filesystem.cpp
    network/unix/network.cpp)

target_link_libraries(moviedb curl)

add_executable(tests
    tests/tests.cpp
    rename_object.cpp
    functions.cpp
    config/config.cpp
    filesystem/unix/filesystem.cpp
    jwt.cpp
    )
set_source_files_properties(tests/tests.cpp PROPERTIES COMPILE_OPTIONS "--coverage;-g;-fsanitize=address,undefined")
target_compile_options(tests PUBLIC -Wall -fprofile-arcs -ftest-coverage --coverage -g)
target_link_options(tests PUBLIC --coverage -Wall -fprofile-arcs -ftest-coverage -fsanitize=address,undefined)
target_link_libraries(tests PRIVATE restbed config++ jwt Catch2::Catch2WithMain)
