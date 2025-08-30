//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Valve didn't included a header file for this, so I need to make one so I can reference the entity
//
//=============================================================================
#ifndef TF_PROJECTILE_DRAGONS_FURY_H
#define TF_PROJECTILE_DRAGONS_FURY_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "tf_weapon_dragons_fury.h"

#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include "dlight.h"
#include "iefx.h"
#include "tempent.h"
#include "debugoverlay_shared.h"
#else
#include "tf_player.h"
#include "tf_fx.h"
#include "tf_projectile_rocket.h"
#include "tf_logic_robot_destruction.h"
#include "tf_weapon_compound_bow.h"
#include "tf_pumpkin_bomb.h"
#include "halloween/merasmus/merasmus_trick_or_treat_prop.h"
#include "tf_robot_destruction_robot.h"
#include "tf_generic_bomb.h"
#endif


#ifdef CLIENT_DLL
#define CTFProjectile_BallOfFire				C_TFProjectile_BallOfFire
#define CTFProjectile_FireWalkerFire			C_TFProjectile_FireWalkerFire
#endif

class CTFProjectile_BallOfFire : public CTFProjectile_Rocket
{
public:
	DECLARE_CLASS(CTFProjectile_BallOfFire, CTFProjectile_Rocket);
	DECLARE_NETWORKCLASS();

	CTFProjectile_BallOfFire() {}

	~CTFProjectile_BallOfFire();

	virtual void Precache() OVERRIDE
	{
		PrecacheModel("models/empty.mdl");
		PrecacheScriptSound("Weapon_DragonsFury.Nearmiss");
	}

	virtual void Spawn() OVERRIDE;

#ifdef GAME_DLL
	void SetDurationThink(float duration);
	void SetMaxDistance(float modifier);
	void SetBurnDuration(float modifer);
	void ExpireDelayThink();
	void DistanceLimitThink();
	void StopAndFizzle();
	void SetBonusDamage(bool toggle) { m_bBonusDamage = toggle; }
	virtual const char* GetProjectileModelName(void) { return "models/empty.mdl"; } // We dont have a model by default, and that's OK

	virtual float		GetDamageRadius()	const;
	virtual int			GetCustomDamageType() const OVERRIDE { Assert(false); return TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE; }

	virtual void RocketTouch(CBaseEntity* pOther) OVERRIDE;
	void RefundAmmo();

	virtual const char* GetExplodeEffectSound()	const { return "Halloween.spell_fireball_impact"; }
	void OnCollideWithTeammate(CTFPlayer* pTFPlayer);
	void Burn(CBaseEntity* pTarget);
	bool IsEntityVisible(CBaseEntity* pEntity);

	CUtlVector< int > m_vecHitPlayers;
#endif

#ifdef CLIENT_DLL

	// Do nothing
	virtual void CreateTrails(void) OVERRIDE
	{
	}

	const char* GetParticle(bool bCrit) const { return bCrit ? (GetTeamNumber() == TF_TEAM_BLUE ? "projectile_fireball_crit_blue" : "projectile_fireball_crit_red") : "projectile_fireball"; }

	virtual void OnDataChanged(DataUpdateType_t updateType) OVERRIDE;
	virtual void ClientThink() OVERRIDE;
#endif
protected:
	virtual float GetFireballScale() const { return 1.f; }

private:
#ifdef CLIENT_DLL
	dlight_t* m_pDynamicLight = NULL;
	bool				m_bNearMiss = false;
	float				m_flLastNearMissCheck = 0.f;
	float				m_flTempProjCreationTime = 0.f;
	bool				m_bTempProjCreated = false;
#endif // CLIENT_DLL

	CNetworkVector(m_vecSpawnOrigin);
	CNetworkVector(m_vecInitialVelocity);

	float m_flMaxDistance;
	float m_flBurnDuration;
	bool m_bBonusDamage = true;

#ifdef GAME_DLL
	bool m_bRefunded = false;
	bool m_bFizzling = false;
	bool m_bImpactSoundPlayed = false;
	bool m_bBonusSoundPlayed = false;
#endif
};

#endif