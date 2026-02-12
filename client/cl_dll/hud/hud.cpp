/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// hud.cpp
//
// implementation of CHud class
//

#include <new>

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "parsemsg.h"

#include "demo.h"
#include "demo_api.h"
#include "vgui_parser.h"
#include "rain.h"

#include "event_api.h"
#include "r_efx.h"
#include "com_model.h"
#include "r_studioint.h"

#include "camera.h"

extern engine_studio_api_t IEngineStudio;

static cvar_t *g_pCvarZB3ClawDebug = nullptr;
static cvar_t *g_pCvarZB3ClawYawOffset = nullptr;
static cvar_t *g_pCvarZB3ClawFwdOffset = nullptr;
static cvar_t *g_pCvarZB3ClawZOffset = nullptr;
static cvar_t *g_pCvarZB3ClawPitchInvert = nullptr;
static cvar_t *g_pCvarZB3ClawRenderPitch = nullptr;
static cvar_t *g_pCvarZB3ClawYawNegFlip = nullptr;


extern client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

wrect_t nullrc = { 0, 0, 0, 0 };
float g_lastFOV = 0.0;
const char *sPlayerModelFiles[12] =
{
	"models/player.mdl",
	"models/player/leet/leet.mdl", // t
	"models/player/gign/gign.mdl", // ct
	"models/player/vip/vip.mdl", //ct
	"models/player/gsg9/gsg9.mdl", // ct
	"models/player/guerilla/guerilla.mdl", // t
	"models/player/arctic/arctic.mdl", // t
	"models/player/sas/sas.mdl", // ct
	"models/player/terror/terror.mdl", // t
	"models/player/urban/urban.mdl", // ct
	"models/player/spetsnaz/spetsnaz.mdl", // ct
	"models/player/militia/militia.mdl" // t
};

#define GHUD_DECLARE_MESSAGE(x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf ) { return gHUD.MsgFunc_##x(pszName, iSize, pbuf); }

GHUD_DECLARE_MESSAGE(Logo)
GHUD_DECLARE_MESSAGE(SetFOV)
GHUD_DECLARE_MESSAGE(InitHUD)
GHUD_DECLARE_MESSAGE(Concuss)
GHUD_DECLARE_MESSAGE(ResetHUD)
GHUD_DECLARE_MESSAGE(ViewMode)
GHUD_DECLARE_MESSAGE(GameMode)
GHUD_DECLARE_MESSAGE(ShadowIdx)
GHUD_DECLARE_MESSAGE(ZB3Claw)

void __CmdFunc_InputCommandSpecial()
{
#ifdef _CS16CLIENT_ALLOW_SPECIAL_SCRIPTING
	gEngfuncs.pfnClientCmd("_special");
#endif
}

#define XASH_GENERATE_BUILDNUM

#if defined(XASH_GENERATE_BUILDNUM)
static const char *date = __DATE__;
static const char *mon[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char mond[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
#endif

char *Q_buildnum( void )
{
// do not touch this! Only author of Xash3D can increase buildnumbers!
// Xash3D SDL: HAHAHA! I TOUCHED THIS!
	int m = 0, d = 0, y = 0;
	static int b = 0;
	static char buildnum[16];

	if( b != 0 )
		return buildnum;

	for( m = 0; m < 11; m++ )
	{
		if( !strncasecmp( &date[0], mon[m], 3 ))
			break;
		d += mond[m];
	}

	d += atoi( &date[4] ) - 1;
	y = atoi( &date[7] ) - 1900;
	b = d + (int)((y - 1) * 365.25f );

	if((( y % 4 ) == 0 ) && m > 1 )
	{
		b += 1;
	}
	//b -= 38752; // Feb 13 2007
	b -= 41940; // Oct 29 2015.
	// Happy birthday, cs16client! :)

	_snprintf( buildnum, sizeof(buildnum), "%i", b );

	return buildnum;
}

int __MsgFunc_ADStop( const char *name, int size, void *buf ) { return 1; }
int __MsgFunc_ItemStatus( const char *name, int size, void *buf ) { return 1; }
int __MsgFunc_ReqState( const char *name, int size, void *buf ) { return 1; }
int __MsgFunc_ForceCam( const char *name, int size, void *buf ) { return 1; }
int __MsgFunc_Spectator( const char *name, int size, void *buf ) { return 1; }
int __MsgFunc_ServerName( const char *name, int size, void *buf )
{
	BufferReader reader( name, buf, size );
	strncpy( gHUD.m_szServerName, reader.ReadString(), 64 );
	return 1;
}

int CHud::MsgFunc_ZB3Claw(const char *pszName, int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );
	Vector pos;
	pos.x = reader.ReadCoord();
	pos.y = reader.ReadCoord();
	pos.z = reader.ReadCoord();

	Vector attackerOrg;
	attackerOrg.x = reader.ReadCoord();
	attackerOrg.y = reader.ReadCoord();
	attackerOrg.z = reader.ReadCoord();

	Vector ang;
	ang.x = reader.ReadAngle();
	const float unusedYaw = reader.ReadAngle();
	ang.z = 0.0f;

	Vector dir = pos - attackerOrg;
	if (dir.Length() > 0.001f)
	{
		const float yawRad = atan2f(dir.y, dir.x);
		ang.y = yawRad * (180.0f / 3.14159265358979323846f);
	}
	else
	{
		ang.y = unusedYaw;
	}

	if (ang.x > 180.0f)
		ang.x -= 360.0f;

	const float yawOffset = g_pCvarZB3ClawYawOffset ? g_pCvarZB3ClawYawOffset->value : 0.0f;
	const float fwdOffset = g_pCvarZB3ClawFwdOffset ? g_pCvarZB3ClawFwdOffset->value : -20.0f;
	const float zOffset = g_pCvarZB3ClawZOffset ? g_pCvarZB3ClawZOffset->value : 0.0f;
	const bool pitchInvert = g_pCvarZB3ClawPitchInvert && (g_pCvarZB3ClawPitchInvert->value > 0.0f);
	const bool renderPitch = g_pCvarZB3ClawRenderPitch && (g_pCvarZB3ClawRenderPitch->value > 0.0f);
	const bool yawNegFlip = g_pCvarZB3ClawYawNegFlip && (g_pCvarZB3ClawYawNegFlip->value > 0.0f);

	static float s_lastDebugPrint = 0.0f;
	const float now = gEngfuncs.GetClientTime();
	const bool doPrint = g_pCvarZB3ClawDebug && (g_pCvarZB3ClawDebug->value > 0.0f) && (now - s_lastDebugPrint >= 0.10f);
	if (doPrint)
	{
		s_lastDebugPrint = now;
		char buf[256];
		_snprintf(buf, sizeof(buf), "[ZB3] claw recv pos(%.1f %.1f %.1f) ang(%.1f %.1f %.1f) tune(yaw=%.1f fwd=%.1f z=%.1f pinv=%d rpit=%d yneg=%d)\n",
			pos.x, pos.y, pos.z,
			ang.x, ang.y, ang.z,
			yawOffset, fwdOffset, zOffset, pitchInvert ? 1 : 0, renderPitch ? 1 : 0, yawNegFlip ? 1 : 0);
		gEngfuncs.pfnConsolePrint(buf);
	}

	Vector forward;
	Vector offsetAng = ang;
	offsetAng.x = 0.0f;
	AngleVectors(offsetAng, forward, nullptr, nullptr);
	pos = pos + forward * fwdOffset;
	pos.z += zOffset;

	Vector renderAng = ang;
	if (!renderPitch)
		renderAng.x = 0.0f;
	else if (pitchInvert)
		renderAng.x *= -1.0f;
	renderAng.y += yawOffset;
	if (yawNegFlip && renderAng.y < 0.0f)
		renderAng.y *= -1.0f;
	if (renderAng.y > 180.0f)
		renderAng.y -= 360.0f;
	else if (renderAng.y < -180.0f)
		renderAng.y += 360.0f;
	renderAng.z = 0.0f;

	if (doPrint)
	{
		char buf[256];
		_snprintf(buf, sizeof(buf), "[ZB3] claw fwd(%.3f %.3f %.3f) final pos(%.1f %.1f %.1f) final ang(%.1f %.1f %.1f)\n",
			forward.x, forward.y, forward.z,
			pos.x, pos.y, pos.z,
			renderAng.x, renderAng.y, renderAng.z);
		gEngfuncs.pfnConsolePrint(buf);
	}

	const int modelIdx = gEngfuncs.pEventAPI->EV_FindModelIndex("models/csnc/claweffect.mdl");
	if (!modelIdx)
		return 1;

	model_t *pModel = IEngineStudio.GetModelByIndex(modelIdx);
	if (!pModel)
		return 1;

	TEMPENTITY *te = gEngfuncs.pEfxAPI->CL_TempEntAlloc(pos, (struct model_s *)pModel);
	if (!te)
		return 1;

	te->flags |= (FTENT_CLIENTCUSTOM | FTENT_PERSIST | FTENT_FADEOUT);
	te->die = gEngfuncs.GetClientTime() + 0.7f;
	te->fadeSpeed = 1.0f / 0.7f;
	te->entity.curstate.modelindex = modelIdx;
	te->entity.curstate.effects |= EF_NOINTERP;
	te->entity.baseline.effects |= EF_NOINTERP;
	te->entity.prevstate.effects |= EF_NOINTERP;
	te->entity.curstate.angles[0] = renderAng.x;
	te->entity.curstate.angles[1] = renderAng.y;
	te->entity.curstate.angles[2] = renderAng.z;
	te->entity.baseline.angles[0] = renderAng.x;
	te->entity.baseline.angles[1] = renderAng.y;
	te->entity.baseline.angles[2] = renderAng.z;
	te->entity.prevstate.angles[0] = renderAng.x;
	te->entity.prevstate.angles[1] = renderAng.y;
	te->entity.prevstate.angles[2] = renderAng.z;
	te->entity.latched.prevangles[0] = renderAng.x;
	te->entity.latched.prevangles[1] = renderAng.y;
	te->entity.latched.prevangles[2] = renderAng.z;
	te->entity.angles[0] = renderAng.x;
	te->entity.angles[1] = renderAng.y;
	te->entity.angles[2] = renderAng.z;
	te->entity.curstate.sequence = 0;
	te->entity.curstate.frame = 1.0f;
	te->entity.curstate.framerate = 1.0f;
	te->entity.curstate.animtime = gEngfuncs.GetClientTime();
	te->entity.curstate.rendermode = kRenderTransAdd;
	te->entity.curstate.renderfx = kRenderFxNone;
	te->entity.curstate.renderamt = 255;
	te->entity.baseline.renderamt = 255;
	te->entity.prevstate.renderamt = 255;
	te->entity.curstate.scale = 1.0f;
	return 1;
}

#ifdef __ANDROID__
bool evdev_open = false;
void __CmdFunc_MouseSucksOpen( void ) { evdev_open = true; }
void __CmdFunc_MouseSucksClose( void ) { evdev_open = false; }
#endif


// This is called every time the DLL is loaded
void CHud :: Init( void )
{
	HOOK_COMMAND( "special", InputCommandSpecial );

#ifdef __ANDROID__
	HOOK_COMMAND( "evdev_mouseopen", MouseSucksOpen );
	HOOK_COMMAND( "evdev_mouseclose", MouseSucksClose );
#endif
	
	HOOK_MESSAGE( Logo );
	HOOK_MESSAGE( ResetHUD );
	HOOK_MESSAGE( GameMode );
	HOOK_MESSAGE( InitHUD );
	HOOK_MESSAGE( ViewMode );
	HOOK_MESSAGE( SetFOV );
	HOOK_MESSAGE( Concuss );

	HOOK_MESSAGE( ADStop );
	HOOK_MESSAGE( ItemStatus );
	HOOK_MESSAGE( ReqState );
	HOOK_MESSAGE( ForceCam );
	HOOK_MESSAGE( Spectator ); // ignored due to touch menus
	HOOK_MESSAGE( ServerName );


	HOOK_MESSAGE( ShadowIdx );
	HOOK_MESSAGE( ZB3Claw );

	CVAR_CREATE( "_vgui_menus", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );
	CVAR_CREATE( "_cl_autowepswitch", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );
	CVAR_CREATE( "_ah", "0", FCVAR_ARCHIVE | FCVAR_USERINFO );
	g_pCvarZB3ClawDebug = CVAR_CREATE( "zb3_claw_debug", "0", 0 );
	g_pCvarZB3ClawYawOffset = CVAR_CREATE( "zb3_claw_yaw_offset", "0", 0 );
	g_pCvarZB3ClawFwdOffset = CVAR_CREATE( "zb3_claw_fwd_offset", "-20", 0 );
	g_pCvarZB3ClawZOffset = CVAR_CREATE( "zb3_claw_z_offset", "0", 0 );
	g_pCvarZB3ClawPitchInvert = CVAR_CREATE( "zb3_claw_pitch_invert", "0", 0 );
	g_pCvarZB3ClawRenderPitch = CVAR_CREATE( "zb3_claw_render_pitch", "0", 0 );
	g_pCvarZB3ClawYawNegFlip = CVAR_CREATE( "zb3_claw_yaw_neg_flip", "0", 0 );

	hud_textmode = CVAR_CREATE( "hud_textmode", "0", FCVAR_ARCHIVE );
	hud_colored  = CVAR_CREATE( "hud_colored", "0", FCVAR_ARCHIVE );
	cl_righthand = CVAR_CREATE( "hand", "1", FCVAR_ARCHIVE );
	cl_weather   = CVAR_CREATE( "cl_weather", "1", FCVAR_ARCHIVE );
	cl_minmodels = CVAR_CREATE( "cl_minmodels", "0", FCVAR_ARCHIVE );
	cl_min_t     = CVAR_CREATE( "cl_min_t", "1", FCVAR_ARCHIVE );
	cl_min_ct    = CVAR_CREATE( "cl_min_ct", "2", FCVAR_ARCHIVE );
	cl_lw        = gEngfuncs.pfnGetCvarPointer( "cl_lw" );
	cl_predict   = gEngfuncs.pfnGetCvarPointer( "cl_predict" );
#ifdef __ANDROID__
	cl_android_force_defaults  = CVAR_CREATE( "cl_android_force_defaults", "1", FCVAR_ARCHIVE );
#endif
	cl_shadows   = CVAR_CREATE( "cl_shadows", "1", FCVAR_ARCHIVE );
	default_fov  = CVAR_CREATE( "default_fov", "90", 0 );
	m_pCvarDraw  = CVAR_CREATE( "hud_draw", "1", FCVAR_ARCHIVE );
	cl_gunsmoke  = CVAR_CREATE( "cl_gunsmoke", "0", FCVAR_ARCHIVE );
	cl_weapon_sparks = CVAR_CREATE( "cl_weapon_sparks", "1", FCVAR_ARCHIVE );
	cl_weapon_wallpuff = CVAR_CREATE( "cl_weapon_wallpuff", "1", FCVAR_ARCHIVE );
	zoom_sens_ratio = CVAR_CREATE( "zoom_sensitivity_ratio", "1.2", 0 );
	sv_skipshield = gEngfuncs.pfnGetCvarPointer( "sv_skipshield" );

	CVAR_CREATE( "cscl_ver", Q_buildnum(), FCVAR_INIT | FCVAR_USERINFO ); // init and userinfo

	m_iLogo = 0;
	m_iFOV = 0;

	m_pSpriteList = NULL;

	// Clear any old HUD list
	for( HUDLIST *pList = m_pHudList; pList; pList = m_pHudList )
	{
		m_pHudList = m_pHudList->pNext;
		delete pList;
	}
	m_pHudList = NULL;

	// In case we get messages before the first update -- time will be valid
	m_flTime = 1.0;
	m_iNoConsolePrint = 0;
	m_szServerName[0] = 0;

	Localize_Init();

	// fullscreen overlays
	m_SniperScope.Init();
	m_NVG.Init();
	m_SpectatorGui.Init();

	// Game HUD things
	m_Ammo.Init();
	m_Health.Init();
	m_Radio.Init();
	m_Timer.Init();
	m_Money.Init();
	m_AmmoSecondary.Init();
	m_Train.Init();
	m_Battery.Init();
	m_StatusIcons.Init();
	m_Radar.Init();

	// chat, death notice, status bars and other
	m_SayText.Init();
	m_Spectator.Init();
	m_Geiger.Init();
	m_Flash.Init();
	m_Message.Init();
	m_StatusBar.Init();
	m_DeathNotice.Init();
	m_TextMessage.Init();
	m_MOTD.Init();

	// all things that have own background and must be drawn last
	m_ProgressBar.Init();
	m_Menu.Init();
	m_Scoreboard.Init();

	InitRain();

	//ServersInit();

	gEngfuncs.Cvar_SetValue( "hand", 1 );
	gEngfuncs.Cvar_SetValue( "sv_skipshield", 1.0f );
#ifdef __ANDROID__
	gEngfuncs.Cvar_SetValue( "hud_fastswitch", 1 );
#endif

	MsgFunc_ResetHUD(0, 0, NULL );
}

// CHud destructor
// cleans up memory allocated for m_rg* arrays
CHud :: ~CHud()
{
	delete [] m_rghSprites;
	delete [] m_rgrcRects;
	delete [] m_rgszSpriteNames;

	// Clear any old HUD list
	for( HUDLIST *pList = m_pHudList; pList; pList = m_pHudList )
	{
		m_pHudList = m_pHudList->pNext;
		delete pList;
	}
	m_pHudList = NULL;
}

void CHud :: VidInit( void )
{
	static bool firstinit = true;
	m_scrinfo.iSize = sizeof( m_scrinfo );
	GetScreenInfo( &m_scrinfo );

	m_truescrinfo.iWidth = CVAR_GET_FLOAT("width");
	m_truescrinfo.iHeight = CVAR_GET_FLOAT("height");

	// ----------
	// Load Sprites
	// ---------
	//	m_hsprFont = LoadSprite("sprites/%d_font.spr");
	
	m_hsprLogo = 0;

	// assume cs16-client is launched in landscape mode
	// must be only TrueWidth, but due to bug game may sometime rotate to portait mode
	// calc scale depending on max side
	float maxScale = (float)max( TrueWidth, TrueHeight ) / 640.0f;
	
	// REMOVE LATER
	float currentScale = CVAR_GET_FLOAT("hud_scale");
	float invalidScale = (float)min( TrueWidth, TrueHeight ) / 640.0f;
	// REMOVE LATER
	
	if( currentScale > maxScale ||
		( currentScale == invalidScale &&
		  currentScale != 1.0f &&
		  currentScale != 0.0f &&
		  invalidScale <  1.0f ) )
	{
		gEngfuncs.Cvar_SetValue( "hud_scale", maxScale );
		gEngfuncs.Con_Printf("^3Maximum scale factor reached. Reset: %f\n", maxScale );
		GetScreenInfo( &m_scrinfo );
	}

	m_flScale = CVAR_GET_FLOAT( "hud_scale" );

	// give a real values to other code. It's not anymore an actual CVar value
	if( m_flScale == 0.0f )
		m_flScale = 1.0f;

	m_iRes = 640;

	// Only load this once
	if( !m_pSpriteList )
	{
		// we need to load the hud.txt, and all sprites within
		m_pSpriteList = SPR_GetList("sprites/hud.txt", &m_iSpriteCountAllRes);

		if( m_pSpriteList )
		{
			// count the number of sprites of the appropriate res
			m_iSpriteCount = 0;
			client_sprite_t *p = m_pSpriteList;
			for ( int j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if ( p->iRes == m_iRes )
					m_iSpriteCount++;
				p++;
			}

			// allocated memory for sprite handle arrays
			m_rghSprites      = new(std::nothrow) _HSPRITE[m_iSpriteCount];
			m_rgrcRects       = new(std::nothrow) wrect_t[m_iSpriteCount];
			m_rgszSpriteNames = new(std::nothrow) char[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH];;

			if( !m_rghSprites || !m_rgrcRects || !m_rgszSpriteNames )
			{
				gEngfuncs.pfnConsolePrint("CHud::VidInit(): Cannot allocate memory");
				if( g_iXash )
					gRenderAPI.Host_Error("CHud::VidInit(): Cannot allocate memory");
			}

			p = m_pSpriteList;
			for ( int index = 0, j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if ( p->iRes == m_iRes )
				{
					char sz[256];
					sprintf(sz, "sprites/%s.spr", p->szSprite);
					m_rghSprites[index] = SPR_Load(sz);
					m_rgrcRects[index] = p->rc;
					strncpy( &m_rgszSpriteNames[index * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH );

					index++;
				}

				p++;
			}
		}
	}
	else
	{
		// we have already have loaded the sprite reference from hud.txt, but
		// we need to make sure all the sprites have been loaded (we've gone through a transition, or loaded a save game)
		client_sprite_t *p = m_pSpriteList;
		int index = 0;
		for ( int j = 0; j < m_iSpriteCountAllRes; j++ )
		{
			if ( p->iRes == m_iRes )
			{
				char sz[256];
				sprintf( sz, "sprites/%s.spr", p->szSprite );
				m_rghSprites[index] = SPR_Load(sz);
				index++;
			}

			p++;
		}
	}

	// assumption: number_1, number_2, etc, are all listed and loaded sequentially
	m_HUD_number_0 = GetSpriteIndex( "number_0" );

	if( m_HUD_number_0 == -1 && g_iXash )
	{
		gRenderAPI.Host_Error( "Failed to get number_0 sprite index. Check your game data!" );
		return;
	}

	m_iFontHeight = GetSpriteRect(m_HUD_number_0).bottom - GetSpriteRect(m_HUD_number_0).top;

	m_hGasPuff = SPR_Load("sprites/gas_puff_01.spr");


	/*m_Ammo.VidInit();
	m_Health.VidInit();
	m_Spectator.VidInit();
	m_Geiger.VidInit();
	m_Train.VidInit();
	m_Battery.VidInit();
	m_Flash.VidInit();
	m_Message.VidInit();
	m_StatusBar.VidInit();
	m_DeathNotice.VidInit();
	m_SayText.VidInit();
	m_Menu.VidInit();
	m_AmmoSecondary.VidInit();
	m_TextMessage.VidInit();
	m_StatusIcons.VidInit();
	m_Scoreboard.VidInit();
	m_MOTD.VidInit();
	m_Timer.VidInit();
	m_Money.VidInit();
	m_ProgressBar.VidInit();
	m_SniperScope.VidInit();
	m_Radar.VidInit();
	m_SpectatorGui.VidInit();*/

	for( HUDLIST *pList = m_pHudList; pList; pList = pList->pNext )
		pList->p->VidInit();

	if( firstinit && gEngfuncs.CheckParm( "-firsttime", NULL ) )
	{
		ConsolePrint( "firstrun\n" );

		ClientCmd( "exec touch_presets/phone_ahsim" );
		gEngfuncs.Cvar_Set( "touch_config_file", "touch_presets/phone_ahsim.cfg" );
	}

	firstinit = false;
}

void CHud::Shutdown( void )
{
	for( HUDLIST *pList = m_pHudList; pList; pList = pList->pNext )
	{
		pList->p->Shutdown();
	}
}

int CHud::MsgFunc_Logo(const char *pszName,  int iSize, void *pbuf)
{
	BufferReader reader( pszName, pbuf, iSize );

	// update Train data
	m_iLogo = reader.ReadByte();

	return 1;
}

/*
============
COM_FileBase
============
*/
// Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
void COM_FileBase ( const char *in, char *out)
{
	int len, start, end;

	len = strlen( in );
	
	// scan backward for '.'
	end = len - 1;
	while ( end && in[end] != '.' && in[end] != '/' && in[end] != '\\' )
		end--;
	
	if ( in[end] != '.' )		// no '.', copy to end
		end = len-1;
	else
		end--;					// Found ',', copy to left of '.'


	// Scan backward for '/'
	start = len-1;
	while ( start >= 0 && in[start] != '/' && in[start] != '\\' )
		start--;

	if ( in[start] != '/' && in[start] != '\\' )
		start = 0;
	else
		start++;

	// Length of new sting
	len = end - start + 1;

	// Copy partial string
	strncpy( out, &in[start], len );
	// Terminate it
	out[len] = 0;
}

/*
=================
HUD_IsGame

=================
*/
int HUD_IsGame( const char *game )
{
	const char *gamedir;
	char gd[ 1024 ];

	gamedir = gEngfuncs.pfnGetGameDirectory();
	if ( gamedir && gamedir[0] )
	{
		COM_FileBase( gamedir, gd );
		if ( !stricmp( gd, game ) )
			return 1;
	}
	return 0;
}

/*
=====================
HUD_GetFOV

Returns last FOV
=====================
*/
float HUD_GetFOV( void )
{
	if ( gEngfuncs.pDemoAPI->IsRecording() )
	{
		// Write it
		unsigned char buf[ sizeof(float) ];

		// Active
		*( float * )&buf = g_lastFOV;

		Demo_WriteBuffer( TYPE_ZOOM, sizeof(float), buf );
	}

	if ( gEngfuncs.pDemoAPI->IsPlayingback() )
	{
		g_lastFOV = g_demozoom;
	}
	return g_lastFOV;
}

int CHud::MsgFunc_SetFOV(const char *pszName,  int iSize, void *pbuf)
{
	//Weapon prediction already takes care of changing the fog. ( g_lastFOV ).
#if 0 // VALVEWHY: original client checks for "tfc" here.
	if ( cl_lw && cl_lw->value )
		return 1;
#endif

	BufferReader reader( pszName, pbuf, iSize );

	int newfov = reader.ReadByte();
	int def_fov = default_fov->value;

	g_lastFOV = newfov;
	m_iFOV = newfov ? newfov : def_fov;

	// the clients fov is actually set in the client data update section of the hud

	if ( m_iFOV == def_fov ) // reset to saved sensitivity
		m_flMouseSensitivity = 0;
	else // set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)def_fov) * zoom_sens_ratio->value;

	return 1;
}

void CHud::AddHudElem(CHudBase *phudelem)
{
	assert( phudelem );

	HUDLIST *pdl, *ptemp;

	pdl = new(std::nothrow) HUDLIST;
	if( !pdl )
	{
		ConsolePrint( "Cannot allocate memory!\n" );
		return;
	}

	pdl->p = phudelem;
	pdl->pNext = NULL;

	if (!m_pHudList)
	{
		m_pHudList = pdl;
		return;
	}

	// find last
	for( ptemp = m_pHudList; ptemp->pNext; ptemp = ptemp->pNext );

	ptemp->pNext = pdl;
}
