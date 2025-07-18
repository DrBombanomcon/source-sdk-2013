//========= Copyright Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================
#ifndef TF_WEAPON_CHAINGUN_H
#define TF_WEAPON_CHAINGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weapon_rocketlauncher.h"

#ifdef CLIENT_DLL
#define CTFChainGun C_TFChainGun
#endif

#define TF_CHAINGUN_MAX_FIRING 7.0f
#define TF_CHAINGUN_OVERHEAT_TIME 4.0f

#define TF_CHAINGUN_MIN_RAMP 1.5f
#define TF_CHAINGUN_MAX_RAMP 1.0f
#define TF_CHAINGUN_RAMP_WITHDRAW 2.0f

class CTFChainGun : public CTFRocketLauncher
{
public:

	DECLARE_CLASS(CTFChainGun, CTFRocketLauncher);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	// Server specific.
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CTFChainGun();
	~CTFChainGun();

	virtual int		GetWeaponID(void) const { return TF_WEAPON_CHAINGUN; }
	virtual float	GetProjectileSpeed(void);
	virtual float	GetProjectileGravity(void);

	virtual void	WeaponRegenerate(void);
	virtual float	GetRampUp(void) { return m_flRampUp; }
	virtual bool	IsOverheated(void) { return m_bOverheatState; }
	virtual float	GetOverheatEndTime(void) { float flOverheatTime = TF_CHAINGUN_OVERHEAT_TIME; return flOverheatTime; }

	virtual void	PrimaryAttack(void);
	virtual void	WeaponIdle();
	virtual void	ItemPostFrame(void);


private:

	CTFChainGun( const CTFChainGun& ) {}

	#ifdef GAME_DLL
		CDmgAccumulator m_Accumulator;
	#endif // GAME_DLL

	CNetworkVar(bool, m_bCritShot);
	CNetworkVar(float, m_flRampUp);
	CNetworkVar(float, m_flFiringStartTime);
	CNetworkVar(bool, m_bOverheatState);
	CNetworkVar(float, m_flOverheatStartTime);
	CNetworkVar(float, m_flOverheatPercent);

	float m_flOldOverheatPercent;
	float m_flStopFiringTime;
};

#endif