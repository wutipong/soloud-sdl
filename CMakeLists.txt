cmake_minimum_required(VERSION 3.17)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

# set the project name
project(soloud-sdl)

# add the executable
add_executable(soloud-sdl 
  "main.cpp" 
  "soloud/src/audiosource/sfxr/soloud_sfxr.cpp"
  "soloud/src/audiosource/speech/darray.cpp"
  "soloud/src/audiosource/speech/klatt.cpp"
  "soloud/src/audiosource/speech/resonator.cpp"
  "soloud/src/audiosource/speech/soloud_speech.cpp"
  "soloud/src/audiosource/speech/tts.cpp"
  "soloud/src/audiosource/wav/dr_impl.cpp"
  "soloud/src/audiosource/wav/soloud_wav.cpp"
  "soloud/src/audiosource/wav/soloud_wavstream.cpp"
  "soloud/src/audiosource/wav/stb_vorbis.c"
  "soloud/src/backend/sdl/soloud_sdl2.cpp"
  "soloud/src/backend/sdl/soloud_sdl2_dll.c"
  "soloud/src/core/soloud.cpp"
  "soloud/src/core/soloud_audiosource.cpp"
  "soloud/src/core/soloud_bus.cpp"
  "soloud/src/core/soloud_core_3d.cpp"
  "soloud/src/core/soloud_core_basicops.cpp"
  "soloud/src/core/soloud_core_faderops.cpp"
  "soloud/src/core/soloud_core_filterops.cpp"
  "soloud/src/core/soloud_core_getters.cpp"
  "soloud/src/core/soloud_core_setters.cpp"
  "soloud/src/core/soloud_core_voicegroup.cpp"
  "soloud/src/core/soloud_core_voiceops.cpp"
  "soloud/src/core/soloud_fader.cpp"
  "soloud/src/core/soloud_fft.cpp"
  "soloud/src/core/soloud_fft_lut.cpp"
  "soloud/src/core/soloud_file.cpp"
  "soloud/src/core/soloud_filter.cpp"
  "soloud/src/core/soloud_misc.cpp"
  "soloud/src/core/soloud_queue.cpp"
  "soloud/src/core/soloud_thread.cpp"
)

add_compile_definitions(WITH_SDL2)

target_compile_features(soloud-sdl PRIVATE cxx_std_20)
set_property(TARGET soloud-sdl PROPERTY CXX_STANDARD 20)
set_property(TARGET soloud-sdl PROPERTY CXX_STANDARD_REQUIRED ON)

if (MSVC)
    set_property(TARGET soloud-sdl PROPERTY WIN32_EXECUTABLE ON)
    target_compile_options(soloud-sdl PRIVATE /Zc:__cplusplus)
endif ()

find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

target_include_directories(soloud-sdl PRIVATE "soloud/include")
target_include_directories(soloud-sdl PRIVATE "imgui-filebrowser")

target_link_libraries(soloud-sdl PRIVATE
        SDL2::SDL2 SDL2::SDL2main
        imgui::imgui
)