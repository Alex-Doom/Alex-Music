cmake_minimum_required(VERSION 3.16)
project(CURS_WORK LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)

add_executable(CURS_WORK
    main.cpp
    MainWindow.h MainWindow.cpp
    Track.h Track.cpp
    Playlist.h Playlist.cpp
    PlayerControls.h PlayerControls.cpp
)

target_link_libraries(CURS_WORK
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
)
