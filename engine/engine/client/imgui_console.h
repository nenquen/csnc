#ifndef IMGUI_CONSOLE_H
#define IMGUI_CONSOLE_H

// C-compatible header - no C++ includes here
// ImGui is implemented in the .cpp file

#ifdef __cplusplus
extern "C" {
#endif

// Initialize/Shutdown ImGui system
void ImGui_Init(void);
void ImGui_Shutdown(void);
// Called when video system restarts (e.g. resolution change)
void ImGui_VidInit(void);

// Call at start of frame
void ImGui_NewFrame(void);

// Call at end of frame (before SDL_GL_SwapWindow)
void ImGui_Render(void);

// Process SDL events for ImGui
void ImGui_ProcessEvent(void *event);

// Console window functions
void ImGuiConsole_Toggle(void);
void ImGuiConsole_Draw(void);
qboolean ImGuiConsole_IsActive(void);
void ImGuiConsole_AddToOutput(const char *text);

// Check if ImGui system is initialized
qboolean ImGui_IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif // IMGUI_CONSOLE_H
