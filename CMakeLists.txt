cmake_minimum_required(VERSION 3.16)

project(untitled VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Политика для ускорения FetchContent (доступна с CMake 3.24)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# ------------------------------------------------------------------
# 1. Библиотека CPR (C++ Requests) через FetchContent
# ------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG        1.11.0                # тег без префикса 'v'
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(cpr)

# ------------------------------------------------------------------
# 2. Поиск Qt
# ------------------------------------------------------------------
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick Widgets LinguistTools)

# ------------------------------------------------------------------
# 3. SQLite3 (встраиваем исходники, без внешней библиотеки)
# ------------------------------------------------------------------
# Путь к папке с файлами sqlite3.c и sqlite3.h (амальгама)
set(SQLITE3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sqlite3")
set(SQLITE3_SRC "${SQLITE3_DIR}/sqlite3.c")
set(SQLITE3_HDR "${SQLITE3_DIR}/sqlite3.h")

# Проверяем наличие файлов
if(NOT EXISTS ${SQLITE3_SRC} OR NOT EXISTS ${SQLITE3_HDR})
    message(FATAL_ERROR
        "SQLite3 amalgamation files not found in ${SQLITE3_DIR}.\n"
        "Please download from https://www.sqlite.org/download.html and place sqlite3.c and sqlite3.h there.")
endif()

# Добавляем путь к заголовкам, чтобы #include <sqlite3.h> работал
include_directories(${SQLITE3_DIR})

# ------------------------------------------------------------------
# 4. Файлы перевода
# ------------------------------------------------------------------
set(TS_FILES untitled_ru_RU.ts)

# ------------------------------------------------------------------
# 5. Основные исходники проекта
# ------------------------------------------------------------------
set(PROJECT_SOURCES
    main.cpp
    qml.qrc
    ${TS_FILES}
    ${SQLITE3_SRC}          # добавляем исходник SQLite3
)


# Дополнительные файлы (printermanager, params, docxtopdf)
set(EXTRA_SOURCES
    scr/printermanager.h
    scr/printermanager.cpp
    scr/params.cpp
    scr/params.h
    scr/docxtopdf.cpp
    scr/docxtopdf.h
    scr/db_manager.h
    scr/db_manager.cpp
    scr/sqlite.h
    scr/sqlite.cpp
    scr/dropboxapi.h
    scr/dropboxapi.cpp
    scr/userip.h
    scr/userip.cpp
)

# ------------------------------------------------------------------
# 6. Создание исполняемого файла (с учётом версии Qt)
# ------------------------------------------------------------------
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(untitled
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${EXTRA_SOURCES}


    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(untitled SHARED
            ${PROJECT_SOURCES}
            ${EXTRA_SOURCES}
        )
    else()
        add_executable(untitled
            ${PROJECT_SOURCES}
            ${EXTRA_SOURCES}
        )
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# ------------------------------------------------------------------
# 7. Линковка с Qt и CPR
# ------------------------------------------------------------------
target_link_libraries(untitled
    PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick Qt${QT_VERSION_MAJOR}::Widgets
    PRIVATE cpr::cpr
    PRIVATE iphlpapi ws2_32          # <-- добавлено
)

# ------------------------------------------------------------------
# 8. Настройки для macOS/iOS и Windows
# ------------------------------------------------------------------
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.untitled)
endif()
set_target_properties(untitled PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# ------------------------------------------------------------------
# 9. Установка (для всех платформ)
# ------------------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS untitled
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# ------------------------------------------------------------------
# 10. Финализация для Qt6
# ------------------------------------------------------------------
if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(untitled)
    qt_finalize_executable(untitled)
endif()
