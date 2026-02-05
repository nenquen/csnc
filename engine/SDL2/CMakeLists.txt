cmake_minimum_required(VERSION 3.15)
project(SDL2 C)

set(SDL_SHARED ON CACHE BOOL "Build SDL2 as shared library")
set(SDL_STATIC OFF CACHE BOOL "Build SDL2 as static library")
set(SDL_TEST OFF CACHE BOOL "Build SDL2 tests")

# Collect all SDL2 source files automatically
file(GLOB SDL2_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/atomic/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/directsound/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/disk/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/dummy/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/winmm/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/wasapi/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpuinfo/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dynapi/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/events/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/haptic/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/haptic/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hidapi/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/joystick/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/joystick/hidapi/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/joystick/virtual/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/joystick/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libm/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loadso/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/locale/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/locale/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/misc/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/power/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/power/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/direct3d/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/direct3d11/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/direct3d12/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/opengl/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/opengles/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/opengles2/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/ps2/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/psp/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/software/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/render/vitagxm/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stdlib/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/thread/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/thread/generic/SDL_syscond.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/thread/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timer/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timer/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/video/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/video/dummy/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/video/offscreen/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/video/windows/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/video/yuv2rgb/*.c
)

# Create SDL2 library
add_library(SDL2 SHARED ${SDL2_SOURCES})

target_include_directories(SDL2 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(SDL2 PRIVATE
    DLL_EXPORT
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_WARNINGS
    HAVE_STDINT_H
    SDL_BUILD_MAJOR_VERSION=2
    SDL_BUILD_MINOR_VERSION=32
    SDL_BUILD_MICRO_VERSION=10
    HAVE_LIBC
    HAVE_FLOOR
    HAVE_CEIL
    HAVE_TRUNC
    HAVE_FMOD
    HAVE_POW
    HAVE_SQRT
    HAVE_LOG
    HAVE_LOG10
    HAVE_EXP
    HAVE_FABSF
    HAVE_SINF
    HAVE_COSF
    HAVE_TANF
    HAVE_ATANF
    HAVE_ATAN2F
    HAVE_SIN
    HAVE_COS
    HAVE_TAN
    HAVE_ATAN
    HAVE_ATAN2
    HAVE_FABSF
    HAVE_FABS
    HAVE_SCALBN
)

if(MSVC)
    target_compile_options(SDL2 PRIVATE /wd4996 /wd4244 /wd4305 /EHsc /fp:except-)
endif()

target_link_libraries(SDL2 PRIVATE
    user32
    gdi32
    winmm
    imm32
    ole32
    oleaut32
    version
    uuid
    advapi32
    shell32
    dinput8
    dxguid
    xinput
    SetupAPI
    CfgMgr32
    Hid
    legacy_stdio_definitions
)

set_target_properties(SDL2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    OUTPUT_NAME "SDL2"
)

# SDL2main static library
add_library(SDL2main STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/main/windows/SDL_windows_main.c)
target_include_directories(SDL2main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
