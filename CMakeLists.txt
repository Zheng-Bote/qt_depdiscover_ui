cmake_minimum_required(VERSION 3.28)
project(QtDependencyTrackerUI 
    VERSION 0.1.0
    DESCRIPTION "Depdiscover Dashboard"
    HOMEPAGE_URL "https://github.com/Zheng-Bote/qt_depdiscover_ui"
    LANGUAGES CXX)

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
