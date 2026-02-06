/*
 * imgui_mainmenu.cpp - ImGui Main Menu System for Xash3D-NG
 * 
 * Modern ImGui-based menu system with proper background rendering
 * Features:
 * - Counter-Strike / GoldSrc inspired styling
 * - Smooth animated transitions
 * - Professional gradient backgrounds
 * - Optimized rendering pipeline
 */

#include "common.h"
#include "client.h"
#include "imgui.h"
#include "imgui_console.h"
#include "imgui_mainmenu.h"
#include <GL/gl.h>
#include "SDL.h"

// stb_image for loading background images
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// In engine context, check if client is active
#define CL_IsActive() (cls.state == ca_active)

// Background texture
static GLuint g_BackgroundTexture = 0;
static int g_BackgroundWidth = 0;
static int g_BackgroundHeight = 0;
static bool g_BackgroundLoaded = false;

// Menu state
// Menu items
typedef enum
{
    MENU_MAIN,
    MENU_NEWGAME, // Changed from CONFIGURATION to support direct switch
    MENU_CONFIGURATION,
    MENU_MULTIPLAYER,
    MENU_AUDIO,
    MENU_VIDEO,
    MENU_CONTROLS,
    MENU_PLAYER
} menu_type_t;

// Create Server State
static struct {
    char hostname[64];
    int maxPlayers;
    char mapName[64];
    int timeLimit;
    int winLimit;
    int roundLimit;
    int freezeTime;
    int buyTime;
    int startMoney;
    bool footsteps;
    int currentTab; // 0: Server, 1: Game
    int selectedMapIndex;
} g_CreateServerState;

// Maps list (placeholder)
static const char* g_MapList[] = {
    "cs_assault", "cs_estate", "cs_italy", "cs_militia", "cs_office", 
    "de_aztec", "de_cbble", "de_dust", "de_dust2", "de_inferno", 
    "de_nuke", "de_prodigy", "de_train"
};

static struct
{
    bool initialized;
    bool active;
    bool styleInitialized; // Check if style was applied
    menu_type_t currentMenu;
    
    // Animation
    float animAlpha;
    
    // Dialog states
    bool showQuitDialog;
    bool showDisconnectDialog;
    bool showCreateServerDialog; // New dialog
    
} g_MainMenu;

// Forward declarations
static void DrawMainMenu(void);
static void DrawConfigMenu(void);
static void DrawMultiplayerMenu(void);
static void DrawQuitDialog(void);
static void DrawDisconnectDialog(void);
static void DrawCreateServerDialog(void); // Add this line
static void SetupCSSStyle(void);
static void MenuButton(const char* label, bool* pressed, bool disabled = false);
static void LoadBackgroundImage(const char* filename);
static void DrawBackground(void);
static void DrawBackgroundGradient(void);

// Draw pure black background (CS:Source style)
static void DrawBackgroundGradient(void)
{
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    
    // Pure black background like CS:Source
    drawList->AddRectFilled(
        ImVec2(0, 0),
        displaySize,
        IM_COL32(0, 0, 0, 255)
    );
}

// Load background image from file using engine filesystem
static void LoadBackgroundImage(const char* filename)
{
    if (g_BackgroundLoaded)
        return;

    MsgDev(D_INFO, "Loading background: %s\n", filename);

    // Check if file exists first
    if (!FS_FileExists(filename, false))
    {
        MsgDev(D_WARN, "Background file not found: %s\n", filename);
        return;
    }

    MsgDev(D_INFO, "Background file exists: %s\n", filename);

    // Use stbi_load directly - try with full path
    char fullpath[MAX_SYSPATH];
    Q_snprintf(fullpath, sizeof(fullpath), "%s/%s", host.rootdir, filename);
    
    int width, height, channels;
    unsigned char* data = stbi_load(fullpath, &width, &height, &channels, 4);
    
    if (!data)
    {
        // Try without full path
        data = stbi_load(filename, &width, &height, &channels, 4);
    }
    
    if (!data)
    {
        MsgDev(D_WARN, "Failed to load background image with stbi: %s\n", filename);
        return;
    }

    MsgDev(D_INFO, "Background image data loaded: %dx%d\n", width, height);

    // Generate OpenGL texture
    glGenTextures(1, &g_BackgroundTexture);
    if (!g_BackgroundTexture)
    {
        MsgDev(D_ERROR, "Failed to generate OpenGL texture\n");
        stbi_image_free(data);
        return;
    }

    glBindTexture(GL_TEXTURE_2D, g_BackgroundTexture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    stbi_image_free(data);
    
    g_BackgroundWidth = width;
    g_BackgroundHeight = height;
    g_BackgroundLoaded = true;
    
    MsgDev(D_INFO, "Background texture created: ID=%u (%dx%d)\n", g_BackgroundTexture, width, height);
}

// Draw background (CS:Source style - pure black)
static void DrawBackground(void)
{
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    
    // Always draw pure black background
    DrawBackgroundGradient();
    
    // If we have a texture, draw it very dimly in the background
    if (g_BackgroundLoaded && g_BackgroundTexture)
    {
        // Calculate scaling to cover screen while maintaining aspect ratio
        float scaleX = displaySize.x / (float)g_BackgroundWidth;
        float scaleY = displaySize.y / (float)g_BackgroundHeight;
        float scale = (scaleX > scaleY) ? scaleX : scaleY;
        
        float drawWidth = g_BackgroundWidth * scale;
        float drawHeight = g_BackgroundHeight * scale;
        float offsetX = (displaySize.x - drawWidth) * 0.5f;
        float offsetY = (displaySize.y - drawHeight) * 0.5f;
        
        // Draw using ImGui's background draw list for proper layering
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        
        // Add the texture with heavy transparency (very subtle)
        drawList->AddImage(
            (void*)(intptr_t)g_BackgroundTexture,
            ImVec2(offsetX, offsetY),
            ImVec2(offsetX + drawWidth, offsetY + drawHeight),
            ImVec2(0, 0),
            ImVec2(1, 1),
            IM_COL32(255, 255, 255, 30) // Very subtle
        );
    }
}

// Initialize main menu
void ImGui_MainMenu_Init(void)
{
    if (g_MainMenu.initialized)
        return;

    g_MainMenu.active = false;
    g_MainMenu.styleInitialized = false;
    g_MainMenu.currentMenu = MENU_MAIN;
    g_MainMenu.animAlpha = 0.0f;
    g_MainMenu.showQuitDialog = false;
    g_MainMenu.showDisconnectDialog = false;
    g_MainMenu.showCreateServerDialog = false; // Initialize new dialog state
    g_MainMenu.initialized = true;

    // Register menu command
    Cmd_AddCommand("menu_imgui", ImGui_MainMenu_Toggle, "toggle ImGui main menu");

    // Load background image from media folder (if exists)
    LoadBackgroundImage("cstrike/media/wallpaper.png");
    if (!g_BackgroundLoaded)
        LoadBackgroundImage("cstrike/media/wallpaper.jpg");
    if (!g_BackgroundLoaded)
        LoadBackgroundImage("valve/media/background.png");

    // Initialize Create Server defaults
    strcpy(g_CreateServerState.hostname, "Counter-Strike");
    g_CreateServerState.maxPlayers = 32;
    strcpy(g_CreateServerState.mapName, "de_dust2");
    g_CreateServerState.timeLimit = 20;
    g_CreateServerState.winLimit = 0;
    g_CreateServerState.roundLimit = 0;
    g_CreateServerState.freezeTime = 6;
    g_CreateServerState.buyTime = 90; // seconds? standard is usually 1.5 min
    g_CreateServerState.startMoney = 800;
    g_CreateServerState.footsteps = true;
    g_CreateServerState.currentTab = 0;
    g_CreateServerState.selectedMapIndex = 0;

    MsgDev(D_INFO, "ImGui Main Menu initialized\n");
}

// Toggle main menu visibility
void ImGui_MainMenu_Toggle(void)
{
    if (!g_MainMenu.initialized)
        ImGui_MainMenu_Init();

    // Only allow menu toggle if:
    // 1. Already in menu (to close it)
    // 2. In game (cls.state == ca_active)
    // 3. At main menu (cls.state == ca_disconnected && cls.key_dest == key_menu)
    if (!g_MainMenu.active)
    {
        // Trying to open menu - check if allowed
        if (cls.state != ca_active && cls.state != ca_disconnected)
        {
            MsgDev(D_INFO, "[MainMenu] Cannot open menu - not in game or main menu\n");
            return;
        }
    }
    
    // If console is active, close it first
    if (ImGuiConsole_IsActive())
    {
        ImGuiConsole_Toggle();
    }
    
    g_MainMenu.active = !g_MainMenu.active;
    
    if (g_MainMenu.active)
    {
        // Reset animation
        g_MainMenu.animAlpha = 0.0f;
        
        // Open menu - set appropriate initial menu
        g_MainMenu.currentMenu = MENU_MAIN;
        
        // Set key destination to menu for input
        Key_SetKeyDest(key_menu);
        
        // Pause game when menu opens
        if (CL_Active())
        {
            Cvar_SetFloat("paused", 1);
        }
        
        // Show mouse cursor
        SDL_ShowCursor(SDL_ENABLE);
        
        MsgDev(D_INFO, "[MainMenu] Menu opened, key_dest set to menu\n");
    }
    else
    {
        // Close menu
        g_MainMenu.currentMenu = MENU_MAIN;
        g_MainMenu.showQuitDialog = false;
        g_MainMenu.showDisconnectDialog = false;
        g_MainMenu.showCreateServerDialog = false; // Ensure this is closed
        
        // Return to game mode
        Key_SetKeyDest(key_game);
        
        // Unpause game
        if (CL_Active())
        {
            Cvar_SetFloat("paused", 0);
        }
        
        // Hide mouse cursor if in game
        if (CL_IsActive())
        {
            SDL_ShowCursor(SDL_DISABLE);
        }
        
        MsgDev(D_INFO, "[MainMenu] Menu closed, key_dest set to game\n");
    }
}

// Check if main menu is active
qboolean ImGui_MainMenu_IsActive(void)
{
    return g_MainMenu.active;
}

// Draw main menu - called from ImGui_Render
void ImGui_MainMenu_Draw(void)
{
    if (!g_MainMenu.active)
        return;

    if (!g_MainMenu.initialized)
        ImGui_MainMenu_Init();

    // Apply CS:Source menu styling (only once)
    if (!g_MainMenu.styleInitialized)
    {
        SetupCSSStyle();
        g_MainMenu.styleInitialized = true;
    }

    // Draw background first (behind everything)
    DrawBackground();

    // Animate menu alpha for smooth fade-in
    if (g_MainMenu.animAlpha < 1.0f)
    {
        g_MainMenu.animAlpha += ImGui::GetIO().DeltaTime * 4.0f; // Smooth animation
        if (g_MainMenu.animAlpha > 1.0f)
            g_MainMenu.animAlpha = 1.0f;
    }

    // Apply animation alpha to all windows
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_MainMenu.animAlpha);

    // Draw appropriate menu based on current state
    switch (g_MainMenu.currentMenu)
    {
        case MENU_MAIN:
            DrawMainMenu();
            break;
        case MENU_CONFIGURATION:
            DrawConfigMenu();
            break;
        case MENU_MULTIPLAYER:
            DrawMultiplayerMenu();
            break;
        default:
            g_MainMenu.currentMenu = MENU_MAIN;
            DrawMainMenu();
            break;
    }

    // Draw dialogs on top
    if (g_MainMenu.showQuitDialog)
        DrawQuitDialog();
    if (g_MainMenu.showDisconnectDialog)
        DrawDisconnectDialog();
    if (g_MainMenu.showCreateServerDialog) // Add this line
        DrawCreateServerDialog();
    
    ImGui::PopStyleVar(); // Alpha
}

// Apply CS:Source style - authentic minimal design
static void SetupCSSStyle(void)
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Window background - dark with subtle transparency
    colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.92f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.95f);
    
    // Border - subtle
    colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.2f, 0.3f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Title bar
    colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Button colors - CS:Source style (transparent with subtle hover)
    colors[ImGuiCol_Button] = ImVec4(0.15f, 0.18f, 0.2f, 0.5f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.28f, 0.3f, 0.7f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.3f, 0.33f, 0.35f, 0.85f);
    
    // Text - pure white
    colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    
    // Headers
    colors[ImGuiCol_Header] = ImVec4(0.15f, 0.18f, 0.2f, 0.5f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.28f, 0.3f, 0.7f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.3f, 0.33f, 0.35f, 0.85f);
    
    // Separator - subtle line
    colors[ImGuiCol_Separator] = ImVec4(0.3f, 0.3f, 0.3f, 0.3f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.4f, 0.4f, 0.4f, 0.5f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.5f, 0.5f, 0.5f, 0.7f);
    
    // CS:Source style - NO ROUNDING, minimal borders
    style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = ImVec2(15, 15);
    style.FramePadding = ImVec2(12, 8);
    style.ItemSpacing = ImVec2(8, 8);
    style.ItemInnerSpacing = ImVec2(6, 6);
    
    // Font scale
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.0f;
}

// Custom menu button with CS:Source styling
static void MenuButton(const char* label, bool* pressed, bool disabled)
{
    ImVec2 size = ImVec2(280, 40);
    
    if (disabled)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.2f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.2f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.2f, 0.4f));
    }
    
    if (ImGui::Button(label, size))
    {
        if (!disabled && pressed)
            *pressed = true;
    }
    
    if (disabled)
        ImGui::PopStyleColor(4);
}

// Draw main menu
static void DrawMainMenu(void)
{
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float menuWidth = 320;
    float menuHeight = 600;
    
    // Left aligned position (CS:Source style)
    ImVec2 windowPos(60.0f, (displaySize.y - menuHeight) * 0.5f);
    
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight), ImGuiCond_Always);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoCollapse
                           | ImGuiWindowFlags_NoTitleBar;
    
    if (ImGui::Begin("Main Menu", nullptr, flags))
    {
        // Title - CS:Source style
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 10));
        
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
        
        ImGui::Text("MAIN MENU");
        
        ImGui::Separator();
        ImGui::Spacing();
        
        bool pressed = false;
        
        // Game state buttons
        if (CL_IsActive())
        {
            // Resume Game
            pressed = false;
            MenuButton("Resume Game", &pressed);
            if (pressed)
            {
                ImGui_MainMenu_Toggle(); // Close menu
            }
            ImGui::Spacing();
            
            // Disconnect
            pressed = false;
            MenuButton("Disconnect", &pressed);
            if (pressed)
            {
                g_MainMenu.showDisconnectDialog = true;
            }
            ImGui::Spacing();
            ImGui::Spacing();
        }
        
        // New Game
        pressed = false;
        MenuButton("New Game", &pressed, !CL_IsActive() ? false : true);
        if (pressed)
        {
            g_MainMenu.showCreateServerDialog = true; // Open create server dialog
        }
        ImGui::Spacing();
        
        // Configuration
        pressed = false;
        MenuButton("Configuration", &pressed);
        if (pressed)
        {
            g_MainMenu.currentMenu = MENU_CONFIGURATION;
        }
        ImGui::Spacing();
        
        // Multiplayer
        pressed = false;
        MenuButton("Multiplayer", &pressed);
        if (pressed)
        {
            g_MainMenu.currentMenu = MENU_MULTIPLAYER;
        }
        ImGui::Spacing();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Console (only in developer mode)
        if (Cvar_VariableValue("developer") > 0)
        {
            pressed = false;
            MenuButton("Console", &pressed);
            if (pressed)
            {
                ImGui_MainMenu_Toggle();
                ImGuiConsole_Toggle(); // Open console
            }
            ImGui::Spacing();
        }
        
        // Quit
        pressed = false;
        MenuButton("Quit", &pressed);
        if (pressed)
        {
            g_MainMenu.showQuitDialog = true;
        }
        
        ImGui::PopStyleVar();
    }
    ImGui::End();
}

// Draw configuration menu
static void DrawConfigMenu(void)
{
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float menuWidth = 500;
    float menuHeight = 600;
    
    ImVec2 windowPos((displaySize.x - menuWidth) * 0.5f, (displaySize.y - menuHeight) * 0.5f);
    
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight), ImGuiCond_Always);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoCollapse;
    
    if (ImGui::Begin("Configuration", nullptr, flags))
    {
        // Back button
        if (ImGui::Button("< Back"))
        {
            g_MainMenu.currentMenu = MENU_MAIN;
        }
        
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();
        
        ImGui::Indent(20);
        
        bool pressed = false;
        
        // Audio
        pressed = false;
        MenuButton("Audio Settings", &pressed);
        // TODO: Open audio menu
        ImGui::Spacing();
        
        // Video
        pressed = false;
        MenuButton("Video Settings", &pressed);
        // TODO: Open video menu
        ImGui::Spacing();
        
        // Controls
        pressed = false;
        MenuButton("Controls", &pressed);
        // TODO: Open controls menu
        ImGui::Spacing();
        
        // Player Setup
        pressed = false;
        MenuButton("Player Setup", &pressed);
        if (pressed)
        {
            g_MainMenu.currentMenu = MENU_PLAYER;
        }
        ImGui::Spacing();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Game Options
        pressed = false;
        MenuButton("Game Options", &pressed);
        // TODO: Open game options
        ImGui::Spacing();
        
        // Advanced
        pressed = false;
        MenuButton("Advanced Options", &pressed);
        // TODO: Open advanced menu
    }
    ImGui::End();
}

// Draw multiplayer menu
static void DrawMultiplayerMenu(void)
{
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float menuWidth = 500;
    float menuHeight = 500;
    
    ImVec2 windowPos((displaySize.x - menuWidth) * 0.5f, (displaySize.y - menuHeight) * 0.5f);
    
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight), ImGuiCond_Always);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoCollapse;
    
    if (ImGui::Begin("Multiplayer", nullptr, flags))
    {
        // Back button
        if (ImGui::Button("< Back"))
        {
            g_MainMenu.currentMenu = MENU_MAIN;
        }
        
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();
        
        ImGui::Indent(20);
        
        bool pressed = false;
        
        // Internet Games
        pressed = false;
        MenuButton("Internet Games", &pressed);
        if (pressed)
        {
            // TODO: Open server browser
            Cbuf_AddText("menu_internetgames\n");
        }
        ImGui::Spacing();
        
        // LAN Games
        pressed = false;
        MenuButton("LAN Games", &pressed);
        if (pressed)
        {
            // TODO: Open LAN browser
            Cbuf_AddText("menu_lan\n");
        }
        ImGui::Spacing();
        
        // Create Game
        pressed = false;
        MenuButton("Create Game", &pressed);
        if (pressed)
        {
            g_MainMenu.showCreateServerDialog = true;
        }
        ImGui::Spacing();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Spectate
        pressed = false;
        MenuButton("Spectate Game", &pressed);
        // TODO: Open spectate menu
        ImGui::Spacing();
    }
    ImGui::End();
}

// Draw Classic Create Server Dialog - DARK THEME
static void DrawCreateServerDialog(void)
{
    if (!g_MainMenu.showCreateServerDialog) return;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    float width = 500;
    float height = 550; 
    
    ImGui::SetNextWindowPos(ImVec2((displaySize.x - width) * 0.5f, (displaySize.y - height) * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    // Modern Dark Theme - Consistent with Main Menu
    // No specific color pushes needed if we rely on global style, 
    // but we can enforce specific dark look here.
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
    
    // Use darker background specifically for this dialog if needed, or rely on default
    if (ImGui::Begin("Creat Server", &g_MainMenu.showCreateServerDialog, flags))
    {
        // Tabs
        if (ImGui::BeginTabBar("ServerTabs"))
        {
            // TAB 1: SERVER
            if (ImGui::BeginTabItem("Server"))
            {
                ImGui::Spacing();
                ImGui::Spacing();
                
                // Content with padding
                ImGui::Indent(10);
                
                // Map Selection 
                ImGui::Text("Map");
                ImGui::Spacing();
                
                ImGui::SetNextItemWidth(400); // Wider input
                if (ImGui::BeginCombo("##mapselect", g_MapList[g_CreateServerState.selectedMapIndex]))
                {
                    for (int n = 0; n < IM_ARRAYSIZE(g_MapList); n++)
                    {
                        const bool is_selected = (g_CreateServerState.selectedMapIndex == n);
                        if (ImGui::Selectable(g_MapList[n], is_selected))
                            g_CreateServerState.selectedMapIndex = n;

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                // Map selection combo is sufficient
                ImGui::Unindent(10);
                ImGui::EndTabItem();
                
                ImGui::EndTabItem();
            }

            // TAB 2: GAME
            if (ImGui::BeginTabItem("Game"))
            {
                ImGui::Spacing();
                ImGui::Indent(10);
                
                auto ConfigRow = [](const char* label, const char* id, void* var, bool isInt) {
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%s", label);
                    ImGui::SameLine(250); // Fixed alignment
                    ImGui::SetNextItemWidth(150);
                    if (isInt) ImGui::InputInt(id, (int*)var, 0); 
                    else ImGui::InputText(id, (char*)var, 64);
                };

                ConfigRow("Hostname", "##hostname", g_CreateServerState.hostname, false);
                ImGui::Spacing();
                ConfigRow("Max. players", "##maxplayers", &g_CreateServerState.maxPlayers, true);
                ConfigRow("Time per map (min)", "##timelimit", &g_CreateServerState.timeLimit, true);
                ConfigRow("Win limit (rounds)", "##winlimit", &g_CreateServerState.winLimit, true);
                ConfigRow("Round limit (rounds)", "##roundlimit", &g_CreateServerState.roundLimit, true);
                ConfigRow("Time per round (min)", "##roundtime", &g_CreateServerState.timeLimit, true); 
                ConfigRow("Freeze time (sec)", "##freezetime", &g_CreateServerState.freezeTime, true);
                ConfigRow("Buy time (min)", "##buytime", &g_CreateServerState.buyTime, true);
                ConfigRow("Starting money", "##startmoney", &g_CreateServerState.startMoney, true);
                
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Text("Game Options");
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::Checkbox("Enable Footsteps", &g_CreateServerState.footsteps);

                ImGui::Unindent(10);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        // Bottom Area: Start / Cancel
        float buttonWidth = 100;
        float windowWidth = ImGui::GetWindowWidth();
        float windowHeight = ImGui::GetWindowHeight();
        
        ImGui::SetCursorPos(ImVec2(windowWidth - (buttonWidth * 2 + 30), windowHeight - 50));
        
        if (ImGui::Button("START", ImVec2(buttonWidth, 35)))
        {
            // Start Server Logic
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "map %s\n", g_MapList[g_CreateServerState.selectedMapIndex]);
            Cbuf_AddText(cmd);
            
            g_MainMenu.showCreateServerDialog = false;
            ImGui_MainMenu_Toggle(); // Close menu
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("CANCEL", ImVec2(buttonWidth, 35)))
        {
            g_MainMenu.showCreateServerDialog = false;
        }
    }
    ImGui::End();
}

// Draw quit confirmation dialog (CS:Source style)
static void DrawQuitDialog(void)
{
    if (!g_MainMenu.showQuitDialog) return;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float dialogWidth = 360;
    float dialogHeight = 140;
    
    ImVec2 windowPos((displaySize.x - dialogWidth) * 0.5f, (displaySize.y - dialogHeight) * 0.5f);
    
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(dialogWidth, dialogHeight), ImGuiCond_Always);
    
    // Custom colors for this dialog - darker greenish tint like CS:Source
    // Custom colors for this dialog - consistent dark theme
    // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.3f, 0.25f, 0.95f)); // Removed green
    // ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.2f, 0.25f, 0.2f, 1.0f));
    // ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.25f, 0.3f, 0.25f, 1.0f));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoCollapse;
    
    if (ImGui::Begin("Quit game", nullptr, flags))
    {
        ImGui::Spacing();
        
        // Icon and text on same line
        ImGui::Text("X"); // Simple icon placeholder
        ImGui::SameLine();
        ImGui::SetCursorPosX(40);
        ImGui::Text("Do you wish to stop playing now?");
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Buttons centered
        float buttonWidth = 100;
        float totalButtonWidth = buttonWidth * 2 + 10; // 10 for spacing
        float startX = (dialogWidth - totalButtonWidth) * 0.5f;
        
        ImGui::SetCursorPosX(startX);
        if (ImGui::Button("Quit game", ImVec2(buttonWidth, 30)))
        {
            Cbuf_AddText("quit\n");
            g_MainMenu.showQuitDialog = false;
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 30)))
        {
            g_MainMenu.showQuitDialog = false;
        }
    }
    ImGui::End();
    
    // ImGui::PopStyleColor(3);
}

// Draw disconnect confirmation dialog (CS:Source style)
static void DrawDisconnectDialog(void)
{
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float dialogWidth = 360;
    float dialogHeight = 140;
    
    ImVec2 windowPos((displaySize.x - dialogWidth) * 0.5f, (displaySize.y - dialogHeight) * 0.5f);
    
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(dialogWidth, dialogHeight), ImGuiCond_Always);
    
    // Custom colors for this dialog - darker greenish tint like CS:Source
    // Custom colors for this dialog - consistent dark theme
    // ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.25f, 0.3f, 0.25f, 0.95f)); // Removed green
    // ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.2f, 0.25f, 0.2f, 1.0f));
    // ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.25f, 0.3f, 0.25f, 1.0f));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove 
                           | ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoCollapse;
    
    if (ImGui::Begin("Disconnect", nullptr, flags))
    {
        ImGui::Spacing();
        
        // Icon and text on same line
        ImGui::Text("X"); // Simple icon placeholder
        ImGui::SameLine();
        ImGui::SetCursorPosX(40);
        ImGui::Text("Disconnect from current game?");
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Buttons centered
        float buttonWidth = 100;
        float totalButtonWidth = buttonWidth * 2 + 10; // 10 for spacing
        float startX = (dialogWidth - totalButtonWidth) * 0.5f;
        
        ImGui::SetCursorPosX(startX);
        if (ImGui::Button("Disconnect", ImVec2(buttonWidth, 30)))
        {
            Cbuf_AddText("cmd disconnect;endgame disconnect\n");
            g_MainMenu.showDisconnectDialog = false;
            g_MainMenu.currentMenu = MENU_MAIN;
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 30)))
        {
            g_MainMenu.showDisconnectDialog = false;
        }
    }
    ImGui::End();
    
    // ImGui::PopStyleColor(3);
}
