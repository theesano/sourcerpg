//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
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
#include "npc_metropolice.h"
#include "ai_eventresponse.h"



// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST(CBaseMeleeWeapon, DT_BaseMeleeWeapon)
END_SEND_TABLE()

#define BLUDGEON_HULL_DIM		16

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

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
	m_nExecutionTime = 0.0f;
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

	if (pOwner == NULL)
		return;

	if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		PrimaryAttack();
	}
	else if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		SecondaryAttack();
	}
	else
	{
		WeaponIdle();
		return;
	}
}

void CBaseMeleeWeapon::SkillsHandler(void)
{
	Vector fwd;
	AngleVectors(UTIL_GetLocalPlayer()->GetAbsAngles(), &fwd);

	////zero out vector angles
	fwd.z = 0;
	VectorNormalize(fwd);
	////UTIL_EntitiesInSphere

	//CBaseEntity *pEntity = NULL;
	//if ((pEntity = gEntList.FindEntityByClassnameNearest("npc_metropolice", UTIL_GetLocalPlayer()->GetAbsOrigin(), 192.0f)) != NULL)
	//{
	//	
	//}
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if ((pOwner->m_nButtons & IN_ATTACK2) && !m_bIsSkCoolDown)
	{
		Skill_Evade();
	}
	if ((pOwner->m_nButtons & IN_RELOAD) && !m_bIsSkCoolDown2)
	{
		Skill_RadialSlash();
	}
	if (gpGlobals->curtime < m_nExecutionTime)
	{
		UTIL_GetLocalPlayer()->SetAbsVelocity(vec3_origin);
	}

	if (pOwner->m_nButtons & IN_SCORE)
	{
		Warning("Lole time %.2f \n", gpGlobals->curtime - m_nExecutionTime);
		if (m_nExecutionTime >= 0)
		{
			m_nExecutionTime = 0.0f;
			pOwner->ApplyAbsVelocityImpulse(fwd * 96);
		}
	}
	if (gpGlobals->curtime - m_nSkCoolDownTime < 0)
	{
		float cdtimer = gpGlobals->curtime - m_nSkCoolDownTime;
		DevMsg("Spinning Demon %.2f \n ", cdtimer);
		m_bIsSkCoolDown = false;
	}
	if (gpGlobals->curtime - m_nSkCoolDownTime2 < 0)
	{
		float cdtimer = gpGlobals->curtime - m_nSkCoolDownTime2;
		DevMsg("Evil Slash CD  %.2f \n ", cdtimer);
		m_bIsSkCoolDown2 = false;
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

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CBaseMeleeWeapon::SecondaryAttack()
{
	Swing2(true);
}


//------------------------------------------------------------------------------
// Purpose: Implement impact function
//------------------------------------------------------------------------------
void CBaseMeleeWeapon::Hit(trace_t &traceHit, Activity nHitActivity, bool bIsSecondary)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	//Do view kick
	AddViewKick();

	//Make sound for the AI
	CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer);

	// This isn't great, but it's something for when the crowbar hits.
	pPlayer->RumbleEffect(RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART);

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if (pHitEntity != NULL)
	{
		float m_nDamageRadius = 128.0f;
		Vector hit1 = GetAbsOrigin();
		Vector hitDirection;
		pPlayer->EyeVectors(&hitDirection, NULL, NULL);
		VectorNormalize(hitDirection);

		Vector	dir = traceHit.endpos;
		Vector vecForce = dir * ImpulseScale(75, 700);
		VectorNormalize(dir);
		dir *= 500.0f;
		ApplyAbsVelocityImpulse(dir);

		CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);
		//CTakeDamageInfo info(this, m_hThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported);

		//Makes weapon produce AoE damage
		//RadiusDamage(info, hit1, m_nDamageRadius, CLASS_NONE, NULL);

		if (pPlayer && pHitEntity->IsNPC())
		{
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel();
		}

		UTIL_GetLocalPlayer()->SetAbsVelocity(vec3_origin);



		CalculateMeleeDamageForce( &info, hitDirection, traceHit.endpos );

		pHitEntity->DispatchTraceAttack( info, hitDirection, &traceHit ); 
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers( info, traceHit.startpos, traceHit.endpos, hitDirection );

		if (ToBaseCombatCharacter(pHitEntity))
		{
			gamestats->Event_WeaponHit(pPlayer, !bIsSecondary, GetClassname(), info);
		}
	}

	// Apply an impact effect
	ImpactEffect(traceHit);
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
// Input  : &traceHit - 
//-----------------------------------------------------------------------------
bool CBaseMeleeWeapon::ImpactWater(const Vector &start, const Vector &end)
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...

	// We must start outside the water
	if (UTIL_PointContents(start) & (CONTENTS_WATER | CONTENTS_SLIME))
		return false;

	// We must end inside of water
	if (!(UTIL_PointContents(end) & (CONTENTS_WATER | CONTENTS_SLIME)))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine(start, end, (CONTENTS_WATER | CONTENTS_SLIME), GetOwner(), COLLISION_GROUP_NONE, &waterTrace);

	if (waterTrace.fraction < 1.0f)
	{
		CEffectData	data;

		data.m_fFlags = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if (waterTrace.contents & CONTENTS_SLIME)
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect("watersplash", data);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseMeleeWeapon::ImpactEffect(trace_t &traceHit)
{
	// See if we hit water (we don't do the other impact effects in this case)
	if (ImpactWater(traceHit.startpos, traceHit.endpos))
		return;

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
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	Activity nHitActivity = ACT_VM_HITCENTER;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_SLASH);
	triggerInfo.SetDamagePosition( traceHit.startpos );
	triggerInfo.SetDamageForce( forward );
	TraceAttackToTriggers( triggerInfo, traceHit.startpos, traceHit.endpos, forward );

	Vector fwd;
	AngleVectors(UTIL_GetLocalPlayer()->GetAbsAngles(), &fwd);

	//zero out vector angles
	fwd.z = 0;
	VectorNormalize(fwd);
	//Makes weapon produce AoE damage
	RadiusDamage(triggerInfo, swingEnd, m_nDamageRadius, CLASS_NONE, pOwner);

	Vector KnockBackdir = fwd;
		KnockBackdir *= 500.0f;
		KnockBackdir.z = 128.0f;
		AddKnockback(KnockBackdir);
	//Stops player from moving for each swing
	UTIL_GetLocalPlayer()->SetAbsVelocity(vec3_origin);
	//Move player forward for each swing.
	AddSkillMovementImpulse(1.0f);
	//Hard coded value, should change to SequenceDuration()
	m_nExecutionTime = gpGlobals->curtime +	1.3f;


	//if ( traceHit.fraction == 1.0 )
	//{
	//	float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

	//	// Back off by hull "radius"
	//	swingEnd -= forward * bludgeonHullRadius;

	//	UTIL_TraceHull( swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit );
	//	if ( traceHit.fraction < 1.0 && traceHit.m_pEnt )
	//	{
	//		Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
	//		VectorNormalize( vecToTarget );

	//		float dot = vecToTarget.Dot( forward );

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
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired(pOwner, !bIsSecondary, GetClassname());

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		nHitActivity = bIsSecondary ? ACT_VM_MISSCENTER2 : ACT_VM_MISSCENTER;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetRange();

		// See if we happened to hit water
		ImpactWater(swingStart, testEnd);
	}
	else
	{
		Hit( traceHit, nHitActivity, bIsSecondary ? true : false );
	}

	// Send the anim
	SendWeaponAnim(nHitActivity);

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

	//Play swing sound
	WeaponSound(SINGLE);
	pOwner->SetAnimation(PLAYER_ATTACK1);
}

//Secondary Attack Swing
//------------------------------------------------------------------------------
// Purpose : Starts the swing of the weapon and determines the animation
// Input   : bIsSecondary - is this a secondary attack?
//------------------------------------------------------------------------------
void CBaseMeleeWeapon::Swing2(int bIsSecondary)
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	Vector swingEnd = swingStart + forward * GetRange();
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	Activity nHitActivity = ACT_VM_HITCENTER;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity(traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner);
			}
		}
	}

	if (!bIsSecondary)
	{
		m_iPrimaryAttacks++;
	}
	else
	{
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired(pOwner, !bIsSecondary, GetClassname());

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		nHitActivity = bIsSecondary ? ACT_VM_MISSCENTER2 : ACT_VM_MISSCENTER;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetRange();

		// See if we happened to hit water
		ImpactWater(swingStart, testEnd);
	}
	else
	{
		Hit(traceHit, nHitActivity, bIsSecondary ? true : false);
	}

	// Send the anim
	SendWeaponAnim(nHitActivity);

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

	//Play swing sound
	WeaponSound(SINGLE);
	pOwner->SetAnimation(PLAYER_ATTACK1);
}
//Skill: Spinning Demon.
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
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);
	float m_nDamageRadius = 192.0f;
	//Only run when the cooldown time is 0
	if (!m_bIsSkCoolDown && gpGlobals->curtime > m_nSkCoolDownTime)
	{
		m_nExecutionTime = 0.0f;
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
		if (!pOwner)
			return;
		float m_nStepVelocity = 1024.0f;
		Vector fwd;
		AngleVectors(UTIL_GetLocalPlayer()->GetAbsAngles(), &fwd);
		fwd.z = 0;
		VectorNormalize(fwd);
		AddKnockback(fwd * 350);
		UTIL_GetLocalPlayer()->SetAbsVelocity(fwd*m_nStepVelocity);

			RadiusDamage(triggerInfo, UTIL_GetLocalPlayer()->GetAbsOrigin(), m_nDamageRadius, CLASS_NONE, pOwner);
			WeaponSound(SINGLE);
			pOwner->SetAnimation(PLAYER_ATTACK1);

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
	triggerInfo.ScaleDamage(3.5f);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	//The area in radius that the skill is going to affect.
	float m_nDamageRadius = 264.0f;

	if (!m_bIsSkCoolDown2 && gpGlobals->curtime > m_nSkCoolDownTime2)
	{
		//Initialize the variable for moving the player on each attack
		AddSkillMovementImpulse(1.0f);
		m_nExecutionTime = gpGlobals->curtime + 2.0f;

		if (gpGlobals->curtime - m_nExecutionTime < 0)
		{
			RadiusDamage(triggerInfo, UTIL_GetLocalPlayer()->GetAbsOrigin(), m_nDamageRadius, CLASS_NONE, pOwner); //Attack
			WeaponSound(SINGLE);
			pOwner->SetAnimation(PLAYER_ATTACK1);
		}
		m_nSkCoolDownTime2 = gpGlobals->curtime + 7.0f;
		m_bIsSkCoolDown2 = true;
	}

}

void CBaseMeleeWeapon::AddKnockback(Vector dir)
{
	//UTIL_EntitiesInSphere

	CBaseEntity *pEntity = NULL;
	if ((pEntity = gEntList.FindEntityByClassnameNearest("npc_metropolice", UTIL_GetLocalPlayer()->GetAbsOrigin(), 192.0f)) != NULL)
	{
		CNPC_MetroPolice *pCop = dynamic_cast<CNPC_MetroPolice *>(pEntity);
		pCop->ApplyAbsVelocityImpulse(dir);

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
	//Make sure the player won't accelerating forward when using the skill
	fwd.z = 0;
	//Normalize the vector so as not to making the value used going out of control
	VectorNormalize(fwd);

	//Give the player the push they need 
	UTIL_GetLocalPlayer()->SetAbsVelocity(fwd*m_nStepVelocity);
}