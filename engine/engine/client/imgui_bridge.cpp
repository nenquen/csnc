#include "imgui_bridge.h"

#ifdef XASH_SDL

#include <SDL.h>
#include <stdio.h>

#include "common.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl2.h"

static bool g_imgui_inited = false;
static bool g_imgui_enabled = false;

static bool ImGuiBridge_FileExists( const char *path )
{
	if( path == NULL || path[0] == '\0' )
		return false;
	FILE *f = fopen( path, "rb" );
	if( f == NULL )
		return false;
	fclose( f );
	return true;
}

static void ImGuiBridge_AddTextOutlined( ImDrawList *draw_list, const ImVec2 &pos, ImU32 col_text, ImU32 col_outline, int thickness, const char *text )
{
	if( draw_list == NULL || text == NULL || text[0] == '\0' )
		return;
	if( thickness < 1 )
		thickness = 1;

	for( int dy = -thickness; dy <= thickness; ++dy )
	{
		for( int dx = -thickness; dx <= thickness; ++dx )
		{
			if( dx == 0 && dy == 0 )
				continue;
			draw_list->AddText( ImVec2( pos.x + (float)dx, pos.y + (float)dy ), col_outline, text );
		}
	}

	draw_list->AddText( pos, col_text, text );
}

extern "C" int ImGuiBridge_IsEnabled( void )
{
	return g_imgui_enabled ? 1 : 0;
}

extern "C" void ImGuiBridge_Init( struct SDL_Window *window )
{
	if( g_imgui_inited )
		return;

	g_imgui_enabled = Sys_CheckParm( "-imgui" ) ? true : false;
	if( !g_imgui_enabled )
		return;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	{
		ImGuiIO &io = ImGui::GetIO();
		io.Fonts->Clear();
		ImFontConfig cfg;
		cfg.SizePixels = 13.0f;
		cfg.OversampleH = 1;
		cfg.OversampleV = 1;
		cfg.PixelSnapH = true;
		if( ImGuiBridge_FileExists( "resource/font/font.ttf" ))
			io.FontDefault = io.Fonts->AddFontFromFileTTF( "resource/font/font.ttf", cfg.SizePixels, &cfg );
		else
			io.FontDefault = io.Fonts->AddFontDefaultBitmap( &cfg );
	}

	ImGui_ImplSDL2_InitForOpenGL( window, SDL_GL_GetCurrentContext() );
	ImGui_ImplOpenGL2_Init();

	g_imgui_inited = true;
}

extern "C" void ImGuiBridge_Shutdown( void )
{
	if( !g_imgui_inited )
		return;

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	g_imgui_inited = false;
}

extern "C" void ImGuiBridge_ProcessEvent( const SDL_Event *event )
{
	if( !g_imgui_inited )
		return;

	ImGui_ImplSDL2_ProcessEvent( event );
}

extern "C" void ImGuiBridge_NewFrame( void )
{
	if( !g_imgui_inited )
		return;

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

extern "C" void ImGuiBridge_Render( void )
{
	if( !g_imgui_inited )
		return;

	{
		ImDrawList *dl = ImGui::GetForegroundDrawList();
		const ImVec2 base = ImGui::GetMainViewport()->Pos;
		ImGuiBridge_AddTextOutlined( dl, ImVec2( base.x + 10.0f, base.y + 10.0f ), IM_COL32( 240, 180, 24, 255 ), IM_COL32( 0, 0, 0, 255 ), 2,
			"OpenGL Renderer: (example)" );
	}

	ImGui::Begin( "ImGui" );
	ImGui::Text( "imgui enabled" );
	ImGui::Text( "%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate );
	ImGui::Text( "%.1f FPS", ImGui::GetIO().Framerate );
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );
}

#else

extern "C" int ImGuiBridge_IsEnabled( void ) { return 0; }
extern "C" void ImGuiBridge_Init( struct SDL_Window *window ) { (void)window; }
extern "C" void ImGuiBridge_Shutdown( void ) {}
extern "C" void ImGuiBridge_ProcessEvent( const SDL_Event *event ) { (void)event; }
extern "C" void ImGuiBridge_NewFrame( void ) {}
extern "C" void ImGuiBridge_Render( void ) {}

#endif
