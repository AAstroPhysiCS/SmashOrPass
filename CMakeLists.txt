cmake_minimum_required(VERSION 3.25)

project(
    SmashOrPass
    VERSION 0.1.0
    DESCRIPTION "SmashOrPass - The Game"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE SOP_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE SOP_HEADER_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(smashorpass
    ${SOP_SOURCE_FILES}
    ${SOP_HEADER_FILES}
)

target_include_directories(smashorpass PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_definitions(smashorpass PRIVATE
    SOP_ASSET_ROOT_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/assets\"
)

target_link_libraries(smashorpass PRIVATE
    SDL3::SDL3
    $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>
    SDL3_ttf::SDL3_ttf
    imgui::imgui
    fmt::fmt
    spdlog::spdlog
    glm::glm
    nlohmann_json::nlohmann_json
)
