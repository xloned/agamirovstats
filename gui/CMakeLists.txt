cmake_minimum_required(VERSION 3.16)
project(StatisticalAnalysisGUI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Убрать зависимость от устаревших фреймворков macOS
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
endif()

# Указать директорию для UI файлов
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

# Найти Qt6 или Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Charts)

# Найти Boost
find_package(Boost REQUIRED)

# Включить директории
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${Boost_INCLUDE_DIRS}
)

# UI файлы
set(UI_FILES
    ui/mainwindow.ui
)

# Заголовочные файлы GUI
set(GUI_HEADERS
    include/mainwindow.h
    include/statisticsworker.h
    include/chartviewer.h
    include/dataeditor.h
)

# Исходные файлы GUI
set(GUI_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/statisticsworker.cpp
    src/chartviewer.cpp
    src/dataeditor.cpp
)

# Исходные файлы библиотеки статистики (из родительской директории)
set(STATS_SOURCES
    ../src/boost_distributions.cpp
    ../src/matrix_operations.cpp
    ../src/nelder_mead.cpp
    ../src/mle_methods.cpp
    ../src/confidence_intervals.cpp
    ../src/order.cpp
    ../src/statistical_tests.cpp
    ../src/anova.cpp
    ../src/shapiro_wilk.cpp
    ../src/wilcoxon_ranksum.cpp
)

# Ресурсы
set(RESOURCES
    resources/resources.qrc
)

# Создать исполняемый файл
add_executable(${PROJECT_NAME}
    ${GUI_SOURCES}
    ${GUI_HEADERS}
    ${STATS_SOURCES}
    ${UI_FILES}
    ${RESOURCES}
)

# Линковка с Qt и Boost
target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    ${Boost_LIBRARIES}
    
)

# macOS специфичные настройки
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    )
endif()

# Копировать данные в build директорию
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/../input
    ${CMAKE_CURRENT_BINARY_DIR}/input
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    ${CMAKE_CURRENT_BINARY_DIR}/output
)

message(STATUS "Qt version: ${QT_VERSION}")
message(STATUS "Boost found: ${Boost_FOUND}")
