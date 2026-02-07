cmake_minimum_required(VERSION 3.16) # min версия Cmake - 3.16
project(AlexMusic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # стандарт С++ - 17 года
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Включение автоматической генерации MOC файлов для Qt
set(CMAKE_AUTOMOC ON)
# Включение автоматической компиляции UI файлов
set(CMAKE_AUTOUIC ON)
# Включение автоматической компиляции ресурсных файлов
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)

# Создаем .rc файл для иконки
if(WIN32)
    set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.ico")
    if(EXISTS "${APP_ICON}")
        configure_file( # копирование файлов
            "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.ico" # ${CMAKE_CURRENT_SOURCE_DIR} - встроенная переменная, содержит путь к текущей исходной директории
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

add_executable(AlexMusic
    main.cpp
    MainWindow.h MainWindow.cpp
    Track.h Track.cpp
    Playlist.h Playlist.cpp
    PlayerControls.h PlayerControls.cpp
    ${RC_FILE}
    TrackText.h
    TrackValidator.h
    TrackValidator.cpp
    BadTrackDialog.h
    BadTrackDialog.cpp
    resource_finder.h
    HtmlDelegate.h
    HtmlDelegate.cpp
    SettingsDialog.h
    SettingsDialog.cpp
    Mp3Metadata.h
    Mp3Metadata.cpp

)
# Установка совйств файла .exe (Windows)
if(WIN32)
    set_target_properties(AlexMusic PROPERTIES WIN32_EXECUTABLE TRUE)

    # Windows API библиотеки для thumbnail toolbar-миниатюры на панели задач
    target_link_libraries(AlexMusic
        # Основные классы Qt
        Qt6::Widgets     # Виджеты GUI
        Qt6::Multimedia  # Мультимедийные возможности
        ole32            # COM (Component Object Model) для Windows
        shell32          # Windows Shell API
        user32           # Windows GUI API
    )
else()
    target_link_libraries(AlexMusic
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
    )
endif()
