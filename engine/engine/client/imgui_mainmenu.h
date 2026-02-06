/*
 * imgui_mainmenu.h - ImGui Main Menu System Header
 */

#ifndef IMGUI_MAINMENU_H
#define IMGUI_MAINMENU_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialize main menu system
void ImGui_MainMenu_Init(void);

// Toggle main menu visibility
void ImGui_MainMenu_Toggle(void);

// Check if main menu is active
qboolean ImGui_MainMenu_IsActive(void);

// Draw main menu (called from ImGui_Render)
void ImGui_MainMenu_Draw(void);

#ifdef __cplusplus
}
#endif

#endif // IMGUI_MAINMENU_H
