//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_swarmer_melee.h"
#include "decals.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_basedoor.h"
#include "c_tf_player.h"
#include "IEffects.h"
#include "bone_setup.h"
#include "c_tf_gamestats.h"
// Server specific.
#else
#include "doors.h"
#include "tf_player.h"
#include "tf_ammo_pack.h"
#include "tf_gamestats.h"
#include "ilagcompensationmanager.h"
#include "collisionutils.h"
#include "particle_parse.h"
#include "halloween/zombie/zombie.h"
#include "tf_projectile_base.h"
#include "tf_gamerules.h"
#endif

//TODO: Convert these into weapon script keys
//Convar mod_tf_swarmer_spawn_count("mod_swarmer_spawn_count", "2", FCVAR_CHEAT);
//Convar mod_tf_swarmer_spawn_max("mod_swarmer_spawn_max", "4", FCVAR_CHEAT);

//======================================================
//
// Weapon Data table
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFSwarmer_Melee, DT_TFWeaponSwarmer_Melee )

BEGIN_NETWORK_TABLE(CTFSwarmer_Melee, DT_TFWeaponSwarmer_Melee)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CTFSwarmer_Melee)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(tf_weapon_swarmer_melee, CTFSwarmer_Melee);
PRECACHE_WEAPON_REGISTER(tf_weapon_swarmer_melee);

#ifndef CLIENT_DLL
BEGIN_DATADESC(CTFSwarmer_Melee)
END_DATADESC()
#endif

CTFSwarmer_Melee::CTFSwarmer_Melee()
{
}

void CTFSwarmer_Melee::Precache(void)
{
	#ifdef GAME_DLL
		CZombie::PrecacheZombie();
	#endif
	BaseClass::Precache();
}

void CTFSwarmer_Melee::Smack(void)
{
	BaseClass::Smack();
}

void CTFSwarmer_Melee::SecondaryAttack(void)
{
	CTFPlayer* pOwner = GetTFPlayerOwner();
	if (!pOwner)
		return;

	if (!CanAttack())
		return;

	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return;

#ifdef GAME_DLL
	if (pOwner->GetSwarmAmount() > 4)
		return;
#endif

	int iSpawnActive = pOwner->GetAmmoCount(TF_AMMO_GRENADES1);

	if (CanSummon(pOwner) && (iSpawnActive > 0))
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.25;

#ifdef GAME_DLL
		if (pOwner->m_Shared.IsStealthed())
		{
			pOwner->RemoveInvisibility();
		}
#endif // GAME_DLL
		SpawnSwarm(pOwner);
		pOwner->m_Shared.OnAttack();
	}

}

bool CTFSwarmer_Melee::CanSummon(CTFPlayer* pPlayer)
{
	if (pPlayer->GetWaterLevel() == WL_Eyes)
		return false;

	Vector vecForward, vecUp, vecRight;
	AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);
	Vector vecSkeleLeft = pPlayer->GetAbsOrigin() + (vecRight * -98) + Vector(0, 0, 50);
	Vector vecSkeleRight = pPlayer->GetAbsOrigin() + (vecRight * 98) + Vector(0, 0, 50);
	
	//Trace out until we hit a wall
	trace_t traceLeft, traceRight;
	CTraceFilterSimple traceFilter(this, COLLISION_GROUP_NONE);
	UTIL_TraceHull((pPlayer->GetAbsOrigin() + Vector(0, 0, 50)), vecSkeleLeft, -Vector(16, 16, 16), Vector(16, 16, 16), MASK_SOLID_BRUSHONLY, &traceFilter, &traceLeft);
	UTIL_TraceHull((pPlayer->GetAbsOrigin() + Vector(0, 0, 50)), vecSkeleLeft, -Vector(16, 16, 16), Vector(16, 16, 16), MASK_SOLID_BRUSHONLY, &traceFilter, &traceRight);

	if (traceLeft.DidHitWorld() || traceRight.DidHitWorld() || traceLeft.startsolid || traceRight.startsolid)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CTFSwarmer_Melee::SpawnSwarm(CTFPlayer* pPlayer)
{
#ifdef GAME_DLL
	int skeletonType = 0;
	CALL_ATTRIB_HOOK_INT(skeletonType, spawnType);
	Vector vecForward, vecUp, vecRight;
	AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);
	Vector vecSkeleLeft = pPlayer->GetAbsOrigin() + (vecRight * -98);
	Vector vecSkeleRight = pPlayer->GetAbsOrigin() + (vecRight * 98);
	pPlayer->AddSwarm(CSwarmie::SpawnAtPos(vecSkeleLeft, 90.0f, GetTeamNumber(), pPlayer, (CZombie::SkeletonType_t)skeletonType));
	pPlayer->AddSwarm(CSwarmie::SpawnAtPos(vecSkeleRight, 90.0f, GetTeamNumber(), pPlayer, (CZombie::SkeletonType_t)skeletonType));
	pPlayer->RemoveAmmo(1, TF_AMMO_GRENADES1);

#endif

	StartEffectBarRegen();
	return true;
}