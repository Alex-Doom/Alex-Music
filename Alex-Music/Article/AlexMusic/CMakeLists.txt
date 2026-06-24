cmake_minimum_required(VERSION 3.16)
project(AlexMusic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)

if(WIN32)
    set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.ico")
    if(EXISTS "${APP_ICON}")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.ico"
            "${CMAKE_CURRENT_BINARY_DIR}/app_icon.ico"
            COPYONLY
        )
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc"
            "IDI_ICON1 ICON DISCARDABLE \"app_icon.ico\"\n"
        )
        set(RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
    endif()
endif()

add_executable(AlexMusic
    main.cpp
    MainWindow.h MainWindow.cpp
    Track.h Track.cpp
    Playlist.h Playlist.cpp
    PlayerControls.h PlayerControls.cpp
    ${RC_FILE}

    FastMetadataReader.h FastMetadataReader.cpp
)

if(WIN32)
    set_target_properties(AlexMusic PROPERTIES WIN32_EXECUTABLE TRUE)
    target_link_libraries(AlexMusic
        Qt6::Widgets
        Qt6::Multimedia
        ole32
        shell32
        user32
    )
else()
    target_link_libraries(AlexMusic
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
    )
endif()
