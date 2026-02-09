#include "imgui_bridge.h"

int ImGuiBridge_IsEnabled( void )
{
	return 0;
}

void ImGuiBridge_Init( struct SDL_Window *window )
{
	(void)window;
}

void ImGuiBridge_Shutdown( void )
{
}

void ImGuiBridge_ProcessEvent( const SDL_Event *event )
{
	(void)event;
}

void ImGuiBridge_NewFrame( void )
{
}

void ImGuiBridge_Render( void )
{
}
