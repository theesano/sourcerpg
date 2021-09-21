//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "soundent.h"
#include "npcevent.h"
#include "globalstate.h"
#include "ai_squad.h"
#include "ai_tacticalservices.h"
#include "npc_bob2.h"
#include "ai_route.h"
#include "hl2_player.h"
#include "iservervehicle.h"
#include "items.h"
#include "hl2_gamerules.h"
#include "usermessages.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//#define SF_BOB_					0x00010000
#define SF_BOB_NOCHATTER			0x00100000
#define SF_BOB_ALLOWED_TO_RESPOND	0x01000000
#define SF_BOB_MID_RANGE_ATTACK		0x02000000


#define BOB_MID_RANGE_ATTACK_RANGE	3500.0f

#define RECENT_DAMAGE_INTERVAL		3.0f
#define RECENT_DAMAGE_THRESHOLD		0.2f



enum SpeechMemory_t
{
	bits_MEMORY_PAIN_LIGHT_SOUND = bits_MEMORY_CUSTOM1,
	bits_MEMORY_PAIN_HEAVY_SOUND = bits_MEMORY_CUSTOM2,
	bits_MEMORY_PLAYER_HURT = bits_MEMORY_CUSTOM3,
	bits_MEMORY_PLAYER_HARASSED = bits_MEMORY_CUSTOM4,
};


extern int g_interactionHitByPlayerThrownPhysObj;

ConVar	sk_bob2_health("sk_bob2_health", "0");

ConVar  bob2_move_and_melee("bob2_move_and_melee", "0");
ConVar	bob2_melee_damage("bob2_melee_damage", "20");
ConVar  bob2_charge("bob2_charge", "1");

//ConVar lilyss_bob2_model("lilyss_bob_model", "models/monster/mob1_puppet.mdl");
ConVar lilyss_bob2_model("lilyss_bob2_model", "models/monster/bob2.mdl");

// -----------------------------------------------
//	> Squad slots
// -----------------------------------------------
enum SquadSlot_T
{
	SQUAD_SLOT_POLICE_CHARGE_ENEMY = LAST_SHARED_SQUADSLOT,
	SQUAD_SLOT_POLICE_HARASS, // Yell at the player with a megaphone, etc.
	SQUAD_SLOT_POLICE_ADVANCE,
	SQUAD_SLOT_POLICE_ATTACK_OCCLUDER1,
	SQUAD_SLOT_POLICE_ATTACK_OCCLUDER2,
	SQUAD_SLOT_POLICE_COVERING_FIRE1,
	SQUAD_SLOT_POLICE_COVERING_FIRE2,
	//	SQUAD_SLOT_POLICE_ARREST_ENEMY,
};
//==============================================
// Custom Activities
//==============================================

LINK_ENTITY_TO_CLASS(npc_bob2, CNPC_Bob2);

BEGIN_DATADESC(CNPC_Bob2)

DEFINE_EMBEDDED(m_BatonSwingTimer),
DEFINE_EMBEDDED(m_NextChargeTimer),

DEFINE_FIELD(m_flLastPhysicsFlinchTime, FIELD_TIME),
DEFINE_FIELD(m_flLastDamageFlinchTime, FIELD_TIME),

DEFINE_FIELD(m_hBlockingProp, FIELD_EHANDLE),

DEFINE_FIELD(m_nRecentDamage, FIELD_INTEGER),
DEFINE_FIELD(m_flRecentDamageTime, FIELD_TIME),

DEFINE_FIELD(m_flNextPainSoundTime, FIELD_TIME),
DEFINE_FIELD(m_flNextLostSoundTime, FIELD_TIME),

DEFINE_FIELD(m_flLastHitYaw, FIELD_FLOAT),

DEFINE_FIELD(m_bPlayerTooClose, FIELD_BOOLEAN),
DEFINE_FIELD(m_bKeepFacingPlayer, FIELD_BOOLEAN),
DEFINE_FIELD(m_flChasePlayerTime, FIELD_TIME),
DEFINE_FIELD(m_vecPreChaseOrigin, FIELD_VECTOR),
DEFINE_FIELD(m_flPreChaseYaw, FIELD_FLOAT),
DEFINE_FIELD(m_iNumPlayerHits, FIELD_INTEGER),

//								m_ActBusyBehavior (auto saved by AI)
//								m_StandoffBehavior (auto saved by AI)
//								m_AssaultBehavior (auto saved by AI)
//								m_FuncTankBehavior (auto saved by AI)
//								m_PolicingBehavior (auto saved by AI)
//								m_FollowBehavior (auto saved by AI)

END_DATADESC()

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Purpose 
//------------------------------------------------------------------------------
CBaseEntity *CNPC_Bob2::CheckTraceHullAttack(float flDist, const Vector &mins, const Vector &maxs, int iDamage, int iDmgType, float forceScale, bool bDamageAnyNPC)
{
	// If only a length is given assume we want to trace in our facing direction
	Vector forward;
	AngleVectors(GetAbsAngles(), &forward);
	Vector vStart = GetAbsOrigin();

	// The ideal place to start the trace is in the center of the attacker's bounding box.
	// however, we need to make sure there's enough clearance. Some of the smaller monsters aren't 
	// as big as the hull we try to trace with. (SJB)
	float flVerticalOffset = WorldAlignSize().z * 0.5;

	if (flVerticalOffset < maxs.z)
	{
		// There isn't enough room to trace this hull, it's going to drag the ground.
		// so make the vertical offset just enough to clear the ground.
		flVerticalOffset = maxs.z + 1.0;
	}

	vStart.z += flVerticalOffset;
	Vector vEnd = vStart + (forward * flDist);
	return CheckTraceHullAttack(vStart, vEnd, mins, maxs, iDamage, iDmgType, forceScale, bDamageAnyNPC);
}

//------------------------------------------------------------------------------
// Melee filter for Bobs
//------------------------------------------------------------------------------
class CTraceFilterBob : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE(CTraceFilterBob);

	CTraceFilterBob(const IHandleEntity *passentity, int collisionGroup, CTakeDamageInfo *dmgInfo, float flForceScale, bool bDamageAnyNPC)
		: m_pPassEnt(passentity), m_collisionGroup(collisionGroup), m_dmgInfo(dmgInfo), m_pHit(NULL), m_flForceScale(flForceScale), m_bDamageAnyNPC(bDamageAnyNPC)
	{
	}

	virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
	{
		if (!StandardFilterRules(pHandleEntity, contentsMask))
			return false;

		if (!PassServerEntityFilter(pHandleEntity, m_pPassEnt))
			return false;

		// Don't test if the game code tells us we should ignore this collision...
		CBaseEntity *pEntity = EntityFromEntityHandle(pHandleEntity);

		if (pEntity)
		{
			if (!pEntity->ShouldCollide(m_collisionGroup, contentsMask))
				return false;

			if (!g_pGameRules->ShouldCollide(m_collisionGroup, pEntity->GetCollisionGroup()))
				return false;

			if (pEntity->m_takedamage == DAMAGE_NO)
				return false;

			Vector	attackDir = pEntity->WorldSpaceCenter() - m_dmgInfo->GetAttacker()->WorldSpaceCenter();
			VectorNormalize(attackDir);

			CTakeDamageInfo info = (*m_dmgInfo);
			CalculateMeleeDamageForce(&info, attackDir, info.GetAttacker()->WorldSpaceCenter(), m_flForceScale);

			if (!(pEntity->GetFlags() & FL_ONGROUND))
			{
				// Don't hit airborne entities so hard. They fly farther since
				// there's no friction with the ground.
				info.ScaleDamageForce(0.001);
			}

			CBaseCombatCharacter *pBCC = info.GetAttacker()->MyCombatCharacterPointer();
			CBaseCombatCharacter *pVictimBCC = pEntity->MyCombatCharacterPointer();

			// Only do these comparisons between NPCs
			if (pBCC && pVictimBCC)
			{
				// Can only damage other NPCs that we hate
				if (m_bDamageAnyNPC || pBCC->IRelationType(pEntity) == D_HT || pEntity->IsPlayer())
				{
					if (info.GetDamage())
					{
						// If gordon's a criminal, do damage now
					//	if (!pEntity->IsPlayer())
						if (pEntity->IsPlayer())
						{
//							if (pEntity->IsPlayer() && ((CBasePlayer *)pEntity)->IsSuitEquipped())
//							{
//								info.ScaleDamage(.25);
//								info.ScaleDamageForce(.25);
//							}

							pEntity->TakeDamage(info);
						}
					}

					m_pHit = pEntity;
					return true;
				}
			}
			else
			{
				// Make sure if the player is holding this, he drops it
				//Pickup_ForcePlayerToDropThisObject(pEntity);

				// Otherwise just damage passive objects in our way
				if (info.GetDamage())
				{
					pEntity->TakeDamage(info);
				}
			}
		}

		return false;
	}

public:
	const IHandleEntity *m_pPassEnt;
	int					m_collisionGroup;
	CTakeDamageInfo		*m_dmgInfo;
	CBaseEntity			*m_pHit;
	float				m_flForceScale;
	bool				m_bDamageAnyNPC;
};

//------------------------------------------------------------------------------
// Purpose :	start and end trace position, amount 
//				of damage to do, and damage type. Returns a pointer to
//				the damaged entity in case the NPC wishes to do
//				other stuff to the victim (punchangle, etc)
//
//				Used for many contact-range melee attacks. Bites, claws, etc.
// Input   :
// Output  :
//------------------------------------------------------------------------------
CBaseEntity *CNPC_Bob2::CheckTraceHullAttack(const Vector &vStart, const Vector &vEnd, const Vector &mins, const Vector &maxs, int iDamage, int iDmgType, float flForceScale, bool bDamageAnyNPC)
{

	CTakeDamageInfo	dmgInfo(this, this, iDamage, DMG_SLASH);

	CTraceFilterBob traceFilter(this, COLLISION_GROUP_NONE, &dmgInfo, flForceScale, bDamageAnyNPC);

	Ray_t ray;
	ray.Init(vStart, vEnd, mins, maxs);

	trace_t tr;
	enginetrace->TraceRay(ray, MASK_SHOT, &traceFilter, &tr);

	CBaseEntity *pEntity = traceFilter.m_pHit;

	if (pEntity == NULL)
	{
		// See if perhaps I'm trying to claw/bash someone who is standing on my head.
		Vector vecTopCenter;
		Vector vecEnd;
		Vector vecMins, vecMaxs;

		// Do a tracehull from the top center of my bounding box.
		vecTopCenter = GetAbsOrigin();
		CollisionProp()->WorldSpaceAABB(&vecMins, &vecMaxs);
		vecTopCenter.z = vecMaxs.z + 1.0f;
		vecEnd = vecTopCenter;
		vecEnd.z += 2.0f;

		ray.Init(vecTopCenter, vEnd, mins, maxs);
		enginetrace->TraceRay(ray, MASK_SHOT_HULL, &traceFilter, &tr);

		pEntity = traceFilter.m_pHit;
	}

	return pEntity;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CNPC_Bob2::CNPC_Bob2()
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob2::OnScheduleChange()
{
	BaseClass::OnScheduleChange();

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob2::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	if (IsOnFire())
	{
		SetCondition(COND_BOB2_ON_FIRE);
	}
	else
	{
		ClearCondition(COND_BOB2_ON_FIRE);
	}

	if (gpGlobals->curtime > m_flRecentDamageTime + RECENT_DAMAGE_INTERVAL)
	{
		m_nRecentDamage = 0;
		m_flRecentDamageTime = 0;
	}
	
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &move - 
//			flInterval - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Bob2::OverrideMoveFacing(const AILocalMoveGoal_t &move, float flInterval)
{
	// Don't do this if we're scripted
	if (IsInAScript())
		return BaseClass::OverrideMoveFacing(move, flInterval);

	// ROBIN: Disabled at request of mapmakers for now
	/*
	// If we're moving during a police sequence, always face our target
	if ( m_PolicingBehavior.IsEnabled() )
	{
	CBaseEntity *pTarget = m_PolicingBehavior.GetGoalTarget();

	if ( pTarget )
	{
	AddFacingTarget( pTarget, pTarget->WorldSpaceCenter(), 1.0f, 0.2f );
	}
	}
	*/

	return BaseClass::OverrideMoveFacing(move, flInterval);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob2::Precache(void)
{
	string_t iszNPCName = AllocPooledString(lilyss_bob2_model.GetString());

	string_t iszNPCName_Cheap = AllocPooledString("models/police.mdl");

	if (HasSpawnFlags(SF_NPC_START_EFFICIENT))
	{
		SetModelName(iszNPCName_Cheap);
	}
	else
	{
		SetModelName(iszNPCName);
	}

	PrecacheModel(STRING(GetModelName()));

	PrecacheScriptSound("NPC_Metropolice.Shove");
	PrecacheScriptSound("NPC_MetroPolice.WaterSpeech");
	PrecacheScriptSound("NPC_MetroPolice.HidingSpeech");

	PrecacheScriptSound("NPC_HeadCrab.Idle");
	PrecacheScriptSound("NPC_HeadCrab.Alert");
	PrecacheScriptSound("NPC_HeadCrab.Pain");
	PrecacheScriptSound("NPC_HeadCrab.Die");
	PrecacheScriptSound("NPC_HeadCrab.Attack");

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Create components
//-----------------------------------------------------------------------------
bool CNPC_Bob2::CreateComponents()
{
	if (!BaseClass::CreateComponents())
		return false;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_Bob2::Spawn(void)
{
	Precache();

#ifdef _XBOX
	// Always fade the corpse
	AddSpawnFlags(SF_NPC_FADE_CORPSE);
#endif // _XBOX

	SetModel(STRING(GetModelName()));

	SetHullType(HULL_MEDIUM_TALL);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(DONT_BLEED);

	if (HasSpawnFlags(SF_BOB_NOCHATTER))
	{
		AddSpawnFlags(SF_NPC_GAG);
	}

	m_iHealth = sk_bob2_health.GetFloat();
	m_iArmor = 100;

	m_flFieldOfView = -0.2;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;
	if (!HasSpawnFlags(SF_NPC_START_EFFICIENT))
	{
		CapabilitiesAdd(bits_CAP_TURN_HEAD | bits_CAP_ANIMATEDFACE);
	}
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1); //added innate melee attack 1
	CapabilitiesAdd(bits_CAP_NO_HIT_SQUADMATES);
	CapabilitiesAdd(bits_CAP_SQUAD);


	NPCInit();

	// NOTE: This must occur *after* init, since init sets default dist look
	if (HasSpawnFlags(SF_BOB_MID_RANGE_ATTACK))
	{
	m_flDistTooFar = BOB_MID_RANGE_ATTACK_RANGE;
		SetDistLook(BOB_MID_RANGE_ATTACK_RANGE);
	}


	GetEnemies()->SetFreeKnowledgeDuration(6.0);

	m_bPlayerTooClose = false;
	m_bKeepFacingPlayer = false;
	m_flChasePlayerTime = 0;
	m_vecPreChaseOrigin = vec3_origin;
	m_flPreChaseYaw = 0;

}

//-----------------------------------------------------------------------------
// Behaviors! Lovely behaviors
//-----------------------------------------------------------------------------
bool CNPC_Bob2::CreateBehaviors()
{
	AddBehavior(&m_ActBusyBehavior);
	AddBehavior(&m_AssaultBehavior);

	return BaseClass::CreateBehaviors();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//-----------------------------------------------------------------------------
void CNPC_Bob2::AlertSound(void)
{
	EmitSound("NPC_HeadCrab.Alert");
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//-----------------------------------------------------------------------------
void CNPC_Bob2::DeathSound(const CTakeDamageInfo &info)
{
	if (IsOnFire())
		return;

	EmitSound("NPC_HeadCrab.Die");

}


//-----------------------------------------------------------------------------
// Purpose: implemented by subclasses to give them an opportunity to make
//			a sound when they lose their enemy
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_Bob2::LostEnemySound(void)
{

	if (gpGlobals->curtime <= m_flNextLostSoundTime)
		return;

}


//-----------------------------------------------------------------------------
// Purpose: implemented by subclasses to give them an opportunity to make
//			a sound when they lose their enemy
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_Bob2::FoundEnemySound(void)
{
	//emitsound("foundenemy");
}

//-----------------------------------------------------------------------------
// IdleSound 
//-----------------------------------------------------------------------------
void CNPC_Bob2::IdleSound(void)
{
	EmitSound("NPC_HeadCrab.Idle");

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob2::PainSound(const CTakeDamageInfo &info)
{
	if (gpGlobals->curtime < m_flNextPainSoundTime)
		return;

	// Don't make pain sounds if I'm on fire. The looping sound will take care of that for us.
	if (IsOnFire())
		return;

	float healthRatio = (float)GetHealth() / (float)GetMaxHealth();
	if (healthRatio > 0.0f)
	{
		if (!HasMemory(bits_MEMORY_PAIN_HEAVY_SOUND) && (healthRatio < 0.25f))
		{
			EmitSound("NPC_HeadCrab.Pain");
			Remember(bits_MEMORY_PAIN_HEAVY_SOUND | bits_MEMORY_PAIN_LIGHT_SOUND);
		}
		else if (!HasMemory(bits_MEMORY_PAIN_LIGHT_SOUND) && healthRatio > 0.8f)
		{
			EmitSound("NPC_HeadCrab.Pain");
			Remember(bits_MEMORY_PAIN_LIGHT_SOUND);
		}

		// This causes it to speak it no matter what; doesn't bother with setting sounds.
		m_flNextPainSoundTime = gpGlobals->curtime + 1;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_Bob2::GetSoundInterests(void)
{
	return SOUND_WORLD | SOUND_COMBAT | SOUND_PLAYER | SOUND_PLAYER_VEHICLE | SOUND_DANGER |
		SOUND_PHYSICS_DANGER | SOUND_BULLET_IMPACT | SOUND_MOVE_AWAY;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CNPC_Bob2::MaxYawSpeed(void)
{
	switch (GetActivity())
	{
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		return 120;

	case ACT_RUN:
	case ACT_RUN_HURT:
		return 15;

	case ACT_WALK:
	case ACT_WALK_CROUCH:
	case ACT_RUN_CROUCH:
		return 25;

	default:
		return 45;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
Class_T	CNPC_Bob2::Classify(void)
{
	return CLASS_BOB;
}

//-----------------------------------------------------------------------------
// Purpose: Overridden because if the player is a criminal, we hate them.
// Input  : pTarget - Entity with which to determine relationship.
// Output : Returns relationship value.
//-----------------------------------------------------------------------------
Disposition_t CNPC_Bob2::IRelationType(CBaseEntity *pTarget)
{
	Disposition_t disp = BaseClass::IRelationType(pTarget);

	if (pTarget == NULL)
		return disp;

	// If the player's not a criminal, then we don't necessary hate him
	if (pTarget->Classify() == CLASS_PLAYER)
	{

	}

	return disp;
}

//-----------------------------------------------------------------------------
// Purpose: Get our conditions for a melee attack
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Bob2::MeleeAttack1Conditions(float flDot, float flDist)
{
	Vector	predictedDir = ((GetEnemy()->GetAbsOrigin() + (GetEnemy()->GetSmoothedVelocity())) - GetAbsOrigin());
	float	flPredictedDist = VectorNormalize(predictedDir);

	Vector	vBodyDir;
	GetVectors(&vBodyDir, NULL, NULL);

	float	flPredictedDot = DotProduct(predictedDir, vBodyDir);

	if (flPredictedDot < 0.8f)
		return COND_NOT_FACING_ATTACK;

	if ((flPredictedDist >(GetAbsVelocity().Length() * 0.5f)) && (flDist > 128.0f))
		return COND_TOO_FAR_TO_ATTACK;

	return COND_CAN_MELEE_ATTACK1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob2::OnAnimEventMeleeAttack1(void)
{
	CBaseEntity *pHurt = CheckTraceHullAttack(24, Vector(-16, -16, -16), Vector(24, 24, 24), bob2_melee_damage.GetFloat(), DMG_CLUB, 1.0f, false);

	if (pHurt)
	{
		Vector vecForceDir = (pHurt->WorldSpaceCenter() - WorldSpaceCenter());

		CBasePlayer *pPlayer = ToBasePlayer(pHurt);
		CHL2_Player *pPlayer2 = dynamic_cast<CHL2_Player *>(pPlayer);

		if (pPlayer != NULL)
		{
			//Kick the player angles
			pPlayer->ViewPunch(QAngle(8, 14, 0));

			Vector	dir = pHurt->GetAbsOrigin() - GetAbsOrigin();
			VectorNormalize(dir);

			QAngle angles;
			VectorAngles(dir, angles);
			Vector forward, right;
			AngleVectors(angles, &forward, &right, NULL);

			//If not on ground, then don't make them fly!
			if (!(pHurt->GetFlags() & FL_ONGROUND))
				forward.z = 0.0f;

			if (!(pPlayer->GetFlags() & FL_FROZEN_ACT))
			{
					dir *= 256.0f;
					pHurt->ApplyAbsVelocityImpulse(dir);
					pPlayer2->SetDebuff(DEBUFF_STATE_KNOCKBACK);
			}

		
		}

		// Play a random attack hit sound
		EmitSound("NPC_Metropolice.Shove");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : *pEvent - 
//
//-----------------------------------------------------------------------------
void CNPC_Bob2::HandleAnimEvent(animevent_t *pEvent)
{
	if (pEvent->event == 11)
	{
		OnAnimEventMeleeAttack1();
	}

	if (pEvent->event == AE_NPC_WEAPON_SET_ACTIVITY)
	{		
		// dont do anything just ignore the warning 
		return;
	}

	BaseClass::HandleAnimEvent(pEvent);
}


//-----------------------------------------------------------------------------
// Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
// Input  :  Constant for the type of interaction
// Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CNPC_Bob2::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{

	// React to being hit by physics objects
	if (interactionType == g_interactionHitByPlayerThrownPhysObj)
	{
		// Ignore if I'm in scripted state
		if (!IsInAScript() && (m_NPCState != NPC_STATE_SCRIPT))
		{
			SetCondition(COND_BOB2_PHYSOBJECT_ASSAULT);
		}
		else
		{
			AdministerJustice();
		}

		return true;
	}

	return BaseClass::HandleInteraction(interactionType, data, sourceEnt);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Activity CNPC_Bob2::NPC_TranslateActivity(Activity newActivity)
{
	if (IsOnFire() && newActivity == ACT_RUN)
	{
		return ACT_RUN_ON_FIRE;
	}

	// If we're shoving, see if we should be more forceful in doing so
	if (newActivity == ACT_PUSH_PLAYER)
	{
		return ACT_MELEE_ATTACK1;
	}

	newActivity = BaseClass::NPC_TranslateActivity(newActivity);

	return newActivity;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CNPC_Bob2::Event_Killed(const CTakeDamageInfo &info)
{
	//int iRNGSim;
	//iRNGSim = random->RandomInt(1, 2); //50% chance of dropping

	CBasePlayer *pPlayer = ToBasePlayer(info.GetAttacker());

	if (pPlayer != NULL)
	{
		CHalfLife2 *pHL2GameRules = static_cast<CHalfLife2 *>(g_pGameRules);
		//if (iRNGSim == 1)//50% chance of dropping
			//DropItem("item_rage", WorldSpaceCenter() + RandomVector(-4, 4), RandomAngle(0, 360));

		// Attempt to drop health
		//if (pHL2GameRules->NPC_ShouldDropHealth(pPlayer))
		//{
		//	DropItem("item_healthvial", WorldSpaceCenter() + RandomVector(-4, 4), RandomAngle(0, 360));
		//	//DropItem("item_rotating", WorldSpaceCenter() + RandomVector(-4, 4), RandomAngle(0, 360));
		//	pHL2GameRules->NPC_DroppedHealth();
		//}
	}

	BaseClass::Event_Killed(info);
}

//-----------------------------------------------------------------------------
// Combat schedule selection 
//-----------------------------------------------------------------------------
int CNPC_Bob2::SelectScheduleNewEnemy()
{
	//	int nSched = SelectScheduleArrestEnemy();
	//	if (nSched != SCHED_NONE)
	//		return nSched;

	if (HasCondition(COND_NEW_ENEMY))
	{
		//m_flNextLedgeCheckTime = gpGlobals->curtime;
	}

	return SCHED_NONE;
}


//-----------------------------------------------------------------------------
// Sound investigation 
//-----------------------------------------------------------------------------
int CNPC_Bob2::SelectScheduleInvestigateSound()
{
	// SEE_ENEMY is set if LOS is available *and* we're looking the right way
	// Don't investigate if the player's not a criminal.
	if (!HasCondition(COND_SEE_ENEMY))
	{
		if (HasCondition(COND_HEAR_COMBAT) || HasCondition(COND_HEAR_PLAYER))
		{
			if (m_pSquad && OccupyStrategySlot(SQUAD_SLOT_INVESTIGATE_SOUND))
			{
				return SCHED_BOB2_INVESTIGATE_SOUND;
			}
		}
	}

	return SCHED_NONE;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_Bob2::OnObstructionPreSteer(AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult)
{
	if (pMoveGoal->directTrace.pObstruction)
	{
		// Is it a physics prop? Store it off as the last thing to block me
		CPhysicsProp *pProp = dynamic_cast<CPhysicsProp*>(pMoveGoal->directTrace.pObstruction);
		if (pProp && pProp->GetHealth())
		{
			m_hBlockingProp = pProp;
		}
		else
		{
			m_hBlockingProp = NULL;
		}
	}

	return BaseClass::OnObstructionPreSteer(pMoveGoal, distClear, pResult);
}

//-----------------------------------------------------------------------------
// Combat schedule selection 
//-----------------------------------------------------------------------------
int CNPC_Bob2::SelectScheduleNoDirectEnemy()
{
	// If you can't attack, but you have a baton & there's a physics object in front of you, swat it
	if (m_hBlockingProp) // && HasBaton
	{
		SetTarget(m_hBlockingProp);
		m_hBlockingProp = NULL;
		return SCHED_BOB2_SMASH_PROP;
	}

	//return SCHED_BOB2_CHASE_ENEMY;
	return SCHED_CHASE_ENEMY;
}


//-----------------------------------------------------------------------------
// Combat schedule selection 
//-----------------------------------------------------------------------------
int CNPC_Bob2::SelectCombatSchedule()
{
	// Announce a new enemy
	if (HasCondition(COND_NEW_ENEMY))
	{
	}

	int nResult = SelectScheduleNewEnemy();
	if (nResult != SCHED_NONE)
		return nResult;

	if (((float)m_nRecentDamage / (float)GetMaxHealth()) > RECENT_DAMAGE_THRESHOLD)
	{
		m_nRecentDamage = 0;
		m_flRecentDamageTime = 0;

		return SCHED_TAKE_COVER_FROM_ENEMY;
	}

	//if (HasCondition(COND_CAN_RANGE_ATTACK1))
	//{
	//	if (!GetShotRegulator()->IsInRestInterval())
	//		return SelectRangeAttackSchedule();
	//	else
	//		return SCHED_BOB_ADVANCE;
	//}

	if (HasCondition(COND_CAN_MELEE_ATTACK1))
	{
		if (m_BatonSwingTimer.Expired())
		{
			// Stop chasing the player now that we've taken a swing at them
			m_flChasePlayerTime = 0;
			m_BatonSwingTimer.Set(1.8,2.0);
			return SCHED_MELEE_ATTACK1;
		}
		else
			return SCHED_COMBAT_FACE;
	}

	if (HasCondition(COND_TOO_CLOSE_TO_ATTACK))
	{
		return SCHED_BACK_AWAY_FROM_ENEMY;
	}

	if (HasCondition(COND_LIGHT_DAMAGE))
	{
		return SCHED_NONE;
	}

	if (HasCondition(COND_ENEMY_OCCLUDED))
	{
		if (GetEnemy() && !(GetEnemy()->GetFlags() & FL_NOTARGET))
		{
			// Charge in and break the enemy's cover!
			return SCHED_ESTABLISH_LINE_OF_FIRE;
		}
	}

	nResult = SelectScheduleNoDirectEnemy();
	if (nResult != SCHED_NONE)
		return nResult;

	return SCHED_NONE;
}


//-----------------------------------------------------------------------------
// Can me enemy see me? 
//-----------------------------------------------------------------------------
bool CNPC_Bob2::CanEnemySeeMe()
{
	if (GetEnemy()->IsPlayer())
	{
		if (static_cast<CBasePlayer*>(GetEnemy())->FInViewCone(this))
		{
			return true;
		}
	}
	return false;
}



//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Bob2::IsHeavyDamage(const CTakeDamageInfo &info)
{
	// Bobs considers bullet fire heavy damage
	if (info.GetDamageType() & DMG_BULLET)
		return true;

	return BaseClass::IsHeavyDamage(info);
}

//-----------------------------------------------------------------------------
// TraceAttack
//-----------------------------------------------------------------------------
void CNPC_Bob2::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{
	BaseClass::TraceAttack(info, vecDir, ptr, pAccumulator);
}

//-----------------------------------------------------------------------------
// Determines the best type of flinch anim to play.
//-----------------------------------------------------------------------------
Activity CNPC_Bob2::GetFlinchActivity(bool bHeavyDamage, bool bGesture)
{
	return BaseClass::GetFlinchActivity(bHeavyDamage, bGesture);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob2::PlayFlinchGesture(void)
{
	BaseClass::PlayFlinchGesture();

	// To ensure old playtested difficulty stays the same, stop bobs shooting for a bit after gesture flinches
	//GetShotRegulator()->FireNoEarlierThan(gpGlobals->curtime + 0.5);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CNPC_Bob2::GetIdealAccel(void) const
{
	return GetIdealSpeed() * 2.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Chase after a player who's just pissed us off, and hit him
//-----------------------------------------------------------------------------
void CNPC_Bob2::AdministerJustice(void)
{
	if (!AI_IsSinglePlayer())
		return;

	// If we're allowed to chase the player, do so. Otherwise, just threaten.
	if (!IsInAScript() && (m_NPCState != NPC_STATE_SCRIPT) && HasSpawnFlags(SF_BOB_ALLOWED_TO_RESPOND))
	{
		if (m_vecPreChaseOrigin == vec3_origin)
		{
			m_vecPreChaseOrigin = GetAbsOrigin();
			m_flPreChaseYaw = GetAbsAngles().y;
		}
		m_flChasePlayerTime = gpGlobals->curtime + RandomFloat(3, 7);

		// Attack the target
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);
		SetEnemy(pPlayer);
		SetState(NPC_STATE_COMBAT);
		UpdateEnemyMemory(pPlayer, pPlayer->GetAbsOrigin());
	}
	else
	{
		// Watch the player for a time.
		m_bKeepFacingPlayer = true;

		// Try and find a nearby bob to administer justice
		CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
		int nAIs = g_AI_Manager.NumAIs();
		for (int i = 0; i < nAIs; i++)
		{
			if (ppAIs[i] == this)
				continue;

			if (ppAIs[i]->Classify() == CLASS_BOB && FClassnameIs(ppAIs[i], "npc_bob2"))
			{
				CNPC_Bob2 *pNPC = assert_cast<CNPC_Bob2*>(ppAIs[i]);
				if (pNPC->HasSpawnFlags(SF_BOB_ALLOWED_TO_RESPOND))
				{
					// Is he within site & range?
					if (FVisible(pNPC) && pNPC->FVisible(UTIL_PlayerByIndex(1)) &&
						UTIL_DistApprox(WorldSpaceCenter(), pNPC->WorldSpaceCenter()) < 512)
					{
						pNPC->AdministerJustice();
						break;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Schedule selection 
//-----------------------------------------------------------------------------
int CNPC_Bob2::SelectSchedule(void)
{
	if (!GetEnemy() && HasCondition(COND_IN_PVS) && AI_GetSinglePlayer() && !AI_GetSinglePlayer()->IsAlive())
	{
		return SCHED_PATROL_WALK;
	}

	if (HasCondition(COND_BOB2_ON_FIRE))
	{
		return SCHED_BOB2_BURNING_STAND;
	}

	// React to being struck by a physics object
	if (HasCondition(COND_BOB2_PHYSOBJECT_ASSAULT))
	{
		ClearCondition(COND_BOB2_PHYSOBJECT_ASSAULT);

	}

	int nSched = SelectFlinchSchedule();
	if (nSched != SCHED_NONE)
		return nSched;

	// Cower when physics objects are thrown at me
	if (HasCondition(COND_HEAR_PHYSICS_DANGER))
	{
		if (m_flLastPhysicsFlinchTime + 4.0f <= gpGlobals->curtime)
		{
			m_flLastPhysicsFlinchTime = gpGlobals->curtime;
			return SCHED_FLINCH_PHYSICS;
		}
	}

	// Always run for cover from danger sounds
	if (HasCondition(COND_HEAR_DANGER))
	{
		CSound *pSound;
		pSound = GetBestSound();

		Assert(pSound != NULL);
		if (pSound)
		{
			if (pSound->m_iType & SOUND_DANGER)
			{
				return SCHED_TAKE_COVER_FROM_BEST_SOUND;
			}
			if (!HasCondition(COND_SEE_ENEMY) && (pSound->m_iType & (SOUND_PLAYER | SOUND_PLAYER_VEHICLE | SOUND_COMBAT)))
			{
				GetMotor()->SetIdealYawToTarget(pSound->GetSoundReactOrigin());
			}
		}
	}


	// If we're clubbing someone who threw something at us. chase them
	if (m_NPCState == NPC_STATE_COMBAT && m_flChasePlayerTime > gpGlobals->curtime)
		return SCHED_CHASE_ENEMY;

	if (!BehaviorSelectSchedule())
	{

		switch (m_NPCState)
		{
		case NPC_STATE_IDLE:
		{
			nSched = SelectScheduleInvestigateSound();
			if (nSched != SCHED_NONE)
				return nSched;
			break;
		}

		case NPC_STATE_ALERT:
		{
			nSched = SelectScheduleInvestigateSound();
			if (nSched != SCHED_NONE)
				return nSched;
		}
		break;

		case NPC_STATE_COMBAT:
				int nResult = SelectCombatSchedule();
				if (nResult != SCHED_NONE)
					return nResult;
			break;
		}
	}

	// If we're not in combat, and we've got a pre-chase origin, move back to it
	if ((m_NPCState != NPC_STATE_COMBAT) &&
		(m_vecPreChaseOrigin != vec3_origin) &&
		(m_flChasePlayerTime < gpGlobals->curtime))
	{
		return SCHED_BOB2_RETURN_TO_PRECHASE;
	}

	return BaseClass::SelectSchedule();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : failedSchedule - 
//			failedTask - 
//			taskFailCode - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Bob2::SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode)
{
	//if (failedSchedule == SCHED_BOB2_CHASE_ENEMY)
	if (failedSchedule == SCHED_CHASE_ENEMY)
	{
		//return SCHED_BOB2_ESTABLISH_LINE_OF_FIRE;
		return SCHED_ESTABLISH_LINE_OF_FIRE;
	}

	return BaseClass::SelectFailSchedule(failedSchedule, failedTask, taskFailCode);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CNPC_Bob2::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_ALERT_FACE_BESTSOUND:
		if (!IsCurSchedule(SCHED_BOB2_ALERT_FACE_BESTSOUND, false))
		{
			return SCHED_BOB2_ALERT_FACE_BESTSOUND;
		}
		return SCHED_ALERT_FACE_BESTSOUND;

	//case SCHED_CHASE_ENEMY:

	//	if (!IsRunningBehavior())
	//	{
	//		return SCHED_BOB2_CHASE_ENEMY;
	//	}

	//	break;

//	case SCHED_ESTABLISH_LINE_OF_FIRE:
//	case SCHED_BOB2_ESTABLISH_LINE_OF_FIRE:
//		return SCHED_BOB2_ESTABLISH_LINE_OF_FIRE;

	case SCHED_WAKE_ANGRY:
		return SCHED_BOB2_WAKE_ANGRY;

	case SCHED_FAIL_TAKE_COVER:

		if (HasCondition(COND_CAN_RANGE_ATTACK1))
		{
				return SCHED_RANGE_ATTACK1;
		}


	case SCHED_RANGE_ATTACK1:
		Assert(!HasCondition(COND_NO_PRIMARY_AMMO));

	case SCHED_BOB2_ADVANCE:
		if (m_NextChargeTimer.Expired() && bob2_charge.GetBool())
		{
			//if (Weapon_OwnsThisType("weapon_pistol"))
		//	{
			//	if (GetEnemy() && GetEnemy()->GetAbsOrigin().DistToSqr(GetAbsOrigin()) > 300 * 300)
			//	{
			//		if (OccupyStrategySlot(SQUAD_SLOT_POLICE_CHARGE_ENEMY))
			//		{
			//			m_NextChargeTimer.Set(3, 7);
			//			return SCHED_BOB_CHARGE;
			//		}
			//	}
			//}
			//else
			//{
				m_NextChargeTimer.Set(99999);
			//}
		}
		break;
	}


	return BaseClass::TranslateSchedule(scheduleType);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CNPC_Bob2::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_BOB2_WAIT_FOR_SENTENCE:
	{
		if (FOkToMakeSound(pTask->flTaskData))
		{
			TaskComplete();
		}
	}
	break;

	case TASK_BOB2_GET_PATH_TO_PRECHASE:
	{
		Assert(m_vecPreChaseOrigin != vec3_origin);
		if (GetNavigator()->SetGoal(m_vecPreChaseOrigin))
		{
			QAngle vecAngles(0, m_flPreChaseYaw, 0);
			GetNavigator()->SetArrivalDirection(vecAngles);
			TaskComplete();
		}
		else
		{
			TaskFail(FAIL_NO_ROUTE);
		}
		break;
	}

	case TASK_BOB2_CLEAR_PRECHASE:
	{
		m_vecPreChaseOrigin = vec3_origin;
		m_flPreChaseYaw = 0;
		TaskComplete();
		break;
	}

	case TASK_BOB2_DIE_INSTANTLY:
	{
		CTakeDamageInfo info;

		info.SetAttacker(this);
		info.SetInflictor(this);
		info.SetDamage(m_iHealth);
		info.SetDamageType(pTask->flTaskData);
		info.SetDamageForce(Vector(0.1, 0.1, 0.1));

		TakeDamage(info);

		TaskComplete();
	}
	break;

	case TASK_BOB2_HARASS:
	{
		if (!(m_spawnflags & SF_BOB_NOCHATTER))
		{
			if (GetEnemy() && GetEnemy()->GetWaterLevel() > 0)
			{
				EmitSound("NPC_MetroPolice.WaterSpeech");
			}
			else
			{
				EmitSound("NPC_MetroPolice.HidingSpeech");
			}
		}

		TaskComplete();
	}
	break;

	case TASK_BOB2_GET_PATH_TO_BESTSOUND_LOS:
	{
	}
	break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}


//-----------------------------------------------------------------------------
//
// Run tasks!
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
#define FLEEING_DISTANCE_SQR (100 * 100)

void CNPC_Bob2::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_WAIT_FOR_MOVEMENT:
		BaseClass::RunTask(pTask);
		break;

	case TASK_BOB2_WAIT_FOR_SENTENCE:
	{
		if (FOkToMakeSound(pTask->flTaskData))
		{
			TaskComplete();
		}
	}
	break;

	case TASK_BOB2_GET_PATH_TO_BESTSOUND_LOS:
	{
		switch (GetTaskInterrupt())
		{
		case 0:
		{
			CSound *pSound = GetBestSound();
			if (!pSound)
			{
				TaskFail(FAIL_NO_SOUND);
			}
			else
			{
				float flMaxRange = 2000;
				float flMinRange = 0;

				// Check against NPC's max range
				if (flMaxRange > m_flDistTooFar)
				{
					flMaxRange = m_flDistTooFar;
				}

				// Why not doing lateral LOS first?

				Vector losTarget = pSound->GetSoundReactOrigin();
				if (GetTacticalServices()->FindLos(pSound->GetSoundReactOrigin(), losTarget, flMinRange, flMaxRange, 1.0, &m_vInterruptSavePosition))
				{
					TaskInterrupt();
				}
				else
				{
					TaskFail(FAIL_NO_SHOOT);
				}
			}
		}
		break;

		case 1:
		{
			AI_NavGoal_t goal(m_vInterruptSavePosition, ACT_RUN, AIN_HULL_TOLERANCE);
			GetNavigator()->SetGoal(goal);
		}
		break;
		}
	}
	break;

	default:
		BaseClass::RunTask(pTask);
		break;
	}
}


int CNPC_Bob2::OnTakeDamage(const CTakeDamageInfo &inputInfo)
{	
	
	if (!m_bIsArmorBreakState)
	{
		//Armor Break state = Being freezed for 3 seconds.
		//Activate Armor break state once the armor points are exhausted
		if ((!m_bIsArmorBreak) && (m_iArmor <= 0))
		{
			m_bIsArmorBreak = true;
			m_bIsArmorBreakState = true; //always true to avoid triggering the armor break again
			//Add armor break sound
			//Add armor break particle effect;
			m_ArmorBreakDownTime.Set(3.0f);
		}
	}
	
	CSingleUserRecipientFilter PlayerFilter4(UTIL_GetLocalPlayer());
	PlayerFilter4.MakeReliable();
	UserMessageBegin(PlayerFilter4, "MonsterName");
	WRITE_STRING("bob2");
	MessageEnd();



	return BaseClass::OnTakeDamage(inputInfo);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pevInflictor - 
//			pAttacker - 
//			flDamage - 
//			bitsDamageType - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Bob2::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	CTakeDamageInfo info = inputInfo;
	
#if 0
	// Die instantly from a hit in idle/alert states
	if (m_NPCState == NPC_STATE_IDLE || m_NPCState == NPC_STATE_ALERT)
	{
		info.SetDamage(m_iHealth);
	}
#endif //0

	if (info.GetAttacker() == GetEnemy())
	{
		// Keep track of recent damage by my attacker. If it seems like we're
		// being killed, consider running off and hiding.
		m_nRecentDamage += info.GetDamage();
		m_flRecentDamageTime = gpGlobals->curtime;
	}

	
	//25% of dropping rage energy for every hit 
	int iRNGSim = RandomInt(1, 11);
	
	if (iRNGSim == 1)
	{
		DropItem("item_rage", WorldSpaceCenter() + RandomVector(-4, 4), RandomAngle(0, 360));
	}

	//show damage number
	//char tempstr[512];
	//Q_snprintf(tempstr, sizeof(tempstr), "%.0f", info.GetDamage());
	//EntityText(5, tempstr, 1);

	return BaseClass::OnTakeDamage_Alive(info);
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_Bob2::BuildScheduleTestBits(void)
{
	BaseClass::BuildScheduleTestBits();

	//FIXME: Always interrupt for now
	if (!IsInAScript() &&
		!IsCurSchedule(SCHED_MELEE_ATTACK1) &&
		!IsCurSchedule(SCHED_RELOAD))
	{
		SetCustomInterruptCondition(COND_BOB2_PLAYER_TOO_CLOSE);
	}

	if (!IsCurSchedule(SCHED_BOB2_BURNING_RUN) && !IsCurSchedule(SCHED_BOB2_BURNING_STAND) && !IsMoving())
	{
		SetCustomInterruptCondition(COND_BOB2_ON_FIRE);
	}

	if (IsCurSchedule(SCHED_TAKE_COVER_FROM_ENEMY))
	{
		ClearCustomInterruptCondition(COND_LIGHT_DAMAGE);
		ClearCustomInterruptCondition(COND_HEAVY_DAMAGE);
	}

	if (IsCurSchedule(SCHED_MELEE_ATTACK1))
	{
		if (gpGlobals->curtime - m_flLastDamageFlinchTime < 10.0)
		{
			//ClearCustomInterruptCondition( COND_LIGHT_DAMAGE );
			//ClearCustomInterruptCondition( COND_HEAVY_DAMAGE );
		}
	}
	else if (IsCurSchedule(SCHED_COMBAT_FACE) && !m_BatonSwingTimer.Expired()) //HasBaton
	{
		ClearCustomInterruptCondition(COND_CAN_MELEE_ATTACK1);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob2::GatherConditions(void)
{
	BaseClass::GatherConditions();

	if (m_bPlayerTooClose == false)
	{
		ClearCondition(COND_BOB2_PLAYER_TOO_CLOSE);
	}

	//Actually set the frozen condition to achieve the armor break effect.
	if ((m_bIsArmorBreak) && (!m_ArmorBreakDownTime.Expired()))
	{
		SetCondition(COND_NPC_FREEZE);
		EntityText(5, "BREAK!", 1);

	}
	else if ((m_bIsArmorBreak) && (m_ArmorBreakDownTime.Expired()))
	{
		 SetCondition(COND_NPC_UNFREEZE);
		 ClearCondition(COND_NPC_FREEZE);
		 EntityText(5, "UNBREAK!", 1);
		 m_bIsArmorBreak = false; // set as false to avoid triggering the armor break again
	}

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);

	// FIXME: Player can be NULL here during level transitions.
	if (!pPlayer)
		return;

	float distToPlayerSqr = (pPlayer->GetAbsOrigin() - GetAbsOrigin()).LengthSqr();

	// See if we're too close
	if (pPlayer->GetGroundEntity() == this)
	{
		// Always beat a player on our head
		m_iNumPlayerHits = 0;
		SetCondition(COND_BOB2_PLAYER_TOO_CLOSE);
	}
	else if ((distToPlayerSqr < (42.0f*42.0f) && FVisible(pPlayer)))
	{
		// Ignore the player if we've been beating him, but not if we haven't moved
		if (m_iNumPlayerHits < 3 || m_vecPreChaseOrigin == vec3_origin)
		{
			SetCondition(COND_BOB2_PLAYER_TOO_CLOSE);
		}
	}
	else
	{
		ClearCondition(COND_BOB2_PLAYER_TOO_CLOSE);

		// Don't clear out the player hit count for a few seconds after we last hit him
		// This avoids states where two Bobs have the player pinned between them.
		if ((gpGlobals->curtime - GetLastAttackTime()) > 3)
		{
			m_iNumPlayerHits = 0;
		}

		m_bPlayerTooClose = false;
	}

	if (bob2_move_and_melee.GetBool())
	{
		if (IsMoving() && HasCondition(COND_CAN_MELEE_ATTACK1)) //&&HasBaton
		{
			if (m_BatonSwingTimer.Expired())
			{
				m_BatonSwingTimer.Set(2.5, 2.75);

				//Activity activity = TranslateActivity(ACT_MELEE_ATTACK_SWING_GESTURE);
				Activity activity = TranslateActivity(ACT_MELEE_ATTACK1);
				Assert(activity != ACT_INVALID);
				AddGesture(activity);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSound - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Bob2::QueryHearSound(CSound *pSound)
{
	return BaseClass::QueryHearSound(pSound);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : index - 
//			*pEvent - 
//-----------------------------------------------------------------------------
void CNPC_Bob2::VPhysicsCollision(int index, gamevcollisionevent_t *pEvent)
{
	BaseClass::VPhysicsCollision(index, pEvent);

	int otherIndex = !index;

	CBaseEntity *pHitEntity = pEvent->pEntities[otherIndex];

	if (pEvent->pObjects[otherIndex]->GetGameFlags() & FVPHYSICS_PLAYER_HELD)
	{
		CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(UTIL_PlayerByIndex(1));

		// See if it's being held by the player
		if (pPlayer != NULL && pPlayer->IsHoldingEntity(pHitEntity))
		{
			//TODO: Play an angry sentence, "Get that outta here!"

			//			if (IsCurSchedule(SCHED_BOB_SHOVE) == false)
			//			{
			//			SetCondition(COND_BOB_PLAYER_TOO_CLOSE);
			//			m_bPlayerTooClose = true;
			//			}
		}
	}
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC(npc_bob2, CNPC_Bob2)


DECLARE_SQUADSLOT(SQUAD_SLOT_POLICE_CHARGE_ENEMY);
DECLARE_SQUADSLOT(SQUAD_SLOT_POLICE_HARASS);
DECLARE_SQUADSLOT(SQUAD_SLOT_POLICE_ATTACK_OCCLUDER1);
DECLARE_SQUADSLOT(SQUAD_SLOT_POLICE_ATTACK_OCCLUDER2);

DECLARE_TASK(TASK_BOB2_HARASS);
DECLARE_TASK(TASK_BOB2_DIE_INSTANTLY);
DECLARE_TASK(TASK_BOB2_GET_PATH_TO_BESTSOUND_LOS);
DECLARE_TASK(TASK_BOB2_WAIT_FOR_SENTENCE);
DECLARE_TASK(TASK_BOB2_GET_PATH_TO_PRECHASE);
DECLARE_TASK(TASK_BOB2_CLEAR_PRECHASE);

DECLARE_CONDITION(COND_BOB2_ON_FIRE);
DECLARE_CONDITION(COND_BOB2_PLAYER_TOO_CLOSE);
DECLARE_CONDITION(COND_BOB2_PHYSOBJECT_ASSAULT);


//=========================================================
//=========================================================
DEFINE_SCHEDULE
(
SCHED_BOB2_WAKE_ANGRY,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE"
"		TASK_FACE_ENEMY					0"
"	"
"	Interrupts"
);


//=========================================================
// > InvestigateSound
//
//	sends a monster to the location of the
//	sound that was just heard to check things out.
//=========================================================
DEFINE_SCHEDULE
(
SCHED_BOB2_INVESTIGATE_SOUND,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_STORE_LASTPOSITION			0"
"		TASK_BOB_GET_PATH_TO_BESTSOUND_LOS		0"
"		TASK_FACE_IDEAL					0"
//	"		TASK_SET_TOLERANCE_DISTANCE		32"
"		TASK_RUN_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_STOP_MOVING				0"
"		TASK_WAIT						5"
"		TASK_GET_PATH_TO_LASTPOSITION	0"
"		TASK_WALK_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_STOP_MOVING				0"
"		TASK_CLEAR_LASTPOSITION			0"
"		TASK_FACE_REASONABLE			0"
""
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_SEE_FEAR"
"		COND_SEE_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
"		COND_HEAR_DANGER"
);


//=========================================================
//=========================================================
DEFINE_SCHEDULE
(
SCHED_BOB2_HARASS,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_FACE_ENEMY					0"
"		TASK_WAIT_FACE_ENEMY			6"
"		TASK_BOB_HARASS			0"
"		TASK_WAIT_PVS					0"
"	"
"	Interrupts"
"	"
"		COND_CAN_RANGE_ATTACK1"
"		COND_NEW_ENEMY"
);


//=========================================================
// > ChaseEnemy
//=========================================================
//DEFINE_SCHEDULE
//(
//SCHED_BOB2_CHASE_ENEMY,
//
//"	Tasks"
//"		TASK_STOP_MOVING				0"
//"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_BOB2_ESTABLISH_LINE_OF_FIRE"
//"		TASK_SET_TOLERANCE_DISTANCE		24"
//"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
//"		TASK_RUN_PATH					0"
//"		TASK_WAIT_FOR_MOVEMENT			0"
//"		TASK_FACE_ENEMY					0"
//"	"
//"	Interrupts"
//"		COND_NEW_ENEMY"
//"		COND_ENEMY_DEAD"
//"		COND_ENEMY_UNREACHABLE"
//"		COND_CAN_RANGE_ATTACK1"
//"		COND_CAN_MELEE_ATTACK1"
//"		COND_CAN_RANGE_ATTACK2"
//"		COND_CAN_MELEE_ATTACK2"
//"		COND_TOO_CLOSE_TO_ATTACK"
//"		COND_TASK_FAILED"
//"		COND_LOST_ENEMY"
//"		COND_HEAR_DANGER"
//);
//
//
//DEFINE_SCHEDULE
//(
//SCHED_BOB2_ESTABLISH_LINE_OF_FIRE,
//
//"	Tasks "
//"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_FAIL_ESTABLISH_LINE_OF_FIRE"
//"		TASK_FACE_ENEMY					0"
//"		TASK_SET_TOLERANCE_DISTANCE		48"
//"		TASK_GET_PATH_TO_ENEMY_LKP_LOS	0"
//"		TASK_RUN_PATH					0"
//"		TASK_WAIT_FOR_MOVEMENT			0"
//"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
//"	"
//"	Interrupts "
//"		COND_NEW_ENEMY"
//"		COND_ENEMY_DEAD"
//"		COND_CAN_RANGE_ATTACK1"
//"		COND_CAN_RANGE_ATTACK2"
//"		COND_CAN_MELEE_ATTACK1"
//"		COND_CAN_MELEE_ATTACK2"
//"		COND_HEAR_DANGER"
//"		COND_HEAVY_DAMAGE"
//);

//===============================================
//===============================================

DEFINE_SCHEDULE
(
SCHED_BOB2_ADVANCE,

"	Tasks"
"		TASK_STOP_MOVING					0"
"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE_ANGRY"
"		TASK_FACE_ENEMY						0"
"		TASK_WAIT_FACE_ENEMY				1" // give the guy some time to come out on his own
"		TASK_WAIT_FACE_ENEMY_RANDOM			3"
"		TASK_GET_PATH_TO_ENEMY_LOS			0"
"		TASK_RUN_PATH						0"
"		TASK_WAIT_FOR_MOVEMENT				0"
"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE_ANGRY"
"		TASK_FACE_ENEMY						0"
""
"	Interrupts"
"		COND_CAN_RANGE_ATTACK1"
"		COND_ENEMY_DEAD"
""
);

//===============================================
//===============================================

DEFINE_SCHEDULE
(
SCHED_BOB2_CHARGE,

"	Tasks"
"		TASK_STOP_MOVING				0"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_BOB2_ADVANCE"
//	"		TASK_SET_TOLERANCE_DISTANCE		24"
"		TASK_STORE_LASTPOSITION			0"
"		TASK_GET_CHASE_PATH_TO_ENEMY	300"
"		TASK_RUN_PATH_FOR_UNITS			150"
"		TASK_STOP_MOVING				1"
"		TASK_FACE_ENEMY			0"
""
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_LOST_ENEMY"
"		COND_CAN_MELEE_ATTACK1"
"		COND_CAN_MELEE_ATTACK2"
"		COND_HEAR_DANGER"
"		COND_BOB_PLAYER_TOO_CLOSE"
);

//=========================================================
//=========================================================
DEFINE_SCHEDULE
(
SCHED_BOB2_BURNING_RUN,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_BOB2_BURNING_STAND"
"		TASK_SET_TOLERANCE_DISTANCE		24"
"		TASK_GET_PATH_TO_ENEMY			0"
"		TASK_RUN_PATH_TIMED				10"
"		TASK_BOB_DIE_INSTANTLY	0"
"	"
"	Interrupts"
);

//=========================================================
//=========================================================
DEFINE_SCHEDULE
(
SCHED_BOB2_BURNING_STAND,

"	Tasks"
"		TASK_SET_ACTIVITY				ACTIVITY:ACT_IDLE_ON_FIRE"
"		TASK_WAIT						1.5"
"		TASK_BOB_DIE_INSTANTLY	DMG_BURN"
"		TASK_WAIT						1.0"
"	"
"	Interrupts"
);

//=========================================================
//=========================================================
DEFINE_SCHEDULE
(
SCHED_BOB2_RETURN_TO_PRECHASE,

"	Tasks"
"		TASK_WAIT_RANDOM						1"
"		TASK_BOB_GET_PATH_TO_PRECHASE	0"
"		TASK_WALK_PATH							0"
"		TASK_WAIT_FOR_MOVEMENT					0"
"		TASK_STOP_MOVING						0"
"		TASK_BOB_CLEAR_PRECHASE			0"
"	"
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_CAN_MELEE_ATTACK1"
"		COND_CAN_MELEE_ATTACK2"
"		COND_TASK_FAILED"
"		COND_LOST_ENEMY"
"		COND_HEAR_DANGER"
);

//===============================================
//===============================================
DEFINE_SCHEDULE
(
SCHED_BOB2_ALERT_FACE_BESTSOUND,

"	Tasks"
"		TASK_SET_SCHEDULE		SCHEDULE:SCHED_ALERT_FACE_BESTSOUND"
""
"	Interrupts"
""
)

DEFINE_SCHEDULE
(
SCHED_BOB2_SMASH_PROP,

"	Tasks"
"		TASK_GET_PATH_TO_TARGET		0"
"		TASK_MOVE_TO_TARGET_RANGE	50"
"		TASK_STOP_MOVING			0"
"		TASK_FACE_TARGET			0"
"		TASK_ANNOUNCE_ATTACK		1"	// 1 = primary attack
"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_MELEE_ATTACK1"
""
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
);

AI_END_CUSTOM_NPC()

