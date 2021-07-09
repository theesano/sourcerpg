
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
// Looks for enemies, and breaks
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
		STATE_THROWN,
		STATE_LAUNCHED, //by a combine_ball launcher
	};

	void SetState(int state);

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

	// Pow!
	void DoExplosion();

	void StartAnimating(void);
	void StopAnimating(void);

	void SetBallAsLaunched(void);

	void CollisionEventToTrace(int index, gamevcollisionevent_t *pEvent, trace_t &tr);
	void OnHitEntity(CBaseEntity *pHitEntity, float flSpeed, int index, gamevcollisionevent_t *pEvent);
	void DoImpactEffect(const Vector &preVelocity, int index, gamevcollisionevent_t *pEvent);


	// Is this something we can potentially dissolve? 
	bool IsHittableEntity(CBaseEntity *pHitEntity);

	// Sucky. 
	void WhizSoundThink();
	void DieThink();
	void AnimThink(void);

	void FadeOut(float flDuration);



private:

	bool	m_bStruckEntity;		// Has hit an entity already (control accuracy)
	bool	m_bWeaponLaunched;		// Means this was fired from the AR2
	bool	m_bForward;				// Movement direction in ball spawner

	unsigned char m_nState;
	bool	m_bCaptureInProgress;

	float	m_flSpeed;

	float	m_flNextDamageTime;
	float	m_flLastCaptureTime;


	EHANDLE m_hOriginalOwner;

	CNetworkVar(bool, m_bEmit);
	CNetworkVar(bool, m_bHeld);
	CNetworkVar(bool, m_bLaunched);
	CNetworkVar(float, m_flRadius);
};


// Creates a throw
CBaseEntity *CreateThrowable(const Vector &origin, const Vector &velocity, float radius, float mass, float lifetime, CBaseEntity *pOwner);

#endif // SKILL_WEAPON_THROW_H
