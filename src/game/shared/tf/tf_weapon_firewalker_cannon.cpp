//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_firewalker_cannon.h"
#include "tf_fx_shared.h"
#include "in_buttons.h"
#include "ammodef.h"
#include "tf_gamerules.h"
#include "tf_weapon_rocketpack.h"
#include "soundenvelope.h"
#include "tf_projectile_dragons_fury.h"

#if defined( CLIENT_DLL )
#include "c_tf_gamestats.h"
#include "prediction.h"
#else
#include "tf_gamestats.h"
#include "ilagcompensationmanager.h"
#endif

#define RT_FIREWALKER_ALTCOST 5

IMPLEMENT_NETWORKCLASS_ALIASED( TFFirewalkerCannon, DT_WeaponFirewalkerCannon )
BEGIN_NETWORK_TABLE(CTFFirewalkerCannon, DT_WeaponFirewalkerCannon )
#ifdef CLIENT_DLL
	RecvPropFloat(RECVINFO(m_flChargeBeginTime)),
#else
	SendPropFloat(SENDINFO(m_flChargeBeginTime)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CTFFirewalkerCannon)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(tf_weapon_firewalker_cannon, CTFFirewalkerCannon);
PRECACHE_WEAPON_REGISTER(tf_weapon_firewalker_cannon);

void CTFFirewalkerCannon::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	CTFPlayer* pOwner = ToTFPlayer(GetPlayerOwner());
	if (!pOwner)
		return;

	if (m_flChargeBeginTime > 0)
	{
		// If we're not holding down the attack button, launch our grenade
		if (/*m_iClip1 > 0 && */ !(pOwner->m_nButtons & IN_ATTACK)  && (pOwner->m_afButtonReleased & IN_ATTACK))
		{
			/*
			else
			{*/
				LaunchGrenade();
			//}
		}
		else
		{
			float flTotalChargeTime = gpGlobals->curtime - m_flChargeBeginTime;
			if (flTotalChargeTime >= GetChargeForceReleaseTime())
			{
				ForceLaunchGrenade();
			}
		}
	}
}

void CTFFirewalkerCannon::WeaponIdle(void)
{
	CTFPlayer* pPlayer = ToTFPlayer(GetPlayerOwner());
	if (!pPlayer)
		return;

	DevMsg("Weapon Idle \n");

	if (m_flChargeBeginTime > 0 /* && m_iClip1 > 0 */ && (pPlayer->m_afButtonReleased & IN_ATTACK))
	{
		//if (m_iClip1 > 0)
		//{
			LaunchGrenade();
		//} 
	}
	else
	{
		BaseClass::WeaponIdle();
	}
}


void CTFFirewalkerCannon::WeaponReset(void)
{
	BaseClass::WeaponReset();

	m_flChargeBeginTime = 0.0f;
}

void CTFFirewalkerCannon::PrimaryAttack(void)
{
	// Get the player owning the weapon.
	CTFPlayer* pPlayer = ToTFPlayer(GetPlayerOwner());
	if (!pPlayer)
		return;

	int iAmmo = pPlayer->GetAmmoCount(m_iPrimaryAmmoType);
	if (iAmmo == 0)
		return;

	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return;

	if (!CanAttack())
	{
		m_flChargeBeginTime = 0;
		return;
	}

	if (m_flChargeBeginTime <= 0)
	{
		// Set the weapon mode.
		m_iWeaponMode = TF_WEAPON_PRIMARY_MODE;

		// save that we had the attack button down
		m_flChargeBeginTime = gpGlobals->curtime;

		SendWeaponAnim(ACT_VM_PULLBACK);

/*#ifdef CLIENT_DLL
		EmitSound(TF_WEAPON_PIPEBOMB_LAUNCHER_CHARGE_SOUND);
#endif // CLIENT_DLL*/
	}
	else
	{
		float flTotalChargeTime = gpGlobals->curtime - m_flChargeBeginTime;

		if (flTotalChargeTime >= GetChargeMaxTime())
		{
			LaunchGrenade();
		}
	}
}

void CTFFirewalkerCannon::LaunchGrenade(void)
{
	CTFPlayer* pPlayer = ToTFPlayer(GetPlayerOwner());
	if (!pPlayer)
		return;

	m_bWantsToShoot = false;

	if (pPlayer->GetWaterLevel() == 3)
	{
		// This weapon doesn't fire underwater
		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
		m_flChargeBeginTime = 0;
		return;
	}

#ifndef CLIENT_DLL
	if (pPlayer->m_Shared.IsStealthed() && ShouldRemoveInvisibilityOnPrimaryAttack())
	{
		pPlayer->RemoveInvisibility();
	}
	CTF_GameStats.Event_PlayerFiredWeapon(pPlayer, IsCurrentAttackACrit());

	//lagcompensation->StartLagCompensation(pPlayer, pPlayer->GetCurrentCommand());
#else
	C_CTF_GameStats.Event_PlayerFiredWeapon(pPlayer, IsCurrentAttackACrit());
#endif

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);

	pPlayer->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);

	CBaseEntity* pProj = FireProjectile(pPlayer);
	ModifyProjectile(pProj);

	if (ShouldRemoveDisguiseOnPrimaryAttack())
	{
		pPlayer->RemoveDisguise();
	}

	pPlayer->m_Shared.OnAttack();

	// Set next attack times.

	float flFireDelay = ApplyFireDelay(m_pWeaponInfo->GetWeaponData(m_iWeaponMode).m_flTimeFireDelay);

	m_flNextPrimaryAttack = gpGlobals->curtime + flFireDelay;
	//m_flLastDenySoundTime = gpGlobals->curtime;

	SetWeaponIdleTime(gpGlobals->curtime + SequenceDuration());

	m_flChargeBeginTime = 0;

#ifdef GAME_DLL
	//StartPressureSound();
	//lagcompensation->FinishLagCompensation(pPlayer);
#endif
}

void CTFFirewalkerCannon::ModifyProjectile(CBaseEntity* pProj)
{
#ifdef GAME_DLL
	CTFProjectile_BallOfFire* pFireBall = dynamic_cast<CTFProjectile_BallOfFire*>(pProj);
	if (pFireBall == NULL)
		return;

	//This uses the same entity as the dragon's fury (who'd guess)
	//But to prevent conflicts with the original dragon's fury (ie for mvm)
	//We set all of the stuff we want to change to be variables in the projectile
	//Set it to the defaults when it spawns then run these extra commands here
	pFireBall->SetDurationThink(0.5f + GetCurrentCharge());
	pFireBall->SetMaxDistance(0.5f + GetCurrentCharge());
	pFireBall->SetBurnDuration(-1 + (GetCurrentCharge() * 12));
	pFireBall->SetBonusDamage(false);
#endif
}

CBaseEntity* CTFFirewalkerCannon::FireProjectile(CTFPlayer* pPlayer)
{

	// Update the player's punch angle.
	QAngle angle = pPlayer->GetPunchAngle();
	float flPunchAngle = m_pWeaponInfo->GetWeaponData(m_iWeaponMode).m_flPunchAngle;
	angle.x -= flPunchAngle;
	pPlayer->SetPunchAngle(angle);
	m_flLastFireTime = gpGlobals->curtime;

	RemoveProjectileAmmo(pPlayer);

#ifdef GAME_DLL
	Vector vecForward, vecRight, vecUp;
	AngleVectors(pPlayer->EyeAngles(), &vecForward, &vecRight, &vecUp);

	float fRight = 8.f;
	if (IsViewModelFlipped())
	{
		fRight *= -1;
	}
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	// Shoot from the right location
	vecSrc = vecSrc + (vecUp * -9.0f) + (vecRight * 7.0f) + (vecForward * 3.0f);

	QAngle angForward = pPlayer->EyeAngles();

	trace_t trace;
	Vector vecEye = pPlayer->EyePosition();
	CTraceFilterSimple traceFilter(this, COLLISION_GROUP_NONE);
	UTIL_TraceHull(vecEye, vecSrc, -Vector(8, 8, 8), Vector(8, 8, 8), MASK_SOLID_BRUSHONLY, &traceFilter, &trace);

	CTFProjectile_Rocket* pRocket = static_cast<CTFProjectile_Rocket*>(CBaseEntity::CreateNoSpawn("tf_projectile_balloffire", vecSrc, angForward, pPlayer));
	if (pRocket)
	{
		pPlayer->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);

		DoFireEffects();

		pRocket->SetOwnerEntity(pPlayer);
		pRocket->SetLauncher(this);

		Vector vForward;
		AngleVectors(angForward, &vForward, NULL, NULL);

		pRocket->SetAbsVelocity(vForward * 600);

		pRocket->SetDamage(20);
		pRocket->ChangeTeam(pPlayer->GetTeamNumber());
		pRocket->SetCritical(pPlayer->m_Shared.IsCritBoosted());

		DispatchSpawn(pRocket);

		EmitSound(pRocket->IsCritical() ? "Weapon_DragonsFury.SingleCrit" : "Weapon_DragonsFury.Single");

		return pRocket;
	}

#else
	DoFireEffects();
#endif

	return NULL;
}

bool CTFFirewalkerCannon::Deploy(void)
{
	m_flChargeBeginTime = 0;

	return BaseClass::Deploy();
}

bool CTFFirewalkerCannon::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	m_flChargeBeginTime = 0;

	return BaseClass::Holster(pSwitchingTo);
}

//---------------------------------------------------------------------
// We use an alternate functionality for the alt-fire
// However, most of this is reusing a lot of the flamethrower's functionality;
// But since we aren't reusing the player or projectile interactions I can just call the stuff we need
//---------------------------------------------------------------------
void CTFFirewalkerCannon::SecondaryAttack(void)
{
	CTFPlayer* pOwner = GetTFPlayerOwner();
	if (!pOwner)
		return;

	if (m_flNextSecondaryAttack > gpGlobals->curtime)
	{
		return;
	}

	if (pOwner->GetWaterLevel() == WL_Eyes)
		return;

	if (!CanAttack())
	{
		SetWeaponState(FT_STATE_IDLE);
		return;
	}

	int iAmmo = pOwner->GetAmmoCount(m_iPrimaryAmmoType);

	if (iAmmo < RT_FIREWALKER_ALTCOST)
		return;

#ifdef GAME_DLL
	Vector launchDir;
	//Vector vecForce;
	AngleVectors(pOwner->EyeAngles(), &launchDir);
	VectorNormalize(launchDir);

	DeflectPlayer(pOwner, pOwner, launchDir);

	TheNextBots().OnWeaponFired(pOwner, this);
#endif


	float fAirblastRefireTimeScale = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT(fAirblastRefireTimeScale, mult_airblast_refire_time);
	if (fAirblastRefireTimeScale <= 0.0f)
	{
		fAirblastRefireTimeScale = 1.0f;
	}

	float fAirblastPrimaryRefireTimeScale = 1.0f;
	CALL_ATTRIB_HOOK_FLOAT(fAirblastPrimaryRefireTimeScale, mult_airblast_primary_refire_time);
	if (fAirblastPrimaryRefireTimeScale <= 0.0f)
	{
		fAirblastPrimaryRefireTimeScale = 1.0f;
	}

	m_flNextSecondaryAttack = gpGlobals->curtime + (0.75f * fAirblastRefireTimeScale);
	m_flNextPrimaryAttack = gpGlobals->curtime + (1.0f * fAirblastRefireTimeScale * fAirblastPrimaryRefireTimeScale);

	pOwner->RemoveAmmo(RT_FIREWALKER_ALTCOST, m_iPrimaryAmmoType);
}