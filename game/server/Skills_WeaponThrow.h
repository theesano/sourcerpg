
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef SKILL_WEAPON_THROW_H
#define SKILL_WEAPON_THROW_H
#ifdef _WIN32
#pragma once
#endif


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CSpriteTrail;

//-----------------------------------------------------------------------------
// Looks for enemies, bounces a max # of times before it breaks
//-----------------------------------------------------------------------------
class CWeaponThrowingSkills : public CBaseAnimating
{
public:
	DECLARE_CLASS(CWeaponThrowingSkills, CBaseAnimating);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual void Precache();
	virtual void Spawn();
	virtual void UpdateOnRemove();
	void StopLoopingSounds();

	virtual void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent);

	virtual bool OverridePropdata();
	virtual bool CreateVPhysics();


	virtual void ExplodeThink(void);

	void SetRadius(float flRadius);
	void SetSpeed(float flSpeed) { m_flSpeed = flSpeed; }
	float GetSpeed(void) const { return m_flSpeed; }

	bool IsBeingCaptured() const { return m_bCaptureInProgress; }

	// Input
	void InputExplode(inputdata_t &inputdata);
	void InputFadeAndRespawn(inputdata_t &inputdata);
	void InputKill(inputdata_t &inputdata);
	void InputSocketed(inputdata_t &inputdata);

	enum
	{
		STATE_NOT_THROWN = 0,
		STATE_HOLDING,
		STATE_THROWN,
		STATE_LAUNCHED, //by a combine_ball launcher
	};

	void SetState(int state);
	bool IsInField() const;

	void StartWhizSoundThink(void);

	void StartLifetime(float flDuration);
	void ClearLifetime();
	void SetMass(float mass);

	void SetWeaponLaunched(bool state = true) { m_bWeaponLaunched = state; m_bLaunched = state; }
	bool WasWeaponLaunched(void) const { return m_bWeaponLaunched; }

	bool WasFiredByNPC() const { return (GetOwnerEntity() && GetOwnerEntity()->IsNPC()); }

	bool ShouldHitPlayer() const;

	virtual CBasePlayer *HasPhysicsAttacker(float dt);

	float	LastCaptureTime() const;

	unsigned char GetState() const { return m_nState; }

	int  NumBounces(void) const { return m_nBounceCount; }

	void SetMaxBounces(int iBounces)
	{
		m_nMaxBounces = iBounces;
	}

	void SetEmitState(bool bEmit)
	{
		m_bEmit = bEmit;
	}

	void SetOriginalOwner(CBaseEntity *pEntity) { m_hOriginalOwner = pEntity; }
	CBaseEntity *GetOriginalOwner() { return m_hOriginalOwner; }

	void StartSkillsStat(void);
	void StopSkillsStat(void);
	void SkillsStatThink(void);
	float m_flSkillsRange;

private:

	void SetPlayerLaunched(CBasePlayer *pOwner);

	float GetBallHoldDissolveTime();
	float GetBallHoldSoundRampTime();

	// Pow!
	void DoExplosion();

	void StartAnimating(void);
	void StopAnimating(void);

	void SetBallAsLaunched(void);

	void CollisionEventToTrace(int index, gamevcollisionevent_t *pEvent, trace_t &tr);
	bool DissolveEntity(CBaseEntity *pEntity);
	void OnHitEntity(CBaseEntity *pHitEntity, float flSpeed, int index, gamevcollisionevent_t *pEvent);
	void DoImpactEffect(const Vector &preVelocity, int index, gamevcollisionevent_t *pEvent);

	bool IsAttractiveTarget(CBaseEntity *pEntity);

	// Deflects the ball toward enemies in case of a collision 
	void DeflectTowardEnemy(float flSpeed, int index, gamevcollisionevent_t *pEvent);

	// Is this something we can potentially dissolve? 
	bool IsHittableEntity(CBaseEntity *pHitEntity);

	// Sucky. 
	void WhizSoundThink();
	void DieThink();
	void DissolveThink();
	void DissolveRampSoundThink();
	void AnimThink(void);

	void FadeOut(float flDuration);


	bool OutOfBounces(void) const
	{
		return (m_nState == STATE_LAUNCHED && m_nMaxBounces != 0 && m_nBounceCount >= m_nMaxBounces);
	}

private:

	int		m_nBounceCount;
	int		m_nMaxBounces;
	bool	m_bBounceDie;

	float	m_flLastBounceTime;

	bool	m_bFiredGrabbedOutput;
	bool	m_bStruckEntity;		// Has hit an entity already (control accuracy)
	bool	m_bWeaponLaunched;		// Means this was fired from the AR2
	bool	m_bForward;				// Movement direction in ball spawner

	unsigned char m_nState;
	bool	m_bCaptureInProgress;

	float	m_flSpeed;

	CSpriteTrail *m_pGlowTrail;
	CSoundPatch *m_pHoldingSound;

	float	m_flNextDamageTime;
	float	m_flLastCaptureTime;


	EHANDLE m_hOriginalOwner;

	CNetworkVar(bool, m_bEmit);
	CNetworkVar(bool, m_bHeld);
	CNetworkVar(bool, m_bLaunched);
	CNetworkVar(float, m_flRadius);
};


// Creates a throw
CBaseEntity *CreateWpnThrowSkill(const Vector &origin, const Vector &velocity, float radius, float mass, float lifetime, CBaseEntity *pOwner);

// Query function to find out if a physics object is a combine ball (used for collision checks)
//bool UTIL_IsCombineBall(CBaseEntity *pEntity);
//bool UTIL_IsCombineBallDefinite(CBaseEntity *pEntity);
bool UTIL_IsAR2WeaponThrow(CBaseEntity *pEntity);

#endif // SKILL_WEAPON_THROW_H
