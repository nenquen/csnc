#include "common.h"
#include "client.h"
#include "imgui_console.h"
#include "imgui_mainmenu.h"

extern "C" {
#include "input.h"
}

#include <GL/gl.h>
#include "SDL.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"

// --------------------------------------------------------------------------
// Console State
// --------------------------------------------------------------------------

static struct {
    bool initialized;
    bool consoleActive;
    char inputBuffer[1024];
    char outputBuffer[65536]; // Rolling buffer
    bool scrollToBottom;
    ImVector<char*> history;
    int historyPos;    // -1: new line, 0..History.Size-1 browsing history.
    char completionBuffer[1024];
} g_ImGuiConsole;

// Auto-suggestion structures
#define MAX_SUGGESTIONS 256
#define MAX_SUGGESTION_LEN 64

static struct {
    char items[MAX_SUGGESTIONS][MAX_SUGGESTION_LEN];
    int count;
    char prefix[MAX_SUGGESTION_LEN];
    int selectedIndex;
} g_suggestions;

// --------------------------------------------------------------------------
// Helper Functions
// --------------------------------------------------------------------------

static void RenderColoredText(const char* text)
{
    if (!text || !text[0]) return;

    // Fast path: no color codes
    if (!strchr(text, '^'))
    {
        ImGui::TextUnformatted(text);
        return;
    }

    // Color processing is simplified here - relying on main theme primarily
    // But we still parse '^X' codes for minimal color support
    char buffer[1024];
    const char* ptr = text;
    ImVec4 currentColor = ImGui::GetStyle().Colors[ImGuiCol_Text];
    int bufIdx = 0;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    while (*ptr)
    {
        if (*ptr == '^' && *(ptr+1) >= '0' && *(ptr+1) <= '9')
        {
            if (bufIdx > 0)
            {
                buffer[bufIdx] = 0;
                ImGui::TextColored(currentColor, "%s", buffer);
                ImGui::SameLine();
                bufIdx = 0;
            }
            
            // Map codes to simple distinct colors
            switch (*(ptr+1))
            {
                case '1': currentColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); break; // Red
                case '2': currentColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f); break; // Green
                case '3': currentColor = ImVec4(1.0f, 1.0f, 0.3f, 1.0f); break; // Yellow
                case '4': currentColor = ImVec4(0.3f, 0.3f, 1.0f, 1.0f); break; // Blue
                default:  currentColor = ImGui::GetStyle().Colors[ImGuiCol_Text]; break;
            }
            ptr += 2;
        }
        else
        {
            if (bufIdx < sizeof(buffer)-1)
                buffer[bufIdx++] = *ptr;
            ptr++;
        }
    }

    if (bufIdx > 0)
    {
        buffer[bufIdx] = 0;
        ImGui::TextColored(currentColor, "%s", buffer);
    }
    else
    {
        ImGui::NewLine(); 
    }
    
    ImGui::PopStyleVar();
}

void ImGui_Init(void)
{
    if (g_ImGuiConsole.initialized) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.MouseDrawCursor = false;

    // NOTE: We do NOT setup style here anymore. 
    // We assume ImGui_MainMenu will handle the global style or we use default.
    // If MainMenu is initialized, it applies the CS:S style globally.
    
    ImGui_ImplSDL2_InitForOpenGL(host.hWnd, NULL);
    ImGui_ImplOpenGL2_Init();

    // Explicitly build font atlas to ensure texture is created
    // unsigned char* pixels;
    // int width, height;
    // io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    
    // Upload texture to graphics system
    // ImGui_ImplOpenGL2_CreateFontsTexture();

    char tempBuf[sizeof(g_ImGuiConsole.outputBuffer)];
    strcpy(tempBuf, g_ImGuiConsole.outputBuffer);
    
    memset(&g_ImGuiConsole, 0, sizeof(g_ImGuiConsole));
    g_ImGuiConsole.historyPos = -1;
    g_ImGuiConsole.initialized = true;

    strcpy(g_ImGuiConsole.outputBuffer, tempBuf);

    ImGui_MainMenu_Init();

    MsgDev(D_INFO, "ImGui initialized (OpenGL 2 Backend)\n");
}

void ImGui_VidInit(void)
{
    // Perform a full reset of the ImGui subsystem to handle OpenGL context loss
    // and potentially new window handles.
    
    // Ensure we are in a known state
    if (g_ImGuiConsole.initialized)
    {
        ImGui_Shutdown();
    }
    
    // Re-initialize
    ImGui_Init();
    
    MsgDev(D_INFO, "ImGui Full Reset (Video Restart)\n");
}

void ImGui_Shutdown(void)
{
    if (!g_ImGuiConsole.initialized) return;

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    for (int i = 0; i < g_ImGuiConsole.history.Size; i++)
        free(g_ImGuiConsole.history[i]);
    g_ImGuiConsole.history.clear();

    g_ImGuiConsole.initialized = false;
}

void ImGui_NewFrame(void)
{
    if (!g_ImGuiConsole.initialized) return;

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGui_ProcessEvent(void *event)
{
    if (!g_ImGuiConsole.initialized) return;
    ImGui_ImplSDL2_ProcessEvent((SDL_Event*)event);
}

// --------------------------------------------------------------------------
// Console Logic
// --------------------------------------------------------------------------

static void Suggestion_AddCommand( const char *key, const char *value, void *buffer, void *numpairs )
{
    if (*key == '@') return; 
    if (g_suggestions.count >= MAX_SUGGESTIONS) return;
    
    if (Q_strnicmp(key, g_suggestions.prefix, Q_strlen(g_suggestions.prefix)) == 0)
        Q_strncpy(g_suggestions.items[g_suggestions.count++], key, MAX_SUGGESTION_LEN);
}

static int ImGuiConsole_TextEditCallback(ImGuiInputTextCallbackData *data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
    {
        const int prev_history_pos = g_ImGuiConsole.historyPos;
        if (data->EventKey == ImGuiKey_UpArrow)
        {
            if (g_ImGuiConsole.historyPos == -1)
                g_ImGuiConsole.historyPos = g_ImGuiConsole.history.Size - 1;
            else if (g_ImGuiConsole.historyPos > 0)
                g_ImGuiConsole.historyPos--;
        }
        else if (data->EventKey == ImGuiKey_DownArrow)
        {
            if (g_ImGuiConsole.historyPos != -1)
                if (++g_ImGuiConsole.historyPos >= g_ImGuiConsole.history.Size)
                    g_ImGuiConsole.historyPos = -1;
        }

        if (prev_history_pos != g_ImGuiConsole.historyPos)
        {
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, (g_ImGuiConsole.historyPos >= 0) ? g_ImGuiConsole.history[g_ImGuiConsole.historyPos] : "");
        }
    }
    else if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
    {
        const char* word_end = data->Buf + data->CursorPos;
        const char* word_start = word_end;
        while (word_start > data->Buf)
        {
            const char c = word_start[-1];
            if (c == ' ' || c == '\t' || c == ',' || c == ';') break;
            word_start--;
        }

        g_suggestions.count = 0;
        int len = (int)(word_end - word_start);
        if (len > 0 && len < MAX_SUGGESTION_LEN)
        {
            Q_strncpy(g_suggestions.prefix, word_start, len);
            g_suggestions.prefix[len] = 0;
            Cmd_LookupCmds(NULL, NULL, Suggestion_AddCommand);
            Cvar_LookupVars(0, NULL, NULL, Suggestion_AddCommand);
        }

        if (g_suggestions.count > 0)
        {
            data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
            data->InsertChars(data->CursorPos, g_suggestions.items[0]);
            data->InsertChars(data->CursorPos, " ");
        }
    }
    return 0;
}

void ImGuiConsole_AddToOutput(const char *text)
{
    size_t len = strlen(text);
    size_t outputLen = strlen(g_ImGuiConsole.outputBuffer);

    if (outputLen + len + 1 >= sizeof(g_ImGuiConsole.outputBuffer))
    {
        size_t flushSize = sizeof(g_ImGuiConsole.outputBuffer) / 4;
        memmove(g_ImGuiConsole.outputBuffer, 
                g_ImGuiConsole.outputBuffer + flushSize,
                sizeof(g_ImGuiConsole.outputBuffer) - flushSize);
        g_ImGuiConsole.outputBuffer[sizeof(g_ImGuiConsole.outputBuffer) - flushSize] = 0;
    }

    strcat(g_ImGuiConsole.outputBuffer, text);
    g_ImGuiConsole.scrollToBottom = true;
}

static void RenderConsoleOutput()
{
    const char* ptr = g_ImGuiConsole.outputBuffer;
    char line[1024];
    
    while (*ptr)
    {
        int lineLen = 0;
        while (ptr[lineLen] && ptr[lineLen] != '\n' && lineLen < sizeof(line)-1)
            lineLen++;
        
        strncpy(line, ptr, lineLen);
        line[lineLen] = 0;

        RenderColoredText(line);

        ptr += lineLen;
        if (*ptr == '\n') ptr++;
    }
}

// --------------------------------------------------------------------------
// Draw Console - MATCHING MAIN MENU STYLE
// --------------------------------------------------------------------------
void ImGuiConsole_Draw(void)
{
    // Ensure Main Menu is drawn if active
    if (ImGui_MainMenu_IsActive())
    {
        ImGui_MainMenu_Draw();
    }

    if (clgame.dllFuncs.pfnImGuiDraw)
    {
        clgame.dllFuncs.pfnImGuiDraw();
    }
    
    if (!g_ImGuiConsole.consoleActive) return;

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    // Use dimensions similar to main menu dialogs but wider
    // Dialogs are 360 wide, Main Menu is 320 wide.
    // Console should be substantial but clean.
    float width = 600;
    float height = 400;
    
    ImVec2 windowPos((displaySize.x - width) * 0.5f, (displaySize.y - height) * 0.5f);

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

    // EXACTLY the same flags as main menu: 
    // No Title Bar, No Resize, No Collapse, No Move
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar 
                           | ImGuiWindowFlags_NoResize 
                           | ImGuiWindowFlags_NoCollapse
                           | ImGuiWindowFlags_NoMove; 

    // Use a group/window similar to others
    if (ImGui::Begin("Console", nullptr, flags))
    {
        // Title - MATCHING CS:SOURCE MENU STYLE
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
        ImGui::Text("CONSOLE");
        ImGui::Separator();
        ImGui::Spacing();

        // 1. Output Area
        // Calculate height for invalid area to leave room for input at bottom
        float footer_height = ImGui::GetFrameHeightWithSpacing() + 40; // Extra padding
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height), false, ImGuiWindowFlags_HorizontalScrollbar);
        
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
        RenderConsoleOutput();
        ImGui::PopStyleVar();

        if (g_ImGuiConsole.scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        g_ImGuiConsole.scrollToBottom = false;

        ImGui::EndChild();
        
        ImGui::Separator();
        ImGui::Spacing();

        // 2. Input Area
        ImGui::PushItemWidth(-1);
        
        // Auto-focus logic
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere();

        ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
        
        // Input text handling
        if (ImGui::InputText("##Input", g_ImGuiConsole.inputBuffer, IM_ARRAYSIZE(g_ImGuiConsole.inputBuffer), input_flags, &ImGuiConsole_TextEditCallback))
        {
            char* s = g_ImGuiConsole.inputBuffer;
            while(*s == ' ') s++; // trim leading
            
            if (s[0])
            {
                char temp[1200];
                snprintf(temp, sizeof(temp), "] %s\n", s);
                ImGuiConsole_AddToOutput(temp);
                
                g_ImGuiConsole.history.push_back(strdup(s));
                g_ImGuiConsole.historyPos = -1;

                Cbuf_AddText(s);
                Cbuf_AddText("\n");
                
                g_ImGuiConsole.inputBuffer[0] = 0;
            }
            ImGui::SetKeyboardFocusHere(-1); 
        }
        ImGui::PopItemWidth();
        
        // Close Button (Small X specific logic is tricky without custom widgets or title bar)
        // Since we removed title bar, we rely on Toggle key (~) to close.
        // Or we could add a "Close" button at bottom if desired, but CS:S usually just uses toggle.
    }
    ImGui::End();
}

void ImGui_Render(void)
{
    if (!g_ImGuiConsole.initialized) return;
    ImGuiConsole_Draw();
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

// --------------------------------------------------------------------------
// Public API
// --------------------------------------------------------------------------

void ImGuiConsole_Toggle(void)
{
    if (!g_ImGuiConsole.initialized) ImGui_Init();
    
    g_ImGuiConsole.consoleActive = !g_ImGuiConsole.consoleActive;

    if (g_ImGuiConsole.consoleActive)
    {
        // Keep input clear
        memset(g_ImGuiConsole.inputBuffer, 0, sizeof(g_ImGuiConsole.inputBuffer));
        
        // If menu is active, we can keep it active in bg or close it.
        // User requested: "console closes menu".
        if (ImGui_MainMenu_IsActive())
        {
            ImGui_MainMenu_Toggle();
        }
        
        SDL_ShowCursor(SDL_ENABLE);
        Key_SetKeyDest(key_console);
        MsgDev(D_INFO, "Console Opened\n");
    }
    else
    {
        // Restore state
        if (cls.state == ca_disconnected)
        {
            Key_SetKeyDest(key_menu);
            SDL_ShowCursor(SDL_ENABLE);
        }
        else if (cls.state == ca_active)
        {
            Key_SetKeyDest(key_game);
            IN_ActivateMouse(1);
            SDL_ShowCursor(SDL_DISABLE);
        }
        else
        {
            Key_SetKeyDest(key_game);
        }
        MsgDev(D_INFO, "Console Closed\n");
    }
}

qboolean ImGuiConsole_IsActive(void)
{
    return g_ImGuiConsole.consoleActive;
}

qboolean ImGui_IsInitialized(void)
{
    return g_ImGuiConsole.initialized;
}
