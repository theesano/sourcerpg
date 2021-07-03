//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Ichthyosaur - buh bum...  buh bum...
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_task.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_interactions.h"
#include "ai_navigator.h"
#include "activitylist.h"
#include "game.h"
#include "npcevent.h"
#include "player.h"
#include "entitylist.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "movevars_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sk_bob3_health("sk_bob3_health", "100");
ConVar	sk_bob3_melee_dmg("sk_bob3_melee_dmg", "5");

#define	BOB3_MODEL	"models/monster/mob1_puppet.mdl"


#define	ICH_HEIGHT_PREFERENCE	16.0f
#define	ICH_DEPTH_PREFERENCE	8.0f

#define	ICH_WAYPOINT_DISTANCE	64.0f

#define	BOB3_AE_BITE			11
#define	BOB3_AE_BITE_START		12

#define	ICH_SWIM_SPEED_WALK		150
#define	ICH_SWIM_SPEED_RUN		500

#define	ICH_MIN_TURN_SPEED		16.0f
#define	ICH_MAX_TURN_SPEED		30.0f

enum Bob3MoveType_t
{
	ICH_MOVETYPE_SEEK = 0,	// Fly through the target without stopping.
	ICH_MOVETYPE_ARRIVE		// Slow down and stop at target.
};

//
// CNPC_Bob3
//

class CNPC_Bob3 : public CAI_BaseNPC
{
public:
	DECLARE_CLASS(CNPC_Bob3, CAI_BaseNPC);
	DECLARE_DATADESC();

	CNPC_Bob3(void) {}

	int		SelectSchedule(void);
	int		MeleeAttack1Conditions(float flDot, float flDist);
	int		OnTakeDamage_Alive(const CTakeDamageInfo &info);
	int		TranslateSchedule(int type);

	void	Precache(void);
	void	Spawn(void);
	void	MoveFlyExecute(CBaseEntity *pTargetEnt, const Vector & vecDir, float flDistance, float flInterval);
	void	HandleAnimEvent(animevent_t *pEvent);
	void	PrescheduleThink(void);
	bool	OverrideMove(float flInterval);
	void	StartTask(const Task_t *pTask);
	void	RunTask(const Task_t *pTask);
	void	TranslateNavGoal(CBaseEntity *pEnemy, Vector &chasePosition);
	float	GetDefaultNavGoalTolerance();

	float	MaxYawSpeed(void);

	Class_T Classify(void)	{ return CLASS_ANTLION; }	//FIXME: No classification for various wildlife?

	bool	FVisible(CBaseEntity *pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = NULL);

private:

	bool	SteerAvoidObstacles(Vector &Steer, const Vector &Velocity, const Vector &Forward, const Vector &Right, const Vector &Up);

	void	DoMovement(float flInterval, const Vector &MoveTarget, int eMoveType);
	void	SteerArrive(Vector &Steer, const Vector &Target);
	void	SteerSeek(Vector &Steer, const Vector &Target);
	void	ClampSteer(Vector &SteerAbs, Vector &SteerRel, Vector &forward, Vector &right, Vector &up);

	void	Bite(void);

	//void	IchTouch( CBaseEntity *pOther );

	float	GetGroundSpeed(void);

#if FEELER_COLLISION
	Vector	DoProbe(const Vector &Probe);
	Vector	m_LastSteer;
#endif

	CBaseEntity	*m_pVictim;

	static const Vector	m_vecAccelerationMax;
	static const Vector	m_vecAccelerationMin;

	Vector	m_vecLastMoveTarget;

	float	m_flNextBiteTime;
	float	m_flHoldTime;
	float	m_flSwimSpeed;
	float	m_flTailYaw;
	float	m_flTailPitch;

	float	m_flNextPingTime;
	float	m_flNextGrowlTime;

	bool	m_bHasMoveTarget;
	bool	m_bIgnoreSurface;

	//CSoundPatch	*m_pSwimSound;
	//CSoundPatch	*m_pVoiceSound;

	DEFINE_CUSTOM_AI;
};

//Acceleration definitions
const Vector CNPC_Bob3::m_vecAccelerationMax = Vector(256, 1024, 512);
const Vector CNPC_Bob3::m_vecAccelerationMin = Vector(-256, -1024, -512);

//Data description
BEGIN_DATADESC(CNPC_Bob3)

// Silence classcheck
//	DEFINE_FIELD( m_LastSteer, FIELD_VECTOR ),

DEFINE_FIELD(m_pVictim, FIELD_CLASSPTR),
DEFINE_FIELD(m_vecLastMoveTarget, FIELD_VECTOR),
DEFINE_FIELD(m_flNextBiteTime, FIELD_FLOAT),
DEFINE_FIELD(m_flHoldTime, FIELD_FLOAT),
DEFINE_FIELD(m_flSwimSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_flTailYaw, FIELD_FLOAT),
DEFINE_FIELD(m_flTailPitch, FIELD_FLOAT),
DEFINE_FIELD(m_flNextPingTime, FIELD_FLOAT),
DEFINE_FIELD(m_flNextGrowlTime, FIELD_FLOAT),
DEFINE_FIELD(m_bHasMoveTarget, FIELD_BOOLEAN),
DEFINE_FIELD(m_bIgnoreSurface, FIELD_BOOLEAN),

//DEFINE_FUNCTION( IchTouch ),

END_DATADESC()

//Schedules
enum Bob3Schedules
{
	SCHED_BOB3_CHASE_ENEMY = LAST_SHARED_SCHEDULE,
	SCHED_BOB3_PATROL_RUN,
	SCHED_BOB3_PATROL_WALK,
	SCHED_BOB3_MELEE_ATTACK1,
};

//Tasks
enum Bob3Tasks
{
	TASK_BOB3_CUSTOMTASK = LAST_SHARED_TASK,
};

//Activities
int	ACT_BOB3_BITE_HIT;
int	ACT_BOB3_BITE_MISS;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob3::InitCustomSchedules(void)
{
	INIT_CUSTOM_AI(CNPC_Bob3);

	//Interaction	REGISTER_INTERACTION( g_interactionAntlionAttacked );

	//Schedules
	ADD_CUSTOM_SCHEDULE(CNPC_Bob3, SCHED_BOB3_CHASE_ENEMY);
	ADD_CUSTOM_SCHEDULE(CNPC_Bob3, SCHED_BOB3_PATROL_RUN);
	ADD_CUSTOM_SCHEDULE(CNPC_Bob3, SCHED_BOB3_PATROL_WALK);
	ADD_CUSTOM_SCHEDULE(CNPC_Bob3, SCHED_BOB3_MELEE_ATTACK1);

	//Tasks
	ADD_CUSTOM_TASK(CNPC_Bob3, TASK_BOB3_CUSTOMTASK);

	//Conditions	ADD_CUSTOM_CONDITION( CNPC_CombineGuard,	COND_ANTLIONGRUB_HEARD_SQUEAL );

	//Activities
	ADD_CUSTOM_ACTIVITY(CNPC_Bob3, ACT_BOB3_BITE_HIT);
	ADD_CUSTOM_ACTIVITY(CNPC_Bob3, ACT_BOB3_BITE_MISS);

	AI_LOAD_SCHEDULE(CNPC_Bob3, SCHED_BOB3_CHASE_ENEMY);
	AI_LOAD_SCHEDULE(CNPC_Bob3, SCHED_BOB3_PATROL_RUN);
	AI_LOAD_SCHEDULE(CNPC_Bob3, SCHED_BOB3_PATROL_WALK);
	AI_LOAD_SCHEDULE(CNPC_Bob3, SCHED_BOB3_MELEE_ATTACK1);
}

LINK_ENTITY_TO_CLASS(npc_bob3, CNPC_Bob3);
IMPLEMENT_CUSTOM_AI(npc_bob3, CNPC_Bob3);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob3::Precache(void)
{
	PrecacheModel(BOB3_MODEL);

	PrecacheScriptSound("NPC_Ichthyosaur.Bite");
	PrecacheScriptSound("NPC_Ichthyosaur.BiteMiss");
	PrecacheScriptSound("NPC_Ichthyosaur.AttackGrowl");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob3::Spawn(void)
{
	Precache();

	SetModel(BOB3_MODEL);

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();
	SetDefaultEyeOffset();

	m_NPCState = NPC_STATE_NONE;
	SetBloodColor(BLOOD_COLOR_RED);
	m_iHealth = sk_bob3_health.GetFloat();
	m_iMaxHealth = m_iHealth;
	m_flFieldOfView = 0.6;
	
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	//AddFlag(FL_FLY | FL_STEPMOVEMENT);

	m_flGroundSpeed = ICH_SWIM_SPEED_RUN;

	m_bIgnoreSurface = false;

	m_flSwimSpeed = 0.0f;
	m_flTailYaw = 0.0f;
	m_flTailPitch = 0.0f;

	m_flNextBiteTime = gpGlobals->curtime;
	m_flHoldTime = gpGlobals->curtime;
	m_flNextPingTime = gpGlobals->curtime;
	m_flNextGrowlTime = gpGlobals->curtime;

#if FEELER_COLLISION

	Vector	forward;

	GetVectors(&forward, NULL, NULL);

	m_vecCurrentVelocity = forward * m_flGroundSpeed;

#endif

	//SetTouch( IchTouch );

	CapabilitiesClear();
	//CapabilitiesAdd(bits_CAP_MOVE_FLY | bits_CAP_INNATE_MELEE_ATTACK1);
	CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1);

	NPCInit();

	//m_pSwimSound	= ENVELOPE_CONTROLLER.SoundCreate( edict(), CHAN_BODY,	"xxxCONVERTTOGAMESOUNDS!!!npc/ichthyosaur/ich_amb1wav", ATTN_NORM );
	//m_pVoiceSound	= ENVELOPE_CONTROLLER.SoundCreate( edict(), CHAN_VOICE,	"xxxCONVERTTOGAMESOUNDS!!!npc/ichthyosaur/water_breathwav", ATTN_IDLE );

	//ENVELOPE_CONTROLLER.Play( m_pSwimSound,	1.0f, 100 );
	//ENVELOPE_CONTROLLER.Play( m_pVoiceSound,1.0f, 100 );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
/*
void CNPC_Bob3::IchTouch( CBaseEntity *pOther )
{
}
*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_Bob3::SelectSchedule(void)
{
	if (m_NPCState == NPC_STATE_COMBAT)
	{
		if (m_flNextBiteTime > gpGlobals->curtime)
			return	SCHED_PATROL_RUN;

		if (HasCondition(COND_CAN_MELEE_ATTACK1))
			return	SCHED_MELEE_ATTACK1;

		return SCHED_CHASE_ENEMY;
	}

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: Handles movement towards the last move target.
// Input  : flInterval - 
//-----------------------------------------------------------------------------
bool CNPC_Bob3::OverrideMove(float flInterval)
{
	m_flGroundSpeed = GetGroundSpeed();

	if (m_bHasMoveTarget)
	{
		DoMovement(flInterval, m_vecLastMoveTarget, ICH_MOVETYPE_ARRIVE);
	}
	else
	{
		DoMovement(flInterval, GetLocalOrigin(), ICH_MOVETYPE_ARRIVE);
	}
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
//			&m_LastMoveTarget - 
//			eMoveType - 
//-----------------------------------------------------------------------------
void CNPC_Bob3::DoMovement(float flInterval, const Vector &MoveTarget, int eMoveType)
{
	// dvs: something is setting this bit, causing us to stop moving and get stuck that way
	Forget(bits_MEMORY_TURNING);

	Vector Steer, SteerAvoid, SteerRel;
	Vector forward, right, up;

	//Get our orientation vectors.
	GetVectors(&forward, &right, &up);


	if ((GetActivity() == ACT_MELEE_ATTACK1) && (GetEnemy() != NULL))
	{
		SteerSeek(Steer, GetEnemy()->GetAbsOrigin());
	}
	else
	{
		//If we are approaching our goal, use an arrival steering mechanism.
		if (eMoveType == ICH_MOVETYPE_ARRIVE)
		{
			SteerArrive(Steer, MoveTarget);
		}
		else
		{
			//Otherwise use a seek steering mechanism.
			SteerSeek(Steer, MoveTarget);
		}
	}

		//FIXME: Until the sequence info is reset properly after SetPoseParameter
		if ((GetActivity() == ACT_RUN) || (GetActivity() == ACT_WALK))
		{
			ResetSequenceInfo();
		}

	//See if we need to avoid any obstacles.
	//if (SteerAvoidObstacles(SteerAvoid, GetAbsVelocity(), forward, right, up))
	//{
	//	//Take the avoidance vector
	//	Steer = SteerAvoid;
	//}

	//Clamp our ideal steering vector to within our physical limitations.
	ClampSteer(Steer, SteerRel, forward, right, up);

	ApplyAbsVelocityImpulse(Steer * flInterval);


	Vector vecNewVelocity = GetAbsVelocity();
	float flLength = vecNewVelocity.Length();

	//Clamp our final speed
	if (flLength > m_flGroundSpeed)
	{
		vecNewVelocity *= (m_flGroundSpeed / flLength);
		flLength = m_flGroundSpeed;
	}

	Vector	workVelocity = vecNewVelocity;


	//Move along the current velocity vector
	if (WalkMove(workVelocity * flInterval, MASK_NPCSOLID) == false)
	{
		//Attempt a half-step
		if (WalkMove((workVelocity*0.5f) * flInterval, MASK_NPCSOLID) == false)
		{
			//Restart the velocity
			//VectorNormalize( m_vecVelocity );
			vecNewVelocity *= 0.5f;
		}
		else
		{
			//Cut our velocity in half
			vecNewVelocity *= 0.5f;
		}
	}

	SetAbsVelocity(vecNewVelocity);

}

//-----------------------------------------------------------------------------
// Purpose: Gets a steering vector to arrive at a target location with a
//			relatively small velocity.
// Input  : Steer - Receives the ideal steering vector.
//			Target - Target position at which to arrive.
//-----------------------------------------------------------------------------
void CNPC_Bob3::SteerArrive(Vector &Steer, const Vector &Target)
{
	Vector Offset = Target - GetLocalOrigin();
	float fTargetDistance = Offset.Length();

	float fIdealSpeed = m_flGroundSpeed * (fTargetDistance / ICH_WAYPOINT_DISTANCE);
	float fClippedSpeed = MIN(fIdealSpeed, m_flGroundSpeed);

	Vector DesiredVelocity(0, 0, 0);

	if (fTargetDistance > ICH_WAYPOINT_DISTANCE)
	{
		DesiredVelocity = (fClippedSpeed / fTargetDistance) * Offset;
	}

	Steer = DesiredVelocity - GetAbsVelocity();
}


//-----------------------------------------------------------------------------
// Purpose: Gets a steering vector to move towards a target position as quickly
//			as possible.
// Input  : Steer - Receives the ideal steering vector.
//			Target - Target position to seek.
//-----------------------------------------------------------------------------
void CNPC_Bob3::SteerSeek(Vector &Steer, const Vector &Target)
{
	Vector offset = Target - GetLocalOrigin();

	VectorNormalize(offset);

	Vector DesiredVelocity = m_flGroundSpeed * offset;

	Steer = DesiredVelocity - GetAbsVelocity();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &Steer - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Bob3::SteerAvoidObstacles(Vector &Steer, const Vector &Velocity, const Vector &Forward, const Vector &Right, const Vector &Up)
{
	trace_t	tr;

	bool	collided = false;
	Vector	dir = Velocity;
	float	speed = VectorNormalize(dir);

	//Look ahead one second and avoid whatever is in our way.
	AI_TraceHull(GetAbsOrigin(), GetAbsOrigin() + (dir*speed), GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, COLLISION_GROUP_NONE, &tr);

	Vector	forward;

	GetVectors(&forward, NULL, NULL);

	//If we're hitting our enemy, just continue on
	if ((GetEnemy() != NULL) && (tr.m_pEnt == GetEnemy()))
		return false;

	if (tr.fraction < 1.0f)
	{
		CBaseEntity *pBlocker = tr.m_pEnt;

		if ((pBlocker != NULL) && (pBlocker->MyNPCPointer() != NULL))
		{
			DevMsg(2, "Avoiding an NPC\n");

			Vector HitOffset = tr.endpos - GetAbsOrigin();

			Vector SteerUp = CrossProduct(HitOffset, Velocity);
			Steer = CrossProduct(SteerUp, Velocity);
			VectorNormalize(Steer);

			/*Vector probeDir = tr.endpos - GetAbsOrigin();
			Vector normalToProbeAndWallNormal = probeDir.Cross( tr.plane.normal );

			Steer = normalToProbeAndWallNormal.Cross( probeDir );
			VectorNormalize( Steer );*/

			if (tr.fraction > 0)
			{
				Steer = (Steer * Velocity.Length()) / tr.fraction;
				//NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin()+Steer, 255, 0, 0, false, 0.1f );
			}
			else
			{
				Steer = (Steer * 1000 * Velocity.Length());
				//NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin()+Steer, 255, 0, 0, false, 0.1f );
			}
		}
		else
		{
			if ((pBlocker != NULL) && (pBlocker == GetEnemy()))
			{
				DevMsg("Avoided collision\n");
				return false;
			}

			DevMsg(2, "Avoiding the world\n");

			Vector	steeringVector = tr.plane.normal;

			if (tr.fraction == 0.0f)
				return false;

			Steer = steeringVector * (Velocity.Length() / tr.fraction);

			//NDebugOverlay::Line( GetLocalOrigin(), GetLocalOrigin()+Steer, 255, 0, 0, false, 0.1f );
		}

		//return true;
		collided = true;
	}

	//Try to remain 8 feet above the ground.
	AI_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -ICH_HEIGHT_PREFERENCE), MASK_NPCSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0f)
	{
		Steer += Vector(0, 0, m_vecAccelerationMax.z / tr.fraction);
		collided = true;
	}

	//Stay under the surface
	//if (m_bIgnoreSurface == false)
	//{
	//	float waterLevel = (UTIL_WaterLevel(GetAbsOrigin(), GetAbsOrigin().z, GetAbsOrigin().z + ICH_DEPTH_PREFERENCE) - GetAbsOrigin().z) / ICH_DEPTH_PREFERENCE;

	//	if (waterLevel < 1.0f)
	//	{
	//		Steer += -Vector(0, 0, m_vecAccelerationMax.z / waterLevel);
	//		collided = true;
	//	}
	//}

	return collided;
}


//-----------------------------------------------------------------------------
// Purpose: Clamps the desired steering vector based on the limitations of this
//			vehicle.
// Input  : SteerAbs - The vector indicating our ideal steering vector. Receives
//				the clamped steering vector in absolute (x,y,z) coordinates.
//			SteerRel - Receives the clamped steering vector in relative (forward, right, up)
//				coordinates.
//			forward - Our current forward vector.
//			right - Our current right vector.
//			up - Our current up vector.
//-----------------------------------------------------------------------------
void CNPC_Bob3::ClampSteer(Vector &SteerAbs, Vector &SteerRel, Vector &forward, Vector &right, Vector &up)
{
	float fForwardSteer = DotProduct(SteerAbs, forward);
	float fRightSteer = DotProduct(SteerAbs, right);
	float fUpSteer = DotProduct(SteerAbs, up);

	if (fForwardSteer > 0)
	{
		fForwardSteer = MIN(fForwardSteer, m_vecAccelerationMax.x);
	}
	else
	{
		fForwardSteer = MAX(fForwardSteer, m_vecAccelerationMin.x);
	}

	if (fRightSteer > 0)
	{
		fRightSteer = MIN(fRightSteer, m_vecAccelerationMax.y);
	}
	else
	{
		fRightSteer = MAX(fRightSteer, m_vecAccelerationMin.y);
	}

	if (fUpSteer > 0)
	{
		fUpSteer = MIN(fUpSteer, m_vecAccelerationMax.z);
	}
	else
	{
		fUpSteer = MAX(fUpSteer, m_vecAccelerationMin.z);
	}

	SteerAbs = (fForwardSteer*forward) + (fRightSteer*right) + (fUpSteer*up);

	SteerRel.x = fForwardSteer;
	SteerRel.y = fRightSteer;
	SteerRel.z = fUpSteer;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTargetEnt - 
//			vecDir - 
//			flDistance - 
//			flInterval - 
//-----------------------------------------------------------------------------
void CNPC_Bob3::MoveFlyExecute(CBaseEntity *pTargetEnt, const Vector &vecDir, float flDistance, float flInterval)
{
	Bob3MoveType_t eMoveType = (GetNavigator()->CurWaypointIsGoal()) ? ICH_MOVETYPE_ARRIVE : ICH_MOVETYPE_SEEK;

	m_flGroundSpeed = GetGroundSpeed();

	Vector	moveGoal = GetNavigator()->GetCurWaypointPos();

	//See if we can move directly to our goal
	if ((GetEnemy() != NULL) && (GetNavigator()->GetGoalTarget() == (CBaseEntity *)GetEnemy()))
	{
		trace_t	tr;
		Vector	goalPos = GetEnemy()->GetAbsOrigin() + (GetEnemy()->GetSmoothedVelocity() * 0.5f);

		AI_TraceHull(GetAbsOrigin(), goalPos, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, GetEnemy(), COLLISION_GROUP_NONE, &tr);

		if (tr.fraction == 1.0f)
		{
			moveGoal = tr.endpos;
		}
	}

	//Move
	DoMovement(flInterval, moveGoal, eMoveType);

	//Save the info from that run
	m_vecLastMoveTarget = moveGoal;
	m_bHasMoveTarget = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEntity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_Bob3::FVisible(CBaseEntity *pEntity, int traceMask, CBaseEntity **ppBlocker)
{
	// don't look through water
	//if (GetWaterLevel() != pEntity->GetWaterLevel())
	//	return false;

	return BaseClass::FVisible(pEntity, traceMask, ppBlocker);
}

//-----------------------------------------------------------------------------
// Purpose: Get our conditions for a melee attack
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Bob3::MeleeAttack1Conditions(float flDot, float flDist)
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
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CNPC_Bob3::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case BOB3_AE_BITE:
		Bite();
		break;

	case BOB3_AE_BITE_START:
	{
		EmitSound("NPC_Ichthyosaur.AttackGrowl");
	}
	break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob3::Bite(void)
{
	//Don't allow another bite too soon
	if (m_flNextBiteTime > gpGlobals->curtime)
		return;

	CBaseEntity *pHurt;

	//FIXME: E3 HACK - Always damage bullseyes if we're scripted to hit them
	if ((GetEnemy() != NULL) && (GetEnemy()->Classify() == CLASS_BULLSEYE))
	{
		pHurt = GetEnemy();
	}
	else
	{
		pHurt = CheckTraceHullAttack(108, Vector(-32, -32, -32), Vector(32, 32, 32), 0, DMG_CLUB);
	}

	//Hit something
	if (pHurt != NULL)
	{
		CTakeDamageInfo info(this, this, sk_bob3_melee_dmg.GetInt(), DMG_CLUB);

		if (pHurt->IsPlayer())
		{
			CBasePlayer *pPlayer = ToBasePlayer(pHurt);

			if (pPlayer)
			{
				if (((m_flHoldTime < gpGlobals->curtime) && (pPlayer->m_iHealth < (pPlayer->m_iMaxHealth*0.5f))) || (pPlayer->GetWaterLevel() < 1))
				{
				}
				else
				{
					info.SetDamage(sk_bob3_melee_dmg.GetInt() * 3);
				}
				CalculateMeleeDamageForce(&info, GetAbsVelocity(), pHurt->GetAbsOrigin());
				pHurt->TakeDamage(info);

				color32 red = { 64, 0, 0, 255 };
				UTIL_ScreenFade(pPlayer, red, 0.5, 0, FFADE_IN);

				////Disorient the player
				//QAngle angles = pPlayer->GetLocalAngles();

				//angles.x += random->RandomInt(60, 25);
				//angles.y += random->RandomInt(60, 25);
				//angles.z = 0.0f;

				//pPlayer->SetLocalAngles(angles);

				//pPlayer->SnapEyeAngles(angles);
			}
		}
		else
		{
			CalculateMeleeDamageForce(&info, GetAbsVelocity(), pHurt->GetAbsOrigin());
			pHurt->TakeDamage(info);
		}

		m_flNextBiteTime = gpGlobals->curtime + random->RandomFloat(2.0f, 4.0f);

		// Play a random attack hit sound
		EmitSound("NPC_Ichthyosaur.Bite");

		if (GetActivity() == ACT_MELEE_ATTACK1)
		{
			SetActivity((Activity)ACT_MELEE_ATTACK1);
		}

		return;
	}

	//Play the miss animation and sound
	if (GetActivity() == ACT_MELEE_ATTACK1)
	{
		SetActivity((Activity)ACT_MELEE_ATTACK1);
	}

	//Miss sound
	EmitSound("NPC_Ichthyosaur.BiteMiss");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob3::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	//Pings
	if (m_flNextPingTime < gpGlobals->curtime)
	{
		m_flNextPingTime = gpGlobals->curtime + random->RandomFloat(3.0f, 8.0f);
	}

	//Growls
	if ((m_NPCState == NPC_STATE_COMBAT || m_NPCState == NPC_STATE_ALERT) && (m_flNextGrowlTime < gpGlobals->curtime))
	{
		m_flNextGrowlTime = gpGlobals->curtime + random->RandomFloat(2.0f, 6.0f);
	}
//Doesn't matter


	//If we have a victim, update them
	if (m_pVictim != NULL)
	{
		Bite();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pevInflictor - 
//			*pAttacker -  
//			flDamage - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
int	CNPC_Bob3::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
		m_flNextBiteTime = gpGlobals->curtime + 2.0f;

	return BaseClass::OnTakeDamage_Alive(info);
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : speed to move at
//-----------------------------------------------------------------------------
float CNPC_Bob3::GetGroundSpeed(void)
{
	if (m_flHoldTime > gpGlobals->curtime)
		return	ICH_SWIM_SPEED_WALK / 2.0f;

	if (GetIdealActivity() == ACT_WALK)
		return ICH_SWIM_SPEED_WALK;

	return ICH_SWIM_SPEED_RUN;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : type - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_Bob3::TranslateSchedule(int type)
{
	if (type == SCHED_CHASE_ENEMY)	return SCHED_BOB3_CHASE_ENEMY;
	//if ( type == SCHED_IDLE_STAND )		return SCHED_PATROL_WALK;
	if (type == SCHED_PATROL_RUN)		return SCHED_BOB3_PATROL_RUN;
	if (type == SCHED_PATROL_WALK)	return SCHED_BOB3_PATROL_WALK;
	if (type == SCHED_MELEE_ATTACK1)	return SCHED_BOB3_MELEE_ATTACK1;

	return BaseClass::TranslateSchedule(type);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CNPC_Bob3::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{

	case TASK_MELEE_ATTACK1:
		m_flPlaybackRate = 1.0f;
		BaseClass::StartTask(pTask);
		break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CNPC_Bob3::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{

	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : desired yaw speed
//-----------------------------------------------------------------------------
float CNPC_Bob3::MaxYawSpeed(void)
{
	if (GetIdealActivity() == ACT_MELEE_ATTACK1)
		return 96.0f;

	//Ramp up the yaw speed as we increase our speed
	return ICH_MIN_TURN_SPEED + ((ICH_MAX_TURN_SPEED - ICH_MIN_TURN_SPEED) * (fabs(GetAbsVelocity().Length()) / ICH_SWIM_SPEED_RUN));
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEnemy - 
//			&chasePosition - 
//			&tolerance - 
//-----------------------------------------------------------------------------
void CNPC_Bob3::TranslateNavGoal(CBaseEntity *pEnemy, Vector &chasePosition)
{
	Vector offset = pEnemy->EyePosition() - pEnemy->GetAbsOrigin();
	chasePosition += offset;
}

float CNPC_Bob3::GetDefaultNavGoalTolerance()
{
	return GetHullWidth()*2.0f;
}


//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

//==================================================
// CHASE_ENEMY
//==================================================

AI_DEFINE_SCHEDULE
(
SCHED_BOB3_CHASE_ENEMY,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_BOB3_PATROL_WALK"
"		TASK_SET_TOLERANCE_DISTANCE		64"
"		TASK_SET_GOAL					GOAL:ENEMY"
"		TASK_GET_PATH_TO_GOAL			PATH:TRAVEL"
"		TASK_RUN_PATH					0"
"		TASK_WAIT_FOR_MOVEMENT			0"
"		TASK_FACE_ENEMY					0"
""
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_ENEMY_UNREACHABLE"
"		COND_CAN_MELEE_ATTACK1"
"		COND_TOO_CLOSE_TO_ATTACK"
"		COND_LOST_ENEMY"
"		COND_TASK_FAILED"
);


//==================================================
// PATROL_RUN
//==================================================

AI_DEFINE_SCHEDULE
(
SCHED_BOB3_PATROL_RUN,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
"		TASK_SET_TOLERANCE_DISTANCE			64"
"		TASK_SET_ROUTE_SEARCH_TIME			4"
"		TASK_RUN_PATH						0"
"		TASK_WAIT_FOR_MOVEMENT				0"
""
"	Interrupts"
"		COND_CAN_MELEE_ATTACK1"
"		COND_GIVE_WAY"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
);

//==================================================
// PATROL_WALK
//==================================================

AI_DEFINE_SCHEDULE
(
SCHED_BOB3_PATROL_WALK,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_COMBAT_FACE"
"		TASK_SET_TOLERANCE_DISTANCE			64"
"		TASK_SET_ROUTE_SEARCH_TIME			4"
"		TASK_WALK_PATH						0"
"		TASK_WAIT_FOR_MOVEMENT				0"
""
"	Interrupts"
"		COND_CAN_MELEE_ATTACK1"
"		COND_GIVE_WAY"
"		COND_NEW_ENEMY"
"		COND_LIGHT_DAMAGE"
"		COND_HEAVY_DAMAGE"
);


//=========================================================
// MELEE_ATTACK1
//=========================================================

AI_DEFINE_SCHEDULE
(
SCHED_BOB3_MELEE_ATTACK1,

"	Tasks"
"		TASK_ANNOUNCE_ATTACK	1"
"		TASK_MELEE_ATTACK1		0"
""
"	Interrupts"
"		COND_NEW_ENEMY"
"		COND_ENEMY_DEAD"
"		COND_ENEMY_OCCLUDED"
);
