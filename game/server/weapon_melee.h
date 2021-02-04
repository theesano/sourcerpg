//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_MELEE_H
#define WEAPON_MELEE_H

#include "basemeleeweapon.h"


#if defined( _WIN32 )
#pragma once
#endif

#ifdef HL2MP
#error weapon_crowbar.h must not be included in hl2mp. The windows compiler will use the wrong class elsewhere if it is.
#endif

ConVar    sk_plr_dmg_melee("sk_plr_dmg_melee", "0");
ConVar    sk_npc_dmg_melee("sk_npc_dmg_melee", "0");
ConVar    sk_plr_attackinterval("sk_plr_attackinterval", "0.6");
ConVar    sk_plr_attackrange("sk_plr_attackrange", "128");

//-----------------------------------------------------------------------------
// CWeaponCrowbar
//-----------------------------------------------------------------------------

class CWeaponMelee : public CBaseMeleeWeapon
{
public:
	DECLARE_CLASS(CWeaponMelee, CBaseMeleeWeapon);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponMelee();

	float		GetRange(void)		{ return sk_plr_attackrange.GetFloat(); }
	float		GetFireRate(void)		{ return	sk_plr_attackinterval.GetFloat(); }

	void		AddViewKick(void);
	float		GetDamageForActivity(Activity hitActivity);

	virtual int WeaponMeleeAttack1Condition(float flDot, float flDist);
	void		SecondaryAttack(void)	{ return; }

	// Animation event
	virtual void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

private:
	// Animation event handlers
	void HandleAnimEventMeleeHit(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
};

#endif // WEAPON_CROWBAR_H
