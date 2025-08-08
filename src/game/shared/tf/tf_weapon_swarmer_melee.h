//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_SWARMER_MELEE_H
#define TF_WEAPON_SWARMER_MELEE_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"
//#include "tf_weaponbase_grenadeproj.h"
//#include "tf_weapon_grenade_pipebomb.h"
#include "tf_shareddefs.h"
#include "tf_viewmodel.h"

#ifdef GAME_DLL
#include "halloween/zombie/zombie.h"
#endif

#ifdef CLIENT_DLL
#define CTFSwarmer_Melee C_TFSwarmer_Melee
#endif


class CTFSwarmer_Melee : public CTFWeaponBaseMelee
{
public:
	DECLARE_CLASS(CTFSwarmer_Melee, CTFWeaponBaseMelee);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CTFSwarmer_Melee();


	virtual void		Precache(void);
	virtual int			GetWeaponID(void) const { return TF_WEAPON_SWARMER_MELEE; }
	//virtual bool		BatDeflects() { return false; }
	virtual void		SecondaryAttack(void);
	virtual bool		CanSummon(CTFPlayer* pPlayer);

	virtual bool		SpawnSwarm(CTFPlayer* pPlayer);
	virtual void		Smack(void);
	//virtual void		PlayDeflectionSound(bool bPlayer);

	virtual float		InternalGetEffectBarRechargeTime(void) { return 30.f; }
	virtual int			GetEffectBarAmmo(void) { return TF_AMMO_GRENADES1; }

	float				GetProgress(void) { return GetEffectBarProgress(); }
	const char* GetEffectLabelText(void) { return "Spawn Swarm"; }
private:
	CTFSwarmer_Melee(const CTFSwarmer_Melee&) {}
};
#endif
