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

find_package(SDL2 CONFIG REQUIRED)
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

add_library(smashorpass_core
    src/app/Application.cpp
    src/core/Game.cpp
    src/debug/DebugOverlay.cpp
    src/platform/Window.cpp
    src/rendering/Renderer.cpp
)

add_library(SmashOrPass::Core ALIAS smashorpass_core)

target_include_directories(smashorpass_core
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(smashorpass_core
    PUBLIC
        SDL2::SDL2
        fmt::fmt
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

add_executable(smashorpass
    src/EntryPoint.cpp
)

add_executable(SmashOrPass::Game ALIAS smashorpass)

target_link_libraries(smashorpass PRIVATE SmashOrPass::Core)

if(SOP_BUILD_TESTS)
    include(CTest)
    enable_testing()

    find_package(Catch2 CONFIG REQUIRED)

    add_executable(smashorpass_tests
        tests/GameConfigTests.cpp
    )

    target_link_libraries(smashorpass_tests
        PRIVATE
            SmashOrPass::Core
            Catch2::Catch2WithMain
    )

    include(Catch)
    catch_discover_tests(smashorpass_tests)
endif()


install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

