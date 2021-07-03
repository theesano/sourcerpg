//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: ???
//
//=============================================================================//

#include "cbase.h"

#include "doors.h"

#include "simtimer.h"
#include "npc_BaseZombie.h"
#include "ai_hull.h"
#include "ai_navigator.h"
#include "ai_memory.h"
#include "soundenvelope.h"
#include "engine/IEngineSound.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ACT_FLINCH_PHYSICS


ConVar	sk_bob2_health("sk_bob2_health", "0");

envelopePoint_t envBob2MoanVolumeFast[] =
{
	{ 7.0f, 7.0f,
	0.1f, 0.1f,
	},
	{ 0.0f, 0.0f,
	0.2f, 0.3f,
	},
};

envelopePoint_t envBob2MoanVolume[] =
{
	{ 1.0f, 1.0f,
	0.1f, 0.1f,
	},
	{ 1.0f, 1.0f,
	0.2f, 0.2f,
	},
	{ 0.0f, 0.0f,
	0.3f, 0.4f,
	},
};

envelopePoint_t envBob2MoanVolumeLong[] =
{
	{ 1.0f, 1.0f,
	0.3f, 0.5f,
	},
	{ 1.0f, 1.0f,
	0.6f, 1.0f,
	},
	{ 0.0f, 0.0f,
	0.3f, 0.4f,
	},
};

envelopePoint_t envBob2MoanIgnited[] =
{
	{ 1.0f, 1.0f,
	0.5f, 1.0f,
	},
	{ 1.0f, 1.0f,
	30.0f, 30.0f,
	},
	{ 0.0f, 0.0f,
	0.5f, 1.0f,
	},
};


//=============================================================================
//=============================================================================

class CNPC_Bob2 : public CAI_BlendingHost<CNPC_BaseZombie>
{
	DECLARE_DATADESC();
	DECLARE_CLASS(CNPC_Bob2, CAI_BlendingHost<CNPC_BaseZombie>);

public:
	CNPC_Bob2()
	{
	}

	void Spawn(void);
	void Precache(void);

	void SetZombieModel(void);
	void MoanSound(envelopePoint_t *pEnvelope, int iEnvelopeSize);
	bool ShouldBecomeTorso(const CTakeDamageInfo &info, float flDamageThreshold);
	bool CanBecomeLiveTorso() { return !m_fIsHeadless; }

	void GatherConditions(void);

	int SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode);
	int TranslateSchedule(int scheduleType);

#ifndef HL2_EPISODIC
	void CheckFlinches() {} // Zombie has custom flinch code
#endif // HL2_EPISODIC

	Activity NPC_TranslateActivity(Activity newActivity);

	void OnStateChange(NPC_STATE OldState, NPC_STATE NewState);

	void StartTask(const Task_t *pTask);
	void RunTask(const Task_t *pTask);

	virtual const char *GetLegsModel(void);
	virtual const char *GetTorsoModel(void);
	virtual const char *GetHeadcrabClassname(void);
	virtual const char *GetHeadcrabModel(void);

	void Ignite(float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false);
	void Extinguish();
	int OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);
	bool IsHeavyDamage(const CTakeDamageInfo &info);
	bool IsSquashed(const CTakeDamageInfo &info);
	void BuildScheduleTestBits(void);

	void PrescheduleThink(void);
	int SelectSchedule(void);

	void PainSound(const CTakeDamageInfo &info);
	void DeathSound(const CTakeDamageInfo &info);
	void AlertSound(void);
	void IdleSound(void);
	void AttackSound(void);
	void AttackHitSound(void);
	void AttackMissSound(void);
	void FootstepSound(bool fRightFoot);
	void FootscuffSound(bool fRightFoot);

	const char *GetMoanSound(int nSound);

public:
	DEFINE_CUSTOM_AI;

protected:
	static const char *pMoanSounds[];


private:

	Vector				 m_vPositionCharged;
};

LINK_ENTITY_TO_CLASS(npc_bob2, CNPC_Bob2);

//---------------------------------------------------------
//---------------------------------------------------------
const char *CNPC_Bob2::pMoanSounds[] =
{
	"NPC_BaseZombie.Moan1",
	"NPC_BaseZombie.Moan2",
	"NPC_BaseZombie.Moan3",
	"NPC_BaseZombie.Moan4",
};

//=========================================================
// Conditions
//=========================================================
enum
{
	//COND_ZOMBIE_CHARGE_TARGET_MOVED = LAST_BASE_ZOMBIE_CONDITION,
	COND_BLOCKED_BY_DOOR = LAST_BASE_ZOMBIE_CONDITION,
	COND_DOOR_OPENED,
	COND_ZOMBIE_CHARGE_TARGET_MOVED,

};

//=========================================================
// Schedules
//=========================================================
enum
{
	SCHED_ZOMBIE_BASH_DOOR = LAST_BASE_ZOMBIE_SCHEDULE,
	SCHED_ZOMBIE_WANDER_ANGRILY,
	SCHED_ZOMBIE_CHARGE_ENEMY,
	SCHED_ZOMBIE_FAIL,
};

//=========================================================
// Tasks
//=========================================================
enum
{
	TASK_ZOMBIE_EXPRESS_ANGER = LAST_BASE_ZOMBIE_TASK,
	TASK_ZOMBIE_CHARGE_ENEMY,
};

//-----------------------------------------------------------------------------


BEGIN_DATADESC(CNPC_Bob2)

DEFINE_FIELD(m_vPositionCharged, FIELD_POSITION_VECTOR),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob2::Precache(void)
{
	BaseClass::Precache();

	PrecacheModel("models/monster/mob1_puppet.mdl");
	//PrecacheModel("models/police.mdl");

	PrecacheScriptSound("Zombie.FootstepRight");
	PrecacheScriptSound("Zombie.FootstepLeft");
	PrecacheScriptSound("Zombie.FootstepLeft");
	PrecacheScriptSound("Zombie.ScuffRight");
	PrecacheScriptSound("Zombie.ScuffLeft");
	PrecacheScriptSound("Zombie.AttackHit");
	PrecacheScriptSound("Zombie.AttackMiss");
	PrecacheScriptSound("Zombie.Pain");
	PrecacheScriptSound("Zombie.Die");
	PrecacheScriptSound("Zombie.Alert");
	PrecacheScriptSound("Zombie.Idle");
	PrecacheScriptSound("Zombie.Attack");

	PrecacheScriptSound("NPC_BaseZombie.Moan1");
	PrecacheScriptSound("NPC_BaseZombie.Moan2");
	PrecacheScriptSound("NPC_BaseZombie.Moan3");
	PrecacheScriptSound("NPC_BaseZombie.Moan4");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Bob2::Spawn(void)
{
	Precache();

	m_fIsTorso = false;

	m_fIsHeadless = false;

#ifdef HL2_EPISODIC
	SetBloodColor(BLOOD_COLOR_ZOMBIE);
#else
	SetBloodColor(BLOOD_COLOR_GREEN);
#endif // HL2_EPISODIC

	m_iHealth = sk_bob2_health.GetFloat();
	m_flFieldOfView = 0.5;

	CapabilitiesClear();

	//GetNavigator()->SetRememberStaleNodes( false );

	BaseClass::Spawn();

	m_flNextMoanSound = gpGlobals->curtime + random->RandomFloat(1.0, 4.0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Bob2::PrescheduleThink(void)
{
	if (gpGlobals->curtime > m_flNextMoanSound)
	{
		if (CanPlayMoanSound())
		{
			// Classic guy idles instead of moans.
			IdleSound();

			m_flNextMoanSound = gpGlobals->curtime + random->RandomFloat(2.0, 5.0);
		}
		else
		{
			m_flNextMoanSound = gpGlobals->curtime + random->RandomFloat(1.0, 2.0);
		}
	}

	BaseClass::PrescheduleThink();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CNPC_Bob2::SelectSchedule(void)
{
	if (HasCondition(COND_PHYSICS_DAMAGE) && !m_ActBusyBehavior.IsActive())
	{
		return SCHED_FLINCH_PHYSICS;
	}

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Purpose: Sound of a footstep
//-----------------------------------------------------------------------------
void CNPC_Bob2::FootstepSound(bool fRightFoot)
{
	if (fRightFoot)
	{
		EmitSound("Zombie.FootstepRight");
	}
	else
	{
		EmitSound("Zombie.FootstepLeft");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sound of a foot sliding/scraping
//-----------------------------------------------------------------------------
void CNPC_Bob2::FootscuffSound(bool fRightFoot)
{
	if (fRightFoot)
	{
		EmitSound("Zombie.ScuffRight");
	}
	else
	{
		EmitSound("Zombie.ScuffLeft");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack hit sound
//-----------------------------------------------------------------------------
void CNPC_Bob2::AttackHitSound(void)
{
	EmitSound("Zombie.AttackHit");
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack miss sound
//-----------------------------------------------------------------------------
void CNPC_Bob2::AttackMissSound(void)
{
	// Play a random attack miss sound
	EmitSound("Zombie.AttackMiss");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob2::PainSound(const CTakeDamageInfo &info)
{
	// We're constantly taking damage when we are on fire. Don't make all those noises!
	if (IsOnFire())
	{
		return;
	}

	EmitSound("Zombie.Pain");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Bob2::DeathSound(const CTakeDamageInfo &info)
{
	EmitSound("Zombie.Die");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bob2::AlertSound(void)
{
	EmitSound("Zombie.Alert");

	// Don't let a moan sound cut off the alert sound.
	m_flNextMoanSound += random->RandomFloat(2.0, 4.0);
}

//-----------------------------------------------------------------------------
// Purpose: Returns a moan sound for this class of zombie.
//-----------------------------------------------------------------------------
const char *CNPC_Bob2::GetMoanSound(int nSound)
{
	return pMoanSounds[nSound % ARRAYSIZE(pMoanSounds)];
}

//-----------------------------------------------------------------------------
// Purpose: Play a random idle sound.
//-----------------------------------------------------------------------------
void CNPC_Bob2::IdleSound(void)
{
	if (GetState() == NPC_STATE_IDLE && random->RandomFloat(0, 1) == 0)
	{
		// Moan infrequently in IDLE state.
		return;
	}

	if (IsSlumped())
	{
		// Sleeping zombies are quiet.
		return;
	}

	EmitSound("Zombie.Idle");
	MakeAISpookySound(360.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_Bob2::AttackSound(void)
{
	EmitSound("Zombie.Attack");
}

//-----------------------------------------------------------------------------
// Purpose: Returns the classname (ie "npc_headcrab") to spawn when our headcrab bails.
//-----------------------------------------------------------------------------
const char *CNPC_Bob2::GetHeadcrabClassname(void)
{
	return "npc_headcrab";
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const char *CNPC_Bob2::GetHeadcrabModel(void)
{
	return "models/headcrabclassic.mdl";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CNPC_Bob2::GetLegsModel(void)
{
	return "models/zombie/classic_legs.mdl";
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const char *CNPC_Bob2::GetTorsoModel(void)
{
	return "models/zombie/classic_torso.mdl";
}


//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Bob2::SetZombieModel(void)
{
	Hull_t lastHull = GetHullType();

	if (m_fIsTorso)
	{
	}
	else
	{
		SetModel("models/monster/mob1_puppet.mdl");
		//SetModel("models/police.mdl");

		SetHullType(HULL_HUMAN);
	}

	SetBodygroup(ZOMBIE_BODYGROUP_HEADCRAB, !m_fIsHeadless);

	SetHullSizeNormal(true);
	SetDefaultEyeOffset();
	SetActivity(ACT_IDLE);

	// hull changed size, notify vphysics
	// UNDONE: Solve this generally, systematically so other
	// NPCs can change size
	if (lastHull != GetHullType())
	{
		if (VPhysicsGetObject())
		{
			SetupVPhysicsHull();
		}
	}
}

//---------------------------------------------------------
// Classic zombie only uses moan sound if on fire.
//---------------------------------------------------------
void CNPC_Bob2::MoanSound(envelopePoint_t *pEnvelope, int iEnvelopeSize)
{
	if (IsOnFire())
	{
		BaseClass::MoanSound(pEnvelope, iEnvelopeSize);
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CNPC_Bob2::ShouldBecomeTorso(const CTakeDamageInfo &info, float flDamageThreshold)
{
	
	return false;

	return BaseClass::ShouldBecomeTorso(info, flDamageThreshold);
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Bob2::GatherConditions(void)
{
	BaseClass::GatherConditions();

	static int conditionsToClear[] =
	{
		COND_ZOMBIE_CHARGE_TARGET_MOVED,
	};

	ClearConditions(conditionsToClear, ARRAYSIZE(conditionsToClear));

	if (ConditionInterruptsCurSchedule(COND_ZOMBIE_CHARGE_TARGET_MOVED))
	{
		if (GetNavigator()->IsGoalActive())
		{
			const float CHARGE_RESET_TOLERANCE = 60.0;
			if (!GetEnemy() ||
				(m_vPositionCharged - GetEnemyLKP()).Length() > CHARGE_RESET_TOLERANCE)
			{
				SetCondition(COND_ZOMBIE_CHARGE_TARGET_MOVED);
			}

		}
	}
}

//---------------------------------------------------------
//---------------------------------------------------------

int CNPC_Bob2::SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode)
{

	if (failedSchedule != SCHED_ZOMBIE_CHARGE_ENEMY &&
		IsPathTaskFailure(taskFailCode) &&
		random->RandomInt(1, 100) < 50)
	{
		return SCHED_ZOMBIE_CHARGE_ENEMY;
	}

	if (failedSchedule != SCHED_ZOMBIE_WANDER_ANGRILY &&
		(failedSchedule == SCHED_TAKE_COVER_FROM_ENEMY ||
		failedSchedule == SCHED_CHASE_ENEMY_FAILED))
	{
		return SCHED_ZOMBIE_WANDER_ANGRILY;
	}

	return BaseClass::SelectFailSchedule(failedSchedule, failedTask, taskFailCode);
}

//---------------------------------------------------------
//---------------------------------------------------------

int CNPC_Bob2::TranslateSchedule(int scheduleType)
{
	if (scheduleType == SCHED_COMBAT_FACE && IsUnreachable(GetEnemy()))
		return SCHED_TAKE_COVER_FROM_ENEMY;

	if (!m_fIsTorso && scheduleType == SCHED_FAIL)
		return SCHED_ZOMBIE_FAIL;

	return BaseClass::TranslateSchedule(scheduleType);
}

//---------------------------------------------------------

Activity CNPC_Bob2::NPC_TranslateActivity(Activity newActivity)
{
	newActivity = BaseClass::NPC_TranslateActivity(newActivity);

	if (newActivity == ACT_RUN)
		return ACT_WALK;

	return newActivity;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Bob2::OnStateChange(NPC_STATE OldState, NPC_STATE NewState)
{
	BaseClass::OnStateChange(OldState, NewState);
}

//---------------------------------------------------------
//---------------------------------------------------------

void CNPC_Bob2::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{

	case TASK_ZOMBIE_CHARGE_ENEMY:
	{
		if (!GetEnemy())
			TaskFail(FAIL_NO_ENEMY);
		else if (GetNavigator()->SetVectorGoalFromTarget(GetEnemy()->GetLocalOrigin()))
		{
			m_vPositionCharged = GetEnemy()->GetLocalOrigin();
			TaskComplete();
		}
		else
			TaskFail(FAIL_NO_ROUTE);
		break;
	}

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------

void CNPC_Bob2::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{

	case TASK_ZOMBIE_CHARGE_ENEMY:
	{
		break;
	}

	default:
		BaseClass::RunTask(pTask);
		break;
	}
}


//---------------------------------------------------------
// Zombies should scream continuously while burning, so long
// as they are alive... but NOT IN GERMANY!
//---------------------------------------------------------
void CNPC_Bob2::Ignite(float flFlameLifetime, bool bNPCOnly, float flSize, bool bCalledByLevelDesigner)
{
	if (!IsOnFire() && IsAlive())
	{
		BaseClass::Ignite(flFlameLifetime, bNPCOnly, flSize, bCalledByLevelDesigner);

		if (!UTIL_IsLowViolence())
		{
			RemoveSpawnFlags(SF_NPC_GAG);

			MoanSound(envBob2MoanIgnited, ARRAYSIZE(envBob2MoanIgnited));

			if (m_pMoanSound)
			{
				ENVELOPE_CONTROLLER.SoundChangePitch(m_pMoanSound, 120, 1.0);
				ENVELOPE_CONTROLLER.SoundChangeVolume(m_pMoanSound, 1, 1.0);
			}
		}
	}
}

//---------------------------------------------------------
// If a zombie stops burning and hasn't died, quiet him down
//---------------------------------------------------------
void CNPC_Bob2::Extinguish()
{
	if (m_pMoanSound)
	{
		ENVELOPE_CONTROLLER.SoundChangeVolume(m_pMoanSound, 0, 2.0);
		ENVELOPE_CONTROLLER.SoundChangePitch(m_pMoanSound, 100, 2.0);
		m_flNextMoanSound = gpGlobals->curtime + random->RandomFloat(2.0, 4.0);
	}

	BaseClass::Extinguish();
}

//---------------------------------------------------------
//---------------------------------------------------------
int CNPC_Bob2::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
#ifndef HL2_EPISODIC
	if (inputInfo.GetDamageType() & DMG_BUCKSHOT)
	{
		if (!m_fIsTorso && inputInfo.GetDamage() > (m_iMaxHealth / 3))
		{
			// Always flinch if damaged a lot by buckshot, even if not shot in the head.
			// The reason for making sure we did at least 1/3rd of the zombie's max health
			// is so the zombie doesn't flinch every time the odd shotgun pellet hits them,
			// and so the maximum number of times you'll see a zombie flinch like this is 2.(sjb)
			AddGesture(ACT_GESTURE_FLINCH_HEAD);
		}
	}
#endif // HL2_EPISODIC

	return BaseClass::OnTakeDamage_Alive(inputInfo);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CNPC_Bob2::IsHeavyDamage(const CTakeDamageInfo &info)
{
#ifdef HL2_EPISODIC
	if (info.GetDamageType() & DMG_BUCKSHOT)
	{
		if (!m_fIsTorso && info.GetDamage() > (m_iMaxHealth / 3))
			return true;
	}

	// Randomly treat all damage as heavy
	if (info.GetDamageType() & (DMG_BULLET | DMG_BUCKSHOT))
	{
		// Don't randomly flinch if I'm melee attacking
		if (!HasCondition(COND_CAN_MELEE_ATTACK1) && (RandomFloat() > 0.5))
		{
			// Randomly forget I've flinched, so that I'll be forced to play a big flinch
			// If this doesn't happen, it means I may not fully flinch if I recently flinched
			if (RandomFloat() > 0.75)
			{
				Forget(bits_MEMORY_FLINCHED);
			}

			return true;
		}
	}
#endif // HL2_EPISODIC

	return BaseClass::IsHeavyDamage(info);
}

//---------------------------------------------------------
//---------------------------------------------------------
#define ZOMBIE_SQUASH_MASS	300.0f  // Anything this heavy or heavier squashes a zombie good. (show special fx)
bool CNPC_Bob2::IsSquashed(const CTakeDamageInfo &info)
{
	if (GetHealth() > 0)
	{
		return false;
	}

	if (info.GetDamageType() & DMG_CRUSH)
	{
		IPhysicsObject *pCrusher = info.GetInflictor()->VPhysicsGetObject();
		if (pCrusher && pCrusher->GetMass() >= ZOMBIE_SQUASH_MASS && info.GetInflictor()->WorldSpaceCenter().z > EyePosition().z)
		{
			// This heuristic detects when a zombie has been squashed from above by a heavy
			// item. Done specifically so we can add gore effects to Ravenholm cartraps.
			// The zombie must take physics damage from a 300+kg object that is centered above its eyes (comes from above)
			return true;
		}
	}

	return false;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Bob2::BuildScheduleTestBits(void)
{
	BaseClass::BuildScheduleTestBits();

	if (!m_fIsTorso && !IsCurSchedule(SCHED_FLINCH_PHYSICS) && !m_ActBusyBehavior.IsActive())
	{
		SetCustomInterruptCondition(COND_PHYSICS_DAMAGE);
	}
}


//=============================================================================

AI_BEGIN_CUSTOM_NPC(npc_bob2, CNPC_Bob2)

	DECLARE_CONDITION(COND_ZOMBIE_CHARGE_TARGET_MOVED)

	DECLARE_TASK(TASK_ZOMBIE_EXPRESS_ANGER)
	DECLARE_TASK(TASK_ZOMBIE_CHARGE_ENEMY)

	DEFINE_SCHEDULE
	(
	SCHED_ZOMBIE_WANDER_ANGRILY,

	"	Tasks"
	"		TASK_WANDER						480240" // 48 units to 240 units.
	"		TASK_WALK_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			4"
	""
	"	Interrupts"
	"		COND_ZOMBIE_RELEASECRAB"
	"		COND_ENEMY_DEAD"
	"		COND_NEW_ENEMY"
	)

	DEFINE_SCHEDULE
	(
	SCHED_ZOMBIE_CHARGE_ENEMY,


	"	Tasks"
	"		TASK_ZOMBIE_CHARGE_ENEMY		0"
	"		TASK_WALK_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	""
	"	Interrupts"
	"		COND_ZOMBIE_RELEASECRAB"
	"		COND_ENEMY_DEAD"
	"		COND_NEW_ENEMY"
	"		COND_ZOMBIE_CHARGE_TARGET_MOVED"
	)

	DEFINE_SCHEDULE
	(
	SCHED_ZOMBIE_FAIL,

	"	Tasks"
	"		TASK_STOP_MOVING		0"
	"		TASK_WAIT				1"
	"		TASK_WAIT_PVS			0"
	""
	"	Interrupts"
	"		COND_CAN_RANGE_ATTACK1 "
	"		COND_CAN_RANGE_ATTACK2 "
	"		COND_CAN_MELEE_ATTACK1 "
	"		COND_CAN_MELEE_ATTACK2"
	"		COND_GIVE_WAY"
	)

AI_END_CUSTOM_NPC()

//=============================================================================
