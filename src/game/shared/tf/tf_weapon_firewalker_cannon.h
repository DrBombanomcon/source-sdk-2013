//========= Copyright Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================
#ifndef TF_WEAPON_FIREWALKER_CANNON_H
#define TF_WEAPON_FIREWALKER_CANNON_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_dragons_fury.h"

#ifdef CLIENT_DLL
#define CTFFirewalkerCannon C_TFFirewalkerCannon
#endif

#define RT_FIREWALKER_CHARGE_MAX 3.5f

class CTFFirewalkerCannon : public CTFWeaponFlameBall, public ITFChargeUpWeapon
{
public:
	DECLARE_CLASS(CTFFirewalkerCannon, CTFWeaponFlameBall);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CTFFirewalkerCannon() {
		m_bWantsToShoot = false;
	}
	~CTFFirewalkerCannon() {}

	virtual int		GetWeaponID(void) const { return TF_WEAPON_FIREWALKER_CANNON; }

	virtual CBaseEntity* FireProjectile(CTFPlayer* pPlayer);

	virtual void	PrimaryAttack(void) OVERRIDE;
	virtual void	SecondaryAttack(void) OVERRIDE;
	virtual void	ItemPostFrame(void);
	virtual void	WeaponIdle(void);

	virtual void	ModifyProjectile(CBaseEntity* pProj);
	virtual void	WeaponReset(void);

	virtual bool	Holster(CBaseCombatWeapon* pSwitchingTo);
	virtual bool	Deploy(void);

public:
	virtual bool CanCharge(void) { return true; }
	virtual float GetChargeBeginTime(void) { return m_flChargeBeginTime; }
	virtual float GetChargeMaxTime(void) { float flChargeTime = RT_FIREWALKER_CHARGE_MAX;	return flChargeTime; }
	virtual float GetChargeForceReleaseTime(void) { return GetChargeMaxTime(); }
	virtual void	LaunchGrenade(void);
	virtual void ForceLaunchGrenade(void) { LaunchGrenade(); }

private:

	CNetworkVar(float, m_flChargeBeginTime);
	CTFFirewalkerCannon(const CTFFirewalkerCannon&) {}
	virtual void SetInternalChargeBeginTime(float flChargeBeginTime) { m_flChargeBeginTime = flChargeBeginTime; }
	float	GetInternalChargeBeginTime() const { return m_flChargeBeginTime; }
	bool	m_bWantsToShoot;

#ifdef GAME_DLL
	CDmgAccumulator m_Accumulator;
#endif // GAME_DLL
};


#endif