//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
// NOTE:  m_nExecutionTime which is used to freeze the player during skill use is bugged, when it reaches below .20 ,using evade will not give players additional speed boost
// which is why m_nExecutionTimeFix is used in combination to temporary fix it until a better solution is written.
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "animation.h"
#include "ai_condition.h"
#include "basemeleeweapon.h"
#include "ndebugoverlay.h"
#include "te_effect_dispatch.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "ai_eventresponse.h"
#include "ai_basenpc.h"
#include "grenade_frag.h"
#include "Skills_WeaponThrow.h"
#include "hl2_player.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST(CBaseMeleeWeapon, DT_BaseMeleeWeapon)
END_SEND_TABLE()

#define BLUDGEON_HULL_DIM		16

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

//TODO : Hook attackspeed mod to attack, skills and everything
extern ConVar pl_isattacking("pl_isattacking", "0");
extern ConVar sk_plr_attackinterval("sk_plr_attackinterval", "0.6");
//Weapon Effects
extern ConVar sk_npcknockbackathealth("sk_npcknockbackathealth", "100");

// Combat stats behavior 
extern ConVar sk_plr_melee_dmg_critical("sk_plr_melee_dmg_critical", "0");
extern ConVar sk_plr_melee_dmg_critical_chance("sk_plr_melee_dmg_critical", "0");

extern ConVar sk_plr_melee_mp_bonus("sk_plr_melee_mp_bonus", "4");
extern ConVar sk_plr_melee_normal_range("sk_plr_melee_normal_range", "128");


//Current cooldown time (Shared Variables with server)
extern ConVar sk_plr_skills_2_cd("sk_plr_skills_2_cd", "0");
extern ConVar sk_plr_skills_3_cd("sk_plr_skills_3_cd", "0");
extern ConVar sk_plr_skills_4_cd("sk_plr_skills_4_cd", "0");
extern ConVar sk_plr_skills_5_cd("sk_plr_skills_5_cd", "0");
extern ConVar sk_plr_skills_6_cd("sk_plr_skills_6_cd", "0");

//TODO: list skills timer to this one
extern ConVar sk_plr_skills_1_cooldown_time("sk_plr_skills_1_cooldown_time", "5");
extern ConVar sk_plr_skills_2_cooldown_time("sk_plr_skills_2_cooldown_time", "10");
extern ConVar sk_plr_skills_3_cooldown_time("sk_plr_skills_3_cooldown_time", "15");
extern ConVar sk_plr_skills_4_cooldown_time("sk_plr_skills_4_cooldown_time", "15");
extern ConVar sk_plr_skills_5_cooldown_time("sk_plr_skills_5_cooldown_time", "16");
extern ConVar sk_plr_skills_6_cooldown_time("sk_plr_skills_6_cooldown_time", "30");


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CBaseMeleeWeapon::CBaseMeleeWeapon()
{
	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the weapon
//-----------------------------------------------------------------------------
void CBaseMeleeWeapon::Spawn(void)
{
	m_fMinRange1 = 0;
	m_fMinRange2 = 0;
	m_fMaxRange1 = 64;
	m_fMaxRange2 = 64;

	m_bIsSkCoolDown = false;
	m_nSkCoolDownTime = 0.0f;
	m_bIsSkCoolDown2 = false;
	m_nSkCoolDownTime2 = 0.0f;
	m_bIsSkCoolDown3 = false;
	m_nSkCoolDownTime3 = 0.0f;
	m_bIsSkCoolDown4 = false;
	m_nSkCoolDownTime4 = 0.0f;
	m_bIsSkCoolDown5 = false;
	m_nSkCoolDownTime5 = 0.0f;

	m_nExecutionTime = 0.0f;//This Var is Tied to Camera control in "in_camera" , be advised when changing it.
	m_nSkillHitRefireTime = 0.0f;

	m_bWIsAttack1 = true;
	m_bWIsAttack2 = false;
	m_bWIsAttack3 = false;
	m_bWIsAttack2 = false;
	m_bWIsAttack3 = false;

	m_SpeedModActiveTime = 0.0f;
	m_flNPCFreezeTime = 0.0f;
	m_flSkillAttributeRange = 0.0f;
	m_flTotalAttackTime = 0.0f;
	m_flInAirTime = 0.0f;


	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Precache the weapon
//-----------------------------------------------------------------------------
void CBaseMeleeWeapon::Precache(void)
{
	//Call base class first
	BaseClass::Precache();
	PrecacheScriptSound("Weapon_Melee.SPECIAL4");
	PrecacheScriptSound("Weapon_Melee.SPECIAL5");
	PrecacheScriptSound("Weapon_Melee.ATTACK4");
	PrecacheScriptSound("Weapon_Melee.ATTACK5");
	PrecacheScriptSound("Weapon_Melee.SPEVADE");
	PrecacheScriptSound("Weapon_Melee.AIRATTACK1");
	PrecacheScriptSound("Weapon_Melee.AIRATTACK2");
	PrecacheScriptSound("Weapon_Melee.AIRATTACK3");

	//Call base class first
	PrecacheParticleSystem("aoehint");
	PrecacheParticleSystem("aoehint2");
	PrecacheParticleSystem("aoehint22");
	PrecacheParticleSystem("striderbuster_shotdown_core_flash");
	PrecacheParticleSystem("choreo_skyflower_nexus");
	PrecacheParticleSystem("tornado1");

}

int CBaseMeleeWeapon::CapabilitiesGet()
{
	return bits_CAP_WEAPON_MELEE_ATTACK1;
}


int CBaseMeleeWeapon::WeaponMeleeAttack1Condition(float flDot, float flDist)
{
	if (flDist > 64)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if (flDot < 0.7)
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}

Vector CBaseMeleeWeapon::GetWeaponAimDirection()
{
	CBasePlayer *pUser = ToBasePlayer(GetOwner());

	//Determine the position of the damage zone.
	Vector swingStart = pUser->Weapon_ShootPosition();
	swingStart.z -= 30;

	//Get Aiming Direction
	Vector forward;
	forward = pUser->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	//GetRange() for this one actually defines the range from player's position to the AoE damage zone
	Vector swingEnd = swingStart + forward * GetRange();

	return swingEnd;
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CBaseMeleeWeapon::ItemPostFrame(void)
{
	
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	SkillsHandler();

	m_flAttackInterval = sk_plr_attackinterval.GetFloat();

	ConVar* pAnimTime = cvar->FindVar("animtime");
	m_flAnimTime = pAnimTime->GetFloat();
	
	//The timer that makes combo chain reverts back to 1 when expired.
	if ((m_flTotalAttackTime < gpGlobals->curtime) && (!m_bWIsAttack1))
	{
		m_bWIsAttack1 = true;
		m_bWIsAttack2 = false;
		m_bWIsAttack3 = false;
		m_bWIsAttack4 = false;
		m_bWIsAttack5 = false;
	}
	

	if (pOwner == NULL)
		return;

	if (m_nExecutionTime - gpGlobals->curtime <= 0)
	{
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			PrimaryAttack();
		}
		else if ((pOwner->m_afButtonPressed & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
		{
			//SecondaryAttack();
		}
		else
		{
			WeaponIdle();
			return;
		}
	}


}

//m_nExecutionTime handles freezing the player for a certain amount of time
void CBaseMeleeWeapon::SkillsHandler(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());

	m_flPlayerStats_BaseDamage = pPlayer->GetPlayerBaseDamage();
	m_flPlayerStats_AttackSpeed = pPlayer->GetPlayerAttackSpeed();

	Activity nHitActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;

	m_flCooldown = 1/pPlayer->GetPlayerCooldownReductionRate();

	
	/*CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);*/

	//Warning("Attack speed %.2f \n", sk_atkspeedmod.GetFloat());
	//if (m_SpeedModActiveTime >= gpGlobals->curtime )
	//{
	//}
	//else if (m_SpeedModActiveTime <= gpGlobals->curtime )
	//{
	//	//Reason why some stats can't be changed on the character panel

	//}

	//Freeze NPC Time 
	if (m_flNPCFreezeTime > gpGlobals->curtime)
		AddKnockbackXY(1.0f, 2);
	else
		AddKnockbackXY(1.0f, 3);
	
	//Skill 1 Special Evade
	if ((pOwner->m_nButtons & IN_ATTACK2) && !m_bIsSkCoolDown)
	{
		if (m_nExecutionTime - gpGlobals->curtime <= 0)
		{
			GetPlayerAnglesOnce();
			Skill_Evade();
		}
		
	}

	if (m_flInAirTime >= gpGlobals->curtime)
	{
		playerPosInAir.z += 0.15;
		UTIL_GetLocalPlayer()->SetAbsOrigin(playerPosInAir);
		UTIL_GetLocalPlayer()->SetAbsVelocity(vec3_origin);
	
	}

	
	//Run the Evil Slash skill code
	if ((pOwner->m_afButtonPressed & IN_SLOT1) && !m_bIsSkCoolDown2)
	{
		if (pPlayer->GetPlayerMP() > 30)
		{
			if (!m_bIsSkCoolDown2)
			{
				if (m_nExecutionTime - gpGlobals->curtime <= 0)
					Skill_RadialSlash();
			}
		}
		else
		{
			//Print SG insufficient warning on player's screen
			SkillStatNotification_HUD(1);
		}

	}
	else if ((pOwner->m_afButtonPressed & IN_SLOT1) && m_bIsSkCoolDown2)
		SkillStatNotification_HUD(2);

	
	Skill_RadialSlash_LogicEx();


	// Skill 3 loop
	if ((pOwner->m_afButtonPressed & IN_SLOT2) && !m_bIsSkCoolDown3)
	{
		if (pPlayer->GetPlayerMP() > 25)
		{
			if (!m_bIsSkCoolDown3)
			{
				if (m_nExecutionTime - gpGlobals->curtime <= 0)
					Skill_GrenadeEX();
			}
		}
		else
			SkillStatNotification_HUD(1);
	}
	else if ((pOwner->m_afButtonPressed & IN_SLOT2) && m_bIsSkCoolDown3)
	{
		SkillStatNotification_HUD(2);

	}
		
	//Skill 4 loop
	if ((pOwner->m_afButtonPressed & IN_SLOT3) && !m_bIsSkCoolDown4)
	{
		if (pPlayer->GetPlayerMP() > 50)
		{
			if (!m_bIsSkCoolDown4)
			{ 
				if (m_nExecutionTime - gpGlobals->curtime <= 0)
					Skill_HealSlash();
			}
		}
		else
			SkillStatNotification_HUD(1);

	}
	else if ((pOwner->m_afButtonPressed & IN_SLOT3) && m_bIsSkCoolDown4)
		SkillStatNotification_HUD(2);


		//Skill 5 loop
	if (pOwner->m_afButtonPressed & IN_SLOT4 && !m_bIsSkCoolDown5)
	{
		if (pPlayer->GetPlayerMP() > 30)
		{
			if (!m_bIsSkCoolDown5)
			{
				if (m_nExecutionTime - gpGlobals->curtime <= 0)
					Skill_Trapping();
			}
		}
		else
		{
			SkillStatNotification_HUD(1);
		}
	}
	else if ((pOwner->m_afButtonPressed & IN_SLOT4) && & m_bIsSkCoolDown5)
		SkillStatNotification_HUD(2);


	Skill_Trapping_LogicEx();

		//Skill 6 loop
	if (pOwner->m_afButtonPressed & IN_SLOT5 && !m_bIsSkCoolDown6)
	{
		if (pPlayer->GetPlayerMP() > 50)
		{
			if (!m_bIsSkCoolDown6)
			{
				if (m_nExecutionTime - gpGlobals->curtime <= 0)
					Skill_Tornado();
			}
		}
		else
		{
			SkillStatNotification_HUD(1);
		}
	}
	else if (pOwner->m_afButtonPressed & IN_SLOT5 && m_bIsSkCoolDown6)
	{
		SkillStatNotification_HUD(2);

	}

		Skill_Tornado_LogicEx();

	ConVar* pAerialEvadeDistance = cvar->FindVar("sk_evadedistance");

	Vector AerialEvadeDir;
	//Makes the player stand still when activating a skill

	//TEMP
	if (pOwner->m_afButtonPressed & IN_SPEED)
	{
		m_nExecutionTime = 0.0f;
		
		m_bWIsAttack1 = true;
		m_bWIsAttack2 = false;
		m_bWIsAttack3 = false;
		m_bWIsAttack4 = false;
		m_bWIsAttack5 = false;

		if (m_flInAirTime >= gpGlobals->curtime)
		{
			m_flInAirTime = 0.0f;
			AerialEvadeDir = dirkb;
			AerialEvadeDir.z += 0.25f;
			pOwner->SetAbsVelocity(AerialEvadeDir * pAerialEvadeDistance->GetFloat());
		}
		
			
	}

	//Skill 1 CoolDown
	if (m_nSkCoolDownTime - gpGlobals->curtime >= 0)
	{
		//PlaceHolder
	}
	else
		m_bIsSkCoolDown = false;
	
	//Skill 2 CoolDown
	if ( m_nSkCoolDownTime2 - gpGlobals->curtime >= 0)
	{
		//Placeholder
	}
	else
	{
		m_bIsSkCoolDown2 = false;
	}

	//Skill 3 CoolDown
	if (m_nSkCoolDownTime3 - gpGlobals->curtime >= 0)
	{
		
	}
	else
	{
		m_bIsSkCoolDown3 = false;
	}
	
	if (m_nSkCoolDownTime4 - gpGlobals->curtime >= 0)
	{
		
	}
	else
	{
		m_bIsSkCoolDown4 = false;
	}

	if (m_nSkCoolDownTime5 - gpGlobals->curtime >= 0)
	{
		//engine->Con_NPrintf(14, "Skill 5 Cooldown time  %6.1f   ", m_nSkCoolDownTime5 - gpGlobals->curtime);
	}
	else
	{
		m_bIsSkCoolDown5 = false;
	}

	if (m_nSkCoolDownTime6 - gpGlobals->curtime >= 0)
	{
		//engine->Con_NPrintf(15, "Skill 6 Cooldown time  %6.1f   ", m_nSkCoolDownTime6 - gpGlobals->curtime);
	}
	else
	{
		m_bIsSkCoolDown6 = false;
	}

	//Skill 3 Grenade Detonate/Properties
	//if (pOwner->m_nButtons & IN_USE)
	//if (m_nSkCoolDownTime3 - gpGlobals->curtime >= 0)
	//{
	//	CBaseEntity *pEntity = NULL;

	//	//while ((pEntity = gEntList.FindEntityByClassname(pEntity, "npc_grenade_frag")) != NULL)
	//	while ((pEntity = gEntList.FindEntityByClassname(pEntity, "skills_weaponthrow")) != NULL)
	//	{
	//		/*CGrenadeFrag *pFrag = dynamic_cast<CGrenadeFrag *>(pEntity);
	//		if (pFrag->m_bIsLive && pFrag->GetThrower() && GetOwner() && pFrag->GetThrower() == GetOwner())
	//		{
	//			pFrag->Use(GetOwner(), GetOwner(), USE_ON, 0);
	//			pFrag->SetTimer(0, 0);

	//		}*/
	//		CWeaponThrowingSkills *pSkWpnThrow = dynamic_cast<CWeaponThrowingSkills *>(pEntity);
	//		if (pSkWpnThrow->IsAlive())
	//			AddKnockbackXY(3.0f, 5);
	//			//pSkWpnThrow->ExplodeThink();

	//	}
	//}

	SkillStatNotification();

	//This is a Hacky way to report if player is attacking.
	if (m_nExecutionTime > gpGlobals->curtime)
	{
		pl_isattacking.SetValue(1);
	}
	else
	{
		pl_isattacking.SetValue(0);
	}

	
}

void CBaseMeleeWeapon::SkillStatNotification(void)
{
	//External ( using CVars as synced variables between servers and client )
	//Display Info for HUDs
	int skill2cdtimer = m_nSkCoolDownTime2 - gpGlobals->curtime;
	sk_plr_skills_2_cd.SetValue(skill2cdtimer);

	int skill3cdtimer = m_nSkCoolDownTime3 - gpGlobals->curtime;
	sk_plr_skills_3_cd.SetValue(skill3cdtimer);

	int skill4cdtimer = m_nSkCoolDownTime4 - gpGlobals->curtime;
	sk_plr_skills_4_cd.SetValue(skill4cdtimer);

	int skill5cdtimer = m_nSkCoolDownTime5 - gpGlobals->curtime;
	sk_plr_skills_5_cd.SetValue(skill5cdtimer);

	int skill6cdtimer = m_nSkCoolDownTime6 - gpGlobals->curtime;
	sk_plr_skills_6_cd.SetValue(skill6cdtimer);


	//engine->Con_NPrintf(9, "Current Attack in the chain %i %i %i %i %i ", m_bWIsAttack1, m_bWIsAttack2, m_bWIsAttack3, m_bWIsAttack4, m_bWIsAttack5);
	
	//engine->Con_NPrintf(11, "Current Attack Interval %.2f ",m_flAttackInterval );

	//if (UTIL_GetLocalPlayer()->GetGroundEntity() == NULL)
	//{
	//	if (m_flInAirTime - gpGlobals->curtime >= 0)
	//		DevMsg("Time In Air: %.2f \n", m_flInAirTime - gpGlobals->curtime);
	//	else
	//		DevMsg("Time In Air:0 \n");

		//if (m_nExecutionTime - gpGlobals->curtime >= 0)
		//	DevMsg("Time: Total Execution (Freeze Mvmt): %.2f \n", m_nExecutionTime - gpGlobals->curtime);
		//else
		//	DevMsg("Time: Total Execution (Freeze Mvmt): 0 \n");
	//}
	
	/*if (m_flNextPrimaryAttack - gpGlobals->curtime >=0)
		engine->Con_NPrintf(12, "Time Until Next Attack %.2f", m_flNextPrimaryAttack - gpGlobals->curtime);
	else
		engine->Con_NPrintf(12, "Time Until Next Attack: 0");*/

	//if (m_nSkCoolDownTime - gpGlobals->curtime >= 0)
	//	engine->Con_NPrintf(10, "Skill 1 Cooldown time %6.1f Attack speed %6.1f ", m_nSkCoolDownTime - gpGlobals->curtime, sk_atkspeedmod.GetFloat());


	//if (m_nSkCoolDownTime2 - gpGlobals->curtime >= 0)
	//	engine->Con_NPrintf(11, "Skill 2 Cooldown time  %6.1f ", m_nSkCoolDownTime2 - gpGlobals->curtime);


	//if (m_nSkCoolDownTime3 - gpGlobals->curtime >= 0)
	//	engine->Con_NPrintf(12, "Skill 3 Cooldown time  %6.1f  Press USE to Detonate Now ", m_nSkCoolDownTime3 - gpGlobals->curtime);

	//if (m_nSkCoolDownTime4 - gpGlobals->curtime >= 0)
	//	engine->Con_NPrintf(13, "Skill 4 Cooldown time  %6.1f   ", m_nSkCoolDownTime4 - gpGlobals->curtime);

	//engine->Con_NPrintf(9, "Enemy HP  %i ", m_iEnemyHealth);

	//if (m_nSkCoolDownTime5 - gpGlobals->curtime >= 0)
	//{
	//	engine->Con_NPrintf(14, "Skill 5 Cooldown time  %6.1f   ", m_nSkCoolDownTime5 - gpGlobals->curtime);
	//}
	//else
	//{
	//	m_bIsSkCoolDown5 = false;	
	//}

	//if (m_nSkCoolDownTime6 - gpGlobals->curtime >= 0)
	//{
	//	engine->Con_NPrintf(15, "Skill 6 Cooldown time  %6.1f   ", m_nSkCoolDownTime6 - gpGlobals->curtime);
	//}
	//else
	//{
	//	m_bIsSkCoolDown6 = false;
	//}

	//	if (m_bIsSkCoolDown5)
	//		engine->Con_NPrintf(13, "Skill 5 is in cooldown");
	//	else if (!m_bIsSkCoolDown5)
	//		engine->Con_NPrintf(13, "Skill 5 is NOT in cooldown");

	//	if (m_bIsSkCoolDown6)
	//		engine->Con_NPrintf(16, "Skill 6 is in cooldown");
	//	else if (!m_bIsSkCoolDown6)
	//		engine->Con_NPrintf(16, "Skill 6 is NOT in cooldown");

	//	engine->Con_NPrintf(16,"Is player attacking? : %i", pl_isattacking.GetInt());
	//	engine->Con_NPrintf(17, "MP unit: %i", m_iPlayerMP);




}

void CBaseMeleeWeapon::SkillStatNotification_HUD(int messageoption)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	hudtextparms_s tTextParam;
	tTextParam.x = 0.6;
	tTextParam.y = 0.65;
	tTextParam.effect = 0;
	tTextParam.r1 = 255;
	tTextParam.g1 = 255;
	tTextParam.b1 = 255;
	tTextParam.a1 = 255;
	tTextParam.r2 = 255;
	tTextParam.g2 = 255;
	tTextParam.b2 = 255;
	tTextParam.a2 = 255;
	tTextParam.fadeinTime = 0;
	tTextParam.fadeoutTime = 0;
	tTextParam.holdTime = 0.6;
	tTextParam.fxTime = 0;
	tTextParam.channel = 2;

		if (messageoption == 1)
		{
			UTIL_HudMessage(pOwner, tTextParam, "SG insufficient");
			//ClientPrint(pOwner, HUD_PRINTCENTER, "SG insufficient");
		}
		else if (messageoption == 2)
		{
			UTIL_HudMessage(pOwner, tTextParam, "In cooldown");
		}
	
}
//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CBaseMeleeWeapon::PrimaryAttack()
{
	Swing(false);
}


Activity CBaseMeleeWeapon::ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner)
{
	int			i, j, k;
	float		distance;
	const float	*minmaxs[2] = { mins.Base(), maxs.Base() };
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
	if (tmpTrace.fraction == 1.0)
	{
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 2; j++)
			{
				for (k = 0; k < 2; k++)
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine(vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
					if (tmpTrace.fraction < 1.0)
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if (thisDistance < distance)
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}


	return ACT_VM_HITCENTER;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseMeleeWeapon::ImpactEffect(trace_t &traceHit)
{

	//FIXME: need new decals
	UTIL_ImpactTrace(&traceHit, DMG_SLASH);
}


//------------------------------------------------------------------------------
// Purpose : Starts the swing of the weapon and determines the animation
// Input   : bIsSecondary - is this a secondary attack?
//------------------------------------------------------------------------------
void CBaseMeleeWeapon::Swing(int bIsSecondary)
{
	Vector vecShootOrigin, vecShootDir;

	float speedmod = 1 / m_flPlayerStats_AttackSpeed;
	// Try a ray
	trace_t traceHit;
	
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;
	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());

	float AoeDamageRadius = sk_plr_melee_normal_range.GetFloat();

	UTIL_TraceLine(pOwner->Weapon_ShootPosition(), GetWeaponAimDirection(), MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);

	Activity nHitActivity = ACT_MELEE_ATTACK1;
	// Damage info for 
	CTakeDamageInfo dmginfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	if (m_SpeedModActiveTime >= gpGlobals->curtime)
		dmginfo.SetDamage(GetDamageForActivity(nHitActivity)*2);
	else if (m_SpeedModActiveTime <= gpGlobals->curtime)
		dmginfo.SetDamage(RandomFloat(GetDamageForActivity(nHitActivity), GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage));

	//Makes weapon produce AoE damage
	RadiusDamage_EX(dmginfo, GetWeaponAimDirection(), AoeDamageRadius, CLASS_NONE, pOwner,true);
	//Move player forward for each swing.
	AddSkillMovementImpulse(2.0f);

	int iScytheBlade = LookupAttachment("attach_blade");

	Vector particlepos = GetAbsOrigin() + Vector(0,0,32);
	//DispatchParticleEffect("aoehint2", GetWeaponAimDirection(), vec3_angle);
	DispatchParticleEffect("aoehint2", PATTACH_ABSORIGIN_FOLLOW,this,iScytheBlade,true);

	m_iPrimaryAttacks++;

	// Send the anim
	SendWeaponAnim(nHitActivity);
	GetPlayerAnglesOnce();
	//Setup our next attack times
	m_flNPCFreezeTime = gpGlobals->curtime + 0.6f;
	UTIL_ScreenShake(GetAbsOrigin(), 2.4f, 100.0, 0.5, 256.0f, SHAKE_START);

	//Air
	if (pOwner->GetGroundEntity() == NULL)
	{
		GetPlayerPosOnce();
		m_flInAirTime = gpGlobals->curtime + 0.6666f;
		UTIL_ScreenShake(GetAbsOrigin(), 3.2f, 100.0, 0.5, 256.0f, SHAKE_START,true);

	}
	
	//m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

	//Set and cycles between attack states, play animation and make sound. 
	if (pOwner->GetGroundEntity() != NULL)
	{
		if (m_bWIsAttack1 == true)
		{
			AoeDamageRadius = 128.0f;
			m_flSkillAttributeRange = AoeDamageRadius;
			//triggerInfo.ScaleDamage(1.0);
			WeaponSound(ATTACK1);
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = false;
			m_bWIsAttack2 = true;
			m_bWIsAttack3 = false;
			m_bWIsAttack4 = false;
			m_bWIsAttack5 = false;
			AddKnockbackXY(2.0f, 1);
			AddKnockbackXY(1, 5); //for npc hitting sound
			//Give the player MP for each enemy hit
			if (m_bIsEnemyInAtkRange)			
			pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() + sk_plr_melee_mp_bonus.GetInt());

			m_nExecutionTime = gpGlobals->curtime + (0.6666f * speedmod );
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* speedmod); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* speedmod) + m_flAnimTime;

		}
		else if (m_bWIsAttack2 == true)
		{
			//AoeDamageRadius = 144.0f;
			//triggerInfo.ScaleDamage(1.5);
			m_flSkillAttributeRange = AoeDamageRadius;
			WeaponSound(ATTACK2);
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = false;
			m_bWIsAttack2 = false;
			m_bWIsAttack3 = true;
			m_bWIsAttack4 = false;
			m_bWIsAttack5 = false;
			AddKnockbackXY(2.0f, 1);
			AddKnockbackXY(1, 5); //for npc hitting sound
			//Give the player MP for each enemy hit
			if (m_bIsEnemyInAtkRange)
				pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() + sk_plr_melee_mp_bonus.GetInt());


			m_nExecutionTime = gpGlobals->curtime + (0.6666f * speedmod);
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* speedmod); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* speedmod) + m_flAnimTime;

		}
		else if (m_bWIsAttack3 == true)
		{
			//AoeDamageRadius = 192.0f;
			//triggerInfo.ScaleDamage(2.0);
			m_flSkillAttributeRange = AoeDamageRadius;
			WeaponSound(ATTACK3);
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = false;
			m_bWIsAttack2 = false;
			m_bWIsAttack3 = false;
			m_bWIsAttack4 = true;
			m_bWIsAttack5 = false;
			AddKnockbackXY(3.0f, 1);
			AddKnockbackXY(1, 5); //for npc hitting sound
			//Give the player MP for each enemy hit
			if (m_bIsEnemyInAtkRange)
				pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() + sk_plr_melee_mp_bonus.GetInt());


			m_nExecutionTime = gpGlobals->curtime + (0.6666f * speedmod);
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* speedmod); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* speedmod) + m_flAnimTime;

		}
		else if (m_bWIsAttack4 == true)
		{
			EmitSound("Weapon_Melee.ATTACK4");
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = false;
			m_bWIsAttack2 = false;
			m_bWIsAttack3 = false;
			m_bWIsAttack4 = false;
			m_bWIsAttack5 = true;
			AddKnockbackXY(3.0f, 1);
			AddKnockbackXY(1, 5); //for npc hitting sound

			//Give the player MP for each enemy hit
			if (m_bIsEnemyInAtkRange)
				pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() + sk_plr_melee_mp_bonus.GetInt()+ 2);


			m_nExecutionTime = gpGlobals->curtime + (1.0f * speedmod);

			m_flNextPrimaryAttack = gpGlobals->curtime + (1.0f * speedmod);

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* speedmod) + (m_flAnimTime + 0.4f);

		}
		else if (m_bWIsAttack5 == true)
		{
			EmitSound("Weapon_Melee.ATTACK5");
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = true; // end of the chain
			m_bWIsAttack2 = false;
			m_bWIsAttack3 = false;
			m_bWIsAttack4 = false;
			m_bWIsAttack5 = false;
			AddKnockbackXY(3.0f, 1);
			AddKnockbackXY(1, 5); //for npc hitting sound
			if (m_bIsEnemyInAtkRange)
				pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() + sk_plr_melee_mp_bonus.GetInt()+4);


			m_nExecutionTime = gpGlobals->curtime + (1.0f * speedmod);

			m_flNextPrimaryAttack = gpGlobals->curtime + (1.0f * speedmod);

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* speedmod) + (m_flAnimTime + 0.4f);

		}
	}
	else if (pOwner->GetGroundEntity() == NULL)
	{
		if (m_bWIsAttack1 == true)
		{
			AoeDamageRadius = 128.0f;
			m_flSkillAttributeRange = AoeDamageRadius;
			//triggerInfo.ScaleDamage(1.0);
			EmitSound("Weapon_Melee.AIRATTACK1");
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = false;
			m_bWIsAttack2 = true;
			m_bWIsAttack3 = false;
			m_bWIsAttack4 = false;
			m_bWIsAttack5 = false;
			AddKnockbackXY(2.0f, 1);
			AddKnockbackXY(1, 5); //for npc hitting sound

			m_nExecutionTime = gpGlobals->curtime + (0.6666f * speedmod);
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* speedmod); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* speedmod) + m_flAnimTime;

		}
		else if (m_bWIsAttack2 == true)
		{
			//AoeDamageRadius = 144.0f;
			//triggerInfo.ScaleDamage(1.5);
			m_flSkillAttributeRange = AoeDamageRadius;
			EmitSound("Weapon_Melee.AIRATTACK2");
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = false;
			m_bWIsAttack2 = false;
			m_bWIsAttack3 = true;
			m_bWIsAttack4 = false;
			m_bWIsAttack5 = false;
			AddKnockbackXY(2.0f, 1);
			AddKnockbackXY(1, 5); //for npc hitting sound

			m_nExecutionTime = gpGlobals->curtime + (0.6666f * speedmod);
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* speedmod); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* speedmod) + m_flAnimTime;

		}
		else if (m_bWIsAttack3 == true)
		{
			//AoeDamageRadius = 192.0f;
			//triggerInfo.ScaleDamage(2.0);
			m_flSkillAttributeRange = AoeDamageRadius;
			EmitSound("Weapon_Melee.AIRATTACK3");
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = true;
			m_bWIsAttack2 = false;
			m_bWIsAttack3 = false;
			m_bWIsAttack4 = false;
			m_bWIsAttack5 = false;
			AddKnockbackXY(3.0f, 1);
			AddKnockbackXY(1, 5); //for npc hitting sound

			m_nExecutionTime = gpGlobals->curtime + (0.6666f * speedmod);
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* speedmod); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* speedmod) + m_flAnimTime;

		}
	}


}

//Skill1: Spinning Demon.
void CBaseMeleeWeapon::Skill_Evade(void)
{
	float speedmod = 1 / m_flPlayerStats_AttackSpeed;

	trace_t traceHit;
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	float nStepVelocity = 1024.0f;

	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	//Use Base Damage
	Activity nHitActivity = ACT_VM_HITCENTER;

	CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	if (m_SpeedModActiveTime >= gpGlobals->curtime)
		info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
	else if (m_SpeedModActiveTime <= gpGlobals->curtime)
		info.SetDamage(RandomFloat(GetDamageForActivity(nHitActivity), GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage));
	
	info.SetDamagePosition(traceHit.startpos);
	info.SetDamageForce(forward); //apply force to physics props 
	info.ScaleDamage(2.0f);



	float AoeDamageRadius = 192.0f;
	//Only run when the cooldown time is 0


	//Plummet 
	Vector aFwd;
	aFwd = dirkb;
	aFwd.z -= 0.5;

	if (pOwner->GetGroundEntity() != NULL)
	{
		pOwner->SetAbsVelocity(dirkb*(nStepVelocity*1.5));
		m_nExecutionTime = gpGlobals->curtime + (1.0f * speedmod);

	}
	else if (pOwner->GetGroundEntity() == NULL) //Plummet
	{
		m_flInAirTime = 0.0f;
		m_nExecutionTime = gpGlobals->curtime + (2.0f * speedmod);
		pOwner->SetAbsVelocity(aFwd*nStepVelocity);
	}


	UTIL_ScreenShake(GetAbsOrigin(), 2.0f, 100.0, 0.7, 256.0f, SHAKE_START);

	RadiusDamage(info, UTIL_GetLocalPlayer()->GetAbsOrigin(), AoeDamageRadius, CLASS_NONE, pOwner);
		
	EmitSound("Weapon_Melee.SPEVADE");
	pOwner->SetAnimation(PLAYER_EVADE); 

		//Sync the time with flFreezingMovementTime in in_main.cpp

	m_nSkCoolDownTime = gpGlobals->curtime + (sk_plr_skills_1_cooldown_time.GetFloat()*m_flCooldown);
	m_bIsSkCoolDown = true;

}

//Skill2:Evil Slash
float flSkill_RadialSlash_ActiveTime;
void CBaseMeleeWeapon::Skill_RadialSlash(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());


	float AoeDamageRadius = 264.0f;
	Activity nHitActivity = ACT_VM_IDLE;
	CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	if (m_SpeedModActiveTime >= gpGlobals->curtime)
		info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
	else if (m_SpeedModActiveTime <= gpGlobals->curtime)
		info.SetDamage(RandomFloat(GetDamageForActivity(nHitActivity), GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage));

				//Initialize the variable for moving the player on each attack
	AddSkillMovementImpulse(1.0f);
	GetPlayerAnglesOnce();
	m_nExecutionTime = gpGlobals->curtime + 2.0f;
				//HACK! Fire the timer
	m_nSkillHitRefireTime = gpGlobals->curtime + 0.3f; //delta between refire
				if (gpGlobals->curtime - m_nExecutionTime < 0)
				{
					RadiusDamage(info, UTIL_GetLocalPlayer()->GetAbsOrigin(), AoeDamageRadius, CLASS_NONE, pOwner); //Attack
					WeaponSound(SPECIAL3);
					pOwner->SetAnimation(PLAYER_SKILL_USE);
					DispatchParticleEffect("aoehint", GetAbsOrigin(), vec3_angle);
									
					pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() - 30);
				}

				m_nSkCoolDownTime2 = gpGlobals->curtime + (sk_plr_skills_2_cooldown_time.GetFloat()*m_flCooldown);
	flSkill_RadialSlash_ActiveTime = gpGlobals->curtime + 2.0f;
	m_bIsSkCoolDown2 = true;
	
}

void CBaseMeleeWeapon::Skill_RadialSlash_LogicEx(void)
{
	trace_t traceHit;
	//Initialize the player pointer
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	//Setup the aiming direction
	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	//Set the ideal activity (for animation)
	Activity nHitActivity = ACT_VM_HITCENTER;

	//Setting up the damage info for the skill
	CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	if (m_SpeedModActiveTime >= gpGlobals->curtime)
		info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
	else if (m_SpeedModActiveTime <= gpGlobals->curtime)
		info.SetDamage(RandomFloat(GetDamageForActivity(nHitActivity), GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage));
	
	info.SetDamagePosition(traceHit.startpos);
	info.SetDamageForce(forward * 10);
	info.ScaleDamage(0.8f);

	//TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	//The area in radius that the skill is going to affect.
	float AoeDamageRadius = 264.0f;
	//HACK! 

	if (m_nExecutionTime > gpGlobals->curtime)
	{
		if (flSkill_RadialSlash_ActiveTime > gpGlobals->curtime)
		{
			if ((flSkill_RadialSlash_ActiveTime - gpGlobals->curtime <= 2.0f) && (flSkill_RadialSlash_ActiveTime - gpGlobals->curtime >= 0.5f))
			{
				m_flSkillAttributeRange = AoeDamageRadius;
				//HACK! This is a really hacky way to do DPS , Todo: make a proper system or function so every skills can be added dps property easily.
				if (gpGlobals->curtime >= m_nSkillHitRefireTime)
				{
					//DevMsg("Evil Slash Hit! \n");
					RadiusDamage(info, UTIL_GetLocalPlayer()->GetAbsOrigin(), AoeDamageRadius, CLASS_NONE, pOwner); //Attack
					AddKnockbackXY(1, 5); //for npc hitting sound
					UTIL_ScreenShake(GetAbsOrigin(), 1.5f, 100.0, 0.5, 256.0f, SHAKE_START);

					//HACK! Reset the timer
					m_nSkillHitRefireTime = gpGlobals->curtime + 0.3f; //delta between refire
					m_flNPCFreezeTime = gpGlobals->curtime + 0.6f;
					//AddKnockbackXY(1.1f, 4);
				}
			}

			if ((flSkill_RadialSlash_ActiveTime - gpGlobals->curtime <= 0.5f) && (flSkill_RadialSlash_ActiveTime - gpGlobals->curtime >= 0.0f))
			{ //Doesnt work because it pushes the enemy outside the player's damage range
				m_flSkillAttributeRange = AoeDamageRadius;

				if (gpGlobals->curtime >= m_nSkillHitRefireTime)
				{
					RadiusDamage(info, UTIL_GetLocalPlayer()->GetAbsOrigin(), AoeDamageRadius, CLASS_NONE, pOwner); //Attack
					AddKnockbackXY(1, 5); //for npc hitting sound

					m_nSkillHitRefireTime = gpGlobals->curtime + 0.5f; //delta between refire
					m_flNPCFreezeTime = gpGlobals->curtime + 0.6f;
					AddKnockbackXY(10, 1);
					UTIL_ScreenShake(GetAbsOrigin(), 2.5f, 115.0, 0.6, 256.0f, SHAKE_START);
				}

			}
		}
	}
}

//Skill 3 
void CBaseMeleeWeapon::Skill_Grenade(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
	//CheckThrowPosition(pPlayer, vecEye, vecSrc);
	//	vForward[0] += 0.1f;
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vForward * 1200;
	//Fraggrenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer, GRENADE_TIMER, false );
	if (!m_bIsSkCoolDown3 && gpGlobals->curtime > m_nSkCoolDownTime3)
	{
		m_nExecutionTime = 1.0f;
		CGrenadeFrag *pFrag = (CGrenadeFrag*)Create("npc_grenade_frag", vecSrc, vec3_angle, GetOwner());
		pFrag->SetThrower(GetOwner());
		pFrag->ApplyAbsVelocityImpulse(vecThrow);
		pFrag->SetLocalAngularVelocity(QAngle(0, 400, 0));
		pFrag->m_bIsLive = true;
		pFrag->SetTimer(2, 2);

		WeaponSound(SINGLE);
		pPlayer->SetAnimation(PLAYER_ATTACK1);

		m_nSkCoolDownTime3 = gpGlobals->curtime + 3.0f;
		m_bIsSkCoolDown3 = true;
	}

	
}


//Skill 3 :Death Grinder
void CBaseMeleeWeapon::Skill_GrenadeEX(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;
	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());

	// Fire the bullets
	Vector vecSrc = pOwner->Weapon_ShootPosition();
	vecSrc.z -= 22;
	Vector vecAiming = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
	Vector impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	// Fire the bullets
	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire!
	m_nExecutionTime = gpGlobals->curtime + 1.0f;
	AddSkillMovementImpulse(2.0f);
	CreateWpnThrowSkill(vecSrc, vecVelocity, 10, 150, 1.5, pOwner);
	UTIL_ScreenShake(GetAbsOrigin(), 3.0f, 130.0, 0.7, 256.0f, SHAKE_START);

	pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() - 25);
	WeaponSound(SINGLE);
	pOwner->SetAnimation(PLAYER_SKILL_USE);

	m_nSkCoolDownTime3 = gpGlobals->curtime + (sk_plr_skills_3_cooldown_time.GetFloat()*m_flCooldown);
	m_bIsSkCoolDown3 = true;


}

//Skill 4 
void CBaseMeleeWeapon::Skill_HealSlash(void)
{
	trace_t traceHit;
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;
	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());

	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());
	Activity nHitActivity = ACT_VM_HITCENTER;

	CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	if (m_SpeedModActiveTime >= gpGlobals->curtime)
		info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
	else if (m_SpeedModActiveTime <= gpGlobals->curtime)
		info.SetDamage(RandomFloat(GetDamageForActivity(nHitActivity), GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage));

	info.SetDamagePosition(traceHit.startpos);
	info.SetDamageForce(forward);
	info.ScaleDamage(2.0f);

	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();
	string_t iszNPCName = AllocPooledString("npc_metropolice");
	Vector playernpcdist;

	GetPlayerAnglesOnce();

	float m_nDamageRadius = 128.0f;
	//Only run when the cooldown time is 0

	m_nExecutionTime = gpGlobals->curtime + 1.5f;
		
	AddKnockbackXY(10.0f, 1);
	RadiusDamage(info, UTIL_GetLocalPlayer()->GetAbsOrigin(), m_nDamageRadius, CLASS_NONE, pOwner);
	//WeaponSound(SINGLE);
	EmitSound("Weapon_Melee.SPECIAL5");
	pOwner->SetAnimation(PLAYER_SKILL_USE);
	DispatchParticleEffect("striderbuster_shotdown_core_flash", GetAbsOrigin(), vec3_angle);

		for (int i = 0; i < nAIs; i++)
		{
			//if (ppAIs[i]->m_iClassname == iszNPCName)
			if (ppAIs[i])
			{
				playernpcdist.x = abs(UTIL_GetLocalPlayer()->GetAbsOrigin().x - ppAIs[i]->GetAbsOrigin().x);
				playernpcdist.y = abs(UTIL_GetLocalPlayer()->GetAbsOrigin().y - ppAIs[i]->GetAbsOrigin().y);

				if (playernpcdist.x <= m_flSkillAttributeRange && playernpcdist.y <= m_flSkillAttributeRange)
				{
					pOwner->SetHealth(pOwner->GetHealth() + 25);
				}
			}
		}
		pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() - 50);

		m_nSkCoolDownTime4 = gpGlobals->curtime + (sk_plr_skills_4_cooldown_time.GetFloat()*m_flCooldown);
		m_bIsSkCoolDown4 = true;

}

//Skill 5
float flSkillTrapping_ActiveTime;
Vector effectpos;
bool bCanPullEnemies = false;
void CBaseMeleeWeapon::Skill_Trapping()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;
	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());

	Activity nHitActivity = ACT_VM_HITCENTER;

	CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	if (m_SpeedModActiveTime >= gpGlobals->curtime)
		info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
	else if (m_SpeedModActiveTime <= gpGlobals->curtime)
		info.SetDamage(RandomFloat(GetDamageForActivity(nHitActivity), GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage));

		Vector fwd;
		AngleVectors(pOwner->GetAbsAngles(), &fwd);
		fwd.z = 0;
		VectorNormalize(fwd);

	
			float fmagnitude = 350;
			effectpos = pOwner->GetAbsOrigin() + (fwd * fmagnitude);
			//effectpos.z = 0;

			//WeaponSound(SPECIAL4);
			EmitSound("Weapon_Melee.SPECIAL4");
			pOwner->SetAnimation(PLAYER_SKILL_USE);
			RadiusDamage(info, effectpos, 192.0f, CLASS_NONE, pOwner);
			m_nExecutionTime = gpGlobals->curtime + 1.0f;
			pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() - 30);
			DispatchParticleEffect("aoehint", effectpos, vec3_angle);

			//Init Cooldown
			m_nSkCoolDownTime5 = gpGlobals->curtime + (sk_plr_skills_5_cooldown_time.GetFloat()*m_flCooldown);
			flSkillTrapping_ActiveTime = gpGlobals->curtime + 3.0f;
			m_bIsSkCoolDown5 = true;
		
}

void CBaseMeleeWeapon::Skill_Trapping_LogicEx(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	float skillrange = 192.0f;
	Vector fwd;
	AngleVectors(pOwner->GetAbsAngles(), &fwd);
	fwd.z = 0;
	VectorNormalize(fwd);

	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();
	string_t iszNPCExcludeName = AllocPooledString("npc_bob");
	Vector SkillOriginNPCdist;

	trace_t traceHit;

	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());
	Activity nHitActivity = ACT_VM_HITCENTER;

		if (flSkillTrapping_ActiveTime > gpGlobals->curtime)
		{
			for (int i = 0; i < nAIs; i++)
			{
				//if (ppAIs[i]->m_iClassname == iszNPCName)
				//if (ppAIs[i]) //affects all NPCs
				if (ppAIs[i]->m_iClassname == iszNPCExcludeName)
				{
					SkillOriginNPCdist.x = abs(effectpos.x - ppAIs[i]->GetAbsOrigin().x);
					SkillOriginNPCdist.y = abs(effectpos.y - ppAIs[i]->GetAbsOrigin().y);
					//SkillOriginNPCdist.z = 0;

					if (flSkillTrapping_ActiveTime - gpGlobals->curtime >= 1.0f)
					{
						if (SkillOriginNPCdist.x <= skillrange && SkillOriginNPCdist.y <= skillrange)
						{
							ppAIs[i]->SetCondition(COND_NPC_FREEZE);
							ppAIs[i]->SetRenderMode(kRenderTransColor);
							ppAIs[i]->SetRenderColor(128, 128, 128, 128);
						}
					}
					else if ((flSkillTrapping_ActiveTime - gpGlobals->curtime <= 1.0f) && (flSkillTrapping_ActiveTime - gpGlobals->curtime >= 0.9f))
					{
						if (SkillOriginNPCdist.x <= skillrange + 30 && SkillOriginNPCdist.y <= skillrange + 30)
						{
							ppAIs[i]->SetCondition(COND_NPC_UNFREEZE);
							ppAIs[i]->SetRenderMode(kRenderNormal);
						}
					}

					if ((flSkillTrapping_ActiveTime - gpGlobals->curtime <= 2.5f) && (flSkillTrapping_ActiveTime - gpGlobals->curtime >= 2.4f))
						bCanPullEnemies = true;

					if (bCanPullEnemies) //Need to make run once only
					{
						if (SkillOriginNPCdist.x <= skillrange && SkillOriginNPCdist.y <= skillrange)
							ppAIs[i]->ApplyAbsVelocityImpulse((effectpos - ppAIs[i]->GetAbsOrigin()));
						UTIL_ScreenShake(GetAbsOrigin(), 3.0f, 130.0, 0.7, 256.0f, SHAKE_START);

						bCanPullEnemies = false;

					}

				}

			}
		}

	
}

//Skill 6
Vector skpos;
float flTorSkillRefireTime;
float flSkillActiveTime;
void CBaseMeleeWeapon::Skill_Tornado(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;
	CHL2_Player* pPlayer = dynamic_cast<CHL2_Player *>(pOwner);
	float skillrange = 224.0f;

	WeaponSound(SPECIAL2);
	pOwner->SetAnimation(PLAYER_SKILL_USE);
	m_flSkillAttributeRange = skillrange;
	m_nExecutionTime = gpGlobals->curtime + 1.0f;
	pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() - 50);

	skpos = pOwner->GetAbsOrigin();

	GetPlayerPosOnce();
	//Init Cooldown
	DispatchParticleEffect("tornado1", skpos, vec3_angle);
	flTorSkillRefireTime = gpGlobals->curtime + 0.3f;
	m_nSkCoolDownTime6 = gpGlobals->curtime + (sk_plr_skills_6_cooldown_time.GetFloat()*m_flCooldown);
	flSkillActiveTime = gpGlobals->curtime + 4.0f;
	m_SpeedModActiveTime = gpGlobals->curtime + 10.0f;
	pPlayer->SetPlayerAttackSpeedBonus(0.5f, 10.f);
	pPlayer->SetPlayerCooldownReductionRateBonus(0.5f, 10.f);
	m_bIsSkCoolDown6 = true;

}

void CBaseMeleeWeapon::Skill_Tornado_LogicEx(void)
{
	//LOOP
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	float skillrange = 224.0f;

	trace_t traceHit;

	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());
	Activity nHitActivity = ACT_VM_HITCENTER;

	CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	if (m_SpeedModActiveTime >= gpGlobals->curtime)
		info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
	else if (m_SpeedModActiveTime <= gpGlobals->curtime)
		info.SetDamage(RandomFloat(GetDamageForActivity(nHitActivity), GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage));

	info.SetDamagePosition(traceHit.startpos);
	info.SetDamageForce(forward);
	info.ScaleDamage(1.5f);
	

	if (m_bIsSkCoolDown6 && gpGlobals->curtime < m_nSkCoolDownTime6)
	{
		if (flSkillActiveTime > gpGlobals->curtime)
		{
			if (flSkillActiveTime - gpGlobals->curtime >= 1.0f)
			{
				m_flSkillAttributeRange = skillrange;

				if (gpGlobals->curtime >= flTorSkillRefireTime)
				{
					AddKnockbackXY(1, 4);
					RadiusDamage(info, skpos, skillrange, CLASS_PLAYER, pOwner);
					AddKnockbackXY(1, 5); //for npc hitting sound
				
					flTorSkillRefireTime = gpGlobals->curtime + 0.3f;
				}
			}
			else if (flSkillActiveTime - gpGlobals->curtime <= 1.0f)
			{
				m_flSkillAttributeRange = skillrange;
				AddKnockbackXY(1, 3);
			}
		}

	}
}


void CBaseMeleeWeapon::AddKnockback(Vector dir)
{
	//UTIL_EntitiesInSphere

	CBaseEntity *pEntity = NULL;
	if ((pEntity = gEntList.FindEntityByClassnameNearest("npc_metropolice", UTIL_GetLocalPlayer()->GetAbsOrigin(), 192.0f)) != NULL)
	{
		//CNPC_MetroPolice *pCop = dynamic_cast<CNPC_MetroPolice *>(pEntity);
		//pCop->ApplyAbsVelocityImpulse(dir);
	}

}

// Use in combination with Vector dirkb
void CBaseMeleeWeapon::GetPlayerAnglesOnce(void)
{
	AngleVectors(UTIL_GetLocalPlayer()->GetAbsAngles(), &dirkb);
	dirkb.z = 0;
	VectorNormalize(dirkb);

	AngleVectors(UTIL_GetLocalPlayer()->GetAbsAngles(), &dirkbZ);
	dirkbZ.x = 0;
	dirkbZ.y = 0;
	VectorNormalize(dirkbZ);
}

Vector playerPos;
void CBaseMeleeWeapon::GetPlayerPosOnce(void)
{
	playerPos = UTIL_GetLocalPlayer()->GetAbsOrigin();
	playerPosInAir = UTIL_GetLocalPlayer()->GetAbsOrigin();

}
void CBaseMeleeWeapon::AddKnockbackXY(float magnitude,int options)
{
	float flKnockbackVelocity = 128.0f*magnitude;
	
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();
	string_t iszNPCName = AllocPooledString("npc_metropolice");
	Vector playernpcdist;
	Vector staticplayernpcdist;
	

	for (int i = 0; i < nAIs; i++)
	{
		//TODO: use exclusion list. 
		//if (ppAIs[i]->m_iClassname == iszNPCName)
		//if (ppAIs[i])  //affects every npcs ingame.
		if (ppAIs[i])
		{
				playernpcdist.x = abs(UTIL_GetLocalPlayer()->GetAbsOrigin().x - ppAIs[i]->GetAbsOrigin().x);
				playernpcdist.y = abs(UTIL_GetLocalPlayer()->GetAbsOrigin().y - ppAIs[i]->GetAbsOrigin().y);

				staticplayernpcdist.x = abs(playerPos.x - ppAIs[i]->GetAbsOrigin().x);
				staticplayernpcdist.y = abs(playerPos.y - ppAIs[i]->GetAbsOrigin().y);

				if (playernpcdist.x <= m_flSkillAttributeRange && playernpcdist.y <= m_flSkillAttributeRange)
				{
					m_iEnemyHealth = ppAIs[i]->GetHealth();


					if (ppAIs[i]->IsAlive())
					{
						m_bIsEnemyInAtkRange = true;						// display text if they are within range
					}
					
					int NPCHealth = ppAIs[i]->GetHealth();
					if (options == 1)
					{
						ppAIs[i]->SetRenderMode(kRenderTransColor);
						ppAIs[i]->SetRenderColor(128, 128, 128, 128);

						if (UTIL_GetLocalPlayer()->GetGroundEntity() != NULL)
						{	
							if ((NPCHealth > 0) && (NPCHealth < sk_npcknockbackathealth.GetInt()))
								ppAIs[i]->ApplyAbsVelocityImpulse(dirkb*flKnockbackVelocity);
						}
						else if (UTIL_GetLocalPlayer()->GetGroundEntity() == NULL)
						{
							if ((NPCHealth > 0) && (NPCHealth < sk_npcknockbackathealth.GetInt()))
								ppAIs[i]->ApplyAbsVelocityImpulse(Vector(0,0,flKnockbackVelocity/2));
						}
					}
					else if (options == 2)
					{
						if ((NPCHealth > 0) && (NPCHealth < sk_npcknockbackathealth.GetInt()))
						ppAIs[i]->SetCondition(COND_NPC_FREEZE);						
						//ppAIs[i]->SetMoveType(MOVETYPE_NONE);
						ppAIs[i]->SetActivity(ACT_IDLE);
					}
					else if (options == 3)
					{
						if ((NPCHealth > 0) && (NPCHealth < sk_npcknockbackathealth.GetInt()))
							ppAIs[i]->SetCondition(COND_NPC_UNFREEZE);

						ppAIs[i]->SetRenderMode(kRenderNormal);
					}
					else if (options == 5)
					{
						if (ppAIs[i]->IsAlive())
						WeaponSound(MELEE_HIT);
						
					}
					
						
				}
				else
				{
					m_bIsEnemyInAtkRange = false;
				}
				
				if (staticplayernpcdist.x <= m_flSkillAttributeRange && staticplayernpcdist.y <= m_flSkillAttributeRange)
				{
					if (options == 4)
					{
						//m_iEnemyHealth = ppAIs[i]->GetHealth();
						ppAIs[i]->SetCondition(COND_NPC_FREEZE);
						ppAIs[i]->SetRenderMode(kRenderTransColor);
						ppAIs[i]->SetRenderColor(128, 128, 128, 128);
						ppAIs[i]->ApplyAbsVelocityImpulse(Vector(0, 0, 212));
					}
				}



		}
	}
}
//Make the player move forward
void CBaseMeleeWeapon::AddSkillMovementImpulse(float magnitude)
{
	float nStepVelocity = 128.0f * magnitude;

	if (UTIL_GetLocalPlayer()->GetGroundEntity() != NULL)
	{
		//Initialize vector fwd
		Vector fwd;
		//Get the player's viewangle and copy it to the fwd vector
		AngleVectors(UTIL_GetLocalPlayer()->GetAbsAngles(), &fwd);
		//Make sure the player won't accelerating upward when using the skill
		fwd.z = 0;
		//Normalize the vector so as not to making the value used going out of control
		VectorNormalize(fwd);
		//Give the player the push they need 
		UTIL_GetLocalPlayer()->SetAbsVelocity(fwd*nStepVelocity);
	}
}

bool CBaseMeleeWeapon::ShouldCollide(int collisionGroup, int contentsMask) const
{	
		if (collisionGroup == COLLISION_GROUP_PLAYER || collisionGroup == COLLISION_GROUP_NPC)
			return false;

	return BaseClass::ShouldCollide(collisionGroup, contentsMask);
}