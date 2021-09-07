//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef NPC_BOB2_H
#define NPC_BOB2_H
#ifdef _WIN32
#pragma once
#endif

#include "rope.h"
#include "rope_shared.h"
#include "ai_baseactor.h"
#include "ai_basenpc.h"
#include "ai_behavior.h"
#include "ai_behavior_assault.h"
#include "ai_behavior_actbusy.h"
#include "props.h"

class CNPC_Bob2;

class CNPC_Bob2 : public CAI_BaseActor
{
	DECLARE_CLASS(CNPC_Bob2, CAI_BaseActor);
	DECLARE_DATADESC();

public:
	CNPC_Bob2();

	virtual bool CreateComponents();
	bool CreateBehaviors();
	void Spawn(void);
	void Precache(void);

	Class_T		Classify(void);
	Disposition_t IRelationType(CBaseEntity *pTarget);
	float		MaxYawSpeed(void);
	void		HandleAnimEvent(animevent_t *pEvent);
	Activity NPC_TranslateActivity(Activity newActivity);

	Vector		EyeDirection3D(void)	{ return CAI_BaseHumanoid::EyeDirection3D(); } // cops don't have eyes

	virtual void Event_Killed(const CTakeDamageInfo &info);

	virtual void OnScheduleChange();

	float		GetIdealAccel(void) const;
	int			ObjectCaps(void) { return UsableNPCObjectCaps(BaseClass::ObjectCaps()); }

	//Internal non weapon melee attack
	int		MeleeAttack1Conditions(float flDot, float flDist);

	// These are overridden so that the cop can shove and move a non-criminal player safely
	CBaseEntity *CheckTraceHullAttack(float flDist, const Vector &mins, const Vector &maxs, int iDamage, int iDmgType, float forceScale, bool bDamageAnyNPC);
	CBaseEntity *CheckTraceHullAttack(const Vector &vStart, const Vector &vEnd, const Vector &mins, const Vector &maxs, int iDamage, int iDmgType, float flForceScale, bool bDamageAnyNPC);

	virtual int	SelectSchedule(void);
	virtual int SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode);
	virtual int TranslateSchedule(int scheduleType);
	void		StartTask(const Task_t *pTask);
	void		RunTask(const Task_t *pTask);

	virtual bool HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);

	//virtual bool OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	bool		OnObstructionPreSteer(AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult);
	bool		ShouldBruteForceFailedNav()	{ return false; }

	virtual void GatherConditions(void);

	virtual bool OverrideMoveFacing(const AILocalMoveGoal_t &move, float flInterval);

	// TraceAttack
	virtual void TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);

	void	AdministerJustice(void);

	bool	QueryHearSound(CSound *pSound);

	virtual	bool		AllowedToIgnite(void) { return true; }

	void	PlayFlinchGesture(void);

protected:
	// Determines the best type of flinch anim to play.
	virtual Activity GetFlinchActivity(bool bHeavyDamage, bool bGesture);

private:

	virtual void	LostEnemySound(void);
	virtual void	FoundEnemySound(void);
	virtual void	AlertSound(void);
	virtual void	PainSound(const CTakeDamageInfo &info);
	virtual void	DeathSound(const CTakeDamageInfo &info);
	virtual void	IdleSound(void);

	int			OnTakeDamage(const CTakeDamageInfo &inputInfo);
	int			OnTakeDamage_Alive(const CTakeDamageInfo &info);

	int			GetSoundInterests(void);

	void		BuildScheduleTestBits(void);

	bool		ShouldHitPlayer(const Vector &targetDir, float targetDist);

	void		PrescheduleThink(void);

	virtual bool		UseAttackSquadSlots()	{ return true; }

	// Inputs

	// Anim event handlers
	void OnAnimEventMeleeAttack1(void); 

	// Normal schedule selection 
	int SelectCombatSchedule();
	int SelectScheduleNewEnemy();
	int SelectScheduleNoDirectEnemy();
	int SelectScheduleInvestigateSound();

	// Handle flinching
	bool IsHeavyDamage(const CTakeDamageInfo &info);

	// Can me enemy see me? 
	bool CanEnemySeeMe();

	void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent);

private:

	enum
	{
		COND_BOB2_ON_FIRE = BaseClass::NEXT_CONDITION,
		COND_BOB2_PLAYER_TOO_CLOSE,
		COND_BOB2_PHYSOBJECT_ASSAULT,

	};

	enum
	{
		SCHED_BOB2_WALK = BaseClass::NEXT_SCHEDULE,
		SCHED_BOB2_WAKE_ANGRY,
		SCHED_BOB2_HARASS,
//		SCHED_BOB2_CHASE_ENEMY,
//		SCHED_BOB2_ESTABLISH_LINE_OF_FIRE,
		SCHED_BOB2_ADVANCE,
		SCHED_BOB2_CHARGE,
		SCHED_BOB2_BURNING_RUN,
		SCHED_BOB2_BURNING_STAND,
		SCHED_BOB2_INVESTIGATE_SOUND,
		SCHED_BOB2_ALERT_FACE_BESTSOUND,
		SCHED_BOB2_RETURN_TO_PRECHASE,
		SCHED_BOB2_SMASH_PROP,
	};

	enum
	{
		TASK_BOB2_HARASS = BaseClass::NEXT_TASK,
		TASK_BOB2_DIE_INSTANTLY,
		TASK_BOB2_GET_PATH_TO_BESTSOUND_LOS,
		TASK_BOB2_WAIT_FOR_SENTENCE,
		TASK_BOB2_GET_PATH_TO_PRECHASE,
		TASK_BOB2_CLEAR_PRECHASE,
	};


private:

	CSimpleSimTimer m_BatonSwingTimer;
	CSimpleSimTimer m_NextChargeTimer;

	CSimpleSimTimer m_ArmorBreakDownTime;
	
	bool			m_bIsArmorBreakState;
	bool			m_bIsArmorBreak;

	float			m_flBatonDebounceTime;	// Minimum amount of time before turning the baton off
	float			m_flLastPhysicsFlinchTime;
	float			m_flLastDamageFlinchTime;

	// Sentences
	float			m_flNextPainSoundTime;
	float			m_flNextLostSoundTime;

	// Policing state
	bool			m_bPlayerTooClose;
	bool			m_bKeepFacingPlayer;
	float			m_flChasePlayerTime;
	Vector			m_vecPreChaseOrigin;
	float			m_flPreChaseYaw;
	int				m_iNumPlayerHits;

	// Outputs

	CHandle<CPhysicsProp>	m_hBlockingProp;

	CAI_ActBusyBehavior		m_ActBusyBehavior;
	CAI_AssaultBehavior		m_AssaultBehavior;

	int				m_nRecentDamage;
	float			m_flRecentDamageTime;

	// The last hit direction, measured as a yaw relative to our orientation
	float			m_flLastHitYaw;

public:
	DEFINE_CUSTOM_AI;
};

#endif