cmake_minimum_required(VERSION 3.15)
project(workplace)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# Force 32-bit on Windows
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS)
endif()

# SDL2 source build - integrated directly
set(SDL_SHARED ON)
set(SDL_STATIC OFF)
set(SDL_TEST OFF)
set(SDL_TESTS OFF)

# SDL2 Windows-specific source files
set(SDL2_SOURCES
    engine/SDL2/src/SDL.c
    engine/SDL2/src/SDL_assert.c
    engine/SDL2/src/SDL_dataqueue.c
    engine/SDL2/src/SDL_error.c
    engine/SDL2/src/SDL_guid.c
    engine/SDL2/src/SDL_hints.c
    engine/SDL2/src/SDL_list.c
    engine/SDL2/src/SDL_log.c
    engine/SDL2/src/SDL_utils.c
    engine/SDL2/src/atomic/SDL_atomic.c
    engine/SDL2/src/atomic/SDL_spinlock.c
    engine/SDL2/src/audio/SDL_audio.c
    engine/SDL2/src/audio/SDL_audiocvt.c
    engine/SDL2/src/audio/SDL_audiodev.c
    engine/SDL2/src/audio/SDL_audiotypecvt.c
    engine/SDL2/src/audio/SDL_mixer.c
    engine/SDL2/src/audio/SDL_wave.c
    engine/SDL2/src/cpuinfo/SDL_cpuinfo.c
    engine/SDL2/src/dynapi/SDL_dynapi.c
    engine/SDL2/src/events/SDL_clipboardevents.c
    engine/SDL2/src/events/SDL_displayevents.c
    engine/SDL2/src/events/SDL_dropevents.c
    engine/SDL2/src/events/SDL_events.c
    engine/SDL2/src/events/SDL_gesture.c
    engine/SDL2/src/events/SDL_keyboard.c
    engine/SDL2/src/events/SDL_keysym_to_scancode.c
    engine/SDL2/src/events/SDL_mouse.c
    engine/SDL2/src/events/SDL_quit.c
    engine/SDL2/src/events/SDL_scancode_tables.c
    engine/SDL2/src/events/SDL_touch.c
    engine/SDL2/src/events/SDL_windowevents.c
    engine/SDL2/src/events/imKStoUCS.c
    engine/SDL2/src/file/SDL_rwops.c
    engine/SDL2/src/haptic/SDL_haptic.c
    engine/SDL2/src/joystick/SDL_gamecontroller.c
    engine/SDL2/src/joystick/SDL_joystick.c
    engine/SDL2/src/joystick/SDL_steam_virtual_gamepad.c
    engine/SDL2/src/joystick/controller_type.c
    engine/SDL2/src/libm/e_atan2.c
    engine/SDL2/src/libm/e_exp.c
    engine/SDL2/src/libm/e_fmod.c
    engine/SDL2/src/libm/e_log.c
    engine/SDL2/src/libm/e_log10.c
    engine/SDL2/src/libm/e_pow.c
    engine/SDL2/src/libm/e_rem_pio2.c
    engine/SDL2/src/libm/e_sqrt.c
    engine/SDL2/src/libm/k_cos.c
    engine/SDL2/src/libm/k_rem_pio2.c
    engine/SDL2/src/libm/k_sin.c
    engine/SDL2/src/libm/k_tan.c
    engine/SDL2/src/libm/s_atan.c
    engine/SDL2/src/libm/s_copysign.c
    engine/SDL2/src/libm/s_cos.c
    engine/SDL2/src/libm/s_fabs.c
    engine/SDL2/src/libm/s_floor.c
    engine/SDL2/src/libm/s_scalbn.c
    engine/SDL2/src/libm/s_sin.c
    engine/SDL2/src/libm/s_tan.c
    engine/SDL2/src/locale/SDL_locale.c
    engine/SDL2/src/misc/SDL_url.c
    engine/SDL2/src/power/SDL_power.c
    engine/SDL2/src/render/SDL_d3dmath.c
    engine/SDL2/src/render/SDL_render.c
    engine/SDL2/src/render/SDL_yuv_sw.c
    engine/SDL2/src/render/direct3d/SDL_render_d3d.c
    engine/SDL2/src/render/direct3d11/SDL_render_d3d11.c
    engine/SDL2/src/render/direct3d12/SDL_render_d3d12.c
    engine/SDL2/src/render/opengl/SDL_render_gl.c
    engine/SDL2/src/render/opengles/SDL_render_gles.c
    engine/SDL2/src/render/opengles2/SDL_render_gles2.c
    engine/SDL2/src/render/ps2/SDL_render_ps2.c
    engine/SDL2/src/render/psp/SDL_render_psp.c
    engine/SDL2/src/render/software/SDL_blendfillrect.c
    engine/SDL2/src/render/software/SDL_blendline.c
    engine/SDL2/src/render/software/SDL_blendpoint.c
    engine/SDL2/src/render/software/SDL_drawline.c
    engine/SDL2/src/render/software/SDL_drawpoint.c
    engine/SDL2/src/render/software/SDL_render_sw.c
    engine/SDL2/src/render/software/SDL_rotate.c
    engine/SDL2/src/render/software/SDL_triangle.c
    engine/SDL2/src/render/vitagxm/SDL_render_vita_gxm.c
    engine/SDL2/src/render/vitagxm/SDL_render_vita_gxm_memory.c
    engine/SDL2/src/render/vitagxm/SDL_render_vita_gxm_tools.c
    engine/SDL2/src/sensor/SDL_sensor.c
    engine/SDL2/src/stdlib/SDL_crc16.c
    engine/SDL2/src/stdlib/SDL_crc32.c
    engine/SDL2/src/stdlib/SDL_getenv.c
    engine/SDL2/src/stdlib/SDL_iconv.c
    engine/SDL2/src/stdlib/SDL_malloc.c
    engine/SDL2/src/stdlib/SDL_qsort.c
    engine/SDL2/src/stdlib/SDL_stdlib.c
    engine/SDL2/src/stdlib/SDL_string.c
    engine/SDL2/src/stdlib/SDL_strtokr.c
    engine/SDL2/src/thread/SDL_thread.c
    engine/SDL2/src/timer/SDL_timer.c
    engine/SDL2/src/video/SDL_RLEaccel.c
    engine/SDL2/src/video/SDL_blit.c
    engine/SDL2/src/video/SDL_blit_0.c
    engine/SDL2/src/video/SDL_blit_1.c
    engine/SDL2/src/video/SDL_blit_A.c
    engine/SDL2/src/video/SDL_blit_N.c
    engine/SDL2/src/video/SDL_blit_auto.c
    engine/SDL2/src/video/SDL_blit_copy.c
    engine/SDL2/src/video/SDL_blit_slow.c
    engine/SDL2/src/video/SDL_bmp.c
    engine/SDL2/src/video/SDL_clipboard.c
    engine/SDL2/src/video/SDL_egl.c
    engine/SDL2/src/video/SDL_fillrect.c
    engine/SDL2/src/video/SDL_pixels.c
    engine/SDL2/src/video/SDL_rect.c
    engine/SDL2/src/video/SDL_shape.c
    engine/SDL2/src/video/SDL_stretch.c
    engine/SDL2/src/video/SDL_surface.c
    engine/SDL2/src/video/SDL_video.c
    engine/SDL2/src/video/SDL_vulkan_utils.c
    engine/SDL2/src/video/SDL_yuv.c
    engine/SDL2/src/video/yuv2rgb/yuv_rgb_lsx.c
    engine/SDL2/src/video/yuv2rgb/yuv_rgb_sse.c
    engine/SDL2/src/video/yuv2rgb/yuv_rgb_std.c
)

# Windows-specific SDL2 sources
set(SDL2_WIN32_SOURCES
    engine/SDL2/src/audio/directsound/SDL_directsound.c
    engine/SDL2/src/audio/winmm/SDL_winmm.c
    engine/SDL2/src/audio/wasapi/SDL_wasapi.c
    engine/SDL2/src/audio/wasapi/SDL_wasapi_win32.c
    engine/SDL2/src/core/windows/SDL_windows.c
    engine/SDL2/src/core/windows/SDL_xinput.c
    engine/SDL2/src/filesystem/windows/SDL_sysfilesystem.c
    engine/SDL2/src/haptic/windows/SDL_dinputhaptic.c
    engine/SDL2/src/haptic/windows/SDL_windowshaptic.c
    engine/SDL2/src/haptic/windows/SDL_xinputhaptic.c
    engine/SDL2/src/hidapi/SDL_hidapi.c
    engine/SDL2/src/joystick/windows/SDL_dinputjoystick.c
    engine/SDL2/src/joystick/windows/SDL_rawinputjoystick.c
    engine/SDL2/src/joystick/windows/SDL_windows_gaming_input.c
    engine/SDL2/src/joystick/windows/SDL_windowsjoystick.c
    engine/SDL2/src/joystick/windows/SDL_xinputjoystick.c
    engine/SDL2/src/locale/windows/SDL_syslocale.c
    engine/SDL2/src/loadso/windows/SDL_sysloadso.c
    engine/SDL2/src/power/windows/SDL_syspower.c
    engine/SDL2/src/render/direct3d11/SDL_shaders_d3d11.c
    engine/SDL2/src/render/direct3d12/SDL_shaders_d3d12.c
    engine/SDL2/src/render/opengl/SDL_shaders_gl.c
    engine/SDL2/src/render/opengles2/SDL_shaders_gles2.c
    engine/SDL2/src/sensor/windows/SDL_windowssensor.c
    engine/SDL2/src/thread/windows/SDL_syscond_cv.c
    engine/SDL2/src/thread/windows/SDL_sysmutex.c
    engine/SDL2/src/thread/windows/SDL_syssem.c
    engine/SDL2/src/thread/windows/SDL_systhread.c
    engine/SDL2/src/thread/windows/SDL_systls.c
    engine/SDL2/src/timer/windows/SDL_systimer.c
    engine/SDL2/src/video/windows/SDL_windowsclipboard.c
    engine/SDL2/src/video/windows/SDL_windowsevents.c
    engine/SDL2/src/video/windows/SDL_windowsframebuffer.c
    engine/SDL2/src/video/windows/SDL_windowskeyboard.c
    engine/SDL2/src/video/windows/SDL_windowsmessagebox.c
    engine/SDL2/src/video/windows/SDL_windowsmodes.c
    engine/SDL2/src/video/windows/SDL_windowsmouse.c
    engine/SDL2/src/video/windows/SDL_windowsopengl.c
    engine/SDL2/src/video/windows/SDL_windowsopengles.c
    engine/SDL2/src/video/windows/SDL_windowsshape.c
    engine/SDL2/src/video/windows/SDL_windowsvideo.c
    engine/SDL2/src/video/windows/SDL_windowsvulkan.c
    engine/SDL2/src/video/windows/SDL_windowswindow.c
    engine/SDL2/src/core/windows/SDL_immdevice.c
    engine/SDL2/src/hidapi/windows/hid.c
)

# SDL2 static/dummy sources
set(SDL2_STATIC_SOURCES
    engine/SDL2/src/audio/dummy/SDL_dummyaudio.c
    engine/SDL2/src/audio/disk/SDL_diskaudio.c
    engine/SDL2/src/joystick/virtual/SDL_virtualjoystick.c
    engine/SDL2/src/video/dummy/SDL_nullevents.c
    engine/SDL2/src/video/dummy/SDL_nullframebuffer.c
    engine/SDL2/src/video/dummy/SDL_nullvideo.c
    engine/SDL2/src/video/offscreen/SDL_offscreenevents.c
    engine/SDL2/src/video/offscreen/SDL_offscreenframebuffer.c
    engine/SDL2/src/video/offscreen/SDL_offscreenopengles.c
    engine/SDL2/src/video/offscreen/SDL_offscreenvideo.c
    engine/SDL2/src/video/offscreen/SDL_offscreenwindow.c
)

add_library(SDL2 SHARED ${SDL2_SOURCES} ${SDL2_WIN32_SOURCES} ${SDL2_STATIC_SOURCES})

target_include_directories(SDL2 PUBLIC
    engine/SDL2/include
    engine/SDL2/src
)

target_compile_definitions(SDL2 PRIVATE
    DLL_EXPORT
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_WARNINGS
    HAVE_STDINT_H
    HAVE_DDRAW_H
    HAVE_DINPUT_H
    HAVE_DSOUND_H
    HAVE_DXGI_H
    HAVE_XINPUT_H
    HAVE_Vulkan_HEADER
)

if(MSVC)
    target_compile_options(SDL2 PRIVATE /wd4996 /wd4244 /wd4305)
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
)

set_target_properties(SDL2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)


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
    engine/SDL2/include
    engine/pm_shared
    client/common
    client/public
    client/engine
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
    target_compile_options(xash_engine PRIVATE
        /wd4244 /wd4305 /wd4018 /wd4100 /wd4127 /wd4057 /wd4201 /wd4706 /wd4054 /wd4310
        /wd4267 /wd5286 /wd5287 /wd4273 /wd4013 /wd4723 /wd4101 /wd4005
    )
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
    # SDL2 is now built directly in main CMakeLists.txt
    # Link against the SDL2 target directly
    target_link_libraries(xash_engine PRIVATE SDL2)
endif()

set_target_properties(xash_engine PROPERTIES
    OUTPUT_NAME "xash_sdl"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set(CLEANOUT_DIR "${CMAKE_BINARY_DIR}/cleanout")

add_custom_command(TARGET xash_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CLEANOUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:xash_engine>" "${CLEANOUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_PDB_FILE:xash_engine>" "${CLEANOUT_DIR}"
)

# Copy SDL2.dll to output directories after build (SDL2 is built in subdirectory)
add_custom_command(TARGET xash_engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL2>" "$<TARGET_FILE_DIR:xash_win32>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL2>" "${CLEANOUT_DIR}"
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
    OUTPUT_NAME "xash"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
add_custom_command(TARGET xash_win32 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CLEANOUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:xash_win32>" "${CLEANOUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_PDB_FILE:xash_win32>" "${CLEANOUT_DIR}"
)

# VGUI support DLL (vgui_support.dll)
option(BUILD_VGUI_SUPPORT_DLL "Build vgui_support.dll (requires external Valve VGUI library vgui.lib)" OFF)

set(VGUI_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/client/3rdparty/vgui/lib/vgui.lib")

if(BUILD_VGUI_SUPPORT_DLL)
    if(EXISTS "${VGUI_LIB_PATH}")
        file(GLOB_RECURSE VGUI_SUPPORT_SOURCES
            engine/vgui_support/*.cpp
        )
        add_library(vgui_support_dll SHARED ${VGUI_SUPPORT_SOURCES})
        target_include_directories(vgui_support_dll PRIVATE
            engine
            engine/common
            engine/engine
            engine/engine/common
            engine/vgui_support
            client/3rdparty/vgui/include
        )
        target_link_libraries(vgui_support_dll PRIVATE "${VGUI_LIB_PATH}")
        if(MSVC)
            target_compile_definitions(vgui_support_dll PRIVATE _CRT_SECURE_NO_WARNINGS)
        endif()
        set_target_properties(vgui_support_dll PROPERTIES
            OUTPUT_NAME "vgui_support"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
        add_custom_command(TARGET vgui_support_dll POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CLEANOUT_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:vgui_support_dll>" "${CLEANOUT_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_PDB_FILE:vgui_support_dll>" "${CLEANOUT_DIR}"
        )
    else()
        message(WARNING "BUILD_VGUI_SUPPORT_DLL is ON but vgui.lib was not found at: ${VGUI_LIB_PATH}. Skipping vgui_support.dll.")
    endif()
endif()

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
    client/pm_shared
    engine/SDL2/include
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
    target_compile_options(client_dll PRIVATE
        /wd4244 /wd4305 /wd4018 /wd4100 /wd4127 /wd4057 /wd4201 /wd4706 /wd4054 /wd4310
        /wd4267 /wd4723 /wd4101 /wd4005
    )
endif()

target_link_libraries(client_dll PRIVATE
    wsock32
    winmm
)

set_target_properties(client_dll PROPERTIES
    OUTPUT_NAME "client"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/cl_dlls"
)

add_custom_command(TARGET client_dll POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CLEANOUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CLEANOUT_DIR}/cl_dlls"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:client_dll>" "${CLEANOUT_DIR}/cl_dlls"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_PDB_FILE:client_dll>" "${CLEANOUT_DIR}/cl_dlls"
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
    target_compile_options(mainui_dll PRIVATE
        /wd4244 /wd4305 /wd4018 /wd4100 /wd4127 /wd4057 /wd4201 /wd4706 /wd4054 /wd4310
        /wd4267 /wd4723 /wd4101 /wd4005
    )
endif()

target_link_libraries(mainui_dll PRIVATE
    user32
    gdi32
)

set_target_properties(mainui_dll PROPERTIES
    OUTPUT_NAME "menu"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(TARGET mainui_dll POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CLEANOUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:mainui_dll>" "${CLEANOUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_PDB_FILE:mainui_dll>" "${CLEANOUT_DIR}"
)

# NOTE: mp.dll (server DLL) will be built from regamedll sources
set(REGAMEDLL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/client/regamedll/regamedll")

# Generate appversion.h file (required by regamedll)
file(WRITE "${REGAMEDLL_DIR}/version/appversion.h" "/* Generated app version header */\n#ifndef __APPVERSION_H__\n#define __APPVERSION_H__\n\n#define APP_VERSION \"5.30.0.0-dev\"\n#define APP_VERSION_C 5,30,0,0\n#define APP_VERSION_STRD \"5.30.0.0\"\n#define APP_VERSION_FLAGS 0x0L\n#define APP_COMMIT_DATE \"Feb 5 2026\"\n#define APP_COMMIT_TIME \"00:00:00\"\n#define APP_COMMIT_SHA \"0000000\"\n#define APP_COMMIT_URL \"\"\n\n#endif //__APPVERSION_H__\n")

set(REGAMEDLL_SRCS
    ${REGAMEDLL_DIR}/dlls/airtank.cpp
    ${REGAMEDLL_DIR}/dlls/ammo.cpp
    ${REGAMEDLL_DIR}/dlls/animating.cpp
    ${REGAMEDLL_DIR}/dlls/animation.cpp
    ${REGAMEDLL_DIR}/dlls/basemonster.cpp
    ${REGAMEDLL_DIR}/dlls/bmodels.cpp
    ${REGAMEDLL_DIR}/dlls/buttons.cpp
    ${REGAMEDLL_DIR}/dlls/career_tasks.cpp
    ${REGAMEDLL_DIR}/dlls/cbase.cpp
    ${REGAMEDLL_DIR}/dlls/client.cpp
    ${REGAMEDLL_DIR}/dlls/cmdhandler.cpp
    ${REGAMEDLL_DIR}/dlls/combat.cpp
    ${REGAMEDLL_DIR}/dlls/debug.cpp
    ${REGAMEDLL_DIR}/dlls/doors.cpp
    ${REGAMEDLL_DIR}/dlls/effects.cpp
    ${REGAMEDLL_DIR}/dlls/explode.cpp
    ${REGAMEDLL_DIR}/dlls/func_break.cpp
    ${REGAMEDLL_DIR}/dlls/func_tank.cpp
    ${REGAMEDLL_DIR}/dlls/game.cpp
    ${REGAMEDLL_DIR}/dlls/gamerules.cpp
    ${REGAMEDLL_DIR}/dlls/ggrenade.cpp
    ${REGAMEDLL_DIR}/dlls/gib.cpp
    ${REGAMEDLL_DIR}/dlls/globals.cpp
    ${REGAMEDLL_DIR}/dlls/h_battery.cpp
    ${REGAMEDLL_DIR}/dlls/h_cycler.cpp
    ${REGAMEDLL_DIR}/dlls/h_export.cpp
    ${REGAMEDLL_DIR}/dlls/healthkit.cpp
    ${REGAMEDLL_DIR}/dlls/hintmessage.cpp
    ${REGAMEDLL_DIR}/dlls/items.cpp
    ${REGAMEDLL_DIR}/dlls/lights.cpp
    ${REGAMEDLL_DIR}/dlls/mapinfo.cpp
    ${REGAMEDLL_DIR}/dlls/maprules.cpp
    ${REGAMEDLL_DIR}/dlls/mortar.cpp
    ${REGAMEDLL_DIR}/dlls/multiplay_gamerules.cpp
    ${REGAMEDLL_DIR}/dlls/observer.cpp
    ${REGAMEDLL_DIR}/dlls/pathcorner.cpp
    ${REGAMEDLL_DIR}/dlls/plats.cpp
    ${REGAMEDLL_DIR}/dlls/player.cpp
    ${REGAMEDLL_DIR}/dlls/revert_saved.cpp
    ${REGAMEDLL_DIR}/dlls/saverestore.cpp
    ${REGAMEDLL_DIR}/dlls/singleplay_gamerules.cpp
    ${REGAMEDLL_DIR}/dlls/skill.cpp
    ${REGAMEDLL_DIR}/dlls/sound.cpp
    ${REGAMEDLL_DIR}/dlls/soundent.cpp
    ${REGAMEDLL_DIR}/dlls/spectator.cpp
    ${REGAMEDLL_DIR}/dlls/subs.cpp
    ${REGAMEDLL_DIR}/dlls/training_gamerules.cpp
    ${REGAMEDLL_DIR}/dlls/triggers.cpp
    ${REGAMEDLL_DIR}/dlls/tutor.cpp
    ${REGAMEDLL_DIR}/dlls/tutor_base_states.cpp
    ${REGAMEDLL_DIR}/dlls/tutor_base_tutor.cpp
    ${REGAMEDLL_DIR}/dlls/tutor_cs_states.cpp
    ${REGAMEDLL_DIR}/dlls/tutor_cs_tutor.cpp
    ${REGAMEDLL_DIR}/dlls/util.cpp
    ${REGAMEDLL_DIR}/dlls/vehicle.cpp
    ${REGAMEDLL_DIR}/dlls/weapons.cpp
    ${REGAMEDLL_DIR}/dlls/weapontype.cpp
    ${REGAMEDLL_DIR}/dlls/world.cpp
    ${REGAMEDLL_DIR}/dlls/API/CAPI_Impl.cpp
    ${REGAMEDLL_DIR}/dlls/API/CSEntity.cpp
    ${REGAMEDLL_DIR}/dlls/API/CSPlayer.cpp
    ${REGAMEDLL_DIR}/dlls/API/CSPlayerItem.cpp
    ${REGAMEDLL_DIR}/dlls/API/CSPlayerWeapon.cpp
    ${REGAMEDLL_DIR}/dlls/addons/item_airbox.cpp
    ${REGAMEDLL_DIR}/dlls/addons/point_command.cpp
    ${REGAMEDLL_DIR}/dlls/addons/trigger_random.cpp
    ${REGAMEDLL_DIR}/dlls/addons/trigger_setorigin.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_ak47.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_aug.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_awp.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_c4.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_deagle.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_elite.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_famas.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_fiveseven.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_flashbang.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_g3sg1.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_galil.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_glock18.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_hegrenade.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_knife.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_m3.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_m4a1.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_m249.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_mac10.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_mp5navy.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_p90.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_p228.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_scout.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_sg550.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_sg552.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_smokegrenade.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_tmp.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_ump45.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_usp.cpp
    ${REGAMEDLL_DIR}/dlls/wpn_shared/wpn_xm1014.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_chatter.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_event.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_init.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_learn.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_listen.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_manager.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_nav.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_pathfind.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_radio.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_statemachine.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_update.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_vision.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_bot_weapon.cpp
    ${REGAMEDLL_DIR}/dlls/bot/cs_gamestate.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_attack.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_buy.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_defuse_bomb.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_escape_from_bomb.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_fetch_bomb.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_follow.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_hide.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_hunt.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_idle.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_investigate_noise.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_move_to.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_plant_bomb.cpp
    ${REGAMEDLL_DIR}/dlls/bot/states/cs_bot_use_entity.cpp
    ${REGAMEDLL_DIR}/dlls/hostage/hostage.cpp
    ${REGAMEDLL_DIR}/dlls/hostage/hostage_improv.cpp
    ${REGAMEDLL_DIR}/dlls/hostage/hostage_localnav.cpp
    ${REGAMEDLL_DIR}/dlls/hostage/states/hostage_animate.cpp
    ${REGAMEDLL_DIR}/dlls/hostage/states/hostage_escape.cpp
    ${REGAMEDLL_DIR}/dlls/hostage/states/hostage_follow.cpp
    ${REGAMEDLL_DIR}/dlls/hostage/states/hostage_idle.cpp
    ${REGAMEDLL_DIR}/dlls/hostage/states/hostage_retreat.cpp
    ${REGAMEDLL_DIR}/game_shared/shared_util.cpp
    ${REGAMEDLL_DIR}/game_shared/voice_gamemgr.cpp
    ${REGAMEDLL_DIR}/game_shared/bot/bot.cpp
    ${REGAMEDLL_DIR}/game_shared/bot/bot_manager.cpp
    ${REGAMEDLL_DIR}/game_shared/bot/bot_profile.cpp
    ${REGAMEDLL_DIR}/game_shared/bot/bot_util.cpp
    ${REGAMEDLL_DIR}/game_shared/bot/nav_area.cpp
    ${REGAMEDLL_DIR}/game_shared/bot/nav_file.cpp
    ${REGAMEDLL_DIR}/game_shared/bot/nav_node.cpp
    ${REGAMEDLL_DIR}/game_shared/bot/nav_path.cpp
    ${REGAMEDLL_DIR}/pm_shared/pm_debug.cpp
    ${REGAMEDLL_DIR}/pm_shared/pm_math.cpp
    ${REGAMEDLL_DIR}/pm_shared/pm_shared.cpp
    ${REGAMEDLL_DIR}/regamedll/regamedll.cpp
    ${REGAMEDLL_DIR}/regamedll/precompiled.cpp
    ${REGAMEDLL_DIR}/regamedll/public_amalgamation.cpp
    ${REGAMEDLL_DIR}/regamedll/hookchains_impl.cpp
    ${REGAMEDLL_DIR}/regamedll/sse_mathfun.cpp
    ${REGAMEDLL_DIR}/engine/unicode_strtools.cpp
    ${REGAMEDLL_DIR}/public/FileSystem.cpp
    ${REGAMEDLL_DIR}/public/interface.cpp
    ${REGAMEDLL_DIR}/public/MemPool.cpp
    ${REGAMEDLL_DIR}/public/tier0/dbg.cpp
    ${REGAMEDLL_DIR}/public/tier0/platform_win32.cpp
    ${REGAMEDLL_DIR}/public/tier0/assert_dialog.cpp
)

add_library(mp_dll SHARED ${REGAMEDLL_SRCS})

target_include_directories(mp_dll PRIVATE
    ${REGAMEDLL_DIR}
    ${REGAMEDLL_DIR}/dlls
    ${REGAMEDLL_DIR}/game_shared
    ${REGAMEDLL_DIR}/pm_shared
    ${REGAMEDLL_DIR}/regamedll
    ${REGAMEDLL_DIR}/public
    ${REGAMEDLL_DIR}/public/regamedll
    ${REGAMEDLL_DIR}/engine
    ${REGAMEDLL_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/client/common
    ${CMAKE_CURRENT_SOURCE_DIR}/client/public
    ${CMAKE_CURRENT_SOURCE_DIR}/client/engine
)

target_compile_definitions(mp_dll PRIVATE
    REGAMEDLL_FIXES
    REGAMEDLL_API
    REGAMEDLL_ADD
    UNICODE_FIXES
    BUILD_LATEST
    CLIENT_WEAPONS
    USE_QSTRING
    _WIN32
    WIN32
    NDEBUG
    _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
    target_compile_options(mp_dll PRIVATE
        /wd4244 /wd4305 /wd4018 /wd4100 /wd4127 /wd4057 /wd4201 /wd4706 /wd4054 /wd4310
        /wd4267 /wd4723 /wd4101 /wd4005
    )
endif()

set_target_properties(mp_dll PROPERTIES
    OUTPUT_NAME "mp"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/dlls"
)

add_custom_command(TARGET mp_dll POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CLEANOUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CLEANOUT_DIR}/dlls"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:mp_dll>" "${CLEANOUT_DIR}/dlls"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:mp_dll>/mp.pdb" "${CLEANOUT_DIR}/dlls" 2>NUL || echo "PDB not found, skipping"
)

# Add custom target to build all
add_custom_target(all_targets ALL)
add_dependencies(all_targets xash_engine xash_win32 client_dll mainui_dll mp_dll)
