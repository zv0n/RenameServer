cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)


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
