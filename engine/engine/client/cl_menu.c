/*
cl_menu.c - menu dlls interaction
Copyright (C) 2010 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef XASH_DEDICATED

#include "common.h"
#include "client.h"
#include "const.h"
#include "gl_local.h"
#include "library.h"
#include "input.h"
#include "server.h" // !!svgame.hInstance
#include "gl_vidnt.h"
#include "imgui_mainmenu.h"
#include "imgui_console.h"
#include "SDL.h"

static MENUAPI	GetMenuAPI;
static ADDTOUCHBUTTONTOLIST pfnAddTouchButtonToList;
static void UI_UpdateUserinfo( void );

menu_static_t	menu;

void UI_UpdateMenu( float realtime )
{
	// MainUI no longer used - ImGui menu handles its own rendering
	// Just update userinfo if needed
	UI_UpdateUserinfo();
}

void UI_KeyEvent( int key, qboolean down )
{
	// MainUI no longer used - ImGui handles its own input
	// Input is processed through ImGui_ProcessEvent in the event loop
}

void UI_MouseMove( int x, int y )
{
	// MainUI no longer used - ImGui handles its own mouse input
}

void UI_SetActiveMenu( qboolean fActive )
{
	movie_state_t	*cin_state;

	if( host_xashds_hacks->integer )
	{
		if( !cl.refdef.paused && !cls.changelevel && fActive )
		{
			Cbuf_InsertText("pause\n");
			Cbuf_Execute();
		}
	}

	// Use ImGui menu instead of MainUI
	if( fActive )
	{
		// Toggle ImGui main menu on
		// If ImGui not ready, delay via command buffer
		if( ImGui_IsInitialized() )
		{
			ImGui_MainMenu_Toggle();
		}
		else
		{
			// Delay menu opening until ImGui is ready
			Cbuf_AddText( "menu_imgui\n" );
		}
	}
	else
	{
		// Ensure menu is closed
		if( ImGui_MainMenu_IsActive() )
			ImGui_MainMenu_Toggle();
	}

	if( !fActive )
	{
		// close logo when menu is shutdown
		cin_state = AVI_GetState( CIN_LOGO );
		AVI_CloseVideo( cin_state );
	}
}

void UI_AddServerToList( netadr_t adr, const char *info )
{
	// MainUI no longer used - ImGui menu handles this differently
	// Server list is managed through ImGui multiplayer menu
}

void UI_GetCursorPos( int *pos_x, int *pos_y )
{
	// MainUI no longer used - SDL handles cursor position
	*pos_x = *pos_y = 0;
}

void UI_SetCursorPos( int pos_x, int pos_y )
{
	// MainUI no longer used - SDL handles cursor position
}

void UI_ShowCursor( qboolean show )
{
	// MainUI no longer used - ImGui/SDL handles cursor
	SDL_ShowCursor( show ? SDL_ENABLE : SDL_DISABLE );
}

qboolean UI_CreditsActive( void )
{
	// MainUI no longer used - ImGui handles credits differently
	return false;
}

void UI_CharEvent( int key )
{
	// MainUI no longer used - ImGui handles text input
}

qboolean UI_MouseInRect( void )
{
	// MainUI no longer used - ImGui handles mouse detection
	return true;
}

qboolean UI_IsVisible( void )
{
	// Check if ImGui main menu is active
	return ImGui_MainMenu_IsActive();
}

static void UI_DrawLogo( const char *filename, float x, float y, float width, float height )
{
	static float	cin_time;
	static int	last_frame = -1;
	byte		*cin_data = NULL;
	movie_state_t	*cin_state;
	int		cin_frame;
	qboolean		redraw = false;

	if( !menu.drawLogo ) return;
	cin_state = AVI_GetState( CIN_LOGO );

	if( !AVI_IsActive( cin_state ))
	{
		string		path;
		const char	*fullpath;
	
		// run cinematic if not
		Q_snprintf( path, sizeof( path ), "media/%s", filename );
		FS_DefaultExtension( path, ".avi" );
		fullpath = FS_GetDiskPath( path, false );

		if( FS_FileExists( path, false ) && !fullpath )
		{
			MsgDev( D_ERROR, "Couldn't load %s from packfile. Please extract it\n", path );
			menu.drawLogo = false;
			return;
		}

		AVI_OpenVideo( cin_state, fullpath, false, false, true );
		if( !( AVI_GetVideoInfo( cin_state, (long*)&menu.logo_xres, (long*)&menu.logo_yres, &menu.logo_length )))
		{
			AVI_CloseVideo( cin_state );
			menu.drawLogo = false;
			return;
		}

		cin_time = 0.0f;
		last_frame = -1;
	}

	if( width <= 0 || height <= 0 )
	{
		// precache call, don't draw
		cin_time = 0.0f;
		last_frame = -1;
		return;
	}

	// advances cinematic time (ignores maxfps and host_framerate settings)
	cin_time += host.realframetime;

	// restarts the cinematic
	if( cin_time > menu.logo_length )
		cin_time = 0.0f;

	// read the next frame
	cin_frame = AVI_GetVideoFrameNumber( cin_state, cin_time );

	if( cin_frame != last_frame )
	{
		cin_data = AVI_GetVideoFrame( cin_state, cin_frame );
		last_frame = cin_frame;
		redraw = true;
	}

	R_DrawStretchRaw( x, y, width, height, menu.logo_xres, menu.logo_yres, cin_data, redraw );
}

static int UI_GetLogoWidth( void )
{
	return menu.logo_xres;
}

static int UI_GetLogoHeight( void )
{
	return menu.logo_yres;
}

static float UI_GetLogoLength( void )
{
	return menu.logo_length;
}

static void UI_UpdateUserinfo( void )
{
	player_info_t	*player;

	if( !userinfo->modified ) return;
	player = &menu.playerinfo;

	Q_strncpy( player->userinfo, Cvar_Userinfo(), sizeof( player->userinfo ));
	Q_strncpy( player->name, Info_ValueForKey( player->userinfo, "name" ), sizeof( player->name ));
	Q_strncpy( player->model, Info_ValueForKey( player->userinfo, "model" ), sizeof( player->model ));
	player->topcolor = Q_atoi( Info_ValueForKey( player->userinfo, "topcolor" ));
	player->bottomcolor = Q_atoi( Info_ValueForKey( player->userinfo, "bottomcolor" ));
}
	
void Host_Credits( void )
{
	// MainUI no longer used - credits handled differently
	// Could implement in ImGui if needed
}

static void UI_ConvertGameInfo( GAMEINFO *out, gameinfo_t *in )
{
	Q_strncpy( out->gamefolder, in->gamefolder, sizeof( out->gamefolder ));
	Q_strncpy( out->startmap, in->startmap, sizeof( out->startmap ));
	Q_strncpy( out->trainmap, in->trainmap, sizeof( out->trainmap ));
	Q_strncpy( out->title, in->title, sizeof( out->title ));
	Q_strncpy( out->version, va( "%g", in->version ), sizeof( out->version ));

	Q_strncpy( out->game_url, in->game_url, sizeof( out->game_url ));
	Q_strncpy( out->update_url, in->update_url, sizeof( out->update_url ));
	Q_strncpy( out->size, Q_pretifymem( in->size, 0 ), sizeof( out->size ));
	Q_strncpy( out->type, in->type, sizeof( out->type ));
	Q_strncpy( out->date, in->date, sizeof( out->date ));

	out->gamemode = in->gamemode;

	if( in->nomodels )
		out->flags |= GFL_NOMODELS;
}

static qboolean PIC_Scissor( float *x, float *y, float *width, float *height, float *u0, float *v0, float *u1, float *v1 )
{
	float	dudx, dvdy;

	// clip sub rect to sprite
	if(( width == 0 ) || ( height == 0 ))
		return false;

	if( *x + *width <= menu.ds.scissor_x )
		return false;
	if( *x >= menu.ds.scissor_x + menu.ds.scissor_width )
		return false;
	if( *y + *height <= menu.ds.scissor_y )
		return false;
	if( *y >= menu.ds.scissor_y + menu.ds.scissor_height )
		return false;

	dudx = (*u1 - *u0) / *width;
	dvdy = (*v1 - *v0) / *height;

	if( *x < menu.ds.scissor_x )
	{
		*u0 += (menu.ds.scissor_x - *x) * dudx;
		*width -= menu.ds.scissor_x - *x;
		*x = menu.ds.scissor_x;
	}

	if( *x + *width > menu.ds.scissor_x + menu.ds.scissor_width )
	{
		*u1 -= (*x + *width - (menu.ds.scissor_x + menu.ds.scissor_width)) * dudx;
		*width = menu.ds.scissor_x + menu.ds.scissor_width - *x;
	}

	if( *y < menu.ds.scissor_y )
	{
		*v0 += (menu.ds.scissor_y - *y) * dvdy;
		*height -= menu.ds.scissor_y - *y;
		*y = menu.ds.scissor_y;
	}

	if( *y + *height > menu.ds.scissor_y + menu.ds.scissor_height )
	{
		*v1 -= (*y + *height - (menu.ds.scissor_y + menu.ds.scissor_height)) * dvdy;
		*height = menu.ds.scissor_y + menu.ds.scissor_height - *y;
	}
	return true;
}

/*
====================
PIC_DrawGeneric

draw hudsprite routine
====================
*/
static void PIC_DrawGeneric( float x, float y, float width, float height, const wrect_t *prc )
{
	float	s1, s2, t1, t2;
	int	w, h;

	// assume we get sizes from image
	R_GetTextureParms( &w, &h, menu.ds.gl_texturenum );

	if( prc )
	{
		// calc user-defined rectangle
		s1 = (float)prc->left / (float)w;
		t1 = (float)prc->top / (float)h;
		s2 = (float)prc->right / (float)w;
		t2 = (float)prc->bottom / (float)h;

		if( width == -1 && height == -1 )
		{
			width = prc->right - prc->left;
			height = prc->bottom - prc->top;
		}
	}
	else
	{
		s1 = t1 = 0.0f;
		s2 = t2 = 1.0f;
	}

	if( width == -1 && height == -1 )
	{
		width = w;
		height = h;
	}

	// pass scissor test if supposed
	if( menu.ds.scissor_test && !PIC_Scissor( &x, &y, &width, &height, &s1, &t1, &s2, &t2 ))
		return;

	PicAdjustSize( &x, &y, &width, &height );
	R_DrawStretchPic( x, y, width, height, s1, t1, s2, t2, menu.ds.gl_texturenum );
	pglColor4ub( 255, 255, 255, 255 );
}

/*
===============================================================================
	MainUI Builtin Functions

===============================================================================
*/
/*
=========
pfnPIC_Load

=========
*/
static HIMAGE pfnPIC_Load( const char *szPicName, const byte *image_buf, int image_size, int flags )
{
	HIMAGE	tx;

	if( !szPicName || !*szPicName )
	{
		MsgDev( D_ERROR, "CL_LoadImage: bad name!\n" );
		return 0;
	}

	// add default parms to image
	flags |= TF_IMAGE;

	host.decal_loading = true; // allow decal images for menu
	tx = GL_LoadTexture( szPicName, image_buf, image_size, flags, NULL );
	host.decal_loading = false;

	return tx;
}

/*
=========
pfnPIC_Width

=========
*/
static int pfnPIC_Width( HIMAGE hPic )
{
	int	picWidth;

	R_GetTextureParms( &picWidth, NULL, hPic );

	return picWidth;
}

/*
=========
pfnPIC_Height

=========
*/
static int pfnPIC_Height( HIMAGE hPic )
{
	int	picHeight;

	R_GetTextureParms( NULL, &picHeight, hPic );

	return picHeight;
}

/*
=========
pfnPIC_Set

=========
*/
void pfnPIC_Set( HIMAGE hPic, int r, int g, int b, int a )
{
	menu.ds.gl_texturenum = hPic;
	r = bound( 0, r, 255 );
	g = bound( 0, g, 255 );
	b = bound( 0, b, 255 );
	a = bound( 0, a, 255 );
	pglColor4ub( r, g, b, a );
}

/*
=========
pfnPIC_Draw

=========
*/
void pfnPIC_Draw( int x, int y, int width, int height, const wrect_t *prc )
{
	GL_SetRenderMode( kRenderNormal );
	PIC_DrawGeneric( x, y, width, height, prc );
}

/*
=========
pfnPIC_DrawTrans

=========
*/
void pfnPIC_DrawTrans( int x, int y, int width, int height, const wrect_t *prc )
{
	GL_SetRenderMode( kRenderTransTexture );
	PIC_DrawGeneric( x, y, width, height, prc );
}

/*
=========
pfnPIC_DrawHoles

=========
*/
void pfnPIC_DrawHoles( int x, int y, int width, int height, const wrect_t *prc )
{
	GL_SetRenderMode( kRenderTransAlpha );
	PIC_DrawGeneric( x, y, width, height, prc );
}

/*
=========
pfnPIC_DrawAdditive

=========
*/
void pfnPIC_DrawAdditive( int x, int y, int width, int height, const wrect_t *prc )
{
	GL_SetRenderMode( kRenderTransAdd );
	PIC_DrawGeneric( x, y, width, height, prc );
}

/*
=========
pfnPIC_EnableScissor

=========
*/
static void pfnPIC_EnableScissor( int x, int y, int width, int height )
{
	// check bounds
	x = bound( 0, x, menu.globals->scrWidth );
	y = bound( 0, y, menu.globals->scrHeight );
	width = bound( 0, width, menu.globals->scrWidth - x );
	height = bound( 0, height, menu.globals->scrHeight - y );

	menu.ds.scissor_x = x;
	menu.ds.scissor_width = width;
	menu.ds.scissor_y = y;
	menu.ds.scissor_height = height;
	menu.ds.scissor_test = true;
}

/*
=========
pfnPIC_DisableScissor

=========
*/
static void pfnPIC_DisableScissor( void )
{
	menu.ds.scissor_x = 0;
	menu.ds.scissor_width = 0;
	menu.ds.scissor_y = 0;
	menu.ds.scissor_height = 0;
	menu.ds.scissor_test = false;
}

/*
=============
pfnFillRGBA

=============
*/
static void pfnFillRGBA( int x, int y, int width, int height, int r, int g, int b, int a )
{
	r = bound( 0, r, 255 );
	g = bound( 0, g, 255 );
	b = bound( 0, b, 255 );
	a = bound( 0, a, 255 );
	pglColor4ub( r, g, b, a );
	GL_SetRenderMode( kRenderTransTexture );
	R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cls.fillImage );
	pglColor4ub( 255, 255, 255, 255 );
}

/*
=============
pfnClientCmd

=============
*/
static void pfnClientCmd( int exec_now, const char *szCmdString )
{
	if( !szCmdString || !szCmdString[0] )
		return;

	Cbuf_AddText( szCmdString );
	Cbuf_AddText( "\n" );

	// client command executes immediately
	if( exec_now ) Cbuf_Execute();
}

/*
=============
pfnPlaySound

=============
*/
static void pfnPlaySound( const char *szSound )
{
	if( !szSound || !*szSound ) return;
	S_StartLocalSound( szSound, VOL_NORM, false );
}

/*
=============
pfnDrawCharacter

quakefont draw character
=============
*/
static void pfnDrawCharacter( int ix, int iy, int iwidth, int iheight, int ch, int ulRGBA, HIMAGE hFont )
{
	rgba_t	color;
	float	row, col, size;
	float	s1, t1, s2, t2;
	float	x = ix, y = iy;
	float	width = iwidth;
	float	height = iheight;

	ch &= 255;

	if( ch == ' ' ) return;
	if( y < -height ) return;

	color[3] = (ulRGBA & 0xFF000000) >> 24;
	color[0] = (ulRGBA & 0xFF0000) >> 16;
	color[1] = (ulRGBA & 0xFF00) >> 8;
	color[2] = (ulRGBA & 0xFF) >> 0;
	pglColor4ubv( color );

	col = (ch & 15) * 0.0625f + (0.5f / 256.0f);
	row = (ch >> 4) * 0.0625f + (0.5f / 256.0f);
	size = 0.0625f - (1.0f / 256.0f);

	s1 = col;
	t1 = row;
	s2 = s1 + size;
	t2 = t1 + size;

	// pass scissor test if supposed
	if( menu.ds.scissor_test && !PIC_Scissor( &x, &y, &width, &height, &s1, &t1, &s2, &t2 ))
		return;

	GL_SetRenderMode( kRenderTransTexture );
	R_DrawStretchPic( x, y, width, height, s1, t1, s2, t2, hFont );
	pglColor4ub( 255, 255, 255, 255 );
}

/*
=============
UI_DrawConsoleString

drawing string like a console string 
=============
*/
static int UI_DrawConsoleString( int x, int y, const char *string )
{
	int	drawLen;

	if( !string || !*string ) return 0; // silent ignore
	drawLen = Con_DrawString( x, y, string, menu.ds.textColor );
	MakeRGBA( menu.ds.textColor, 255, 255, 255, 255 );

	return (x + drawLen); // exclude color prexfixes
}

/*
=============
pfnDrawSetTextColor

set color for anything
=============
*/
static void UI_DrawSetTextColor( int r, int g, int b, int alpha )
{
	// bound color and convert to byte
	menu.ds.textColor[0] = r;
	menu.ds.textColor[1] = g;
	menu.ds.textColor[2] = b;
	menu.ds.textColor[3] = alpha;
}
/*
=======================
UI_AddTouchButtonToList

send button parameters to menu
=======================
*/
void UI_AddTouchButtonToList( const char *name, const char *texture, const char *command, unsigned char *color, int flags )
{
	if( pfnAddTouchButtonToList )
	{
		pfnAddTouchButtonToList( name, texture, command, color, flags );
	}
}

/*
====================
pfnGetPlayerModel

for drawing playermodel previews
====================
*/
static cl_entity_t* pfnGetPlayerModel( void )
{
	return &menu.playermodel;
}

/*
====================
pfnSetPlayerModel

for drawing playermodel previews
====================
*/
static void pfnSetPlayerModel( cl_entity_t *ent, const char *path )
{
	Mod_RegisterModel( path, MAX_MODELS - 1 );
	ent->curstate.modelindex = MAX_MODELS - 1;
	ent->model = Mod_Handle( MAX_MODELS - 1 );
}

/*
====================
pfnRenderScene

for drawing playermodel previews
====================
*/
static void pfnRenderScene( const ref_params_t *fd )
{
	if ( !cls.initialized ) return;
	if( cls.state < ca_connecting )
		userinfo->modified = true;

	// to avoid division by zero
	if( !fd || fd->fov_x <= 0.0f || fd->fov_y <= 0.0f )
		return;
	R_RenderFrame( fd, false );
}

/*
====================
pfnClientJoin

send client connect
====================
*/
static void pfnClientJoin( const netadr_t adr )
{
	Cbuf_AddText( va( "connect %s\n", NET_AdrToString( adr )));
}

/*
====================
pfnKeyGetOverstrikeMode

get global key overstrike state
====================
*/
static int pfnKeyGetOverstrikeMode( void )
{
	return host.key_overstrike;
}

/*
====================
pfnKeySetOverstrikeMode

set global key overstrike mode
====================
*/
static void pfnKeySetOverstrikeMode( int fActive )
{
	host.key_overstrike = fActive;
}

/*
====================
pfnKeyGetState

returns kbutton struct if found
====================
*/
static void *pfnKeyGetState( const char *name )
{
	if( clgame.dllFuncs.KB_Find )
		return clgame.dllFuncs.KB_Find( name );
	return NULL;
}

/*
=========
pfnMemAlloc

=========
*/
static void *pfnMemAlloc( size_t cb, const char *filename, const int fileline )
{
	return _Mem_Alloc( menu.mempool, cb, filename, fileline );
}

/*
=========
pfnMemFree

=========
*/
static void pfnMemFree( void *mem, const char *filename, const int fileline )
{
	_Mem_Free( mem, filename, fileline );
}

/*
=========
pfnGetGameInfo

=========
*/
static int pfnGetGameInfo( GAMEINFO *pgameinfo )
{
	if( !pgameinfo ) return 0;

	*pgameinfo = menu.gameInfo;
	return 1;
}

/*
=========
pfnGetGamesList

=========
*/
static GAMEINFO **pfnGetGamesList( int *numGames )
{
	if( numGames ) *numGames = SI.numgames;
	return menu.modsInfo;
}

/*
=========
pfnGetFilesList

release prev search on a next call
=========
*/
static char **pfnGetFilesList( const char *pattern, int *numFiles, int gamedironly )
{
	static search_t	*t = NULL;

	if( t ) Mem_Free( t ); // release prev search

	t = FS_Search( pattern, true, gamedironly );
	if( !t )
	{
		if( numFiles ) *numFiles = 0;
		return NULL;
	}

	if( numFiles ) *numFiles = t->numfilenames;
	return t->filenames;
}

/*
=========
pfnGetClipboardData

pointer must be released in call place
=========
*/
static char *pfnGetClipboardData( void )
{
	char *cb, *copy; 
	cb = Sys_GetClipboardData();
	copy = copystring( cb );
	return copy;
}

/*
=========
pfnCheckGameDll

=========
*/
int pfnCheckGameDll( void )
{
	void	*hInst;

	if( svgame.hInstance )
		return true;

	if( Cvar_VariableInteger("xashds_hacks") )
		return true;

	Com_ResetLibraryError();
	if(( hInst = Com_LoadLibrary( SI.gamedll, true )) != NULL )
	{
		return true;
	}
	MsgDev( D_WARN, "Could not load server library:\n%s", Com_GetLibraryError() );
	Com_ResetLibraryError();
	return false;
}

/*
=========
pfnChangeInstance

=========
*/
static void pfnChangeInstance( const char *newInstance, const char *szFinalMessage )
{
	if( !szFinalMessage ) szFinalMessage = "";
	if( !newInstance || !*newInstance ) return;

	Host_NewInstance( newInstance, szFinalMessage );
}

/*
=========
pfnHostNewGame

=========
*/
static void pfnHostNewGame( const char *mapName )
{
	if( !mapName || !*mapName ) return;
	Host_NewGame( mapName, false );
}

/*
=========
pfnHostEndGame

=========
*/
static void pfnHostEndGame( const char *szFinalMessage )
{
	if( !szFinalMessage ) szFinalMessage = "";
	Host_EndGame( szFinalMessage );
}

/*
=========
pfnStartBackgroundTrack

=========
*/
static void pfnStartBackgroundTrack( const char *introTrack, const char *mainTrack )
{
	S_StartBackgroundTrack( introTrack, mainTrack, 0 );
}

static void pfnEnableTextInput( int enable )
{
	Key_EnableTextInput( enable, false );
}

/*
===========
pfnCvarSet

Since menu can load script files, it's unsafe to allow
menu code change read-only cvar values
===========
*/
static void pfnCvarSet( const char *name, const char *value )
{
	Cvar_Set2( name, value, false );
}

/*
===========
pfnCvarSetValue

Since menu can load script files, it's unsafe to allow
menu code change read-only cvar values
===========
*/
static void pfnCvarSetValue( const char *name, float value )
{
	char val[32];

	if( value == (int)value )
		Q_sprintf( val, "%i", (int)value );
	else Q_sprintf( val, "%f", value );

	Cvar_Set2( name, val, false );
}

static void pfnWriteGameConfig( const char *name )
{
	// do not use game.cfg or listenserver.cfg
	// as it will be executed from hlsdk
	Host_WriteGameConfig( "gamesettings.cfg" );

	// remove old game.cfg, prevent it from resetting settings
	FS_Delete( "game.cfg" );
}

// engine callbacks
static ui_enginefuncs_t gEngfuncs = 
{
	(void*)pfnPIC_Load,
	GL_FreeImage,
	pfnPIC_Width,
	pfnPIC_Height,
	pfnPIC_Set,
	pfnPIC_Draw,
	pfnPIC_DrawHoles,
	pfnPIC_DrawTrans,
	pfnPIC_DrawAdditive,
	pfnPIC_EnableScissor,
	pfnPIC_DisableScissor,
	pfnFillRGBA,
	pfnCvar_RegisterVariable,
	Cvar_VariableValue,
	Cvar_VariableString,
	pfnCvarSet,
	pfnCvarSetValue,
	pfnAddClientCommand,
	pfnClientCmd,
	Cmd_RemoveCommand,
	Cmd_Argc,
	Cmd_Argv,
	Cmd_Args,
	(void*)Con_Printf,
	(void*)Con_DPrintf,
	(void*)UI_NPrintf,
	(void*)UI_NXPrintf,
	pfnPlaySound,
	UI_DrawLogo,
	UI_GetLogoWidth,
	UI_GetLogoHeight,
	UI_GetLogoLength,
	pfnDrawCharacter,
	UI_DrawConsoleString,
	UI_DrawSetTextColor,
	Con_DrawStringLen,
	Con_DefaultColor,
	pfnGetPlayerModel,
	pfnSetPlayerModel,
	R_ClearScene,	
	pfnRenderScene,
	CL_AddEntity,
	Host_Error,
	(void*)FS_FileExists,
	pfnGetGameDir,
	(void*)Cmd_CheckMapsList,
	(void*)CL_Active,
	pfnClientJoin,
	COM_LoadFileForMe,
	COM_ParseFile,
	COM_FreeFile,
	Key_ClearStates,
	Key_SetKeyDest,
	Key_KeynumToString,
	Key_GetBinding,
	Key_SetBinding,
	(void*)Key_IsDown,
	pfnKeyGetOverstrikeMode,
	pfnKeySetOverstrikeMode,
	pfnKeyGetState,
	pfnMemAlloc,
	pfnMemFree,
	pfnGetGameInfo,
	pfnGetGamesList,
	pfnGetFilesList,
	(void*)SV_GetComment,
	(void*)CL_GetComment,
	pfnCheckGameDll,
	pfnGetClipboardData,
	(void*)Sys_ShellExecute,
	pfnWriteGameConfig,
	pfnChangeInstance,
	pfnStartBackgroundTrack,
	pfnHostEndGame,
	Com_RandomFloat,
	(void*)Com_RandomLong,
	(void*)IN_SetCursor,
	pfnIsMapValid,
	GL_ProcessTexture,
	(void*)COM_CompareFileTime,
	VID_GetModeString
};

static ui_textfuncs_t gTextfuncs =
{
	pfnEnableTextInput,
	Con_UtfProcessChar,
	Con_UtfMoveLeft,
	Con_UtfMoveRight
};

void UI_UnloadProgs( void )
{
	// MainUI unloading is no longer needed - we use ImGui menu
	// Free globals if allocated
	if( menu.globals && menu.mempool )
	{
		Mem_Free( menu.globals );
	}
	// Clear the menu state
	Q_memset( &menu, 0, sizeof( menu ));
}

qboolean UI_LoadProgs( void )
{
	// MainUI (menu.dll) is disabled - using ImGui menu instead
	// Initialize menu globals for ImGui compatibility
	menu.mempool = Mem_AllocPool( "Menu Pool" );
	
	// Allocate and initialize globals (needed for VGui_Startup in cl_game.c)
	menu.globals = Mem_Alloc( menu.mempool, sizeof( ui_globalvars_t ) );
	menu.globals->scrWidth = scr_width->integer;
	menu.globals->scrHeight = scr_height->integer;
	menu.globals->developer = host.developer;
	
	// Mark as "loaded" but use ImGui
	menu.hInstance = (void*)1; // Non-null to indicate "loaded"
	
	MsgDev( D_INFO, "UI_LoadProgs: Using ImGui menu system (MainUI disabled)\n" );
	return true;
}
#endif // XASH_DEDICATED
