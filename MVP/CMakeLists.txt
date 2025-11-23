cmake_minimum_required(VERSION 3.16)
project(CURS_WORK LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)

# Создаем .rc файл для иконки
if(WIN32)
    set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.ico")
    if(EXISTS "${APP_ICON}")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.ico"
            "${CMAKE_CURRENT_BINARY_DIR}/app_icon.ico"
            COPYONLY
        )

        # Создаем .rc файл
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc"
            "IDI_ICON1 ICON DISCARDABLE \"app_icon.ico\"\n"
        )

        set(RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
    endif()
endif()

add_executable(CURS_WORK
    main.cpp
    MainWindow.h MainWindow.cpp
    Track.h Track.cpp
    Playlist.h Playlist.cpp
    PlayerControls.h PlayerControls.cpp
    ${RC_FILE}
)

if(WIN32)
    set_target_properties(CURS_WORK PROPERTIES WIN32_EXECUTABLE TRUE)

    # Windows API библиотеки для thumbnail toolbar
    target_link_libraries(CURS_WORK
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        ole32      # Для COM
        shell32    # Для Windows Shell API
        user32     # Для Windows GUI API
    )
else()
    target_link_libraries(CURS_WORK
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
    )
endif()
