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
	//virtual	void	SecondaryAttack(void);

	virtual void	ItemPostFrame(void);

	//Functions to select animation sequences 
	virtual Activity	GetPrimaryAttackActivity(void)	{ return	ACT_VM_HITCENTER; }
	virtual Activity	GetSecondaryAttackActivity(void)	{ return	ACT_VM_HITCENTER2; }

	virtual	float	GetFireRate(void)								{ return	0.6f; }
	virtual float	GetRange(void)								{ return	32.0f; }
	virtual	float	GetDamageForActivity(Activity hitActivity)	{ return	1.0f; }

	virtual int		CapabilitiesGet(void);
	virtual	int		WeaponMeleeAttack1Condition(float flDot, float flDist);

	bool			m_bIsSkCoolDown;
	float			m_nSkCoolDownTime;

	bool			m_bIsSkCoolDown2;
	float			m_nSkCoolDownTime2;

	bool			m_bIsSkCoolDown3;
	float			m_nSkCoolDownTime3;

	bool			m_bIsSkCoolDown4;
	float			m_nSkCoolDownTime4;

	bool			m_bIsSkCoolDown5;
	float			m_nSkCoolDownTime5;

	float			m_flSkillAttributeRange;

	int				m_iEnemyHealth;

protected:
	virtual	void	ImpactEffect(trace_t &trace);

private:
	CHandle<CSpriteTrail>	m_pGlowTrail;

	float			m_SpeedModActiveTime;
	float			m_nExecutionTime;
	float			m_nSkillHitRefireTime;
	float			m_flNPCFreezeTime;

	bool			m_bWIsAttack1;
	bool			m_bWIsAttack2;
	bool			m_bWIsAttack3;

	void			SkillsHandler(void);
	void			AddKnockback(Vector dir);
	void			AddKnockbackXY(float magnitude,int options);
	void			AddSkillMovementImpulse(float magnitude);
	void			Swing(int bIsSecondary);

	void			Skill_Evade(void);
	void			Skill_RadialSlash(void);
	void			Skill_Grenade(void);
	void			Skill_GrenadeEX(void);
	void			Skill_HealSlash(void);
	void			Skill_Trapping(void);
	void			SkillStatNotification(void);

	void	GetPlayerAnglesOnce(void);


	Activity		ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner);
};

#endif