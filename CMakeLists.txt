cmake_minimum_required(VERSION 3.25)

project(
    SmashOrPass
    VERSION 0.1.0
    DESCRIPTION "Co-op boxing game built with SDL and ImGui"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SOP_BUILD_TESTS "Build unit tests" ON)
option(SOP_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(SOP_ENABLE_SANITIZERS "Enable sanitizers for supported compilers" OFF)
option(SOP_ENABLE_LTO "Enable link-time optimization" OFF)
option(SOP_ENABLE_IMGUI_DEMO "Build with ImGui demo window enabled" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(CMakeDependentOption)
include(CheckIPOSupported)
include(PreventInSourceBuilds)

if(SOP_ENABLE_LTO)
    check_ipo_supported(RESULT sop_ipo_supported OUTPUT sop_ipo_error)
    if(sop_ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(WARNING "IPO/LTO requested, but not supported: ${sop_ipo_error}")
    endif()
endif()

find_package(SDL3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(sop_project_options INTERFACE)
add_library(sop_project_warnings INTERFACE)

if(SOP_ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(sop_project_warnings INTERFACE /W4 /permissive- /EHsc)
    else()
        target_compile_options(
            sop_project_warnings
            INTERFACE
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wsign-conversion
                -Wshadow
                -Wnon-virtual-dtor
                -Wold-style-cast
                -Woverloaded-virtual
                -Wnull-dereference
                -Wdouble-promotion
                -Wformat=2
        )
    endif()
endif()

if(SOP_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(sop_project_options INTERFACE -fsanitize=address,undefined)
    target_link_options(sop_project_options INTERFACE -fsanitize=address,undefined)
endif()

# -----------------------------
# Auto-discover source/header files
# -----------------------------

# Public headers intended for consumers of the library.
file(GLOB_RECURSE SOP_PUBLIC_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.ipp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.tpp"
)

# Private/internal headers under src/ so they also show in Visual Studio.
file(GLOB_RECURSE SOP_PRIVATE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hh"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ipp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.tpp"
)

# All implementation files under src/.
file(GLOB_RECURSE SOP_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
)

# EntryPoint.cpp belongs only to the executable, not the core library.
list(FILTER SOP_CORE_SOURCES EXCLUDE REGEX [[(^|[/\\])EntryPoint\.(c|cc|cpp|cxx)$]])

# Unit tests.
file(GLOB_RECURSE SOP_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cxx"
)

# -----------------------------
# Core library
# -----------------------------

add_library(smashorpass_core)
add_library(SmashOrPass::Core ALIAS smashorpass_core)

target_sources(smashorpass_core
    PRIVATE
        ${SOP_CORE_SOURCES}
        ${SOP_PRIVATE_HEADERS}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
        FILES ${SOP_PUBLIC_HEADERS}
)

target_include_directories(smashorpass_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(smashorpass_core
    PUBLIC
        SDL3::SDL3
        spdlog::spdlog
        glm::glm
        nlohmann_json::nlohmann_json
        sop_project_options
        sop_project_warnings
    PRIVATE
        imgui::imgui
)

target_compile_definitions(smashorpass_core
    PUBLIC
        $<$<BOOL:${SOP_ENABLE_IMGUI_DEMO}>:SOP_ENABLE_IMGUI_DEMO>
)

if(WIN32)
    target_compile_definitions(smashorpass_core PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

# -----------------------------
# Main executable
# -----------------------------

add_executable(smashorpass
    "${CMAKE_CURRENT_SOURCE_DIR}/src/EntryPoint.cpp"
)
add_executable(SmashOrPass::Game ALIAS smashorpass)

target_link_libraries(smashorpass PRIVATE SmashOrPass::Core)

# -----------------------------
# Visual Studio / IDE grouping
# -----------------------------

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PREFIX "Header Files"
    FILES ${SOP_PUBLIC_HEADERS}
)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PREFIX "Source Files"
    FILES
        ${SOP_CORE_SOURCES}
        ${SOP_PRIVATE_HEADERS}
        "${CMAKE_CURRENT_SOURCE_DIR}/src/EntryPoint.cpp"
)

if(SOP_BUILD_TESTS AND SOP_TEST_SOURCES)
    source_group(
        TREE "${CMAKE_CURRENT_SOURCE_DIR}/tests"
        PREFIX "Test Files"
        FILES ${SOP_TEST_SOURCES}
    )
endif()

# -----------------------------
# Tests
# -----------------------------

if(SOP_BUILD_TESTS)
    include(CTest)
    enable_testing()

    find_package(Catch2 CONFIG REQUIRED)

    if(SOP_TEST_SOURCES)
        add_executable(smashorpass_tests ${SOP_TEST_SOURCES})
        target_link_libraries(smashorpass_tests PRIVATE SmashOrPass::Core Catch2::Catch2WithMain)

        include(Catch)
        catch_discover_tests(smashorpass_tests)
    else()
        message(STATUS "SOP_BUILD_TESTS is ON, but no test source files were found under tests/")
    endif()
endif()

# -----------------------------
# Install
# -----------------------------

install(TARGETS smashorpass_core
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS smashorpass
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)