#include "precompiled.h"

static bool ZB3_IsBlockedEntityClass(const char *classname)
{
	if (!classname || !*classname)
		return false;

	return !Q_strcmp(classname, "func_bomb_target") ||
	       !Q_strcmp(classname, "info_bomb_target") ||
	       !Q_strcmp(classname, "func_hostage_rescue") ||
	       !Q_strcmp(classname, "hostage_entity") ||
	       !Q_strcmp(classname, "func_escapezone") ||
	       !Q_strcmp(classname, "func_vip_safetyzone") ||
	       !Q_strcmp(classname, "weapon_c4") ||
	       !Q_strcmp(classname, "armoury_entity");
}

class CHalfLifeZombieMod3 : public CHalfLifeMultiplay
{
public:
	CHalfLifeZombieMod3() {}
	virtual ~CHalfLifeZombieMod3() {}

	void InfectPlayer(CBasePlayer *pVictim) override
	{
		if (!UTIL_IsValidPlayer(pVictim))
			return;
		if (!pVictim->IsAlive())
			return;
		if (pVictim->m_bIsZombie)
			return;

		EMIT_SOUND(ENT(pVictim->pev), CHAN_VOICE,
			(RANDOM_LONG(1, 2) == 1) ? "csnc/human_death_01.wav" : "csnc/human_death_02.wav",
			VOL_NORM, ATTN_NORM);

		pVictim->m_bIsZombie = true;
		pVictim->m_iZombieLevel = 0;
		if (pVictim->m_iTeam != TERRORIST)
			ChangePlayerTeam(pVictim, "TERRORIST", FALSE, FALSE);
		ForceZombieLoadoutAndHealth(pVictim);
		pVictim->SetClientUserInfoModel(GET_INFO_BUFFER(pVictim->edict()), (char *)ZB3_ZOMBIE_PLAYER_MODEL);
		pVictim->SetNewPlayerModel(ZB3_ZOMBIE_PLAYER_MODEL_PATH);
	}

private:
	static void ForceZombieLoadoutAndHealth(CBasePlayer *pPlayer)
	{
		if (!UTIL_IsValidPlayer(pPlayer))
			return;

		pPlayer->RemoveAllItems(FALSE);
		pPlayer->GiveNamedItem("weapon_knife");

		pPlayer->pev->health = ZB3_ZOMBIE_HEALTH;
		pPlayer->pev->max_health = ZB3_ZOMBIE_HEALTH;
	}

	static void ForceHumanLoadoutAndHealth(CBasePlayer *pPlayer)
	{
		if (!UTIL_IsValidPlayer(pPlayer))
			return;

		pPlayer->pev->health = ZB3_HUMAN_HEALTH;
		pPlayer->pev->max_health = ZB3_HUMAN_HEALTH;
	}

	static void ForceTeamImmediate(CBasePlayer *pPlayer, TeamName team)
	{
		if (!UTIL_IsValidPlayer(pPlayer))
			return;

		pPlayer->m_iTeam = team;
		pPlayer->TeamChangeUpdate();
	}

	static constexpr float ZB3_COUNTDOWN_TIME = 22.0f;
	static constexpr float ZB3_COUNTDOWN_TEXT_INTERVAL = 1.0f;
	static constexpr float ZB3_COUNTDOWN_TEXT_LASTSECONDS = 20.0f;
	static constexpr float ZB3_ZOMBIE_HEALTH = 3000.0f;
	static constexpr float ZB3_HUMAN_HEALTH = 100.0f;
	static constexpr const char *ZB3_ZOMBIE_PLAYER_MODEL = "zombie_classic";
	static constexpr const char *ZB3_ZOMBIE_PLAYER_MODEL_PATH = "models/player/zombie_classic/zombie_classic.mdl";
	static constexpr const char *ZB3_SKYBOX_NAME = "black";
	static constexpr float ZB3_FOG_DENSITY = 0.0025f;
	static constexpr float ZB3_ROUND_END_DELAY = 5.0f;

	static void ZB3_ApplyAtmosphereCvars()
	{
		CVAR_SET_STRING("sv_skyname", ZB3_SKYBOX_NAME);
		CVAR_SET_FLOAT("sv_skycolor_r", 0.0f);
		CVAR_SET_FLOAT("sv_skycolor_g", 0.0f);
		CVAR_SET_FLOAT("sv_skycolor_b", 0.0f);
	}

	static void ZB3_SendBlackFog(CBasePlayer *pPlayer)
	{
		if (!UTIL_IsValidPlayer(pPlayer))
			return;
		if (!gmsgFog)
			return;

		union
		{
			float f;
			char b[4];
		} density;

		density.f = ZB3_FOG_DENSITY;
		MESSAGE_BEGIN(MSG_ONE, gmsgFog, nullptr, pPlayer->pev);
			WRITE_BYTE(0);
			WRITE_BYTE(0);
			WRITE_BYTE(0);
			WRITE_BYTE(density.b[0]);
			WRITE_BYTE(density.b[1]);
			WRITE_BYTE(density.b[2]);
			WRITE_BYTE(density.b[3]);
		MESSAGE_END();
	}

	static void ZB3_SendCountdownText(CBasePlayer *pPlayer, int seconds)
	{
		if (!UTIL_IsValidPlayer(pPlayer))
			return;

		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, UTIL_VarArgs("Time Remaining for Zombie Selection: %d Secc", seconds));
	}

	void AnnounceZombieWin()
	{
		if (m_bAnnouncedRoundEnd)
			return;
		m_bAnnouncedRoundEnd = true;

		UTIL_ClientPrintAll(HUD_PRINTCENTER, "Zombie Win");
		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer *p = UTIL_PlayerByIndex(i);
			if (!UTIL_IsValidPlayer(p))
				continue;
			CLIENT_COMMAND(p->edict(), "spk \"csnc/zombie_win.wav\"\n");
		}
	}

	void AnnounceHumanWin()
	{
		if (m_bAnnouncedRoundEnd)
			return;
		m_bAnnouncedRoundEnd = true;

		UTIL_ClientPrintAll(HUD_PRINTCENTER, "Human Win");
		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer *p = UTIL_PlayerByIndex(i);
			if (!UTIL_IsValidPlayer(p))
				continue;
			CLIENT_COMMAND(p->edict(), "spk \"csnc/human_win.wav\"\n");
		}
	}

	BOOL IsAllowedToSpawn(CBaseEntity *pEntity) override
	{
		if (!pEntity)
			return FALSE;
		if (ZB3_IsBlockedEntityClass(STRING(pEntity->pev->classname)))
			return FALSE;
		return CHalfLifeMultiplay::IsAllowedToSpawn(pEntity);
	}

	void CheckMapConditions() override
	{
		CHalfLifeMultiplay::CheckMapConditions();

		m_bMapHasBombTarget = false;
		m_bMapHasBombZone = false;
		m_bMapHasBuyZone = false;
		m_bMapHasRescueZone = false;
		m_bMapHasEscapeZone = false;
		m_bMapHasVIPSafetyZone = FALSE;
	}

	CBasePlayer *GiveC4() override
	{
		return nullptr;
	}

	void RestartRound() override
	{
		CHalfLifeMultiplay::RestartRound();
		ZB3_ApplyAtmosphereCvars();
		m_bFreezePeriod = FALSE;
		m_iIntroRoundTime = 0;
		m_iRoundTimeSecs = m_iRoundTime;
		m_fRoundStartTime = m_fRoundStartTimeReal = gpGlobals->time;

		m_bRoundActive = false;
		m_bAnnouncedRoundEnd = false;
		m_iLastCountdownSeconds = -1;
		m_flNextCountdownTextTime = 0.0f;
		m_bPlayedCountdownMusic = false;

		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer *p = UTIL_PlayerByIndex(i);
			if (!UTIL_IsValidPlayer(p))
				continue;
			ZB3_SendBlackFog(p);
			p->pev->flags |= FL_NOTARGET;
			p->m_iHideHUD |= HIDEHUD_TIMER;

			const bool wasZombie = p->m_bIsZombie;
			if (m_bHaveSavedTeams)
			{
				TeamName restoreTeam = static_cast<TeamName>(m_iSavedTeam[i]);
				if (restoreTeam != UNASSIGNED && restoreTeam != p->m_iTeam)
					ForceTeamImmediate(p, restoreTeam);
			}

			m_iSavedTeam[i] = p->m_iTeam;
			p->m_bIsZombie = false;
			p->m_iZombieLevel = 0;
			p->pev->max_health = ZB3_HUMAN_HEALTH;
			if (p->IsAlive())
				p->pev->health = ZB3_HUMAN_HEALTH;
			p->SetPlayerModel(p->m_bHasC4);

			if (wasZombie && p->IsAlive())
			{
				p->RemoveAllItems(FALSE);
				p->GiveNamedItem("weapon_knife");
				p->GiveNamedItem(p->m_iTeam == CT ? "weapon_usp" : "weapon_glock18");
				p->SelectItem("weapon_knife");
			}
		}
		m_bHaveSavedTeams = true;

		m_flPickZombieTime = gpGlobals->time + ZB3_COUNTDOWN_TIME;
	}

	void InitHUD(CBasePlayer *pPlayer) override
	{
		CHalfLifeMultiplay::InitHUD(pPlayer);
		ZB3_ApplyAtmosphereCvars();
		ZB3_SendBlackFog(pPlayer);
		if (!m_bRoundActive)
			pPlayer->m_iHideHUD |= HIDEHUD_TIMER;
	}

	void Think() override
	{
		CHalfLifeMultiplay::Think();

		if (m_bRoundActive && CHalfLifeMultiplay::HasRoundTimeExpired())
		{
			m_bRoundActive = false;
			AnnounceHumanWin();
			TerminateRound(ZB3_ROUND_END_DELAY, WINSTATUS_CTS);
			return;
		}

		if (!m_bRoundActive && m_flPickZombieTime > 0.0f)
		{
			m_bFreezePeriod = FALSE;
			m_fRoundStartTime = m_fRoundStartTimeReal = gpGlobals->time;
			m_iRoundTimeSecs = m_iRoundTime;

			for (int i = 1; i <= gpGlobals->maxClients; ++i)
			{
				CBasePlayer *p = UTIL_PlayerByIndex(i);
				if (!UTIL_IsValidPlayer(p))
					continue;
				p->pev->flags |= FL_NOTARGET;
				p->m_iHideHUD |= HIDEHUD_TIMER;
			}

			const float remaining = m_flPickZombieTime - gpGlobals->time;
			if (remaining > 0.0f)
			{
				if (!m_bPlayedCountdownMusic)
				{
					m_bPlayedCountdownMusic = true;
					for (int i = 1; i <= gpGlobals->maxClients; ++i)
					{
						CBasePlayer *p = UTIL_PlayerByIndex(i);
						if (!UTIL_IsValidPlayer(p))
							continue;

						CLIENT_COMMAND(p->edict(), "mp3 play sound/csnc/zombie_startup.mp3\n");
					}
				}

				if (m_flNextCountdownTextTime <= 0.0f || gpGlobals->time >= m_flNextCountdownTextTime)
				{
					m_flNextCountdownTextTime = gpGlobals->time + ZB3_COUNTDOWN_TEXT_INTERVAL;

					int seconds = static_cast<int>(ceilf(remaining));
					if (seconds < 0)
						seconds = 0;

					if (seconds != m_iLastCountdownSeconds && remaining <= ZB3_COUNTDOWN_TEXT_LASTSECONDS)
					{
						m_iLastCountdownSeconds = seconds;

						const char *voice = nullptr;
						switch (seconds)
						{
						case 20: voice = "spk \"csnc/20secremain.wav\"\n"; break;
						case 10: voice = "spk \"csnc/ten.wav\"\n"; break;
						case 9: voice = "spk \"csnc/nine.wav\"\n"; break;
						case 8: voice = "spk \"csnc/eight.wav\"\n"; break;
						case 7: voice = "spk \"csnc/seven.wav\"\n"; break;
						case 6: voice = "spk \"csnc/six.wav\"\n"; break;
						case 5: voice = "spk \"csnc/five.wav\"\n"; break;
						case 4: voice = "spk \"csnc/four.wav\"\n"; break;
						case 3: voice = "spk \"csnc/three.wav\"\n"; break;
						case 2: voice = "spk \"csnc/two.wav\"\n"; break;
						case 1: voice = "spk \"csnc/one.wav\"\n"; break;
						default: break;
						}

						for (int i = 1; i <= gpGlobals->maxClients; ++i)
						{
							CBasePlayer *p = UTIL_PlayerByIndex(i);
							if (!UTIL_IsValidPlayer(p))
								continue;

							if (voice)
								CLIENT_COMMAND(p->edict(), voice);

							ZB3_SendCountdownText(p, seconds);
						}
					}
				}
			}
		}

		if (m_flPickZombieTime > 0.0f && gpGlobals->time >= m_flPickZombieTime)
		{
			m_flPickZombieTime = 0.0f;
			PickInitialZombie();
		}
	}

	void PlayerSpawn(CBasePlayer *pPlayer) override
	{
		CHalfLifeMultiplay::PlayerSpawn(pPlayer);
		if (!pPlayer)
			return;

		if (pPlayer->m_bIsZombie)
		{
			ForceZombieLoadoutAndHealth(pPlayer);
			pPlayer->SetClientUserInfoModel(GET_INFO_BUFFER(pPlayer->edict()), (char *)ZB3_ZOMBIE_PLAYER_MODEL);
			pPlayer->SetNewPlayerModel(ZB3_ZOMBIE_PLAYER_MODEL_PATH);
		}
		else
		{
			pPlayer->pev->health = ZB3_HUMAN_HEALTH;
			pPlayer->pev->max_health = ZB3_HUMAN_HEALTH;
			pPlayer->SetPlayerModel(pPlayer->m_bHasC4);
		}
	}

	void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) override
	{
		CHalfLifeMultiplay::PlayerKilled(pVictim, pKiller, pInflictor);
	}

	BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) override
	{
		if (!pPlayer)
			return FALSE;

		if (!m_bRoundActive)
			return FALSE;

		if (!pAttacker || !pAttacker->IsPlayer())
			return CHalfLifeMultiplay::FPlayerCanTakeDamage(pPlayer, pAttacker);

		CBasePlayer *pAttackPlayer = static_cast<CBasePlayer *>(pAttacker);
		const bool attackerZombie = pAttackPlayer->m_bIsZombie;
		const bool victimZombie = pPlayer->m_bIsZombie;

		if (!victimZombie && attackerZombie)
		{
			CBasePlayerItem *pActive = pAttackPlayer->m_pActiveItem;
			if (pActive && pActive->m_iId == WEAPON_KNIFE)
				return TRUE;
			return FALSE;
		}

		if (attackerZombie == victimZombie)
			return FALSE;

		return TRUE;
	}

	BOOL CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pItem) override
	{
		if (!pPlayer || !pItem)
			return FALSE;

		if (pPlayer->m_bIsZombie)
		{
			const char *classname = STRING(pItem->pev->classname);
			if (!classname || Q_stricmp(classname, "weapon_knife"))
				return FALSE;
		}

		return CHalfLifeMultiplay::CanHavePlayerItem(pPlayer, pItem);
	}

	int PlayerRelationship(CBasePlayer *pPlayer, CBaseEntity *pTarget) override
	{
		if (!m_bRoundActive)
			return GR_TEAMMATE;

		return CHalfLifeMultiplay::PlayerRelationship(pPlayer, pTarget);
	}

	void CheckWinConditions() override
	{
		int aliveHumans = 0;
		int aliveZombies = 0;
		int alivePlayers = 0;

		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer *p = UTIL_PlayerByIndex(i);
			if (!UTIL_IsValidPlayer(p))
				continue;
			if (!p->IsAlive())
				continue;

			++alivePlayers;

			if (p->m_bIsZombie)
				++aliveZombies;
			else
				++aliveHumans;
		}

		if (alivePlayers <= 1)
			return;

		if (aliveHumans == 0 && aliveZombies > 0)
		{
			m_bRoundActive = false;
			AnnounceZombieWin();
			TerminateRound(ZB3_ROUND_END_DELAY, WINSTATUS_TERRORISTS);
			return;
		}
		if (aliveZombies == 0 && aliveHumans > 0)
		{
			m_bRoundActive = false;
			AnnounceHumanWin();
			TerminateRound(ZB3_ROUND_END_DELAY, WINSTATUS_CTS);
			return;
		}
	}

	bool HasRoundTimeExpired() override
	{
		return false;
	}

private:
	void PickInitialZombie()
	{
		CBasePlayer *candidates[MAX_CLIENTS];
		int count = 0;

		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer *p = UTIL_PlayerByIndex(i);
			if (!UTIL_IsValidPlayer(p))
				continue;
			if (!p->IsAlive())
				continue;
			candidates[count++] = p;
		}

		if (count <= 0)
			return;

		CBasePlayer *pick = candidates[RANDOM_LONG(0, count - 1)];

		EMIT_SOUND(ENT(pick->pev), CHAN_VOICE,
			(RANDOM_LONG(1, 2) == 1) ? "csnc/human_death_01.wav" : "csnc/human_death_02.wav",
			VOL_NORM, ATTN_NORM);

		pick->m_bIsZombie = true;
		pick->m_iZombieLevel = 0;
		if (pick->m_iTeam != TERRORIST)
			ChangePlayerTeam(pick, "TERRORIST", FALSE, FALSE);
		ForceZombieLoadoutAndHealth(pick);
		pick->SetClientUserInfoModel(GET_INFO_BUFFER(pick->edict()), (char *)ZB3_ZOMBIE_PLAYER_MODEL);
		pick->SetNewPlayerModel(ZB3_ZOMBIE_PLAYER_MODEL_PATH);

		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer *p = UTIL_PlayerByIndex(i);
			if (!UTIL_IsValidPlayer(p))
				continue;

			if (p != pick)
			{
				p->m_bIsZombie = false;
				p->m_iZombieLevel = 0;
				if (p->m_iTeam != CT)
					ForceTeamImmediate(p, CT);
				p->pev->health = ZB3_HUMAN_HEALTH;
				p->pev->max_health = ZB3_HUMAN_HEALTH;
			}

			p->pev->flags &= ~FL_NOTARGET;
			p->m_iHideHUD &= ~HIDEHUD_TIMER;
			p->SyncRoundTimer();
		}

		m_bFreezePeriod = FALSE;
		m_fRoundStartTimeReal = m_fRoundStartTime = gpGlobals->time;
		m_iRoundTimeSecs = m_iRoundTime;

		m_bRoundActive = true;
	}

private:
	float m_flPickZombieTime = 0.0f;
	bool m_bRoundActive = false;
	bool m_bAnnouncedRoundEnd = false;
	int m_iLastCountdownSeconds = -1;
	float m_flNextCountdownTextTime = 0.0f;
	bool m_bPlayedCountdownMusic = false;
	int m_iSavedTeam[MAX_CLIENTS + 1] = { 0 };
	bool m_bHaveSavedTeams = false;
};

CGameRules *CreateZombieMod3Rules()
{
	return new CHalfLifeZombieMod3;
}
