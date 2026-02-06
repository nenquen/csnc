/*
 * imgui_client.h - Client DLL ImGui Integration Header
 * 
 * This header provides ImGui access for Counter-Strike client.dll
 * The engine initializes ImGui, client just uses it
 * 
 * Usage:
 * 1. Include this header in your client .cpp files
 * 2. Implement HUD_ImGuiDraw() in cdll_int.cpp
 * 3. Use ImGui functions to draw your UI
 * 
 * Example in cdll_int.cpp:
 * 
 * void DLLEXPORT HUD_ImGuiDraw(void)
 * {
 *     ImGui::Begin("My CS Menu");
 *     ImGui::Text("Health: %d", gHUD.m_Health.m_iHealth);
 *     if (ImGui::Button("Buy Menu"))
 *     {
 *         // Show buy menu
 *     }
 *     ImGui::End();
 * }
 */

#ifndef IMGUI_CLIENT_H
#define IMGUI_CLIENT_H

#ifdef __cplusplus

// ImGui is initialized by the engine
// We just include the headers and use it
#include "imgui.h"

// Helper macro to check if console is active
// Returns true if console is open (ImGui is being rendered)
extern "C" int ImGuiConsole_IsActive(void);

#endif // __cplusplus

#endif // IMGUI_CLIENT_H
