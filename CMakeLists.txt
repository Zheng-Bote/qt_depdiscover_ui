cmake_minimum_required(VERSION 3.28)
project(QtDependencyTrackerUI 
    VERSION 0.2.0
    DESCRIPTION "Depdiscover Dashboard"
    HOMEPAGE_URL "https://github.com/Zheng-Bote/qt_depdiscover_ui"
    LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Configure (für rz_config.hpp) ---
set(EXECUTABLE_NAME "${PROJECT_NAME}")
set(PROG_LONGNAME "Dashboard for the Native C++ Dependency Scanner & SBOM Generator depdiscover")
set(PROG_CREATED "2026")
set(PROG_AUTHOR "ZHENG Bote")
set(PROG_ORGANIZATION_NAME "ZHENG Robert")
set(PROG_ORGANIZATION_DOMAIN "net.hase-zheng")
# create rz_config.hpp
add_subdirectory(configure) 

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Charts Network Concurrent)

include(FetchContent)
FetchContent_Declare(
    qt_gh_update_checker
    GIT_REPOSITORY https://github.com/Zheng-Bote/qt_gh_update_checker.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(qt_gh_update_checker)

include_directories(include)

file(GLOB_RECURSE SOURCES 
    src/*.cpp
    include/*.hpp
)

add_executable(${PROJECT_NAME} 
    ${SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Gui
    Qt6::Charts
    Qt6::Network
    Qt6::Concurrent
    qt_gh_update_checker
)

# Standard flags
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /std:c++latest /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -std=c++23 -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

# --- 4. Installation ---
include(GNUInstallDirs)
install(TARGETS QtDependencyTrackerUI
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install icon and desktop file
install(FILES resources/app_icon.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps/ RENAME QtDependencyTrackerUI.png)
install(FILES resources/QtDependencyTrackerUI.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# --- 5. Custom Generators (Original Code) ---

# CMake File API Query (Datei erzeugen)
set(QUERY_DIR "${CMAKE_BINARY_DIR}/.cmake/api/v1/query/client-project")
file(MAKE_DIRECTORY "${QUERY_DIR}")
file(WRITE "${QUERY_DIR}/codemodel-v2" "") 

# libs.txt Generierung (Timing-sicher via GENERATE)
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/libs.txt" 
     CONTENT "$<TARGET_PROPERTY:QtDependencyTrackerUI,LINK_LIBRARIES>")
