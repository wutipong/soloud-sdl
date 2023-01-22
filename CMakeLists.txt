cmake_minimum_required(VERSION 3.17)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

# set the project name
project(soloud-sdl)

# add the executable
add_executable(soloud-sdl "main.cpp" )

target_compile_features(soloud-sdl PRIVATE cxx_std_17)
set_property(TARGET soloud-sdl PROPERTY CXX_STANDARD 17)
set_property(TARGET soloud-sdl PROPERTY CXX_STANDARD_REQUIRED ON)

if (MSVC)
    set_property(TARGET soloud-sdl PROPERTY WIN32_EXECUTABLE ON)
    target_compile_options(soloud-sdl PRIVATE /Zc:__cplusplus)
endif ()

find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

target_link_libraries(soloud-sdl PRIVATE
        SDL2::SDL2 SDL2::SDL2main
        imgui::imgui
)