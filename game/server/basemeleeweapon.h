//========= Sample code provided by Valve. ============//
//
// Purpose:	 melee
//				weapons manager. 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "basehlcombatweapon.h"
#include "SpriteTrail.h"
#ifndef BASEMELEEWEAPON_H
#define BASEMELEEWEAPON_H

//=========================================================
// CBaseHLBludgeonWeapon 
//=========================================================
class CBaseMeleeWeapon : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CBaseMeleeWeapon, CBaseHLCombatWeapon);
public:
	CBaseMeleeWeapon();

	DECLARE_SERVERCLASS();

	virtual	void	Spawn(void);
	virtual	void	Precache(void);

	//Attack functions
	virtual	void	PrimaryAttack(void);
	virtual	void	SecondaryAttack(void);

	virtual void	ItemPostFrame(void);

	//Functions to select animation sequences 
	virtual Activity	GetPrimaryAttackActivity(void)	{ return	ACT_VM_HITCENTER; }
	virtual Activity	GetSecondaryAttackActivity(void)	{ return	ACT_VM_HITCENTER2; }

	virtual	float	GetFireRate(void)								{ return	0.2f; }
	virtual float	GetRange(void)								{ return	32.0f; }
	virtual	float	GetDamageForActivity(Activity hitActivity)	{ return	1.0f; }

	virtual int		CapabilitiesGet(void);
	virtual	int		WeaponMeleeAttack1Condition(float flDot, float flDist);

protected:
	virtual	void	ImpactEffect(trace_t &trace);

private:
	CHandle<CSpriteTrail>	m_pGlowTrail;
	bool m_bIsSkCoolDown;
	float m_nSkCoolDownTime;
	bool m_bIsSkCoolDown2;
	float m_nSkCoolDownTime2;
	float m_nExecutionTime;
	float m_nSkillHitRefireTime;
	bool m_bIsAttack1;
	bool m_bIsAttack2;
	bool			ImpactWater(const Vector &start, const Vector &end);
	void			SkillsHandler(void);
	void			AddKnockback(Vector dir);
	void			AddKnockbackXY(float magnitude);
	void			AddSkillMovementImpulse(float magnitude);
	void			Swing(int bIsSecondary);
	void			Swing2(int bIsSecondary);
	void			Skill_Evade(void);
	void			Skill_RadialSlash(void);
	void			Hit(trace_t &traceHit, Activity nHitActivity, bool bIsSecondary);
	Activity		ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner);
};

#endif