/*
*
*    This program is free software; you can redistribute it and/or modify it
*    under the terms of the GNU General Public License as published by the
*    Free Software Foundation; either version 2 of the License, or (at
*    your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*    In addition, as a special exception, the author gives permission to
*    link the code of this program with the Half-Life Game Engine ("HL
*    Engine") and Modified Game Libraries ("MODs") developed by Valve,
*    L.L.C ("Valve").  You must obey the GNU General Public License in all
*    respects for all of the code used other than the HL Engine and MODs
*    from Valve.  If you modify this file, you may extend this exception
*    to your version of the file, but you are not obligated to do so.  If
*    you do not wish to do so, delete this exception statement from your
*    version.
*
*/
#include "events.h"

#include "com_model.h"

#include "r_studioint.h"

#include <string.h>
#include <stdarg.h>

extern engine_studio_api_t IEngineStudio;

enum knife_e
{
	KNIFE_IDLE1 = 0,
	KNIFE_SLASH1,
	KNIFE_SLASH2,
	KNIFE_DRAW,
	KNIFE_STAB,
	KNIFE_STAB_MISS,
	KNIFE_MIDSLASH1,
	KNIFE_MIDSLASH2
};

static const char *SOUNDS_NAME = "weapons/knife_miss1.wav";

static const char *ZB3_PLAYER_MODEL = "zombie_classic";
static const char *ZB3_CLAW_EFFECT_MODEL = "models/csnc/claweffect.mdl";
static constexpr float ZB3_CLAW_EFFECT_LIFE = 0.7f;

static void ZB3_ClawDebugPrintf(const char *fmt, ...)
{
	static float s_lastPrintTime = 0.0f;
	const float now = gEngfuncs.GetClientTime();
	if (now - s_lastPrintTime < 0.25f)
		return;
	
	s_lastPrintTime = now;

	va_list argptr;
	char string[512];
	va_start(argptr, fmt);
	vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	gEngfuncs.Con_Printf("[ZB3] %s", string);
}

static bool ZB3_IsLocalZombie()
{
	cl_entity_t *local = gEngfuncs.GetLocalPlayer();
	if (!local)
		return false;

	if (local->model && local->model->name)
	{
		if (strstr(local->model->name, ZB3_PLAYER_MODEL))
			return true;
	}

	hud_player_info_t info{};
	GetPlayerInfo(local->index, &info);
	if (!info.model)
		return false;

	return !stricmp(info.model, ZB3_PLAYER_MODEL);
}

static void ZB3_SpawnClawEffect(struct event_args_s *args, const Vector &origin)
{
	int modelIdx = gEngfuncs.pEventAPI->EV_FindModelIndex(ZB3_CLAW_EFFECT_MODEL);
	if (!modelIdx)
	{
		ZB3_ClawDebugPrintf("claw: EV_FindModelIndex failed for %s\n", ZB3_CLAW_EFFECT_MODEL);
		return;
	}

	model_t *pModel = IEngineStudio.GetModelByIndex(modelIdx);
	if (!pModel)
	{
		ZB3_ClawDebugPrintf("claw: GetModelByIndex failed (idx=%d)\n", modelIdx);
		return;
	}

	float vecSrc[3];
	float originArr[3];
	originArr[0] = origin.x;
	originArr[1] = origin.y;
	originArr[2] = origin.z;
	EV_GetGunPosition(args, vecSrc, originArr);

	float angles[3];
	VectorCopy(v_angles, angles);

	float forward[3], right[3], up[3];
	AngleVectors(angles, forward, right, up);

	float vecEnd[3];
	VectorMA(vecSrc, 64.0f, forward, vecEnd);

	pmtrace_t tr;
	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
	gEngfuncs.pEventAPI->EV_PushPMStates();
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(args->entindex - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_STUDIO_BOX, -1, &tr);
	gEngfuncs.pEventAPI->EV_PopPMStates();

	Vector vPos(tr.endpos);
	Vector vForward(forward);
	vPos = vPos + vForward * -20.0f;

	TEMPENTITY *te = gEngfuncs.pEfxAPI->CL_TempEntAlloc(vPos, (struct model_s *)pModel);
	if (te)
	{
		te->flags |= (FTENT_CLIENTCUSTOM | FTENT_PERSIST);
		te->die = gEngfuncs.GetClientTime() + ZB3_CLAW_EFFECT_LIFE;
		te->entity.curstate.modelindex = modelIdx;
		te->entity.curstate.angles[0] = angles[0];
		te->entity.curstate.angles[1] = angles[1];
		te->entity.curstate.angles[2] = angles[2];
		te->entity.curstate.scale = 1.0f;
		te->entity.curstate.rendermode = kRenderTransAdd;
		te->entity.curstate.renderfx = kRenderFxNone;
		te->entity.curstate.renderamt = 255;
		ZB3_ClawDebugPrintf("claw: spawned te ok (modelIdx=%d model=%s)\n", modelIdx, (pModel->name ? pModel->name : "<null>"));
	}
	else
	{
		ZB3_ClawDebugPrintf("claw: CL_TempEntAlloc returned null (modelIdx=%d)\n", modelIdx);
	}
}

void EV_Knife( struct event_args_s *args )
{
	int    idx = args->entindex;
	Vector origin( args->origin );

	if( EV_IsLocal( idx ))
	{
		cl_entity_t *local = gEngfuncs.GetLocalPlayer();
		hud_player_info_t info{};
		GetPlayerInfo(idx, &info);
		ZB3_ClawDebugPrintf("knife: local event iparam1=%d localModel=%s userinfoModel=%s\n",
			args->iparam1,
			(local && local->model && local->model->name) ? local->model->name : "<null>",
			info.model ? info.model : "<null>");

		gEngfuncs.pEventAPI->EV_WeaponAnimation( args->iparam1, 2 );
	}

	if (EV_IsLocal(idx) && ZB3_IsLocalZombie())
	{
		ZB3_ClawDebugPrintf("knife: local zombie swing (iparam1=%d)\n", args->iparam1);
		ZB3_SpawnClawEffect(args, origin);
	}

	//Play Swing sound
	PLAY_EVENT_SOUND( SOUNDS_NAME );
}
