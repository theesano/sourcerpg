//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef NPC_BOB_H
#define NPC_BOB_H
#ifdef _WIN32
#pragma once
#endif

#include "rope.h"
#include "rope_shared.h"
#include "ai_baseactor.h"
#include "ai_basenpc.h"
#include "ai_goal_police.h"
#include "ai_behavior.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_assault.h"
#include "ai_behavior_functank.h"
#include "ai_behavior_actbusy.h"
#include "ai_behavior_rappel.h"
#include "ai_behavior_police.h"
#include "ai_behavior_follow.h"
#include "props.h"

class CNPC_Bob;

class CNPC_Bob : public CAI_BaseActor
{
	DECLARE_CLASS(CNPC_Bob, CAI_BaseActor);
	DECLARE_DATADESC();

public:
	CNPC_Bob();

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
	void		PrecriminalUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

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
	//virtual Vector GetActualShootTrajectory(const Vector &shootOrigin);
	virtual void FireBullets(const FireBulletsInfo_t &info);
	virtual bool HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
	virtual void Weapon_Equip(CBaseCombatWeapon *pWeapon);

	//virtual bool OverrideMoveFacing( const AILocalMoveGoal_t &move, float flInterval );
	bool		OnObstructionPreSteer(AILocalMoveGoal_t *pMoveGoal, float distClear, AIMoveResult_t *pResult);
	bool		ShouldBruteForceFailedNav()	{ return false; }

	virtual void GatherConditions(void);

	virtual bool OverrideMoveFacing(const AILocalMoveGoal_t &move, float flInterval);

	// Can't move and shoot when the enemy is an airboat
	virtual bool ShouldMoveAndShoot();

	// TraceAttack
	virtual void TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);

	// Set up the shot regulator based on the equipped weapon
	virtual void OnUpdateShotRegulator();

	void	AdministerJustice(void);

	bool	QueryHearSound(CSound *pSound);

	virtual	bool		AllowedToIgnite(void) { return true; }

	void	PlayFlinchGesture(void);

protected:
	// Determines the best type of flinch anim to play.
	virtual Activity GetFlinchActivity(bool bHeavyDamage, bool bGesture);

	// Only move and shoot when attacking
	virtual bool OnBeginMoveAndShoot();
	virtual void OnEndMoveAndShoot();

private:
	bool		PlayerIsCriminal(void);

	virtual void	LostEnemySound(void);
	virtual void	FoundEnemySound(void);
	virtual void	AlertSound(void);
	virtual void	PainSound(const CTakeDamageInfo &info);
	virtual void	DeathSound(const CTakeDamageInfo &info);
	virtual void	IdleSound(void);

	// Burst mode!
	//void		SetBurstMode(bool bEnable);

	int			OnTakeDamage_Alive(const CTakeDamageInfo &info);

	int			GetSoundInterests(void);

	void		BuildScheduleTestBits(void);


	bool		ShouldHitPlayer(const Vector &targetDir, float targetDist);

	void		PrescheduleThink(void);

	void		SetPlayerCriminalDuration(float time);

	void		IncrementPlayerCriminalStatus(void);

	virtual bool		UseAttackSquadSlots()	{ return true; }

	WeaponProficiency_t CalcWeaponProficiency(CBaseCombatWeapon *pWeapon);

	// Inputs
	void InputSetPoliceGoal(inputdata_t &inputdata);

	// Anim event handlers
	void OnAnimEventShove(void);

	// Normal schedule selection 
	int SelectCombatSchedule();
	int SelectScheduleNewEnemy();
	int SelectRangeAttackSchedule();
	int SelectScheduleNoDirectEnemy();
	int SelectScheduleInvestigateSound();

	bool TryToEnterPistolSlot(int nSquadSlot);

	// Handle flinching
	bool IsHeavyDamage(const CTakeDamageInfo &info);

	// How many shots will I fire in a particular amount of time?
	int CountShotsInTime(float flDeltaTime) const;
	float GetTimeForShots(int nShotCount) const;

	// Are we currently firing a burst?
	bool IsCurrentlyFiringBurst() const;

	// Which entity are we actually trying to shoot at?
	CBaseEntity *GetShootTarget();

	// Set up the shot regulator
	int SetupBurstShotRegulator(float flReactionTime);

	// Choose a random vector somewhere between the two specified vectors
	void RandomDirectionBetweenVectors(const Vector &vecStart, const Vector &vecEnd, Vector *pResult);

	// Can me enemy see me? 
	bool CanEnemySeeMe();

	// Deliberately aims as close as possible w/o hitting
	Vector AimCloseToTargetButMiss(CBaseEntity *pTarget, const Vector &shootOrigin);

	// Compute the actual reaction time based on distance + speed modifiers
	//float AimBurstAtReactionTime(float flReactonTime, float flDistToTargetSqr, float flCurrentSpeed);
//int AimBurstAtSetupHitCount(float flDistToTargetSqr, float flCurrentSpeed);

	void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent);

private:

	enum
	{
		COND_BOB_ON_FIRE = BaseClass::NEXT_CONDITION,
		COND_BOB_PLAYER_TOO_CLOSE,
		COND_BOB_PHYSOBJECT_ASSAULT,

	};

	enum
	{
		SCHED_BOB_WALK = BaseClass::NEXT_SCHEDULE,
		SCHED_BOB_WAKE_ANGRY,
		SCHED_BOB_HARASS,
		SCHED_BOB_CHASE_ENEMY,
		SCHED_BOB_ESTABLISH_LINE_OF_FIRE,
		SCHED_BOB_ADVANCE,
		SCHED_BOB_CHARGE,
		SCHED_BOB_BURNING_RUN,
		SCHED_BOB_BURNING_STAND,
		SCHED_BOB_SMG_NORMAL_ATTACK,
		SCHED_BOB_INVESTIGATE_SOUND,
		SCHED_BOB_WARN_TARGET,
		SCHED_BOB_HARASS_TARGET,
		SCHED_BOB_SUPPRESS_TARGET,
		SCHED_BOB_RETURN_FROM_HARASS,
		SCHED_BOB_ALERT_FACE_BESTSOUND,
		SCHED_BOB_RETURN_TO_PRECHASE,
		SCHED_BOB_SMASH_PROP,
	};

	enum
	{
		TASK_BOB_HARASS = BaseClass::NEXT_TASK,
		TASK_BOB_DIE_INSTANTLY,
		TASK_BOB_GET_PATH_TO_BESTSOUND_LOS,
		TASK_BOB_AIM_WEAPON_AT_ENEMY,
		TASK_BOB_SIGNAL_FIRING_TIME,
		TASK_BOB_WAIT_FOR_SENTENCE,
		TASK_BOB_GET_PATH_TO_PRECHASE,
		TASK_BOB_CLEAR_PRECHASE,
	};


private:

	int				m_iPistolClips;		// How many clips the cop has in reserve
	bool			m_fWeaponDrawn;		// Is my weapon drawn? (ready to use)
	int				m_LastShootSlot;
	CRandSimTimer	m_TimeYieldShootSlot;
	CSimpleSimTimer m_BatonSwingTimer;
	CSimpleSimTimer m_NextChargeTimer;

	// All related to burst firing
	float			m_flTaskCompletionTime;

	float			m_flBatonDebounceTime;	// Minimum amount of time before turning the baton off
	float			m_flLastPhysicsFlinchTime;
	float			m_flLastDamageFlinchTime;

	// Sentences
	float			m_flNextPainSoundTime;
	float			m_flNextLostSoundTime;
	bool			m_bPlayerIsNear;

	// Policing state
	bool			m_bPlayerTooClose;
	bool			m_bKeepFacingPlayer;
	float			m_flChasePlayerTime;
	Vector			m_vecPreChaseOrigin;
	float			m_flPreChaseYaw;
	int				m_nNumWarnings;
	int				m_iNumPlayerHits;

	// Outputs
	//COutputEvent	m_OnCupCopped;

	CHandle<CPhysicsProp>	m_hBlockingProp;

	CAI_ActBusyBehavior		m_ActBusyBehavior;
	CAI_StandoffBehavior	m_StandoffBehavior;
	CAI_AssaultBehavior		m_AssaultBehavior;
	CAI_FuncTankBehavior	m_FuncTankBehavior;
	CAI_PolicingBehavior	m_PolicingBehavior;
	CAI_FollowBehavior		m_FollowBehavior;

	int				m_nRecentDamage;
	float			m_flRecentDamageTime;

	// The last hit direction, measured as a yaw relative to our orientation
	float			m_flLastHitYaw;

public:
	DEFINE_CUSTOM_AI;
};

#endif