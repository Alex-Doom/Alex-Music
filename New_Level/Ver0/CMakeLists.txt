cmake_minimum_required(VERSION 3.16)

project(AlexMusic VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia Concurrent)
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)

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

set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AlexMusic
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        BadTrackDialog.h
        FastMetadataReader.h
        HtmlDelegate.h
        Mp3Metadata.h
        PlayerControls.h
        Playlist.h
        resource_finder.h
        ScannerThread.h
        SettingsDialog.h
        Track.h
        TrackValidator.h
        TrackText.h
        BadTrackDialog.cpp
        FastMetadataReader.cpp
        HtmlDelegate.cpp
        Mp3Metadata.cpp
        PlayerControls.cpp
        Playlist.cpp
        ScannerThread.cpp
        SettingsDialog.cpp
        Track.cpp
        TrackValidator.cpp
        FastTagReader.h
        FastTagReader.cpp
    )
else()
    if(ANDROID)
        add_library(AlexMusic SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(AlexMusic
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.AlexMusic)
endif()

set_target_properties(AlexMusic PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# === ВСЯ ЛИНКОВКА ОДНИМ БЛОКОМ (ДО install и finalize!) ===
if(WIN32)
    target_link_libraries(AlexMusic PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Core5Compat
        Qt6::Multimedia
        Qt6::Concurrent
        ole32
        shell32
        user32
        ${TAGLIB_LIBRARIES}
    )
else()
    target_link_libraries(AlexMusic PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        ${TAGLIB_LIBRARIES}
    )
endif()

target_include_directories(AlexMusic PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TAGLIB_INCLUDE_DIRS}
    ${TAGLIB_INCLUDE_DIRS}
    ${Qt6Core5Compat_INCLUDE_DIRS}
)

include(GNUInstallDirs)
install(TARGETS AlexMusic
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AlexMusic)
endif()
