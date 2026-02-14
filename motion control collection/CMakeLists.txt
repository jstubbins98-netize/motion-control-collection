cmake_minimum_required(VERSION 3.10)
project(MotionControlCollection)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    execute_process(COMMAND brew --prefix OUTPUT_VARIABLE HOMEBREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(HOMEBREW_PREFIX)
        link_directories(${HOMEBREW_PREFIX}/lib)
        include_directories(${HOMEBREW_PREFIX}/include)
        set(ENV{PKG_CONFIG_PATH} "${HOMEBREW_PREFIX}/lib/pkgconfig:${HOMEBREW_PREFIX}/opt/sdl2/lib/pkgconfig:${HOMEBREW_PREFIX}/opt/sdl2_mixer/lib/pkgconfig:${HOMEBREW_PREFIX}/opt/opencv/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    endif()
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
pkg_check_modules(OPENCV REQUIRED opencv4)

set(SOURCES
    src/main.cpp
    src/common.cpp
    src/menu.cpp
    src/pickleball_game.cpp
    src/boxing_game.cpp
    src/tennis_game.cpp
    src/archery_game.cpp
    src/rhythm_dance_game.cpp
)

add_executable(motion_control ${SOURCES})

target_include_directories(motion_control PRIVATE 
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${OPENCV_INCLUDE_DIRS}
    src
)

target_link_directories(motion_control PRIVATE
    ${SDL2_LIBRARY_DIRS}
    ${SDL2_MIXER_LIBRARY_DIRS}
    ${OPENCV_LIBRARY_DIRS}
)

target_link_libraries(motion_control 
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${OPENCV_LIBRARIES}
)

target_compile_options(motion_control PRIVATE ${SDL2_CFLAGS_OTHER} ${SDL2_MIXER_CFLAGS_OTHER} ${OPENCV_CFLAGS_OTHER})
