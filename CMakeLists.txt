cmake_minimum_required(VERSION 3.15)
project(workplace)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# Force 32-bit on Windows
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
endif()

# Engine source files
if(WIN32)
    file(GLOB_RECURSE ENGINE_SOURCES
        engine/engine/client/*.c
        engine/engine/common/*.c
        engine/engine/server/*.c
    )

    file(GLOB PLATFORM_SDL_SOURCES engine/engine/platform/sdl/*.c)
    file(GLOB PLATFORM_WIN32_SOURCES engine/engine/platform/win32/*.c)
    list(APPEND ENGINE_SOURCES ${PLATFORM_SDL_SOURCES} ${PLATFORM_WIN32_SOURCES})
else()
    file(GLOB_RECURSE ENGINE_SOURCES
        engine/engine/client/*.c
        engine/engine/common/*.c
        engine/engine/server/*.c
        engine/engine/platform/*.c
    )
endif()

# Loader sources (Unix/Linux only - for Wine-like functionality)
if(NOT WIN32)
    file(GLOB LOADER_SOURCES engine/loader/*.c)
endif()

# Game launch sources
set(GAME_LAUNCH_SOURCES engine/game_launch/xash.c)

# VGUI support sources
set(VGUI_SOURCES)

# MainUI (engine side)
file(GLOB_RECURSE MAINUI_ENGINE_SOURCES engine/mainui/*.cpp)

# Create engine library
add_library(xash_engine SHARED ${ENGINE_SOURCES} ${LOADER_SOURCES} ${VGUI_SOURCES})
target_include_directories(xash_engine PRIVATE
    engine/engine/common
    engine/engine
    engine/common
    engine/engine/client
    engine/engine/client/vgui
    engine/engine/server
    engine/engine/platform
    engine/hlsdk
    engine/hlsdk/external/SDL2
    engine/hlsdk/public
    engine/hlsdk/engine
    engine/hlsdk/utils/vgui/include
    engine/hlsdk/dlls
    engine/hlsdk/dlls/weapons
    engine/hlsdk/dlls/wpn_shared
    engine/pm_shared
    engine/hlsdk/pm_shared
    engine/hlsdk/common
)
target_compile_definitions(xash_engine PRIVATE
    _WIN32
    WIN32
    HAVE_INTTYPES
    HAVE_STDINT
    XASH_SDL
    XASH_VIDEO=VIDEO_SDL
    XASH_TIMER=TIMER_WIN32
    XASH_INPUT=INPUT_SDL
    XASH_SOUND=SOUND_SDL
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

if(WIN32)
    # Explicit SDL2 library paths - verify these exist
    set(SDL2_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/engine/hlsdk/external/SDL2/lib/x86/SDL2.lib")
    set(SDL2MAIN_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/engine/hlsdk/external/SDL2/lib/x86/SDL2main.lib")
    
    if(EXISTS "${SDL2_LIB_PATH}")
        message(STATUS "Found SDL2.lib: ${SDL2_LIB_PATH}")
        target_link_libraries(xash_engine PRIVATE "${SDL2_LIB_PATH}")
        if(EXISTS "${SDL2MAIN_LIB_PATH}")
            message(STATUS "Found SDL2main.lib: ${SDL2MAIN_LIB_PATH}")
            target_link_libraries(xash_engine PRIVATE "${SDL2MAIN_LIB_PATH}")
        endif()
    else()
        message(FATAL_ERROR "SDL2.lib NOT FOUND at: ${SDL2_LIB_PATH}\nPlease download SDL2-devel-VC.zip from https://github.com/libsdl-org/SDL/releases/latest and extract SDL2.lib to the path above, then reconfigure.")
    endif()
endif()

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

file(GLOB_RECURSE PM_SHARED_SOURCES
    client/pm_shared/*.cpp
)

if(WIN32)
    list(REMOVE_ITEM CLIENT_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/client/cl_dll/input_sdl.cpp
    )

    list(REMOVE_ITEM CLIENT_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/client/cl_dll/input_xash3d.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/client/cl_dll/ev_hldm.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/client/cl_dll/com_weapons.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/client/cl_dll/cs_wpn/cs_weapons.cpp
    )
endif()

add_library(client_dll SHARED ${CLIENT_SOURCES})
target_sources(client_dll PRIVATE ${PM_SHARED_SOURCES})
target_sources(client_dll PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/client/cl_dll/cs_wpn/cs_globals.cpp
)
target_include_directories(client_dll PRIVATE
    client/cl_dll/include
    client/cl_dll/include/hud
    client/cl_dll/include/studio
    client/common
    client/pm_shared
    client/engine
    client/public
    client/dlls
    client/dlls/weapons
    client/dlls/wpn_shared
    engine/hlsdk
    engine/hlsdk/public
    engine/hlsdk/engine
    engine/hlsdk/dlls
    engine/hlsdk/dlls/weapons
    engine/hlsdk/dlls/wpn_shared
    engine/hlsdk/pm_shared
    engine/hlsdk/common
    engine/hlsdk/external
    engine/hlsdk/external/SDL2
)
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

# Server game DLL (mp.dll)
file(GLOB_RECURSE MP_DLL_SOURCES
    engine/hlsdk/dlls/*.cpp
)

add_library(mp_dll SHARED ${MP_DLL_SOURCES})
target_include_directories(mp_dll PRIVATE
    engine/hlsdk
    engine/hlsdk/public
    engine/hlsdk/engine
    engine/hlsdk/dlls
    engine/hlsdk/dlls/weapons
    engine/hlsdk/dlls/wpn_shared
    engine/hlsdk/pm_shared
    engine/hlsdk/common
)
target_compile_definitions(mp_dll PRIVATE
    HL_DLL
    _WIN32
    WIN32
)
if(MSVC)
    target_compile_definitions(mp_dll PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_link_libraries(mp_dll PRIVATE
    winmm
)
set_target_properties(mp_dll PROPERTIES
    OUTPUT_NAME "mp"
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
target_include_directories(mainui_dll PRIVATE
    client/mainui
    client/mainui/miniutl
    client/mainui/font
    client/mainui/controls
    client/mainui/menus
    client/mainui/model
    client/common
    client/engine
    client/pm_shared
    client/public
)
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
