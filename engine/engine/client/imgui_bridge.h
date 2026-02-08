#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct SDL_Window;
typedef union SDL_Event SDL_Event;

int ImGuiBridge_IsEnabled( void );
void ImGuiBridge_Init( struct SDL_Window *window );
void ImGuiBridge_Shutdown( void );
void ImGuiBridge_ProcessEvent( const SDL_Event *event );
void ImGuiBridge_NewFrame( void );
void ImGuiBridge_Render( void );

#ifdef __cplusplus
}
#endif
