//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
// NOTE:  m_nExecutionTime which is used freeze the player during skill use is bugged, when it reaches below .20 ,using evade will not give players additional speed boost
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
#include "grenade_ar2.h"
#include "Skills_WeaponThrow.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST(CBaseMeleeWeapon, DT_BaseMeleeWeapon)
END_SEND_TABLE()

#define BLUDGEON_HULL_DIM		16

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

extern ConVar sk_atkspeedmod("sk_atkspeedmod", "1");
extern ConVar pl_isattacking("pl_isattacking", "0");
extern ConVar sk_npcknockbackathealth("sk_npcknockbackathealth", "100");
extern ConVar sk_plr_max_mp("sk_plr_max_mp", "100");
extern ConVar sk_plr_mp_restore("sk_plr_mp_restore", "8");

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CBaseMeleeWeapon::CBaseMeleeWeapon()
{
	m_bFiresUnderwater = true;
	m_bIsSkCoolDown = false;
	m_nSkCoolDownTime = 0.0f;
	m_bIsSkCoolDown2 = false;
	m_nSkCoolDownTime2 = 0.0f;

	m_nExecutionTime = 0.0f;//This Var is Tied to Camera control in "in_camera" , be advised when changing it.
	m_nSkillHitRefireTime = 0.0f;
	m_bWIsAttack1 = true;
	m_bWIsAttack2 = false;
	m_bWIsAttack3 = false; 
	m_SpeedModActiveTime = 0.0f;
	m_flNPCFreezeTime = 0.0f;
	m_flSkillAttributeRange = 0.0f;
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
	//Call base class first
	PrecacheParticleSystem("aoehint");
	PrecacheParticleSystem("striderbuster_shotdown_core_flash");

	m_iPlayerMP = 50;
	m_iPlayerMPMax = sk_plr_max_mp.GetInt();
	m_flPlayerMPRestoreInterval = gpGlobals->curtime;


	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Precache the weapon
//-----------------------------------------------------------------------------
void CBaseMeleeWeapon::Precache(void)
{
	//Call base class first
	BaseClass::Precache();
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

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CBaseMeleeWeapon::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	SkillsHandler();
	HandlePlayerMP();

	if (pOwner == NULL)
		return;

	if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		PrimaryAttack();
	}
	else if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		//SecondaryAttack();
	}
	else
	{
		WeaponIdle();
		return;
	}
}
void CBaseMeleeWeapon::HandlePlayerMP(void)
{
	if (m_iPlayerMP <= 0)
	{
		// clamp to 0 
		// +10 MP unit per second
		m_iPlayerMP = 0;
		m_iPlayerMP += 1;

	}
	else if (m_iPlayerMP < m_iPlayerMPMax)
	{
		// + 10 MP unit per second
		// if ( mp under max limit  , then for each < unit of time>  , restore a certain amount of mp.)

		if (gpGlobals->curtime >= m_flPlayerMPRestoreInterval)
		{
			m_iPlayerMP += sk_plr_mp_restore.GetInt();
			m_flPlayerMPRestoreInterval = gpGlobals->curtime + 1;
		}

	}
	else if (m_iPlayerMP > m_iPlayerMPMax)
	{
		m_iPlayerMP = m_iPlayerMPMax;
	}
}
//m_nExecutionTime handles freezing the player for a certain amount of time
void CBaseMeleeWeapon::SkillsHandler(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	Activity nHitActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;
	
	/*CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);*/



	//Warning("Attack speed %.2f \n", sk_atkspeedmod.GetFloat());
	if (m_SpeedModActiveTime > gpGlobals->curtime)
		sk_atkspeedmod.SetValue("0.75");
	else
		sk_atkspeedmod.SetValue("1");

	if (m_flNPCFreezeTime > gpGlobals->curtime)
	{
		AddKnockbackXY(1.0f, 2);
	}
	else
	{
		AddKnockbackXY(1.0f, 3);
	}

	if ((pOwner->m_nButtons & IN_ATTACK2) && !m_bIsSkCoolDown)
	{
		GetPlayerAnglesOnce();
		Skill_Evade();
		m_SpeedModActiveTime = gpGlobals->curtime + 3.0f;

	}
	
	//Run the Evil Slash skill code
		Skill_RadialSlash();

		if (gpGlobals->curtime - m_nExecutionTime < 0)
			Warning("Execution time %.2f \n", abs(gpGlobals->curtime - m_nExecutionTime));
		

		//A stupid hack to temporary fix the problem when the Execution timer goes below .20 and the player press evade they don't receive the bonus velocity boost.
		//if ((gpGlobals->curtime - m_nExecutionTime >= -0.20f) && (gpGlobals->curtime - m_nExecutionTime <= 0))
			//m_nExecutionTime = 0;

		
		// Skill 3 loop
		if ((pOwner->m_nButtons & IN_SLOT2) && !m_bIsSkCoolDown3)
		{
			Skill_GrenadeEX();
		}
		
		//Skill 4 loop
		if ((pOwner->m_nButtons & IN_SLOT3) && !m_bIsSkCoolDown4)
		{
			Skill_HealSlash();
		}

		/*if ((pOwner->m_nButtons & IN_SLOT4) && !m_bIsSkCoolDown5)
		{
			Skill_Trapping();
		}*/

		//Skill 5 loop
		Skill_Trapping();

		//Skill 6 loop
		Skill_Tornado();



	//Makes the player stand still when activating a skill

	if (pOwner->m_nButtons & IN_SPEED)
		m_nExecutionTime = 0.0f;

	//Skill 1 CoolDown
	if (gpGlobals->curtime - m_nSkCoolDownTime < 0)
	{
		float cdtimer = gpGlobals->curtime - m_nSkCoolDownTime;
		//DevMsg("Spinning Demon %.2f \n ", cdtimer);
		m_bIsSkCoolDown = false;
	}
	
	//Skill 2 CoolDown
	if (gpGlobals->curtime - m_nSkCoolDownTime2 < 0)
	{
		float cdtimer = gpGlobals->curtime - m_nSkCoolDownTime2;
		//DevMsg("Evil Slash CD  %.2f \n ", cdtimer);
		m_bIsSkCoolDown2 = false;
	}

	//Skill 3 CoolDown
	if (gpGlobals->curtime - m_nSkCoolDownTime3 < 0)
	{
		float cdtimer = gpGlobals->curtime - m_nSkCoolDownTime3;
		//DevMsg("Nade CD  %.2f \n ", cdtimer);
		m_bIsSkCoolDown3 = false;
	}
	
	if (gpGlobals->curtime - m_nSkCoolDownTime4 < 0)
	{
		float cdtimer = gpGlobals->curtime - m_nSkCoolDownTime4;
		//DevMsg("Nade CD  %.2f \n ", cdtimer);
		m_bIsSkCoolDown4 = false;
	}

	/*if (gpGlobals->curtime - m_nSkCoolDownTime5 <= 0)
	{
		m_bIsSkCoolDown5 = false;
	}*/

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
	if (m_nSkCoolDownTime - gpGlobals->curtime >= 0)
		engine->Con_NPrintf(10, "Skill 1 Cooldown time %6.1f Attack speed %6.1f ", m_nSkCoolDownTime - gpGlobals->curtime, sk_atkspeedmod.GetFloat());

	if (m_nSkCoolDownTime2 - gpGlobals->curtime >= 0)
		engine->Con_NPrintf(11, "Skill 2 Cooldown time  %6.1f ", m_nSkCoolDownTime2 - gpGlobals->curtime);

	if (m_nSkCoolDownTime3 - gpGlobals->curtime >= 0)
		engine->Con_NPrintf(12, "Skill 3 Cooldown time  %6.1f  Press USE to Detonate Now ", m_nSkCoolDownTime3 - gpGlobals->curtime);

	if (m_nSkCoolDownTime4 - gpGlobals->curtime >= 0)
		engine->Con_NPrintf(13, "Skill 4 Cooldown time  %6.1f   ", m_nSkCoolDownTime4 - gpGlobals->curtime);

	engine->Con_NPrintf(9, "Enemy HP  %i ", m_iEnemyHealth);

	if (m_nSkCoolDownTime5 - gpGlobals->curtime >= 0)
	{
		engine->Con_NPrintf(14, "Skill 5 Cooldown time  %6.1f   ", m_nSkCoolDownTime5 - gpGlobals->curtime);
	}
	else
	{
		m_bIsSkCoolDown5 = false;	
	}

	if (m_nSkCoolDownTime6 - gpGlobals->curtime >= 0)
	{
		engine->Con_NPrintf(15, "Skill 6 Cooldown time  %6.1f   ", m_nSkCoolDownTime6 - gpGlobals->curtime);
	}
	else
	{
		m_bIsSkCoolDown6 = false;
	}

		if (m_bIsSkCoolDown5)
			engine->Con_NPrintf(13, "Skill 5 is in cooldown");
		else if (!m_bIsSkCoolDown5)
			engine->Con_NPrintf(13, "Skill 5 is NOT in cooldown");

		if (m_bIsSkCoolDown6)
			engine->Con_NPrintf(16, "Skill 6 is in cooldown");
		else if (!m_bIsSkCoolDown6)
			engine->Con_NPrintf(16, "Skill 6 is NOT in cooldown");

		engine->Con_NPrintf(16,"Is player attacking? : %i", pl_isattacking.GetInt());
		engine->Con_NPrintf(17, "MP unit: %i", m_iPlayerMP);




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


//------------------------------------------------------------------------------
// Purpose: Implement impact function
//------------------------------------------------------------------------------
//void CBaseMeleeWeapon::Hit(trace_t &traceHit, Activity nHitActivity, bool bIsSecondary)
//{
//	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
//
//	//Do view kick
//	AddViewKick();
//
//	//Make sound for the AI
//	CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer);
//
//	// This isn't great, but it's something for when the crowbar hits.
//	pPlayer->RumbleEffect(RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART);
//
//	CBaseEntity	*pHitEntity = traceHit.m_pEnt;
//
//	//Apply damage to a hit target
//	if (pHitEntity != NULL)
//	{
//		float m_nDamageRadius = 128.0f;
//		Vector hit1 = GetAbsOrigin();
//		Vector hitDirection;
//		pPlayer->EyeVectors(&hitDirection, NULL, NULL);
//		VectorNormalize(hitDirection);
//
//		Vector	dir = traceHit.endpos;
//		Vector vecForce = dir * ImpulseScale(75, 700);
//		VectorNormalize(dir);
//		dir *= 500.0f;
//		ApplyAbsVelocityImpulse(dir);
//
//		CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);
//		//CTakeDamageInfo info(this, m_hThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported);
//
//		//Makes weapon produce AoE damage
//		//RadiusDamage(info, hit1, m_nDamageRadius, CLASS_NONE, NULL);
//
//		if (pPlayer && pHitEntity->IsNPC())
//		{
//			// If bonking an NPC, adjust damage.
//			info.AdjustPlayerDamageInflictedForSkillLevel();
//		}
//
//		UTIL_GetLocalPlayer()->SetAbsVelocity(vec3_origin);
//
//
//
//		CalculateMeleeDamageForce( &info, hitDirection, traceHit.endpos );
//
//		pHitEntity->DispatchTraceAttack( info, hitDirection, &traceHit ); 
//		ApplyMultiDamage();
//
//		// Now hit all triggers along the ray that... 
//		TraceAttackToTriggers( info, traceHit.startpos, traceHit.endpos, hitDirection );
//
//		if (ToBaseCombatCharacter(pHitEntity))
//		{
//			gamestats->Event_WeaponHit(pPlayer, !bIsSecondary, GetClassname(), info);
//		}
//	}
//
//	// Apply an impact effect
//	ImpactEffect(traceHit);
//}

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
	// Try a ray
	trace_t traceHit;
	
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	//float flSpeedMod = 1.5f;

	//use g_EntList  to make reference to the npc????
	float m_nDamageRadius = 128.0f;

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	Vector swingEnd = swingStart + forward * GetRange();

	Vector fwd;
	AngleVectors(UTIL_GetLocalPlayer()->GetAbsAngles(), &fwd);
	//zero out vector angles
	fwd.z = 0;
	VectorNormalize(fwd);

	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	//Activity nHitActivity = ACT_VM_HITCENTER;
	Activity nHitActivity = ACT_MELEE_ATTACK1;
	// Damage info for 
	CTakeDamageInfo dmginfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	dmginfo.SetDamagePosition( traceHit.startpos );
	//TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, forward );

	//Makes weapon produce AoE damage
	RadiusDamage(dmginfo, swingEnd, m_nDamageRadius, CLASS_NONE, pOwner);
	//Move player forward for each swing.
	AddSkillMovementImpulse(2.0f);
	Vector particlepos = GetAbsOrigin() * Vector(1,1,32);
	DispatchParticleEffect("aoehint", particlepos, vec3_angle);

	//Hard coded value, should change to SequenceDuration()

	//if ( traceHit.fraction == 1.0 )
	//{
	//	float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point
	//
	//	// Back off by hull "radius"
	//	swingEnd -= forward * bludgeonHullRadius;
	//
	//	UTIL_TraceHull( swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
	//	if ( traceHit.fraction < 1.0 && traceHit.m_pEnt )
	//	{
	//		Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
	//		VectorNormalize( vecToTarget );
	//
	//		float dot = vecToTarget.Dot( forward );
	//
	//		// YWB:  Make sure they are sort of facing the guy at least...
	//		if ( dot < 0.70721f )
	//		{
	//			// Force amiss
	//			traceHit.fraction = 1.0f;
	//		}
	//		else
	//		{
	//			nHitActivity = ChooseIntersectionPointAndActivity( traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner );
	//		}
	//	}
	//}
		
	if (!bIsSecondary)
	{
		m_iPrimaryAttacks++;
	}
	else
	{
		//m_iSecondaryAttacks++;
	}

	// -------------------------
	//	Miss
	// -------------------------
	//if (traceHit.fraction == 1.0f)
	//{
	//	nHitActivity = bIsSecondary ? ACT_VM_MISSCENTER2 : ACT_VM_MISSCENTER;
	//
	//	// We want to test the first swing again
	//	Vector testEnd = swingStart + forward * GetRange();
	//
	//	// See if we happened to hit water
	//	ImpactWater(swingStart, testEnd);
	//}
	//else
	//{
	//	Hit( traceHit, nHitActivity, bIsSecondary ? true : false );
	//}

	// Send the anim
	SendWeaponAnim(nHitActivity);
	GetPlayerAnglesOnce();
	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNPCFreezeTime = gpGlobals->curtime + 0.6f;

	//Give the player MP for each enemy hit
	if (m_bIsEnemyInAtkRange)
	m_iPlayerMP += 4;

	//m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

	//Set and cycles between attack states, play animation and make sound. 
	if (m_bWIsAttack1 == true)
	{
		m_nDamageRadius = 128.0f;
		m_flSkillAttributeRange = m_nDamageRadius;
		//triggerInfo.ScaleDamage(1.0);
		WeaponSound(ATTACK1);
		pOwner->SetAnimation(PLAYER_ATTACK1);
		m_bWIsAttack2 = true;
		m_bWIsAttack1 = false;
		AddKnockbackXY(2.0f,1);
		AddKnockbackXY(1, 5); //for npc hitting sound
		m_nExecutionTime = gpGlobals->curtime + (0.6666f *sk_atkspeedmod.GetFloat());

	}
	else if (m_bWIsAttack2 == true)
	{
		//m_nDamageRadius = 144.0f;
		//triggerInfo.ScaleDamage(1.5);
		m_flSkillAttributeRange = m_nDamageRadius;
		WeaponSound(ATTACK2);
		pOwner->SetAnimation(PLAYER_ATTACK1);
		m_bWIsAttack2 = false;
		m_bWIsAttack1 = false;
		m_bWIsAttack3 = true;
		AddKnockbackXY(2.0f,1);
		AddKnockbackXY(1, 5); //for npc hitting sound
		m_nExecutionTime = gpGlobals->curtime + (0.6666f *sk_atkspeedmod.GetFloat());

	}
	else if (m_bWIsAttack3 == true)
	{
		//m_nDamageRadius = 192.0f;
		//triggerInfo.ScaleDamage(2.0);
		m_flSkillAttributeRange = m_nDamageRadius;
		WeaponSound(ATTACK3);
		pOwner->SetAnimation(PLAYER_ATTACK1);
		m_bWIsAttack1 = true;
		m_bWIsAttack2 = false;
		m_bWIsAttack3 = false;
		AddKnockbackXY(3.0f,1);
		AddKnockbackXY(1, 5); //for npc hitting sound
		m_nExecutionTime = gpGlobals->curtime + 0.6666f;
	}


}

//Skill: Spinning Demon.
float nStepVelocity = 1024.0f;
void CBaseMeleeWeapon::Skill_Evade(void)
{
	trace_t traceHit;
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;
	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());
	Activity nHitActivity = ACT_VM_HITCENTER;
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	triggerInfo.ScaleDamage(2.0f);

	float m_nDamageRadius = 192.0f;
	//Only run when the cooldown time is 0

	if (!m_bIsSkCoolDown && gpGlobals->curtime > m_nSkCoolDownTime)
	{
		m_nExecutionTime = 0.0f;
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		if (!pOwner)
			return;
	//	Vector fwd;
	//	AngleVectors(UTIL_GetLocalPlayer()->GetAbsAngles(), &fwd);
	//	fwd.z = 0;
	//	VectorNormalize(fwd);
	//	//AddKnockback(fwd * 350);

		UTIL_GetLocalPlayer()->SetAbsVelocity(dirkb*nStepVelocity);

		RadiusDamage(triggerInfo, UTIL_GetLocalPlayer()->GetAbsOrigin(), m_nDamageRadius, CLASS_NONE, pOwner);
		WeaponSound(SINGLE);
		pOwner->SetAnimation(PLAYER_EVADE);

		//Sync the time with flFreezingMovementTime in in_main.cpp
		m_nExecutionTime = gpGlobals->curtime + 1.0f;

		m_nSkCoolDownTime = gpGlobals->curtime + 5.0f;
		m_bIsSkCoolDown = true;

	}

}

//Skill:Evil Slash
void CBaseMeleeWeapon::Skill_RadialSlash(void)
{	
	trace_t traceHit;
	//Initialize the player pointer
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	//Initialize the forward vector
	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	//Set the ideal activity (for animation)
	Activity nHitActivity = ACT_VM_HITCENTER;

	//Setting up the damage info for the skill
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward*10);
	triggerInfo.ScaleDamage(0.8f);
	
	//TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	//The area in radius that the skill is going to affect.
	float m_nDamageRadius = 264.0f;
	//HACK! 
	if ((gpGlobals->curtime - m_nSkCoolDownTime2 > -7) && (gpGlobals->curtime - m_nSkCoolDownTime2 < -5.5f))
	{
		m_flSkillAttributeRange = m_nDamageRadius;
		//HACK! This is a really hacky way to do DPS , Todo: make a proper system or function so every skills can be added dps property easily.
		if (gpGlobals->curtime >= m_nSkillHitRefireTime)
		{
			//DevMsg("Evil Slash Hit! \n");
			RadiusDamage(triggerInfo, UTIL_GetLocalPlayer()->GetAbsOrigin(), m_nDamageRadius, CLASS_NONE, pOwner); //Attack
			AddKnockbackXY(1, 5); //for npc hitting sound

			//HACK! Reset the timer
			m_nSkillHitRefireTime = gpGlobals->curtime + 0.3f; //delta between refire
			m_flNPCFreezeTime = gpGlobals->curtime + 0.6f;
			//AddKnockbackXY(1.1f, 4);
		}
	}

	if ((gpGlobals->curtime - m_nSkCoolDownTime2 > -5.5f) && (gpGlobals->curtime - m_nSkCoolDownTime2 < -5.0f))
	{ //Doesnt work because it pushes the enemy outside the player's damage range
		m_flSkillAttributeRange = m_nDamageRadius;

		if (gpGlobals->curtime >= m_nSkillHitRefireTime)
		{
			RadiusDamage(triggerInfo, UTIL_GetLocalPlayer()->GetAbsOrigin(), m_nDamageRadius, CLASS_NONE, pOwner); //Attack
			AddKnockbackXY(1, 5); //for npc hitting sound

			m_nSkillHitRefireTime = gpGlobals->curtime + 0.3f; //delta between refire
			m_flNPCFreezeTime = gpGlobals->curtime + 0.6f;
			AddKnockbackXY(10, 1);
		}

	}

	if ((pOwner->m_nButtons & IN_SLOT1) && !m_bIsSkCoolDown2)
	{
		if (!m_bIsSkCoolDown2 && gpGlobals->curtime > m_nSkCoolDownTime2)
		{
			//Initialize the variable for moving the player on each attack
			AddSkillMovementImpulse(1.0f);
			GetPlayerAnglesOnce();
			m_nExecutionTime = gpGlobals->curtime + 2.0f;
			//HACK! Fire the timer
			m_nSkillHitRefireTime = gpGlobals->curtime + 0.3f; //delta between refire
			if (gpGlobals->curtime - m_nExecutionTime < 0)
			{
				RadiusDamage(triggerInfo, UTIL_GetLocalPlayer()->GetAbsOrigin(), m_nDamageRadius, CLASS_NONE, pOwner); //Attack
				WeaponSound(SINGLE);
				pOwner->SetAnimation(PLAYER_SKILL_USE);
				DispatchParticleEffect("aoehint", GetAbsOrigin(), vec3_angle);
				m_iPlayerMP -= 15;
			}
			m_nSkCoolDownTime2 = gpGlobals->curtime + 7.0f;
			m_bIsSkCoolDown2 = true;
		}
	}

}

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

void CBaseMeleeWeapon::Skill_GrenadeEX(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	// Fire the bullets
	Vector vecSrc = pOwner->Weapon_ShootPosition();
	Vector vecAiming = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
	Vector impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	// Fire the bullets
	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire the combine ball
	if (!m_bIsSkCoolDown3 && gpGlobals->curtime > m_nSkCoolDownTime3)
	{
		if (m_iPlayerMP > 20)
		{
			m_nExecutionTime = gpGlobals->curtime + 1.0f;
			AddSkillMovementImpulse(2.0f);
			CreateWpnThrowSkill(vecSrc, vecVelocity, 10, 150, 1.5, pOwner);

			m_iPlayerMP -= 20;
			WeaponSound(SINGLE);
			pOwner->SetAnimation(PLAYER_SKILL_USE);

			m_nSkCoolDownTime3 = gpGlobals->curtime + 3.0f;
			m_bIsSkCoolDown3 = true;
		}
		else
			Warning("You don't have enough SG to execute this skill \n");


	}
}

void CBaseMeleeWeapon::Skill_HealSlash(void)
{
	trace_t traceHit;
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());
	Activity nHitActivity = ACT_VM_HITCENTER;

	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	triggerInfo.ScaleDamage(2.0f);

	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();
	string_t iszNPCName = AllocPooledString("npc_metropolice");
	Vector playernpcdist;

	GetPlayerAnglesOnce();

	float m_nDamageRadius = 128.0f;
	//Only run when the cooldown time is 0

	if (!m_bIsSkCoolDown4 && gpGlobals->curtime > m_nSkCoolDownTime4)
	{
		m_nExecutionTime = gpGlobals->curtime + 1.5f;
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		if (!pOwner)
			return;
		
		AddKnockbackXY(10.0f, 1);
		RadiusDamage(triggerInfo, UTIL_GetLocalPlayer()->GetAbsOrigin(), m_nDamageRadius, CLASS_NONE, pOwner);
		WeaponSound(SINGLE);
		pOwner->SetAnimation(PLAYER_SKILL_USE);
		DispatchParticleEffect("striderbuster_shotdown_core_flash", GetAbsOrigin(), vec3_angle);

		for (int i = 0; i < nAIs; i++)
		{
			if (ppAIs[i]->m_iClassname == iszNPCName)
			{
				playernpcdist.x = abs(UTIL_GetLocalPlayer()->GetAbsOrigin().x - ppAIs[i]->GetAbsOrigin().x);
				playernpcdist.y = abs(UTIL_GetLocalPlayer()->GetAbsOrigin().y - ppAIs[i]->GetAbsOrigin().y);

				if (playernpcdist.x <= m_flSkillAttributeRange && playernpcdist.y <= m_flSkillAttributeRange)
				{
					pOwner->SetHealth(pOwner->GetHealth() + 25);
				}
			}
		}
		m_iPlayerMP -= 50;

		m_nSkCoolDownTime4 = gpGlobals->curtime + 5.0f;
		m_bIsSkCoolDown4 = true;

	}

}


Vector effectpos;
bool bCanPullEnemies = false;
void CBaseMeleeWeapon::Skill_Trapping()
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
	string_t iszNPCName = AllocPooledString("npc_metropolice");
	Vector SkillOriginNPCdist;

	trace_t traceHit;

	Vector forward;
	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());
	Activity nHitActivity = ACT_VM_HITCENTER;
	
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	triggerInfo.ScaleDamage(2.0f);

	/*if (!m_bIsSkCoolDown5 && gpGlobals->curtime > m_nSkCoolDownTime5)
	{
		DevMsg("NOT cooldown \n");
		

	}
	else*/ 
	if (m_bIsSkCoolDown5 && gpGlobals->curtime < m_nSkCoolDownTime5)
	{
		for (int i = 0; i < nAIs; i++)
		{
			if (ppAIs[i]->m_iClassname == iszNPCName)
			{
				SkillOriginNPCdist.x = abs(effectpos.x - ppAIs[i]->GetAbsOrigin().x);
				SkillOriginNPCdist.y = abs(effectpos.y - ppAIs[i]->GetAbsOrigin().y);
				SkillOriginNPCdist.z = 0;

				if (m_nSkCoolDownTime5 - gpGlobals->curtime >= 1.0f)
				{
					if (SkillOriginNPCdist.x <= skillrange && SkillOriginNPCdist.y <= skillrange)
					{
						ppAIs[i]->SetCondition(COND_NPC_FREEZE);
						ppAIs[i]->SetRenderMode(kRenderTransColor);
						ppAIs[i]->SetRenderColor(128, 128, 128, 128);
					}
				}
				else if (m_nSkCoolDownTime5 - gpGlobals->curtime <= 1.0f)
				{
					if (SkillOriginNPCdist.x <= skillrange + 30 && SkillOriginNPCdist.y <= skillrange + 30)
					{
						ppAIs[i]->SetCondition(COND_NPC_UNFREEZE);
						ppAIs[i]->SetRenderMode(kRenderNormal);
					}
				}

				if ((m_nSkCoolDownTime5 - gpGlobals->curtime <= 2.5f) && (m_nSkCoolDownTime5 - gpGlobals->curtime >= 2.4f))
					bCanPullEnemies = true;

				if (bCanPullEnemies) //Need to make run once only
				{
					if (SkillOriginNPCdist.x <= skillrange && SkillOriginNPCdist.y <= skillrange)
						ppAIs[i]->ApplyAbsVelocityImpulse((effectpos - ppAIs[i]->GetAbsOrigin()));
					
					bCanPullEnemies = false;
					
				}

			}

		}
	}

	if (pOwner->m_nButtons & IN_SLOT4 && !m_bIsSkCoolDown5)
	{
		if (m_iPlayerMP > 10)
		{
			float fmagnitude = 350;
			effectpos = pOwner->GetAbsOrigin() + (fwd * fmagnitude);
			effectpos.z = 0;

			WeaponSound(SINGLE);
			pOwner->SetAnimation(PLAYER_SKILL_USE);
			RadiusDamage(triggerInfo, effectpos, 192.0f, CLASS_NONE, pOwner);
			m_nExecutionTime = gpGlobals->curtime + 1.0f;
			m_iPlayerMP -= 10;
			DispatchParticleEffect("aoehint", effectpos, vec3_angle);

			//Init Cooldown
			m_nSkCoolDownTime5 = gpGlobals->curtime + 3.0f;
			m_bIsSkCoolDown5 = true;
		}
		else
		{
			Warning("You don't have enough SG to execute this skill \n");
		}
	}

		
}

Vector skpos;
float flTorSkillRefireTime;
void CBaseMeleeWeapon::Skill_Tornado(void)
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

	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	triggerInfo.ScaleDamage(1.5f);


	if (m_bIsSkCoolDown6 && gpGlobals->curtime < m_nSkCoolDownTime6)
	{
		if (m_nSkCoolDownTime6 - gpGlobals->curtime >= 1.0f)
		{
			m_flSkillAttributeRange = skillrange;

			if (gpGlobals->curtime >= flTorSkillRefireTime)
			{
				AddKnockbackXY(1, 4);
				RadiusDamage(triggerInfo, skpos, skillrange, CLASS_PLAYER, pOwner);
				AddKnockbackXY(1, 5); //for npc hitting sound

				flTorSkillRefireTime = gpGlobals->curtime + 0.3f;
			}
		}
		else if (m_nSkCoolDownTime6 - gpGlobals->curtime <= 1.0f)
		{
			m_flSkillAttributeRange = skillrange;
			AddKnockbackXY(1, 3);
		}

		
	}

	if (pOwner->m_nButtons & IN_SLOT5 && !m_bIsSkCoolDown6)
	{
		if (m_iPlayerMP > 35)
		{
			WeaponSound(SINGLE);
			pOwner->SetAnimation(PLAYER_SKILL_USE);
			m_flSkillAttributeRange = skillrange;
			m_nExecutionTime = gpGlobals->curtime + 1.0f;
			m_iPlayerMP -= 35;

			skpos = pOwner->GetAbsOrigin();

			DispatchParticleEffect("aoehint", skpos, vec3_angle);
			GetPlayerPosOnce();
			//Init Cooldown
			flTorSkillRefireTime = gpGlobals->curtime + 0.3f;
			m_nSkCoolDownTime6 = gpGlobals->curtime + 4.0f;
			m_bIsSkCoolDown6 = true;
		}
		else
		{
			Warning("You don't have enough SG to execute this skill \n");
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
}

Vector playerPos;
void CBaseMeleeWeapon::GetPlayerPosOnce(void)
{
	playerPos = UTIL_GetLocalPlayer()->GetAbsOrigin();
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
		if (ppAIs[i]->m_iClassname == iszNPCName)
		{
				playernpcdist.x = abs(UTIL_GetLocalPlayer()->GetAbsOrigin().x - ppAIs[i]->GetAbsOrigin().x);
				playernpcdist.y = abs(UTIL_GetLocalPlayer()->GetAbsOrigin().y - ppAIs[i]->GetAbsOrigin().y);

				staticplayernpcdist.x = abs(playerPos.x - ppAIs[i]->GetAbsOrigin().x);
				staticplayernpcdist.y = abs(playerPos.y - ppAIs[i]->GetAbsOrigin().y);

				if (playernpcdist.x <= m_flSkillAttributeRange && playernpcdist.y <= m_flSkillAttributeRange)
				{
					m_iEnemyHealth = ppAIs[i]->GetHealth();

					if (ppAIs[i]->IsAlive())
					m_bIsEnemyInAtkRange = true;

					int NPCHealth = ppAIs[i]->GetHealth();
					if (options == 1)
					{
						ppAIs[i]->SetRenderMode(kRenderTransColor);
						ppAIs[i]->SetRenderColor(128,128,128,128);
						if ((NPCHealth > 0) && (NPCHealth < sk_npcknockbackathealth.GetInt()))
						ppAIs[i]->ApplyAbsVelocityImpulse(dirkb*flKnockbackVelocity);
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
	float m_nStepVelocity = 128.0f * magnitude;

	//Initialize vector fwd
	Vector fwd;
	//Get the player's viewangle and copy it to the fwd vector
	AngleVectors(UTIL_GetLocalPlayer()->GetAbsAngles(), &fwd);
	//Make sure the player won't accelerating upward when using the skill
	fwd.z = 0;
	//Normalize the vector so as not to making the value used going out of control
	VectorNormalize(fwd);
	//Give the player the push they need 
	UTIL_GetLocalPlayer()->SetAbsVelocity(fwd*m_nStepVelocity);
}