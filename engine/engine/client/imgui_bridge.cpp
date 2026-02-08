#include "imgui_bridge.h"

#ifdef XASH_SDL

#include <SDL.h>

#include "common.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl2.h"

static bool g_imgui_inited = false;
static bool g_imgui_enabled = false;

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
