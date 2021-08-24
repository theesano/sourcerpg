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
extern ConVar sk_plr_skills_1_cd("sk_plr_skills_1_cd", "0");
extern ConVar sk_plr_skills_2_cd("sk_plr_skills_2_cd", "0");
extern ConVar sk_plr_skills_3_cd("sk_plr_skills_3_cd", "0");
extern ConVar sk_plr_skills_4_cd("sk_plr_skills_4_cd", "0");
extern ConVar sk_plr_skills_5_cd("sk_plr_skills_5_cd", "0");
extern ConVar sk_plr_skills_6_cd("sk_plr_skills_6_cd", "0");
extern ConVar sk_plr_skills_7_cd("sk_plr_skills_7_cd", "0");

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
	m_bIsSkCoolDown6 = false;
	m_nSkCoolDownTime6 = 0.0f;
	m_bIsSkCoolDown7 = false;
	m_nSkCoolDownTime7 = 0.0f;

	m_nExecutionTime = 0.0f;//This Var is Tied to Camera control in "in_camera" , be advised when changing it.
	m_nSkillHitRefireTime = 0.0f;

	m_bWIsAttack1 = true; // never change this 
	m_bWIsAttack2 = false;
	m_bWIsAttack3 = false;
	m_bWIsAttack2 = false;
	m_bWIsAttack3 = false;

	m_bIsNmAttack = false;
	m_bIsNmAttack2 = false;
	m_bIsNmAttack4 = false;
	m_bNmAttackSPEvade = false;
	m_bAttackSPAir2 = false;
	m_bNmAirAttack = false;
	m_bNPCFreezeAerial = false;

	m_bIsAICollisionOff = false;

	m_bSkillLiftAttack = false;

	m_flDamageBuffActiveTime = 0.0f;
	m_flNPCFreezeTime = 0.0f;
	m_flSkillAttributeRange = 0.0f;
	m_flTotalAttackTime = 0.0f;
	m_flInAirTime = 0.0f;
	m_flHealSlashDelayTimer = 0.0f;
	m_flSkillTrapping_ActiveTime = 0.0f;
	m_flSkillLiftAttackDelayTimer = 0.0f;


	m_bIsHealSlashAttacking = false;

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
	PrecacheParticleSystem("aoehint2_vertical");
	PrecacheParticleSystem("aoehint22");
	PrecacheParticleSystem("aoehint4");
	PrecacheParticleSystem("striderbuster_shotdown_core_flash");
	PrecacheParticleSystem("choreo_skyflower_nexus");
	PrecacheParticleSystem("tornado1");
	PrecacheParticleSystem("hit_impact");
	PrecacheParticleSystem("grenade_explosion_01e");
	PrecacheModel("models/weapons/melee/alt/guideshape.mdl");

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
	CHL2_Player *pHLOwner = dynamic_cast<CHL2_Player *>(pOwner);

	//DevMsg("Is player running %i \n", pHLOwner->m_bIsRunning);

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

	InflictNormalAttackDamage();
	
	//if ((m_bIsAICollisionOff) && (gpGlobals->curtime >= m_flAICollisionOffTime))
	//{
	//	m_bIsAICollisionOff = false;
	//}

	if (m_nExecutionTime - gpGlobals->curtime <= 0)
	{

			if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
			{
				if (!pHLOwner->m_bIsRunning)
					PrimaryAttack();
				else
				{ 
					if (pOwner->GetGroundEntity() != NULL)
					{
						if (pOwner->m_afButtonPressed & IN_ATTACK)
							Skill_SprintAttack();
					}
					else
						PrimaryAttack();
				}
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

	// Debug
	//CStudioHdr *pModel = GetModelPtr();
	//if (pModel)
	//{
	//	Msg(
	//		"Debug for entity: %i\n" \
		//		"Model: %s\n" \
		//		"Attachments: %i\n",
	//		entindex(),
	//		STRING(GetModelName()),
	//		pModel->GetNumAttachments()
	//		);
	//}
	//else
	//	Warning("%s model pointer is faulty!\n", STRING(GetModelName()));



}

//void CBaseMeleeWeapon::SetAICollisionOffTime(float flDuration)
//{
//	m_bIsAICollisionOff = true;
//	m_flAICollisionOffTime = gpGlobals->curtime + flDuration;
//}

ConVar sk_plr_quickslot1_skill_id("sk_plr_quickslot1_skill_id", "0", FCVAR_ARCHIVE);
ConVar sk_plr_quickslot2_skill_id("sk_plr_quickslot2_skill_id", "0", FCVAR_ARCHIVE);
ConVar sk_plr_quickslot3_skill_id("sk_plr_quickslot3_skill_id", "0", FCVAR_ARCHIVE);
ConVar sk_plr_quickslot4_skill_id("sk_plr_quickslot4_skill_id", "0", FCVAR_ARCHIVE);
ConVar sk_plr_quickslot5_skill_id("sk_plr_quickslot5_skill_id", "0", FCVAR_ARCHIVE);
ConVar sk_plr_quickslot6_skill_id("sk_plr_quickslot6_skill_id", "0", FCVAR_ARCHIVE);

//m_nExecutionTime handles freezing the player for a certain amount of time
void CBaseMeleeWeapon::SkillsHandler(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());


	m_flPlayerStats_BaseDamage = pPlayer->GetPlayerBaseDamage();
	//m_flPlayerStats_AttackSpeed = pPlayer->GetPlayerAttackSpeed();
	m_flPlayerStats_CritDamage = pPlayer->GetPlayerCritDamage();
	m_bIsCritical = pPlayer->IsCritical();

	m_flPlayerStats_AttackSpeed = 1 / pPlayer->GetPlayerAttackSpeed();

	Activity nHitActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;

	m_flCooldown = 1/pPlayer->GetPlayerCooldownReductionRate();

	
	/*CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);*/

	//Warning("Attack speed %.2f \n", sk_atkspeedmod.GetFloat());
	//if (m_flDamageBuffActiveTime >= gpGlobals->curtime )
	//{
	//}
	//else if (m_flDamageBuffActiveTime <= gpGlobals->curtime )
	//{
	//	//Reason why some stats can't be changed on the character panel

	//}

	//Freeze NPC Time 
	if (m_flNPCFreezeTime > gpGlobals->curtime)
		AddKnockbackXY(1.0f, 2);
	else
		AddKnockbackXY(1.0f, 3);
	
	//Skill 1 Special Evade
	//if ((pOwner->m_nButtons & IN_ATTACK2) && !m_bIsSkCoolDown)
	//{
	//	if (m_nExecutionTime - gpGlobals->curtime <= 0)
	//	{
	//		GetPlayerAnglesOnce();
	//		Skill_Evade();
	//	}
	//	
	//}


	// a hack for aerial movement or something
	if (m_flInAirTime >= gpGlobals->curtime)
	{
		playerPosInAir.z += 0.15;
		UTIL_GetLocalPlayer()->SetAbsOrigin(playerPosInAir);
		UTIL_GetLocalPlayer()->SetAbsVelocity(vec3_origin);
	
	}

	if (pOwner->m_nButtons & IN_ATTACK2)
	{
		ExecuteSkillID(1);
	}

	if (pOwner->m_afButtonPressed & IN_SLOT1)
	{
		ExecuteSkillID(sk_plr_quickslot1_skill_id.GetInt()); //2
	}

	if (pOwner->m_afButtonPressed & IN_SLOT2)
	{
		ExecuteSkillID(sk_plr_quickslot2_skill_id.GetInt()); //3
	}

	if (pOwner->m_afButtonPressed & IN_SLOT3)
	{
		ExecuteSkillID(sk_plr_quickslot3_skill_id.GetInt());//4
	}

	if (pOwner->m_afButtonPressed & IN_SLOT4)
	{
		ExecuteSkillID(sk_plr_quickslot4_skill_id.GetInt()); //5
	}

	if (pOwner->m_afButtonPressed & IN_SLOT5)
	{
		ExecuteSkillID(sk_plr_quickslot5_skill_id.GetInt()); //6
	}

	if (pOwner->m_afButtonPressed & IN_SLOT6)
	{
		ExecuteSkillID(sk_plr_quickslot6_skill_id.GetInt()); //7
	}

	if (pOwner->m_afButtonPressed & IN_RAGE)
	{
		ExecuteSkillID(8);
	}

	Skill_RadialSlash_LogicEx();
	Skill_HealSlash_LogicEx();
	Skill_Trapping_LogicEx();
	Skill_Tornado_LogicEx();
	Skill_Lift_LogicEx();

// OLD Input code, left for later analysis of the evade bug that doesn't allow player to gain evade speed, again!
	//Run the Evil Slash skill code
//	if ((pOwner->m_afButtonPressed & IN_SLOT1) && !m_bIsSkCoolDown2)
//	{
//		if (pPlayer->GetPlayerMP() > 30)
//		{
//			if (!m_bIsSkCoolDown2)
//			{
//				if (m_nExecutionTime - gpGlobals->curtime <= 0)
//					Skill_RadialSlash();
//			}
//		}
//		else
//		{
//			//Print SG insufficient warning on player's screen
//			SkillStatNotification_HUD(1);
//		}
//
//	}
//	else if ((pOwner->m_afButtonPressed & IN_SLOT1) && m_bIsSkCoolDown2)
//		SkillStatNotification_HUD(2);
//
//
//	// Skill 3 loop
//	if ((pOwner->m_afButtonPressed & IN_SLOT2) && !m_bIsSkCoolDown3)
//	{
//		if (pPlayer->GetPlayerMP() > 25)
//		{
//			if (!m_bIsSkCoolDown3)
//			{
//				if (m_nExecutionTime - gpGlobals->curtime <= 0)
//					Skill_GrenadeEX();
//			}
//		}
//		else
//			SkillStatNotification_HUD(1);
//	}
//	else if ((pOwner->m_afButtonPressed & IN_SLOT2) && m_bIsSkCoolDown3)
//	{
//		SkillStatNotification_HUD(2);
//
//	}
//		
//	//Skill 4 loop
//	if ((pOwner->m_afButtonPressed & IN_SLOT3) && !m_bIsSkCoolDown4)
//	{
//		if (pPlayer->GetPlayerMP() > 50)
//		{
//			if (!m_bIsSkCoolDown4)
//			{ 
//				if (m_nExecutionTime - gpGlobals->curtime <= 0)
//					Skill_HealSlash();
//			}
//		}
//		else
//			SkillStatNotification_HUD(1);
//
//	}
//	else if ((pOwner->m_afButtonPressed & IN_SLOT3) && m_bIsSkCoolDown4)
//		SkillStatNotification_HUD(2);
//
//
//		//Skill 5 loop
//	if (pOwner->m_afButtonPressed & IN_SLOT4 && !m_bIsSkCoolDown5)
//	{
//		if (pPlayer->GetPlayerMP() > 30)
//		{
//			if (!m_bIsSkCoolDown5)
//			{
//				if (m_nExecutionTime - gpGlobals->curtime <= 0)
//					Skill_Trapping();
//			}
//		}
//		else
//		{
//			SkillStatNotification_HUD(1);
//		}
//	}
//	else if ((pOwner->m_afButtonPressed & IN_SLOT4) && & m_bIsSkCoolDown5)
//		SkillStatNotification_HUD(2);
//
//
//
//		//Skill 6 loop
//	if (pOwner->m_afButtonPressed & IN_SLOT5 && !m_bIsSkCoolDown6)
//	{
//		if (pPlayer->GetPlayerMP() > 50)
//		{
//			if (!m_bIsSkCoolDown6)
//			{
//				if (m_nExecutionTime - gpGlobals->curtime <= 0)
//					Skill_Tornado();
//			}
//		}
//		else
//		{
//			SkillStatNotification_HUD(1);
//		}
//	}
//	else if (pOwner->m_afButtonPressed & IN_SLOT5 && m_bIsSkCoolDown6)
//	{
//		SkillStatNotification_HUD(2);
//
//	}
//
////Skill_Lift activation input
//
//	// Skill 7 loop
//	if ((pOwner->m_afButtonPressed & IN_SLOT6) && !m_bIsSkCoolDown7)
//	{
//		if (!m_bIsSkCoolDown7)
//		{
//			if (m_nExecutionTime - gpGlobals->curtime <= 0)
//				Skill_Lift();
//		}
//		
//	}
//	else if ((pOwner->m_afButtonPressed & IN_SLOT6) && m_bIsSkCoolDown7)
//	{
//		SkillStatNotification_HUD(2);
//
//	}
//
//	if ((pOwner->m_afButtonPressed & IN_RAGE) && (pPlayer->GetPlayerRage() >= 100 ))
//	{
//		Msg("RAGE ENABLED \n");
//		Skill_RageOn();
//	}

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

	if (m_nSkCoolDownTime7 - gpGlobals->curtime >= 0)
	{
		//engine->Con_NPrintf(15, "Skill 7 Cooldown time  %6.1f   ", m_nSkCoolDownTime7 - gpGlobals->curtime);
	}
	else
	{
		m_bIsSkCoolDown7 = false;
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
	if (m_nExecutionTime >= gpGlobals->curtime)
	{
		pl_isattacking.SetValue(1);
		m_bIsAttacking = true;
	}
	else
	{
		pl_isattacking.SetValue(0);
		m_bIsAttacking = false;
	}

	
}

void CBaseMeleeWeapon::SkillStatNotification(void)
{
	//External ( using CVars as synced variables between servers and client )
	//Display Info for HUDs

	int skill1cdtimer = m_nSkCoolDownTime - gpGlobals->curtime;
	sk_plr_skills_1_cd.SetValue(skill1cdtimer);

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

	int skill7cdtimer = m_nSkCoolDownTime7 - gpGlobals->curtime;
	sk_plr_skills_7_cd.SetValue(skill7cdtimer);


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

		//if (messageoption == 1)
		//{
		//	UTIL_HudMessage(pOwner, tTextParam, "SG insufficient");
		//	//ClientPrint(pOwner, HUD_PRINTCENTER, "SG insufficient");
		//}
		//else if (messageoption == 2)
		//{
		//	UTIL_HudMessage(pOwner, tTextParam, "Skill Not Ready");
		//}
		//ClientPrint(pOwner, HUD_PRINTCENTER, "SG insufficient");
		
		switch (messageoption)
		{
		case 0:
			UTIL_HudMessage(pOwner, tTextParam, "Not Assigned");			
			break;
		case 1:
			UTIL_HudMessage(pOwner, tTextParam, "SG insufficient");
			break;
		case 2:
			UTIL_HudMessage(pOwner, tTextParam, "Skill Not Ready");
			break;
		default: UTIL_HudMessage(pOwner, tTextParam, "Not Assigned");
		}
	
}

void CBaseMeleeWeapon::ExecuteSkillID(int skillID)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());

	switch (skillID)
	{
		case 0:
			SkillStatNotification_HUD(0);
			break;
		case 1: //Skill 1
		{
			if (!m_bIsSkCoolDown)
			{
				if (m_nExecutionTime - gpGlobals->curtime <= 0)
				{
					
					GetPlayerAnglesOnce();
					Skill_Evade();
				}

			}
			break;
		}
		case 2: //Skill 2
		{
			if (!m_bIsSkCoolDown2)
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
			else
			{
				SkillStatNotification_HUD(2);

			}
			break;
		}
		case 3: //Skill 3
		{
			if (!m_bIsSkCoolDown3)
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
			else
			{
				SkillStatNotification_HUD(2);

			}
			break;
		}
		case 4: //Skill 4
		{
			if (!m_bIsSkCoolDown4)
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
			else
			{
				SkillStatNotification_HUD(2);

			}
			break;
		}
		case 5: //Skill 5
		{
			if (!m_bIsSkCoolDown5)
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
			else
			{
				SkillStatNotification_HUD(2);

			}
			break;
		}
		case 6: //Skill 6
		{
			if (!m_bIsSkCoolDown6)
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
			else
			{
				SkillStatNotification_HUD(1);

			}
			break;
		}
		case 7: //Skill 7
		{
			if (!m_bIsSkCoolDown7)
			{
				if (!m_bIsSkCoolDown7)
				{
					if (m_nExecutionTime - gpGlobals->curtime <= 0)
						Skill_Lift();
				}

			}
			else
			{
				SkillStatNotification_HUD(2);

			}
			break;
		}
		case 8: //Skill_8
		{
			if (pPlayer->GetPlayerRage() >= 100)
				Skill_RageOn();
			break;
		}
		default:
			SkillStatNotification_HUD(2);
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
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());
	pPlayer->StopRunning();

	float AoeDamageRadius = sk_plr_melee_normal_range.GetFloat();

	Activity nHitActivity = ACT_MELEE_ATTACK1;
	
	//Move player forward for each swing.
	AddSkillMovementImpulse(2.0f);
//	DevMsg("%i \n", iScytheBlade);
	
	int iScytheBlade;

	CBaseAnimating *pAnimating = this->GetBaseAnimating();
	if (pAnimating)
	{
		iScytheBlade = pAnimating->LookupAttachment("blade");
	}
	
	//DispatchParticleEffect("aoehint2", PATTACH_POINT_FOLLOW, this, iScytheBlade, true);
	DispatchParticleEffect("aoehint2", PATTACH_ABSORIGIN_FOLLOW, this, iScytheBlade, true);
	
	//Vector particlepos = GetAbsOrigin();
		//DispatchParticleEffect("aoehint2_vertical", particlepos + Vector(0,0,40), QAngle(90, 0, 90));

	m_iPrimaryAttacks++;

	// Send the anim
	SendWeaponAnim(nHitActivity);
	GetPlayerAnglesOnce();
	//Setup our next attack times
	UTIL_ScreenShake(GetAbsOrigin(), 2.4f, 100.0, 0.5, 256.0f, SHAKE_START);

	//Air
	if (pOwner->GetGroundEntity() == NULL)
	{
		GetPlayerPosOnce();
		m_flInAirTime = gpGlobals->curtime + 0.6666f; //what's this?
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
			WeaponSound(ATTACK1);
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = false;
			m_bWIsAttack2 = true;
			m_bWIsAttack3 = false;
			m_bWIsAttack4 = false;
		//	m_bWIsAttack5 = false;

			m_bIsNmAttack = true;
			m_flNmAttackTimer = gpGlobals->curtime + (0.3f * m_flPlayerStats_AttackSpeed);

			//m_nExecutionTime = gpGlobals->curtime + (0.6666f * speedmod );
			m_nExecutionTime = gpGlobals->curtime + (0.6f * m_flPlayerStats_AttackSpeed);
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* m_flPlayerStats_AttackSpeed); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* m_flPlayerStats_AttackSpeed) + m_flAnimTime;

		}
		else if (m_bWIsAttack2 == true)
		{
			//AoeDamageRadius = 144.0f;
			m_flSkillAttributeRange = AoeDamageRadius;
			WeaponSound(ATTACK2);
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = false;
			m_bWIsAttack2 = false;
			m_bWIsAttack3 = true;
			m_bWIsAttack4 = false;
		//	m_bWIsAttack5 = false;

			m_bIsNmAttack2 = true;
			m_flNmAttackTimer2 = gpGlobals->curtime + (0.5f * m_flPlayerStats_AttackSpeed);
			m_flNmAttackTimer2_rp = gpGlobals->curtime + (0.3f* m_flPlayerStats_AttackSpeed);
			
			m_nExecutionTime = gpGlobals->curtime + (0.6f * m_flPlayerStats_AttackSpeed);
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* m_flPlayerStats_AttackSpeed); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* m_flPlayerStats_AttackSpeed) + m_flAnimTime;

		}
		else if (m_bWIsAttack3 == true)
		{
			//AoeDamageRadius = 192.0f;
			m_flSkillAttributeRange = AoeDamageRadius;
			WeaponSound(ATTACK3);
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = false;
			m_bWIsAttack2 = false;
			m_bWIsAttack3 = false;
			m_bWIsAttack4 = true;
	//		m_bWIsAttack5 = false;

			m_bIsNmAttack = true;
			m_flNmAttackTimer = gpGlobals->curtime + (0.3f * m_flPlayerStats_AttackSpeed);

			m_nExecutionTime = gpGlobals->curtime + (0.6f * m_flPlayerStats_AttackSpeed);
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* m_flPlayerStats_AttackSpeed); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* m_flPlayerStats_AttackSpeed) + m_flAnimTime;

		}
		else if (m_bWIsAttack4 == true)
		{
			EmitSound("Weapon_Melee.ATTACK4");
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = true; // = false for combo step 5
			m_bWIsAttack2 = false;
			m_bWIsAttack3 = false; 
			m_bWIsAttack4 = false; 
		//	m_bWIsAttack5 = true;

			m_bIsNmAttack4 = true;
			m_flNmAttackTimer4 = gpGlobals->curtime + (0.8f * m_flPlayerStats_AttackSpeed);
			m_flNmAttackTimer4_rp = gpGlobals->curtime;

			m_nExecutionTime = gpGlobals->curtime + (1.0f * m_flPlayerStats_AttackSpeed);

			m_flNextPrimaryAttack = gpGlobals->curtime + (1.0f * m_flPlayerStats_AttackSpeed);

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* m_flPlayerStats_AttackSpeed) + (m_flAnimTime + 0.4f);

		}
		//else if (m_bWIsAttack5 == true)
		//{
		//	EmitSound("Weapon_Melee.ATTACK5");
		//	pOwner->SetAnimation(PLAYER_ATTACK1);
		//	m_bWIsAttack1 = true; // end of the chain
		//	m_bWIsAttack2 = false;
		//	m_bWIsAttack3 = false;
		//	m_bWIsAttack4 = false;
		//	m_bWIsAttack5 = false;
		//	AddKnockbackXY(3.0f, 1);
		//	AddKnockbackXY(1, 5); //for npc hitting sound
		//	if (m_bIsEnemyInAtkRange)
		//		pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() + sk_plr_melee_mp_bonus.GetInt()+4);


		//	m_nExecutionTime = gpGlobals->curtime + (1.0f * speedmod);

		//	m_flNextPrimaryAttack = gpGlobals->curtime + (1.0f * speedmod);

		//	m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* speedmod) + (m_flAnimTime + 0.4f);

		//}
	}
	else if (pOwner->GetGroundEntity() == NULL)
	{
		if (m_bWIsAttack1 == true)
		{
			AoeDamageRadius = 128.0f;
			m_flSkillAttributeRange = AoeDamageRadius;
			EmitSound("Weapon_Melee.AIRATTACK1");
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = false;
			m_bWIsAttack2 = true;
			m_bWIsAttack3 = false;
			m_bWIsAttack4 = false;
			m_bWIsAttack5 = false;

			//m_bIsNmAttack = true;
			//m_flNmAttackTimer = gpGlobals->curtime + (0.3f * speedmod);

			m_bNmAirAttack = true;
			m_flNmAirAttackDelayTimer = gpGlobals->curtime + (0.3 * m_flPlayerStats_AttackSpeed);

			m_nExecutionTime = gpGlobals->curtime + (0.6666f * m_flPlayerStats_AttackSpeed);
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* m_flPlayerStats_AttackSpeed); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* m_flPlayerStats_AttackSpeed) + m_flAnimTime;

		}
		else if (m_bWIsAttack2 == true)
		{
			//AoeDamageRadius = 144.0f;
			m_flSkillAttributeRange = AoeDamageRadius;
			EmitSound("Weapon_Melee.AIRATTACK2");
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = false;
			m_bWIsAttack2 = false;
			m_bWIsAttack3 = true;
			m_bWIsAttack4 = false;
			m_bWIsAttack5 = false;


			m_bNmAirAttack = true;
			m_flNmAirAttackDelayTimer = gpGlobals->curtime + (0.3 * m_flPlayerStats_AttackSpeed);
			
			m_nExecutionTime = gpGlobals->curtime + (0.6666f * m_flPlayerStats_AttackSpeed);
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* m_flPlayerStats_AttackSpeed); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* m_flPlayerStats_AttackSpeed) + m_flAnimTime;

		}
		else if (m_bWIsAttack3 == true)
		{
			//AoeDamageRadius = 192.0f;
			m_flSkillAttributeRange = AoeDamageRadius;
			EmitSound("Weapon_Melee.AIRATTACK3");
			pOwner->SetAnimation(PLAYER_ATTACK1);
			m_bWIsAttack1 = true;
			m_bWIsAttack2 = false;
			m_bWIsAttack3 = false;
			m_bWIsAttack4 = false;
			m_bWIsAttack5 = false;

			m_bNmAirAttack = true;
			m_flNmAirAttackDelayTimer = gpGlobals->curtime + (0.3 * m_flPlayerStats_AttackSpeed);

			m_nExecutionTime = gpGlobals->curtime + (0.6666f * m_flPlayerStats_AttackSpeed);
			m_flNextPrimaryAttack = gpGlobals->curtime + (GetFireRate()* m_flPlayerStats_AttackSpeed); 	//Hard coded value, should change to SequenceDuration()

			m_flTotalAttackTime = gpGlobals->curtime + (m_flAttackInterval* m_flPlayerStats_AttackSpeed) + m_flAnimTime;

		}
	}


}


void CBaseMeleeWeapon::InflictNormalAttackDamage(void)
{
	if ((m_bIsNmAttack) && (gpGlobals->curtime >= m_flNmAttackTimer))
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());

		float AoeDamageRadius = sk_plr_melee_normal_range.GetFloat();

		Activity nHitActivity = ACT_MELEE_ATTACK1;

		// Damage info
		CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
		if (m_flDamageBuffActiveTime >= gpGlobals->curtime)
			info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
		else if (m_flDamageBuffActiveTime <= gpGlobals->curtime)
		{
			if (m_bIsCritical)
			{
				info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_CritDamage); //critical
			}
			else
			{
				info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage);
			}
		}

		m_flNPCFreezeTime = gpGlobals->curtime + 0.6f;
		if (!m_bAttackSPAir2)
		{
				AddKnockbackXY(2.0f, 1);
			//Makes weapon produce AoE damage
			RadiusDamage_EX(info, GetWeaponAimDirection(), AoeDamageRadius, CLASS_NONE, pOwner, true);
		}
		else
		{
			RadiusDamage_EX(info, pOwner->GetAbsOrigin(), AoeDamageRadius, CLASS_NONE, pOwner, true);
		
		}

		AddKnockbackXY(1, 5); //for npc hitting sound
		//Give the player MP for each enemy hit
		if (m_bIsEnemyInAtkRange)
			pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() + sk_plr_melee_mp_bonus.GetInt());


		m_bIsNmAttack = false;
	}

	if ((m_bNmAirAttack) && (gpGlobals->curtime >= m_flNmAirAttackDelayTimer))
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		Activity nHitActivity = ACT_MELEE_ATTACK1;

		CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
		if (m_flDamageBuffActiveTime >= gpGlobals->curtime)
			info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
		else if (m_flDamageBuffActiveTime <= gpGlobals->curtime)
		{
			if (m_bIsCritical)
			{
				info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_CritDamage); //critical
			}
			else
			{
				info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage);
			}
		}
		m_flNPCFreezeTime = gpGlobals->curtime + 0.6f;

		//Freeze NPC in air
		m_bNPCFreezeAerial = true;
		m_flNPCFreezeAerialTime = gpGlobals->curtime + 0.4f;
	
		AddKnockbackXY(2.0f, 1);

		RadiusDamage_EX(info, pOwner->GetAbsOrigin(), 128.0f, CLASS_PLAYER, pOwner, true);

		m_bNmAirAttack = false;
	}
	

	if ((m_bIsNmAttack2) && (m_flNmAttackTimer2 >= gpGlobals->curtime))
	{
		if (gpGlobals->curtime >= m_flNmAttackTimer2_rp)
		{
			m_bIsNmAttack = true;
			m_flNmAttackTimer = gpGlobals->curtime;
			m_flNmAttackTimer2_rp = gpGlobals->curtime + (0.0667f * m_flPlayerStats_AttackSpeed); // 3 hits in 0.2 seconds
		}

	}
	else
	{
		m_bIsNmAttack2 = false;
	}

	if ((m_bIsNmAttack4) && (m_flNmAttackTimer4 >= gpGlobals->curtime))
	{	
		if (gpGlobals->curtime >= m_flNmAttackTimer4_rp)
		{
			m_bIsNmAttack = true;
			m_flNmAttackTimer = gpGlobals->curtime;
			m_flNmAttackTimer4_rp = gpGlobals->curtime + (0.2666f * m_flPlayerStats_AttackSpeed);
		}
			
	}
	else
	{
		m_bIsNmAttack4 = false;
	}

	if ((m_bNmAttackSPEvade) && (m_flNmAttackSPEvadeTimer >= gpGlobals->curtime))
	{
		if (gpGlobals->curtime >= m_flNmAttackSPEvadeTimer_rp)
		{
			m_bIsNmAttack = true;
			//m_flNmAttackSPEvadeTimer = gpGlobals->curtime;
			m_flNmAttackSPEvadeTimer_rp = gpGlobals->curtime + (0.2666f * m_flPlayerStats_AttackSpeed);
		}

	}
	else
	{
		m_bNmAttackSPEvade = false;
		m_bIsAICollisionOff = false;
	}

	if ((m_bAttackSPAir2) && (m_flAttackSPAir2Timer >= gpGlobals->curtime))
	{
		if (gpGlobals->curtime >= m_flAttackSPAir2Timer_rp)
		{
			m_bIsNmAttack = true;
			AddKnockbackXY(1, 6);
			m_flAttackSPAir2Timer_rp = gpGlobals->curtime + (0.175f * m_flPlayerStats_AttackSpeed);

		}
	}
	else
	{
		m_bAttackSPAir2 = false;
	}

	//Freeze NPC in air: Actual implementation
	if ((m_bNPCFreezeAerial) && (m_flNPCFreezeAerialTime >= gpGlobals->curtime))
	{
		AddKnockbackXY(1, 8);
	}
	else
	{
		m_bNPCFreezeAerial = false;
	}
	

}

void CBaseMeleeWeapon::Skill_SprintAttack(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	CHL2_Player *pHLOwner = dynamic_cast<CHL2_Player *>(ToBasePlayer(GetOwner()));

	if (pOwner->GetGroundEntity() != NULL)
	{
		//Initialize vector fwd
		Vector fwd;
		//Get the player's viewangle and copy it to the fwd vector
		AngleVectors(UTIL_GetLocalPlayer()->GetAbsAngles(), &fwd);
		//Make sure the player won't accelerating upward when using the skill
		fwd.z = 0;
		//Normalize the vector so as not to making the value used going out of control
		VectorNormalize(fwd);

		pOwner->ApplyAbsVelocityImpulse(fwd * 644);
		
		//Attack delay 0.447s
		//put this in a separate fuction
		m_bIsNmAttack = true;
		m_flNmAttackTimer = gpGlobals->curtime + (0.447f * m_flPlayerStats_AttackSpeed);

		pOwner->SetAnimation(PLAYER_SKILL_AERIAL);
		pHLOwner->ForceViewAngleToCamera(1.0f * m_flPlayerStats_AttackSpeed);
		m_nExecutionTime = gpGlobals->curtime + (1.0f *m_flPlayerStats_AttackSpeed);

		pHLOwner->StopRunning();

	}

}

void CBaseMeleeWeapon::Skill_SprintAttack_LogicEx(void)
{

}

//Skill1: Spinning Demon.
void CBaseMeleeWeapon::Skill_Evade(void)
{

	trace_t traceHit;
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	CHL2_Player *pHLPlayer = dynamic_cast<CHL2_Player *>(ToBasePlayer(GetOwner()));
	pHLPlayer->StopRunning();

	float nStepVelocity = 1024.0f;

	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	//Use Base Damage
	Activity nHitActivity = ACT_VM_HITCENTER;

	float AoeDamageRadius = 192.0f;
	//Only run when the cooldown time is 0


	//Plummet 
	Vector aFwd;
	aFwd = dirkb;
	aFwd.z -= 0.5;

	m_bIsAICollisionOff = true;

	if (pOwner->GetGroundEntity() != NULL)
	{
		pOwner->SetAbsVelocity(dirkb*(nStepVelocity*2.0));
		m_nExecutionTime = gpGlobals->curtime + (1.0f * m_flPlayerStats_AttackSpeed);
		pHLPlayer->ForceViewAngleToCamera(1.0f * m_flPlayerStats_AttackSpeed);
		
		pHLPlayer->SetIgnoreSpeedClampDuration(0.1f);
		
		m_bNmAttackSPEvade = true;
		m_flNmAttackSPEvadeTimer = gpGlobals->curtime + (0.8f * m_flPlayerStats_AttackSpeed);
		pOwner->SetAnimation(PLAYER_SKILL_USE);

	}
	else if (pOwner->GetGroundEntity() == NULL) //Plummet
	{
		m_flInAirTime = 0.0f;
		m_nExecutionTime = gpGlobals->curtime + (1.6f * m_flPlayerStats_AttackSpeed);
		pOwner->SetAbsVelocity(aFwd*nStepVelocity);
		pHLPlayer->ForceViewAngleToCamera(1.0f * m_flPlayerStats_AttackSpeed);
		pHLPlayer->SetAnimation(PLAYER_SKILL_AERIAL);

		int iScytheBlade;

		CBaseAnimating *pAnimating = this->GetBaseAnimating();
		if (pAnimating)
		{
			iScytheBlade = pAnimating->LookupAttachment("blade");
		}
		DispatchParticleEffect("aoehint2_vertical", PATTACH_ABSORIGIN_FOLLOW, this, iScytheBlade, true);

		m_bAttackSPAir2 = true;
		m_flAttackSPAir2Timer = gpGlobals->curtime + (0.7f * m_flPlayerStats_AttackSpeed);


	}

	//SetAICollisionOffTime(1.0f);


	UTIL_ScreenShake(GetAbsOrigin(), 2.0f, 100.0, 0.7, 256.0f, SHAKE_START);

	EmitSound("Weapon_Melee.SPEVADE");
	
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
	pPlayer->StopRunning();

	float AoeDamageRadius = 264.0f;
	Activity nHitActivity = ACT_VM_IDLE;
	CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	if (m_flDamageBuffActiveTime >= gpGlobals->curtime)
		info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
	else if (m_flDamageBuffActiveTime <= gpGlobals->curtime)
	{
		if (pPlayer->IsCritical() == true)
			info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_CritDamage); //critical
		else
		{
			info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage);
		}
	}

				//Initialize the variable for moving the player on each attack
	AddSkillMovementImpulse(1.0f);
	GetPlayerAnglesOnce();
	m_nExecutionTime = gpGlobals->curtime + (2.0f * m_flPlayerStats_AttackSpeed);
				//HACK! Fire the timer
	m_nSkillHitRefireTime = gpGlobals->curtime; //delta between refire
				if (gpGlobals->curtime - m_nExecutionTime < 0)
				{
					RadiusDamage_EX(info, UTIL_GetLocalPlayer()->GetAbsOrigin(), AoeDamageRadius, CLASS_NONE, pOwner,true); //Attack
					WeaponSound(SPECIAL3);
					pPlayer->SetAnimationSkillFlag(2);
					pOwner->SetAnimation(PLAYER_SKILL_USE);
					DispatchParticleEffect("aoehint4", GetAbsOrigin(), vec3_angle);
									
					pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() - 30);
				}

				m_nSkCoolDownTime2 = gpGlobals->curtime + (sk_plr_skills_2_cooldown_time.GetFloat()*m_flCooldown);
	flSkill_RadialSlash_ActiveTime = gpGlobals->curtime + (1.1f * m_flPlayerStats_AttackSpeed);
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
	if (m_flDamageBuffActiveTime >= gpGlobals->curtime)
		info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
	else if (m_flDamageBuffActiveTime <= gpGlobals->curtime)
	{
		if (m_bIsCritical)
			info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_CritDamage); //critical
		else
		{
			info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage);
		}
	}

	info.SetDamagePosition(traceHit.startpos);
	info.SetDamageForce(forward * 10);
	info.ScaleDamage(0.6f);

	Vector vParticles = pOwner->GetAbsOrigin();
	vParticles.z += 48;


	//TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	//The area in radius that the skill is going to affect.
	float AoeDamageRadius = 144.0f;
	//HACK! 

	if (m_nExecutionTime > gpGlobals->curtime)
	{
		if (flSkill_RadialSlash_ActiveTime > gpGlobals->curtime)
		{
			if ((flSkill_RadialSlash_ActiveTime - gpGlobals->curtime <= (1.1f* m_flPlayerStats_AttackSpeed)) && (flSkill_RadialSlash_ActiveTime - gpGlobals->curtime >= (0.183f* m_flPlayerStats_AttackSpeed)))
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
					m_nSkillHitRefireTime = gpGlobals->curtime + (0.183f * m_flPlayerStats_AttackSpeed); //delta between refire
					m_flNPCFreezeTime = gpGlobals->curtime + 0.6f;
					//AddKnockbackXY(1.1f, 4);
					DispatchParticleEffect("aoehint4", vParticles, vec3_angle);

				}
			}

			if ((flSkill_RadialSlash_ActiveTime - gpGlobals->curtime <= (0.183f* m_flPlayerStats_AttackSpeed)) && (flSkill_RadialSlash_ActiveTime - gpGlobals->curtime >= 0.0f))
			{ //Doesnt work because it pushes the enemy outside the player's damage range
				m_flSkillAttributeRange = AoeDamageRadius;

				if (gpGlobals->curtime >= m_nSkillHitRefireTime)
				{
					RadiusDamage(info, UTIL_GetLocalPlayer()->GetAbsOrigin(), AoeDamageRadius, CLASS_NONE, pOwner); //Attack
					AddKnockbackXY(1, 5); //for npc hitting sound
					DispatchParticleEffect("aoehint4", vParticles, vec3_angle);


					m_nSkillHitRefireTime = gpGlobals->curtime + (0.183f *m_flPlayerStats_AttackSpeed); //delta between refire
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
	pPlayer->StopRunning();
	// Fire the bullets
	Vector vecSrc = pOwner->Weapon_ShootPosition();
	vecSrc.z -= 22;
	Vector vecAiming = pOwner->GetAutoaimVector(0); //culprit for why the launched projectile keeps pointing down
	Vector impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	// Fire the bullets
	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire!
	m_nExecutionTime = gpGlobals->curtime + (1.0f * m_flPlayerStats_AttackSpeed);
	AddSkillMovementImpulse(2.0f);
	CreateThrowable(vecSrc, vecVelocity, 10, 150, 1.5, pOwner);
	//CreateThrowable(vecSrc, vecVelocity, 10, 150, 1.5, pOwner);
	UTIL_ScreenShake(GetAbsOrigin(), 3.0f, 130.0, 0.7, 256.0f, SHAKE_START);

	pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() - 25);
	WeaponSound(SINGLE);
	pPlayer->SetAnimationSkillFlag(3);
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
	pPlayer->StopRunning();
	m_nExecutionTime = gpGlobals->curtime + (1.5f * m_flPlayerStats_AttackSpeed);
		
	EmitSound("Weapon_Melee.SPECIAL5");
	pPlayer->SetAnimationSkillFlag(4);
	pOwner->SetAnimation(PLAYER_SKILL_USE);

	pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() - 50);
		
	m_bIsHealSlashAttacking = true;
	m_flHealSlashDelayTimer = gpGlobals->curtime + (1.0f * m_flPlayerStats_AttackSpeed);

	m_nSkCoolDownTime4 = gpGlobals->curtime + (sk_plr_skills_4_cooldown_time.GetFloat()*m_flCooldown);
	m_bIsSkCoolDown4 = true;


}

void CBaseMeleeWeapon::Skill_HealSlash_LogicEx()
{
		
	if ((m_bIsHealSlashAttacking) && (gpGlobals->curtime >= m_flHealSlashDelayTimer))
	{
		trace_t traceHit;
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());

		Vector forward;
		forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());
		Activity nHitActivity = ACT_VM_HITCENTER;

		CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
		if (m_flDamageBuffActiveTime >= gpGlobals->curtime)
			info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
		else if (m_flDamageBuffActiveTime <= gpGlobals->curtime)
		{
			if (pPlayer->IsCritical() == true)
				info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_CritDamage); //critical
			else
			{
				info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage);
			}
		}

		info.SetDamagePosition(traceHit.startpos);
		info.SetDamageForce(forward);
		info.ScaleDamage(2.0f);


		GetPlayerAnglesOnce();

		float m_nDamageRadius = 128.0f;
		//Only run when the cooldown time is 0
		CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
		int nAIs = g_AI_Manager.NumAIs();
		string_t iszNPCName = AllocPooledString("npc_metropolice");
		Vector playernpcdist;

		AddKnockbackXY(10.0f, 1);
		RadiusDamage_EX(info, UTIL_GetLocalPlayer()->GetAbsOrigin(), m_nDamageRadius, CLASS_NONE, pOwner,true);


		for (int i = 0; i < nAIs; i++)
		{
			//if (ppAIs[i]->m_iClassname == iszNPCName)
			if (ppAIs[i])
			{
				playernpcdist.x = abs(UTIL_GetLocalPlayer()->GetAbsOrigin().x - ppAIs[i]->GetAbsOrigin().x);
				playernpcdist.y = abs(UTIL_GetLocalPlayer()->GetAbsOrigin().y - ppAIs[i]->GetAbsOrigin().y);

				if (playernpcdist.x <= m_flSkillAttributeRange && playernpcdist.y <= m_flSkillAttributeRange)
				{
					DispatchParticleEffect("striderbuster_shotdown_core_flash", pPlayer->GetAbsOrigin(), vec3_angle);
					pOwner->SetHealth(pOwner->GetHealth() + 25);

				}
			}
		}

		m_bIsHealSlashAttacking = false;
	}

}

//Skill 5
Vector effectpos;
bool bCanPullEnemies = false;
bool bCanPullEnemies2 = false;
void CBaseMeleeWeapon::Skill_Trapping()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;
	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_GetLocalPlayer());
	pPlayer->StopRunning();

	Activity nHitActivity = ACT_VM_HITCENTER;

	CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	if (m_flDamageBuffActiveTime >= gpGlobals->curtime)
		info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
	else if (m_flDamageBuffActiveTime <= gpGlobals->curtime)
	{
		if (pPlayer->IsCritical() == true)
			info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_CritDamage); //critical
		else
		{
			info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage);
		}
	}

		Vector fwd;
		AngleVectors(pOwner->GetAbsAngles(), &fwd);
		fwd.z = 0;
		VectorNormalize(fwd);

		float fmagnitude = 350;
		effectpos = pOwner->GetAbsOrigin() + (fwd * fmagnitude);
		//effectpos.z = 0;

		//WeaponSound(SPECIAL4);
		EmitSound("Weapon_Melee.SPECIAL4");
		pPlayer->SetAnimationSkillFlag(5);
		pOwner->SetAnimation(PLAYER_SKILL_USE);
		RadiusDamage(info, effectpos, 192.0f, CLASS_NONE, pOwner);

		m_nExecutionTime = gpGlobals->curtime + (1.0f * m_flPlayerStats_AttackSpeed);

		pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() - 30);
		DispatchParticleEffect("aoehint", effectpos, vec3_angle);

		//Init Cooldown
		m_nSkCoolDownTime5 = gpGlobals->curtime + (sk_plr_skills_5_cooldown_time.GetFloat()*m_flCooldown);
		m_flSkillTrapping_ActiveTime = gpGlobals->curtime + (3.0f* m_flPlayerStats_AttackSpeed);
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

		if (m_flSkillTrapping_ActiveTime > gpGlobals->curtime)
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

					if (m_flSkillTrapping_ActiveTime - gpGlobals->curtime >= (1.0f* m_flPlayerStats_AttackSpeed))
					{
						if (SkillOriginNPCdist.x <= skillrange && SkillOriginNPCdist.y <= skillrange)
						{
							ppAIs[i]->SetCondition(COND_NPC_FREEZE);
							//ppAIs[i]->SetRenderMode(kRenderTransColor);
							//ppAIs[i]->SetRenderColor(128, 128, 128, 128);
						}
					}
					else if ((m_flSkillTrapping_ActiveTime - gpGlobals->curtime <= (1.0f* m_flPlayerStats_AttackSpeed)) && (m_flSkillTrapping_ActiveTime - gpGlobals->curtime >= (0.9f* m_flPlayerStats_AttackSpeed)))
					{
						if (SkillOriginNPCdist.x <= skillrange + 30 && SkillOriginNPCdist.y <= skillrange + 30)
						{
							ppAIs[i]->SetCondition(COND_NPC_UNFREEZE);
							//ppAIs[i]->SetRenderMode(kRenderNormal);
						}
					}

					if ((m_flSkillTrapping_ActiveTime - gpGlobals->curtime <= (2.5f* m_flPlayerStats_AttackSpeed)) && (m_flSkillTrapping_ActiveTime - gpGlobals->curtime >= (2.4f* m_flPlayerStats_AttackSpeed)))
						bCanPullEnemies = true;

					if ((m_flSkillTrapping_ActiveTime - gpGlobals->curtime <= (2.2f * m_flPlayerStats_AttackSpeed)) && (m_flSkillTrapping_ActiveTime - gpGlobals->curtime >= (2.1f * m_flPlayerStats_AttackSpeed)))
						bCanPullEnemies2 = true;

					if (bCanPullEnemies) //Need to make run once only
					{
						if (SkillOriginNPCdist.x <= skillrange && SkillOriginNPCdist.y <= skillrange)
							ppAIs[i]->ApplyAbsVelocityImpulse((effectpos - ppAIs[i]->GetAbsOrigin()));
						UTIL_ScreenShake(GetAbsOrigin(), 3.0f, 130.0, 0.7, 256.0f, SHAKE_START);

						bCanPullEnemies = false;

					}

					if (bCanPullEnemies2) //Need to make run once only
					{
						
						if (SkillOriginNPCdist.x <= skillrange && SkillOriginNPCdist.y <= skillrange)
							ppAIs[i]->ApplyAbsVelocityImpulse((pOwner->GetAbsOrigin() - ppAIs[i]->GetAbsOrigin())*0.85);

						UTIL_ScreenShake(GetAbsOrigin(), 3.0f, 130.0, 0.7, 256.0f, SHAKE_START);

						bCanPullEnemies2 = false;

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
	pPlayer->StopRunning();

	float skillrange = 224.0f;

	WeaponSound(SPECIAL2);
	pPlayer->SetAnimationSkillFlag(6);
	pOwner->SetAnimation(PLAYER_SKILL_USE);
	m_flSkillAttributeRange = skillrange;
	m_nExecutionTime = gpGlobals->curtime + (1.4f * m_flPlayerStats_AttackSpeed);
	pPlayer->SetPlayerMP(pPlayer->GetPlayerMP() - 50);

	skpos = pOwner->GetAbsOrigin();

	GetPlayerPosOnce();
	m_bSetTornadoLiftVec = true;
	AddKnockbackXY(1, 9); // Set NPC In air position
	//Init Cooldown
	DispatchParticleEffect("tornado1", skpos, vec3_angle);
	flTorSkillRefireTime = gpGlobals->curtime + 0.3f;
	flSkillActiveTime = gpGlobals->curtime + 4.0f;
	//m_flDamageBuffActiveTime = gpGlobals->curtime + 10.0f; //not actual speedmod, but a timer for damage buff 
	pPlayer->SetPlayerAttackSpeedBonus(0.2f, 15.0f); //20% bonus
	//pPlayer->SetPlayerCooldownReductionRateBonus(0.5f, 10.f);
	//pPlayer->ApplyBattery();
	

	m_bIsSkCoolDown6 = true;
	m_nSkCoolDownTime6 = gpGlobals->curtime + (sk_plr_skills_6_cooldown_time.GetFloat()*m_flCooldown);

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
	if (m_flDamageBuffActiveTime >= gpGlobals->curtime)
		info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
	else if (m_flDamageBuffActiveTime <= gpGlobals->curtime)
	{
		if (m_bIsCritical)
			info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_CritDamage); //critical
		else
		{
			info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage);
		}
	}

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

//Skill_Lift implementation: Lifting player and Mob NPCs up
void CBaseMeleeWeapon::Skill_Lift()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	CHL2_Player *pHLOwner = dynamic_cast<CHL2_Player *>(pOwner);
	pHLOwner->StopRunning();
	pHLOwner->SetAnimationSkillFlag(7);
	pOwner->SetAnimation(PLAYER_SKILL_USE);
	
	//Freeze player and report attack state 
	m_nExecutionTime = gpGlobals->curtime + (0.6f * m_flPlayerStats_AttackSpeed);

	//applying the actual skill effects in a delayed time of 0.1667f
	m_bSkillLiftAttack = true;
	m_flSkillLiftAttackDelayTimer = gpGlobals->curtime + (0.1667f * m_flPlayerStats_AttackSpeed);
	
	//Init cooldown 
	m_bIsSkCoolDown7 = true;
	m_nSkCoolDownTime7 = gpGlobals->curtime + 4.0f;
	
	
}

void CBaseMeleeWeapon::Skill_Lift_LogicEx(void)
{
	if ((m_bSkillLiftAttack) && (gpGlobals->curtime >= m_flSkillLiftAttackDelayTimer))
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		CHL2_Player *pHLOwner = dynamic_cast<CHL2_Player *>(pOwner);

		Activity nHitActivity = ACT_VM_HITCENTER;

		//delay these by 0.1667f;
		float liftAOE = 128.0f;
		m_flSkillAttributeRange = liftAOE;

		CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
		if (m_flDamageBuffActiveTime >= gpGlobals->curtime)
			info.SetDamage(GetDamageForActivity(nHitActivity) * 2);
		else if (m_flDamageBuffActiveTime <= gpGlobals->curtime)
		{
			if (pHLOwner->IsCritical())
				info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_CritDamage); //critical
			else
			{
				info.SetDamage(GetDamageForActivity(nHitActivity) + m_flPlayerStats_BaseDamage);
			}
		}

		RadiusDamage_EX(info, pOwner->GetAbsOrigin(), 128.f, CLASS_PLAYER, pOwner, true);

		Vector vLiftDir(0, 0, 512);
		pOwner->ApplyAbsVelocityImpulse(vLiftDir); // push the player upwards 

		AddKnockbackXY(1, 5); //sound and hit impact of npcs 

		m_flNPCFreezeTime = gpGlobals->curtime + 0.6f;
		AddKnockbackXY(1, 7);
		//pHLOwner->ForceViewAngleToCamera(1.0f);

		m_bSkillLiftAttack = false; //making sure that the skill is only called once
	}
}

void CBaseMeleeWeapon::Skill_RageOn(void)
{
	//HL2 apply rage effect
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	CHL2_Player *pHLOwner = dynamic_cast<CHL2_Player *>(pOwner);

	//sound fx
	//particle fx
	m_nExecutionTime = gpGlobals->curtime + 1.0f;
	pHLOwner->ForceViewAngleToCamera(1.0f);
	pHLOwner->SetAnimation(PLAYER_SKILL_USE);
	pHLOwner->Rage_ApplyRageBuff();

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
						m_bIsEnemyInAtkRange = true;// display text if they are within range
						
					}
					
					int NPCHealth = ppAIs[i]->GetHealth();
					if (options == 1)
					{						
						if (UTIL_GetLocalPlayer()->GetGroundEntity() != NULL)
						{	
							if ((NPCHealth > 0) && (NPCHealth < sk_npcknockbackathealth.GetInt()))
								ppAIs[i]->ApplyAbsVelocityImpulse(dirkb*flKnockbackVelocity);
						}
						else if (UTIL_GetLocalPlayer()->GetGroundEntity() == NULL)
						{
							if ((NPCHealth > 0) && (NPCHealth < sk_npcknockbackathealth.GetInt()))
							{								
								if (ppAIs[i]->GetGroundEntity() == NULL)
								{
									Vector vNPCInAirPos = ppAIs[i]->GetAbsOrigin();
									vNPCInAirPos.z = UTIL_GetLocalPlayer()->GetAbsOrigin().z;

									ppAIs[i]->SetAbsOrigin(vNPCInAirPos);
									ppAIs[i]->SetAbsVelocity(vec3_origin);
								}
								else
								{
									if ((NPCHealth > 0) && (NPCHealth < sk_npcknockbackathealth.GetInt()))
										ppAIs[i]->ApplyAbsVelocityImpulse(dirkb*flKnockbackVelocity);
								}
							}
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
					}
					else if (options == 5)
					{
						if (ppAIs[i]->IsAlive())
						{
							Vector ParticleVec = ppAIs[i]->GetAbsOrigin();
							ParticleVec.z += 48;
							//DispatchParticleEffect("hit_impact", ParticleVec, ppAIs[i]->GetAbsAngles());
							DispatchParticleEffect("grenade_explosion_01e", ParticleVec, ppAIs[i]->GetAbsAngles());
							WeaponSound(MELEE_HIT);
						}
						
					}
					else if (options == 6)
					{
						if (ppAIs[i]->IsAlive())
						{
							Vector vNPCInAirPos = ppAIs[i]->GetAbsOrigin();
							vNPCInAirPos.z = UTIL_GetLocalPlayer()->GetAbsOrigin().z;

							if (ppAIs[i]->GetGroundEntity() != NULL)
								ppAIs[i]->ApplyAbsVelocityImpulse(dirkb*flKnockbackVelocity);
							else
								ppAIs[i]->SetAbsOrigin(vNPCInAirPos);
						}
					}
					else if (options == 7)
					{
						if (ppAIs[i]->IsAlive())
						{
							ppAIs[i]->ApplyAbsVelocityImpulse(Vector(0, 0, 512));
						}
					}
					else if (options == 8) //Freeze NPCs while they are in air 
					{
						if (ppAIs[i]->IsAlive())
						{
							if (ppAIs[i]->GetGroundEntity() == NULL)
							{
								Vector vNPCInAirPos = ppAIs[i]->GetAbsOrigin();
								vNPCInAirPos.z = UTIL_GetLocalPlayer()->GetAbsOrigin().z;

								ppAIs[i]->SetAbsOrigin(vNPCInAirPos);
								ppAIs[i]->SetAbsVelocity(vec3_origin);
							}
						}
					}

				}
				else
				{
					m_bIsEnemyInAtkRange = false;
				}
				
				if (staticplayernpcdist.x <= m_flSkillAttributeRange && staticplayernpcdist.y <= m_flSkillAttributeRange)
				{
					if (options == 4) //force NPC to the desired z height.
					{
						if (ppAIs[i]->IsAlive())
						{

							Vector vfNPCInAirPos = ppAIs[i]->GetAbsOrigin();
							//Vector vfAerialBonus(0, 0, 112);
							vfNPCInAirPos.z = vTornadoNPCPos.z;

							ppAIs[i]->SetAbsOrigin(vfNPCInAirPos);
							ppAIs[i]->SetAbsVelocity(vec3_origin);

						}

					}
					else if (options == 9) //Get NPC origin for tornado skill
					{
						if (ppAIs[i]->IsAlive())
						{
							vTornadoNPCPos = ppAIs[i]->GetAbsOrigin();
							Vector vfAerialBonus(0, 0, 112);
							vTornadoNPCPos.z = vTornadoNPCPos.z + vfAerialBonus.z;
						}
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

	return true;
}