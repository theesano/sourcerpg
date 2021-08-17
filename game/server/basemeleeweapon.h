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
// CBaseMeleeWeapon 
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

	virtual	float	GetFireRate(void)								{ return	0.6f; } // Changing it here doesn't do anything, please refer to the weapon files that inherit this class eg. weapon_melee.h/weapon_melee.cpp
	virtual float	GetRange(void)								{ return	32.0f; }
	virtual	float	GetDamageForActivity(Activity hitActivity)	{ return	1.0f; }

	virtual int		CapabilitiesGet(void);
	virtual	int		WeaponMeleeAttack1Condition(float flDot, float flDist);

	virtual bool	IsAICollisionOff() { return m_bIsAICollisionOff; }
	//void			SetAICollisionOffTime(float flDuration);

	bool			m_bIsEnemyInAtkRange;

	bool			m_bIsSkCoolDown;	
	float			m_nSkCoolDownTime;


	bool			m_bIsSkCoolDown2;
	float			m_nSkCoolDownTime2;

	bool			m_bIsSkCoolDown3;
	float			m_nSkCoolDownTime3;

	bool			m_bIsSkCoolDown4;
	float			m_nSkCoolDownTime4;
	float			m_flSkillTrapping_ActiveTime;

	bool			m_bIsSkCoolDown5;
	float			m_nSkCoolDownTime5;

	bool			m_bIsSkCoolDown6;
	float			m_nSkCoolDownTime6;

	bool			m_bIsSkCoolDown7;
	float			m_nSkCoolDownTime7;

	float			m_flSkillAttributeRange;

	float			m_flCooldown;

	int				m_iEnemyHealth;

	bool			m_bNmAttackSPEvade;
	float			m_flNmAttackSPEvadeTimer;
	float			m_flNmAttackSPEvadeTimer_rp;

	bool			m_bAttackSPAir2;
	float			m_flAttackSPAir2Timer;
	float			m_flAttackSPAir2Timer_rp;

	bool			m_bSkillLiftAttack;
	float			m_flSkillLiftAttackDelayTimer;

	bool			m_bIsAICollisionOff;
	//bool			m_flAICollisionOffTime;


protected:
	virtual	void	ImpactEffect(trace_t &trace);

private:
	CHandle<CSpriteTrail>	m_pGlowTrail;

	float			m_flPlayerStats_BaseDamage;
	float			m_flPlayerStats_AttackSpeed;
	float			m_flPlayerStats_CritDamage;
	float			m_bIsCritical;

	float			m_SpeedModActiveTime;
	float			m_nExecutionTime;
	float			m_nSkillHitRefireTime;
	float			m_flNPCFreezeTime;
	float			m_flInAirTime;

	bool			m_bWIsAttack1;
	bool			m_bWIsAttack2;
	bool			m_bWIsAttack3;
	bool			m_bWIsAttack4;
	bool			m_bWIsAttack5;


	void			SkillsHandler(void);
	void			AddKnockback(Vector dir);
	void			AddKnockbackXY(float magnitude,int options);
	void			AddSkillMovementImpulse(float magnitude);

	void			Swing(int bIsSecondary);
	void			InflictNormalAttackDamage(void);
	bool			m_bIsNmAttack;
	bool			m_bIsNmAttack4;
	bool			m_bNmAirAttack;
	float			m_flNmAirAttackDelayTimer;
	float			m_flNmAttackTimer;
	float			m_flNmAttackTimer4;
	float			m_flNmAttackTimer4_rp;

	bool			m_bNPCFreezeAerial; // Keep the NPCs still at their origins and remove all velocities for a short period of time
	float			m_flNPCFreezeAerialTime;

	void			Skill_Evade(void);
	void			Skill_RadialSlash(void);
	void			Skill_RadialSlash_LogicEx(void);
	void			Skill_Grenade(void);
	void			Skill_GrenadeEX(void);
	void			Skill_HealSlash(void);
	void			Skill_HealSlash_LogicEx(void);
	void			Skill_Trapping(void);
	void			Skill_Trapping_LogicEx(void);
	void			Skill_Tornado(void);
	void			Skill_Tornado_LogicEx(void);
	void			Skill_Lift(void);
	void			Skill_Lift_LogicEx(void);
	void			SkillStatNotification(void);

	bool			m_bIsHealSlashAttacking;
	float			m_flHealSlashDelayTimer;

	virtual void			SkillStatNotification_HUD(int messageoption);

	void	GetPlayerAnglesOnce(void);
	void	GetPlayerPosOnce(void);

	float m_flAttackInterval;
	float m_flAnimTime;

	float m_flTotalAttackTime;

	bool ShouldCollide(int collisionGroup, int contentsMask) const;
	virtual Vector GetWeaponAimDirection();

	//Use with GetPlayerAnglesOnce
	Vector dirkb;
	Vector dirkbZ;
	Vector playerPosInAir;
	Activity		ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner);
};

#endif