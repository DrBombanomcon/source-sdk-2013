//========= Copyright Valve Corporation, All rights reserved. ============//
//
// TF Chain gun
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_chaingun.h"
#include "tf_fx_shared.h"
#include "tf_weaponbase_rocket.h"
#include "in_buttons.h"
#include "tf_gamerules.h"

// Client specific.
#ifdef CLIENT_DLL
#include "c_tf_player.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include "soundenvelope.h"
#include "particle_property.h"
// Server specific.
#else
#include "tf_player.h"
#include "tf_obj_sentrygun.h"
#include "tf_projectile_arrow.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(TFChainGun, DT_WeaponChainGun)

BEGIN_NETWORK_TABLE(CTFChainGun, DT_WeaponChainGun)
#ifndef CLIENT_DLL
	SendPropFloat(SENDINFO(m_flRampUp)),
	SendPropFloat(SENDINFO(m_flOverheatStartTime)),
	SendPropFloat(SENDINFO(m_flOverheatPercent)),
	SendPropFloat(SENDINFO(m_flFiringStartTime)),

	SendPropBool(SENDINFO(m_bCritShot)),
	SendPropBool(SENDINFO(m_bOverheatState)),
#else
	RecvPropFloat(RECVINFO(m_flRampUp)),
	RecvPropFloat(RECVINFO(m_flOverheatStartTime)),
	RecvPropFloat(RECVINFO(m_flOverheatPercent)),
	RecvPropFloat(RECVINFO(m_flFiringStartTime)),
	
	RecvPropBool(RECVINFO(m_bCritShot)),
	RecvPropBool(RECVINFO(m_bOverheatState)),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CTFChainGun)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(tf_weapon_chaingun, CTFChainGun);
PRECACHE_WEAPON_REGISTER(tf_weapon_chaingun);

// Server specific.
#ifndef CLIENT_DLL
BEGIN_DATADESC(CTFChainGun)
END_DATADESC()
#endif

//------------------------------------------
//
//------------------------------------------
CTFChainGun::CTFChainGun()
{
	m_bCritShot = false;
	m_bOverheatState = false;
	m_flRampUp = 0.f;
	m_flFiringStartTime = 0.f;
	m_flOverheatStartTime = 0.f;
	m_flOverheatPercent = 0.f;
	m_flOldOverheatPercent = 0.f;
}

CTFChainGun::~CTFChainGun()
{
}

void CTFChainGun::WeaponRegenerate(void)
{
	BaseClass::WeaponRegenerate();
	m_bCritShot = false;
	m_bOverheatState = false;
	m_flRampUp = 0.f;
	m_flFiringStartTime = 0.f;
	m_flOverheatStartTime = 0.f;
	m_flOverheatPercent = 0.f;
	m_flOldOverheatPercent = 0.f;
}

void CTFChainGun::PrimaryAttack(void)
{
	CTFPlayer* pPlayer = ToTFPlayer(GetPlayerOwner());
	if (!pPlayer)
		return;

	if ( !IsOverheated() )
	{
		BaseClass::PrimaryAttack();
		if (m_flFiringStartTime <= 0.f)
		{
			//Warning("Reseting firing time \n");
			m_flFiringStartTime = gpGlobals->curtime;
			m_flOldOverheatPercent = m_flOverheatPercent;
		}
		else
		{
			float flCurrentOverheatTime = gpGlobals->curtime - m_flFiringStartTime;
			m_flOverheatPercent = (flCurrentOverheatTime / TF_CHAINGUN_MAX_FIRING) + m_flOldOverheatPercent;
			//Warning("Overheat percentage: %fl", m_flOverheatPercent);
			if (m_flOverheatPercent >= 1.0f)
			{
				m_bOverheatState = true;
			}
		}

		if (GetRampUp() <= 1.f)
		{
			float flCurrentRampUp = gpGlobals->curtime - m_flFiringStartTime;
			m_flRampUp = (flCurrentRampUp / TF_CHAINGUN_RAMP_WITHDRAW);
		}

		float offsetPercent = TF_CHAINGUN_MIN_RAMP - (GetRampUp() * TF_CHAINGUN_MAX_RAMP);
		float flFireDelay = ApplyFireDelay(m_pWeaponInfo->GetWeaponData(m_iWeaponMode).m_flTimeFireDelay);
		m_flNextPrimaryAttack = gpGlobals->curtime + (flFireDelay * offsetPercent);
	}
}

//-------------------------------------------
// Purpose: Checks/handles the overheat mechanic
//-------------------------------------------
void CTFChainGun::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();
	//CBasePlayer* pOwner = GetPlayerOwner();
	if ( IsOverheated() )
	{
		if (m_flOverheatStartTime <= 0.f)
		{
			m_flOverheatStartTime = gpGlobals->curtime;

			//Warning("Began overheating [Doing this since no hud yet]\n");
		}
		else
		{
			float overheatCooldown = gpGlobals->curtime - m_flOverheatStartTime;
			if (overheatCooldown >= GetOverheatEndTime())
			{
				//Warning("Overheat over [Again, no hud]\n");
				m_bOverheatState = false;
				m_flRampUp = 0.f;
				m_flFiringStartTime = 0.f;
				m_flOverheatStartTime = 0.f;
				m_flOverheatPercent = 0.f;
			}
		}
	}
}

void CTFChainGun::WeaponIdle()
{
	if (m_flFiringStartTime > 0.f)
	{
		m_flFiringStartTime = 0.f;
		m_flOldOverheatPercent = 0.f;
		m_flStopFiringTime = gpGlobals->curtime;
	}

	if (m_flOverheatPercent > 0.f)
	{
		float cooldownTime = gpGlobals->curtime - m_flStopFiringTime;
		m_flOverheatPercent = m_flOverheatPercent - (cooldownTime / TF_CHAINGUN_OVERHEAT_TIME);
	}

	if (GetRampUp() > 0.f)
	{
		float rampFigtue = gpGlobals->curtime - m_flStopFiringTime;
		m_flRampUp = m_flRampUp - (rampFigtue / TF_CHAINGUN_RAMP_WITHDRAW);
	}
	BaseClass::WeaponIdle();
}

float CTFChainGun::GetProjectileSpeed(void)
{
	return RemapValClamped(0.75f, 0.0f, 1.f, 1200, 1800); // Temp, if we want to ramp.
}

float CTFChainGun::GetProjectileGravity(void)
{
	return RemapValClamped(0.75f, 0.0f, 1.f, 0.5, 0.1); // Temp, if we want to ramp.
}