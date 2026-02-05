cmake_minimum_required(VERSION 3.15)
project(workplace)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# Force 32-bit on Windows
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
endif()

# Include directories
include_directories(
    engine/engine
    engine/engine/common
    engine/engine/client
    engine/engine/server
    engine/engine/platform
    engine/hlsdk
    engine/hlsdk/dlls
    engine/hlsdk/dlls/weapons
    engine/hlsdk/dlls/wpn_shared
    engine/hlsdk/pm_shared
    engine/hlsdk/common
    client/cl_dll/include
    client/cl_dll/../dlls
    client/cl_dll/../dlls/weapons
    client/cl_dll/../dlls/wpn_shared
    client/cl_dll/../common
    client/mainui
    client/mainui/miniutl
    client/mainui/font
    client/mainui/controls
    client/mainui/menus
    client/mainui/model
    client/common
    client/pm_shared
)

# Engine source files
file(GLOB_RECURSE ENGINE_SOURCES
    engine/engine/client/*.c
    engine/engine/common/*.c
    engine/engine/server/*.c
    engine/engine/platform/*.c
)

# Loader sources
file(GLOB LOADER_SOURCES engine/loader/*.c)

# Game launch sources
set(GAME_LAUNCH_SOURCES engine/game_launch/xash.c)

# VGUI support sources
file(GLOB VGUI_SOURCES engine/vgui_support/*.cpp)

# MainUI (engine side)
file(GLOB_RECURSE MAINUI_ENGINE_SOURCES engine/mainui/*.cpp)

# Create engine library
add_library(xash_engine SHARED ${ENGINE_SOURCES} ${LOADER_SOURCES} ${VGUI_SOURCES})
target_compile_definitions(xash_engine PRIVATE
    XASH_SDL
    _WIN32
    WIN32
    HAVE_INTTYPES
    HAVE_STDINT
)
if(MSVC)
    target_compile_definitions(xash_engine PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Link libraries for engine
target_link_libraries(xash_engine PRIVATE
    ws2_32
    winmm
    opengl32
    glu32
    shell32
    user32
    gdi32
    ole32
    oleaut32
    uuid
    shlwapi
    imm32
    version
)

set_target_properties(xash_engine PROPERTIES
    OUTPUT_NAME "xash"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Game launcher executable
add_executable(xash_win32 WIN32 ${GAME_LAUNCH_SOURCES})
target_compile_definitions(xash_win32 PRIVATE
    _WIN32
    WIN32
)
target_link_libraries(xash_win32 PRIVATE
    shell32
    user32
)
set_target_properties(xash_win32 PROPERTIES
    OUTPUT_NAME "xash3d"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Client DLL
file(GLOB_RECURSE CLIENT_SOURCES
    client/cl_dll/*.cpp
    client/cl_dll/hud/*.cpp
    client/cl_dll/studio/*.cpp
)

add_library(client_dll SHARED ${CLIENT_SOURCES})
target_compile_definitions(client_dll PRIVATE
    CLIENT_DLL
    CLIENT_WEAPONS
    HL_DLL
    _WIN32
    WIN32
)
if(MSVC)
    target_compile_definitions(client_dll PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(client_dll PRIVATE
    wsock32
    winmm
)

set_target_properties(client_dll PROPERTIES
    OUTPUT_NAME "client"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# MainUI DLL (client/mainui)
file(GLOB_RECURSE MAINUI_SOURCES
    client/mainui/*.cpp
    client/mainui/controls/*.cpp
    client/mainui/font/*.cpp
    client/mainui/menus/*.cpp
)

add_library(mainui_dll SHARED ${MAINUI_SOURCES})
target_compile_definitions(mainui_dll PRIVATE
    _WIN32
    WIN32
    MAINUI_DONT_USE_VGUI
)
if(MSVC)
    target_compile_definitions(mainui_dll PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(mainui_dll PRIVATE
    user32
    gdi32
)

set_target_properties(mainui_dll PROPERTIES
    OUTPUT_NAME "menu"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Add custom target to build all
add_custom_target(all_targets ALL)
add_dependencies(all_targets xash_engine xash_win32 client_dll mainui_dll)
