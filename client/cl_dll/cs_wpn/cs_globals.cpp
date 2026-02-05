 #include "extdll.h"
#include "com_weapons.h"

enum
{
	WEAPON_NONE = 0,
	WEAPON_P228 = 1,
	WEAPON_SCOUT = 3,
	WEAPON_HEGRENADE = 4,
	WEAPON_XM1014 = 5,
	WEAPON_C4 = 6,
	WEAPON_MAC10 = 7,
	WEAPON_AUG = 8,
	WEAPON_SMOKEGRENADE = 9,
	WEAPON_ELITE = 10,
	WEAPON_FIVESEVEN = 11,
	WEAPON_UMP45 = 12,
	WEAPON_SG550 = 13,
	WEAPON_GALIL = 14,
	WEAPON_FAMAS = 15,
	WEAPON_USP = 16,
	WEAPON_GLOCK18 = 17,
	WEAPON_AWP = 18,
	WEAPON_MP5N = 19,
	WEAPON_M249 = 20,
	WEAPON_M3 = 21,
	WEAPON_M4A1 = 22,
	WEAPON_TMP = 23,
	WEAPON_G3SG1 = 24,
	WEAPON_FLASHBANG = 25,
	WEAPON_DEAGLE = 26,
	WEAPON_SG552 = 27,
	WEAPON_AK47 = 28,
	WEAPON_KNIFE = 29,
	WEAPON_P90 = 30,
};

int g_iWeaponFlags = 0;
bool g_bInBombZone = false;
int g_iFreezeTimeOver = 0;
bool g_bHoldingShield = false;
bool g_bHoldingKnife = false;
float g_flPlayerSpeed = 0.0f;
int g_iPlayerFlags = 0;
vec3_t g_vPlayerVelocity = { 0.0f, 0.0f, 0.0f };

int GetWeaponAccuracyFlags( int weaponid )
{
	int result = 0;

	if( weaponid <= WEAPON_P90 )
	{
		switch( weaponid )
		{
		case WEAPON_AUG:
		case WEAPON_GALIL:
		case WEAPON_M249:
		case WEAPON_SG552:
		case WEAPON_AK47:
		case WEAPON_P90:
			result = ACCURACY_AIR | ACCURACY_SPEED;
			break;
		case WEAPON_P228:
		case WEAPON_FIVESEVEN:
		case WEAPON_DEAGLE:
			result = ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK;
			break;
		case WEAPON_GLOCK18:
			if( g_iWeaponFlags & WPNSTATE_GLOCK18_BURST_MODE )
			{
				result = ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK;
			}
			else
			{
				result = ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK | ACCURACY_MULTIPLY_BY_14_2;
			}
			break;
		case WEAPON_MAC10:
		case WEAPON_UMP45:
		case WEAPON_MP5N:
		case WEAPON_TMP:
			result = ACCURACY_AIR;
			break;
		case WEAPON_M4A1:
			if( g_iWeaponFlags & WPNSTATE_USP_SILENCED )
			{
				result = ACCURACY_AIR | ACCURACY_SPEED;
			}
			else
			{
				result = ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_MULTIPLY_BY_14;
			}
			break;
		case WEAPON_FAMAS:
			if( g_iWeaponFlags & WPNSTATE_FAMAS_BURST_MODE )
			{
				result = ACCURACY_AIR | ACCURACY_SPEED;
			}
			else
			{
				result = ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_MULTIPLY_BY_14;
			}
			break;
		case WEAPON_USP:
			if( g_iWeaponFlags & WPNSTATE_USP_SILENCED )
			{
				result = ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK;
			}
			else
			{
				result = ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK | ACCURACY_MULTIPLY_BY_14_2;
			}
			break;
		}
	}

	return result;
}
