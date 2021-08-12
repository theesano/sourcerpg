//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Crowbar - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "basemeleeweapon.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "weapon_melee.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
ConVar    sk_plr_dmg_melee("sk_plr_dmg_melee", "0");

//-----------------------------------------------------------------------------
// CWeaponMelee
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CWeaponMelee, DT_WeaponMelee)
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS(weapon_melee , CWeaponMelee);
PRECACHE_WEAPON_REGISTER(weapon_melee);
#endif

acttable_t CWeaponMelee::m_acttable[] =
{
	{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK1, false },
	{ ACT_MELEE_ATTACK2, ACT_MELEE_ATTACK2, false},
	{ ACT_MELEE_ATTACK3, ACT_MELEE_ATTACK3, false},
	{ ACT_MELEE_ATTACK4, ACT_MELEE_ATTACK4, false },
	{ ACT_MELEE_ATTACK5, ACT_MELEE_ATTACK5, false },
	{ ACT_SKILL_EVILSLASH, ACT_SKILL_EVILSLASH, false },
	//{ ACT_MELEE_SPEVADE, ACT_MELEE_SPEVADE,false},
	{ ACT_IDLE, ACT_IDLE_ANGRY_MELEE, false },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_MELEE, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_MELEE, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_MELEE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_MELEE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_MELEE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_MELEE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_MELEE, false },
};

IMPLEMENT_ACTTABLE(CWeaponMelee);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponMelee::CWeaponMelee(void)
{

}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponMelee::GetDamageForActivity(Activity hitActivity)
{
	if ((GetOwner() != NULL) && (GetOwner()->IsPlayer()))
		return sk_plr_dmg_melee.GetFloat();
}


//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponMelee::AddViewKick(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat(1.0f, 2.0f);
	punchAng.y = random->RandomFloat(-2.0f, -1.0f);
	punchAng.z = 0.0f;

	pPlayer->ViewPunch(punchAng);
}

