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

ConVar    sk_plr_attackrange("sk_plr_attackrange", "128");

//-----------------------------------------------------------------------------
// CWeaponMelee
//-----------------------------------------------------------------------------

class CWeaponMelee : public CBaseMeleeWeapon
{
public:
	DECLARE_CLASS(CWeaponMelee, CBaseMeleeWeapon);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponMelee();

	float		GetRange(void)		{ return sk_plr_attackrange.GetFloat(); }

	void		AddViewKick(void);
	float		GetDamageForActivity(Activity hitActivity);

	void		SecondaryAttack(void)	{ return; }

private:
	//
};

#endif // WEAPON_MELEE_H
