#include "common.h"
#include "client.h"
#include "imgui_console.h"

// Extern C for C linkage since input.h doesn't have it
extern "C" {
#include "input.h"
}

#include <GL/gl.h>
#include "SDL.h"

// ImGui C++ headers (only in .cpp file)
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

// Use engine functions directly - they are declared in common.h

// GoldSrc console colors
#define CONSOLE_BG_COLOR        ImVec4(0.20f, 0.25f, 0.20f, 1.0f)    // Dark green background
#define CONSOLE_TEXT_COLOR      ImVec4(0.9f, 0.9f, 0.9f, 1.0f)       // Light text
#define CONSOLE_INPUT_BG        ImVec4(0.15f, 0.20f, 0.15f, 1.0f)    // Darker input bg
#define CONSOLE_SCROLL_BG       ImVec4(0.18f, 0.23f, 0.18f, 1.0f)    // Scrollbar bg

// Console state
static struct {
    qboolean initialized;
    qboolean consoleActive;
    char inputBuffer[1024];
    char outputBuffer[65536];
    qboolean scrollToBottom;
    ImVector<char*> history;
    int historyPos;    // -1: new line, 0..History.Size-1 browsing history.
    char completionBuffer[1024];
    int completionSize;
} g_ImGuiConsole;

// Auto-suggestion structures - MUST be defined before use
#define MAX_SUGGESTIONS 256
#define MAX_SUGGESTION_LEN 64

static struct {
    char items[MAX_SUGGESTIONS][MAX_SUGGESTION_LEN];
    int count;
    char prefix[MAX_SUGGESTION_LEN];
    int selectedIndex;
} g_suggestions;

// GoldSrc color codes to ImGui colors
static ImVec4 GetGoldSrcColor(char code)
{
    switch (code)
    {
        case '0': return ImVec4(0.0f, 0.0f, 0.0f, 1.0f);        // Black
        case '1': return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);        // Red
        case '2': return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);        // Green
        case '3': return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);        // Yellow
        case '4': return ImVec4(0.0f, 0.0f, 1.0f, 1.0f);        // Blue
        case '5': return ImVec4(0.0f, 1.0f, 1.0f, 1.0f);        // Cyan
        case '6': return ImVec4(1.0f, 0.0f, 1.0f, 1.0f);        // Magenta
        case '7': return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);        // White
        case '8': return ImVec4(1.0f, 0.5f, 0.0f, 1.0f);        // Orange
        case '9': return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);        // Gray
        default:  return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);        // Default white
    }
}

// Render text with GoldSrc color codes (^1, ^2, etc.)
static void RenderColoredText(const char* text)
{
    if (!text || !text[0])
        return;

    char buffer[1024];
    const char* ptr = text;
    ImVec4 currentColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default white
    int bufIdx = 0;

    while (*ptr)
    {
        // Check for color code
        if (*ptr == '^' && *(ptr + 1) >= '0' && *(ptr + 1) <= '9')
        {
            // Flush current buffer
            if (bufIdx > 0)
            {
                buffer[bufIdx] = '\0';
                ImGui::TextColored(currentColor, "%s", buffer);
                ImGui::SameLine(0, 0);
                bufIdx = 0;
            }
            // Set new color
            currentColor = GetGoldSrcColor(*(ptr + 1));
            ptr += 2;
        }
        else
        {
            // Add character to buffer
            if (bufIdx < sizeof(buffer) - 1)
                buffer[bufIdx++] = *ptr;
            ptr++;
        }
    }

    // Flush remaining buffer
    if (bufIdx > 0)
    {
        buffer[bufIdx] = '\0';
        ImGui::TextColored(currentColor, "%s", buffer);
    }
}

// Helper to strip color codes (for selectable text that needs plain text)
static void StripColorCodes(const char* src, char* dst, size_t dstSize)
{
    size_t j = 0;
    for (size_t i = 0; src[i] && j < dstSize - 1; i++)
    {
        if (src[i] == '^' && src[i+1] && src[i+1] >= '0' && src[i+1] <= '9')
        {
            i++; // skip color code
            continue;
        }
        dst[j++] = src[i];
    }
    dst[j] = '\0';
}

// Setup GoldSrc console style
static void SetupGoldSrcStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // GoldSrc dark green theme
    colors[ImGuiCol_WindowBg] = CONSOLE_BG_COLOR;
    colors[ImGuiCol_ChildBg] = CONSOLE_BG_COLOR;
    colors[ImGuiCol_Border] = ImVec4(0.3f, 0.35f, 0.3f, 1.0f);
    colors[ImGuiCol_FrameBg] = CONSOLE_INPUT_BG;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.25f, 0.20f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.30f, 0.25f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.20f, 0.15f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.25f, 0.30f, 0.25f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.30f, 0.25f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.40f, 0.35f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.35f, 0.30f, 1.0f);
    colors[ImGuiCol_ScrollbarBg] = CONSOLE_SCROLL_BG;
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.4f, 0.45f, 0.4f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.5f, 0.55f, 0.5f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.6f, 0.65f, 0.6f, 1.0f);
    colors[ImGuiCol_Text] = CONSOLE_TEXT_COLOR;
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.4f, 0.5f, 0.4f, 0.8f);

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(4, 2);
    style.WindowPadding = ImVec2(4, 4);
}

void ImGui_Init(void)
{
    if (g_ImGuiConsole.initialized)
        return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    // Disable ImGui's mouse cursor rendering - we use SDL's cursor
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.MouseDrawCursor = false;

    // Apply GoldSrc console style
    SetupGoldSrcStyle();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(host.hWnd, NULL);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Save output buffer before memset (logs collected before ImGui init)
    char tempOutput[sizeof(g_ImGuiConsole.outputBuffer)];
    strcpy(tempOutput, g_ImGuiConsole.outputBuffer);
    
    memset(&g_ImGuiConsole, 0, sizeof(g_ImGuiConsole));
    g_ImGuiConsole.historyPos = -1;
    g_ImGuiConsole.initialized = true;
    
    // Restore output buffer with previously collected logs
    strcpy(g_ImGuiConsole.outputBuffer, tempOutput);

    MsgDev(D_INFO, "ImGui initialized\n");
}

void ImGui_Shutdown(void)
{
    if (!g_ImGuiConsole.initialized)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Free history
    for (int i = 0; i < g_ImGuiConsole.history.Size; i++)
        free(g_ImGuiConsole.history[i]);
    g_ImGuiConsole.history.clear();

    g_ImGuiConsole.initialized = false;
    MsgDev(D_INFO, "ImGui shutdown\n");
}

void ImGui_NewFrame(void)
{
    if (!g_ImGuiConsole.initialized)
        return;

    // Always run ImGui frame processing when initialized
    // This maintains proper input state even when console is closed
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGui_Render(void)
{
    if (!g_ImGuiConsole.initialized)
        return;
    
    // Completely skip all ImGui processing when console is not active
    if (!g_ImGuiConsole.consoleActive)
        return;

    // Draw console if active
    ImGuiConsole_Draw();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGui_ProcessEvent(void *event)
{
    if (!g_ImGuiConsole.initialized)
        return;

    // Always pass events to ImGui backend to maintain proper input state
    // ImGui will internally track keyboard/mouse state even if we don't use the events
    ImGui_ImplSDL2_ProcessEvent((SDL_Event*)event);
}

void ImGuiConsole_Toggle(void)
{
    if (!g_ImGuiConsole.initialized)
    {
        ImGui_Init();
    }

    g_ImGuiConsole.consoleActive = !g_ImGuiConsole.consoleActive;

    // When opening console, clear input buffer and grab keyboard
    if (g_ImGuiConsole.consoleActive)
    {
        Msg("^2[^7CSNC^2]^7 Konsol acildi.\n");
        memset(g_ImGuiConsole.inputBuffer, 0, sizeof(g_ImGuiConsole.inputBuffer));
        
        // Show mouse cursor for console interaction
        SDL_ShowCursor(SDL_ENABLE);
        
        // Set key destination to console for input
        if (cls.key_dest != key_menu)
            Key_SetKeyDest(key_console);
    }
    else
    {
        Msg("^2[^7CSNC^2]^7 Konsol kapandi.\n");
        // Return to game if not in menu
        if (cls.key_dest == key_console && cls.key_dest != key_menu)
            Key_SetKeyDest(key_game);
        
        // Re-activate mouse capture for camera control
        IN_ActivateMouse(1);
        
        // Hide mouse cursor and return to game mode
        SDL_ShowCursor(SDL_DISABLE);
    }
}

qboolean ImGuiConsole_IsActive(void)
{
    return g_ImGuiConsole.consoleActive;
}

// Console text handling
void ImGuiConsole_AddToOutput(const char *text)
{
    // Always store text in output buffer, even if ImGui not initialized yet
    // This ensures logs are captured from startup
    size_t len = strlen(text);
    size_t outputLen = strlen(g_ImGuiConsole.outputBuffer);

    if (outputLen + len + 1 >= sizeof(g_ImGuiConsole.outputBuffer))
    {
        // Shift buffer to make room
        memmove(g_ImGuiConsole.outputBuffer, 
                g_ImGuiConsole.outputBuffer + sizeof(g_ImGuiConsole.outputBuffer) / 4,
                sizeof(g_ImGuiConsole.outputBuffer) - sizeof(g_ImGuiConsole.outputBuffer) / 4);
        g_ImGuiConsole.outputBuffer[sizeof(g_ImGuiConsole.outputBuffer) - sizeof(g_ImGuiConsole.outputBuffer) / 4] = '\0';
        outputLen = strlen(g_ImGuiConsole.outputBuffer);
    }

    strcat(g_ImGuiConsole.outputBuffer, text);
    g_ImGuiConsole.scrollToBottom = true;
}

static int ImGuiConsole_TextEditCallback(ImGuiInputTextCallbackData *data)
{
    // Handle history and suggestions
    switch (data->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackHistory:
        {
            // If suggestions are showing, navigate them with Up/Down
            if (g_suggestions.count > 0)
            {
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (g_suggestions.selectedIndex > 0)
                        g_suggestions.selectedIndex--;
                    else
                        g_suggestions.selectedIndex = g_suggestions.count - 1;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (g_suggestions.selectedIndex < g_suggestions.count - 1)
                        g_suggestions.selectedIndex++;
                    else
                        g_suggestions.selectedIndex = 0;
                }
                return 0; // Don't process history when navigating suggestions
            }
            
            // Otherwise navigate history
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
                data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = (int)snprintf(data->Buf, (size_t)data->BufSize, "%s", (g_ImGuiConsole.historyPos >= 0) ? g_ImGuiConsole.history[g_ImGuiConsole.historyPos] : "");
                data->BufDirty = true;
            }
        }
        break;
        
    case ImGuiInputTextFlags_CallbackCompletion:
        // Tab key - autocomplete with selected suggestion
        if (g_suggestions.count > 0)
        {
            if (g_suggestions.selectedIndex < 0)
                g_suggestions.selectedIndex = 0;
                
            if (g_suggestions.selectedIndex < g_suggestions.count)
            {
                // Copy suggestion to input buffer
                Q_strncpy(data->Buf, g_suggestions.items[g_suggestions.selectedIndex], data->BufSize);
                Q_strncat(data->Buf, " ", data->BufSize);
                data->BufTextLen = (int)strlen(data->Buf);
                data->CursorPos = data->BufTextLen;
                data->BufDirty = true;
                
                // Clear suggestions
                g_suggestions.count = 0;
            }
        }
        break;
    }
    return 0;
}

// Callback for collecting matching commands - matches setpair_t signature
static void Suggestion_AddCommand( const char *key, const char *value, void *buffer, void *numpairs )
{
    if (*key == '@') return; // skip system commands
    if (g_suggestions.count >= MAX_SUGGESTIONS) return;
    
    if (Q_strnicmp(key, g_suggestions.prefix, Q_strlen(g_suggestions.prefix)) == 0)
    {
        Q_strncpy(g_suggestions.items[g_suggestions.count++], key, MAX_SUGGESTION_LEN);
    }
}

// Update suggestions based on current input
static void UpdateSuggestions(const char *input)
{
    g_suggestions.count = 0;
    g_suggestions.selectedIndex = -1;
    
    // Extract first word (command being typed)
    const char *cmdStart = input;
    while (*cmdStart == ' ' || *cmdStart == '\t' || *cmdStart == '/' || *cmdStart == '\\')
        cmdStart++;
    
    // Find end of command
    const char *cmdEnd = cmdStart;
    while (*cmdEnd && *cmdEnd != ' ')
        cmdEnd++;
    
    size_t len = cmdEnd - cmdStart;
    if (len == 0 || len >= MAX_SUGGESTION_LEN)
        return;
    
    // Store prefix and find matches
    Q_strncpy(g_suggestions.prefix, cmdStart, len);
    g_suggestions.prefix[len] = '\0';
    
    // Query commands and cvars
    Cmd_LookupCmds(NULL, NULL, Suggestion_AddCommand);
    Cvar_LookupVars(0, NULL, NULL, Suggestion_AddCommand);
    
    // Sort suggestions alphabetically
    if (g_suggestions.count > 1)
    {
        for (int i = 0; i < g_suggestions.count - 1; i++)
        {
            for (int j = i + 1; j < g_suggestions.count; j++)
            {
                if (Q_stricmp(g_suggestions.items[i], g_suggestions.items[j]) > 0)
                {
                    char temp[MAX_SUGGESTION_LEN];
                    Q_strcpy(temp, g_suggestions.items[i]);
                    Q_strcpy(g_suggestions.items[i], g_suggestions.items[j]);
                    Q_strcpy(g_suggestions.items[j], temp);
                }
            }
        }
    }
}

// Draw suggestions inline below input
static void DrawSuggestionsPopup()
{
    if (g_suggestions.count == 0)
        return;
    
    // Horizontal layout - suggestions side by side
    ImGui::BeginGroup();
    
    for (int i = 0; i < g_suggestions.count; i++)
    {
        if (i > 0)
            ImGui::SameLine();
            
        bool isSelected = (i == g_suggestions.selectedIndex);
        
        if (isSelected)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        
        char btnId[64];
        snprintf(btnId, sizeof(btnId), "%s##sug%d", g_suggestions.items[i], i);
        
        if (ImGui::Button(btnId))
        {
            Q_strncpy(g_ImGuiConsole.inputBuffer, g_suggestions.items[i], sizeof(g_ImGuiConsole.inputBuffer));
            Q_strncat(g_ImGuiConsole.inputBuffer, " ", sizeof(g_ImGuiConsole.inputBuffer));
            g_suggestions.count = 0;
        }
        
        if (isSelected)
            ImGui::PopStyleColor();
    }
    
    ImGui::EndGroup();
}

static void RenderConsoleOutput()
{
    if (!g_ImGuiConsole.outputBuffer[0])
    {
        ImGui::TextUnformatted("Console initialized.");
        return;
    }

    char line[1024];
    const char* ptr = g_ImGuiConsole.outputBuffer;
    int lineNum = 0;

    while (*ptr)
    {
        // Find end of line
        int lineLen = 0;
        while (ptr[lineLen] && ptr[lineLen] != '\n' && lineLen < sizeof(line) - 1)
            lineLen++;

        // Copy line
        strncpy(line, ptr, lineLen);
        line[lineLen] = '\0';

        // Simple colored text rendering without interactive elements
        RenderColoredText(line);

        // Move to next line
        ptr += lineLen;
        if (*ptr == '\n')
            ptr++;
        lineNum++;
    }
}

void ImGuiConsole_Draw(void)
{
    ImGuiIO& io = ImGui::GetIO();

    // Console: centered window, not fullscreen overlay
    ImVec2 displaySize = io.DisplaySize;
    float consoleWidth = displaySize.x * 0.7f;   // 70% of screen width
    float consoleHeight = displaySize.y * 0.5f;  // 50% of screen height
    ImVec2 consolePos((displaySize.x - consoleWidth) * 0.5f, 50.0f); // Centered, 50px from top

    // First use defaults (user can move/resize after)
    ImGui::SetNextWindowSize(ImVec2(consoleWidth, consoleHeight), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(consolePos, ImGuiCond_FirstUseEver);

    bool* p_open = (bool*)&g_ImGuiConsole.consoleActive;
    bool was_open = g_ImGuiConsole.consoleActive;

    // Normal window with title bar, can be moved/resized
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

    if (!ImGui::Begin("Console", p_open, window_flags))
    {
        ImGui::End();
        return;
    }

    // Check if X button was clicked to close console
    if (was_open && !*p_open)
    {
        // X button clicked - close console properly
        Msg("^2[^7CSNC^2]^7 Konsol kapandi.\n");
        if (cls.key_dest == key_console && cls.key_dest != key_menu)
            Key_SetKeyDest(key_game);
        
        // Re-activate mouse capture for camera control
        IN_ActivateMouse(1);
        
        // Hide mouse cursor and return to game
        SDL_ShowCursor(SDL_DISABLE);
        
        ImGui::End();
        return;
    }

    // Output area - takes most of the space
    float inputHeight = ImGui::GetFrameHeightWithSpacing() + 8;
    ImVec2 contentAvail = ImGui::GetContentRegionAvail();
    float outputHeight = contentAvail.y - inputHeight;

    ImGui::BeginChild("ScrollingRegion", ImVec2(0, outputHeight), true,
                      ImGuiWindowFlags_HorizontalScrollbar);

    // Render console output with selectable text (allows Ctrl+C copy)
    RenderConsoleOutput();

    // Auto-scroll to bottom
    if (g_ImGuiConsole.scrollToBottom && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10)
    {
        ImGui::SetScrollHereY(1.0f);
        g_ImGuiConsole.scrollToBottom = false;
    }

    ImGui::EndChild();

    ImGui::Separator();

    // Input area with GoldSrc style ">" prompt and Submit button
    ImGui::PushStyleColor(ImGuiCol_FrameBg, CONSOLE_INPUT_BG);

    // GoldSrc style prompt
    ImGui::Text(">");
    ImGui::SameLine();

    // Input field
    float submitWidth = 70.0f;
    float inputWidth = ImGui::GetContentRegionAvail().x - submitWidth - 10;
    ImGui::SetNextItemWidth(inputWidth);

    ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue |
                                       ImGuiInputTextFlags_CallbackHistory;

    bool reclaim_focus = false;
    bool input_changed = ImGui::InputText("##Input", g_ImGuiConsole.inputBuffer,
                         IM_ARRAYSIZE(g_ImGuiConsole.inputBuffer),
                         input_flags, &ImGuiConsole_TextEditCallback, (void*)&g_ImGuiConsole);
    
    if (input_changed)
    {
        reclaim_focus = true;
    }

    ImGui::SameLine();

    // Submit button like GoldSrc
    if (ImGui::Button("Submit", ImVec2(submitWidth, 0)) || reclaim_focus)
    {
        char *s = g_ImGuiConsole.inputBuffer;
        while (*s == ' ' || *s == '\t') s++;

        if (s[0])
        {
            // Add to history
            g_ImGuiConsole.history.push_back(strdup(s));
            g_ImGuiConsole.historyPos = -1;

            // Show command in console
            char outputLine[1100];
            snprintf(outputLine, sizeof(outputLine), ">%s\n", s);
            ImGuiConsole_AddToOutput(outputLine);

            // Execute command
            Cbuf_AddText(s);
            Cbuf_AddText("\n");
        }

        strcpy(g_ImGuiConsole.inputBuffer, "");
        reclaim_focus = true;
    }

    ImGui::PopStyleColor();

    // Auto-focus input field
    ImGui::SetItemDefaultFocus();
    if (reclaim_focus)
        ImGui::SetKeyboardFocusHere(-1);

    ImGui::End();
}
