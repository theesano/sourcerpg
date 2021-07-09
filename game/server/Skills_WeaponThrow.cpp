#include "cbase.h"
#include "skills_weaponthrow.h"
#include "saverestore_utlvector.h"
#include "hl2_shareddefs.h"
#include "materialsystem/imaterial.h"
#include "beam_flags.h"
#include "physics_prop_ragdoll.h"
#include "soundent.h"
#include "soundenvelope.h"
#include "te_effect_dispatch.h"
#include "ai_basenpc.h"
#include "ai_condition.h"
#include "hl2_player.h"
#include "physics_collisionevent.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define WPNTHROW_LIFETIME	4.0f	// Seconds

#define	MAX_WPNTHROW_RADIUS	256

ConVar	sk_npc_dmg_wpnthrow("sk_npc_dmg_wpnthrow", "15", FCVAR_REPLICATED);
ConVar  lilyss_skill2_throwmodel("lilyss_skill2_throwmodel", "models/weapons/melee/alt/guideshape.mdl");

//-----------------------------------------------------------------------------
// Context think
//-----------------------------------------------------------------------------
static const char *s_pWhizThinkContext = "WhizThinkContext";
static const char *s_pHoldDissolveContext = "HoldDissolveContext";
static const char *s_pExplodeTimerContext = "ExplodeTimerContext";
static const char *s_pAnimThinkContext = "AnimThinkContext";
static const char *s_pCaptureContext = "CaptureContext";
static const char *s_pRemoveContext = "RemoveContext";
static const char *s_pSkillsStatsThinkContext = "AnimThinkContext";


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : radius - 
// Output : CBaseEntity
//-----------------------------------------------------------------------------
CBaseEntity *CreateThrowable(const Vector &origin, const Vector &velocity, float radius, float mass, float lifetime, CBaseEntity *pOwner)
{
	CWeaponThrowingSkills *pSkWpnThrow = static_cast<CWeaponThrowingSkills*>(CreateEntityByName("skills_weaponthrow"));
	pSkWpnThrow->SetRadius(radius);

	pSkWpnThrow->SetAbsOrigin(origin);
	pSkWpnThrow->SetOwnerEntity(pOwner);
	pSkWpnThrow->SetOriginalOwner(pOwner);

	pSkWpnThrow->SetAbsVelocity(velocity);
	pSkWpnThrow->Spawn();

	pSkWpnThrow->SetState(CWeaponThrowingSkills::STATE_THROWN);
	pSkWpnThrow->SetSpeed(velocity.Length());

	//! CHANGE
	pSkWpnThrow->EmitSound("Weapon_Melee.SPECIAL1");

	PhysSetGameFlags(pSkWpnThrow->VPhysicsGetObject(), FVPHYSICS_WAS_THROWN);

	pSkWpnThrow->StartWhizSoundThink();

	pSkWpnThrow->SetMass(mass);
	pSkWpnThrow->StartLifetime(lifetime);
	pSkWpnThrow->SetWeaponLaunched(true);

	return pSkWpnThrow;
}

//-----------------------------------------------------------------------------
// Purpose: Allows game to know if the physics object should kill allies or not
//-----------------------------------------------------------------------------
CBasePlayer *CWeaponThrowingSkills::HasPhysicsAttacker(float dt)
{
	// Must have an owner
	if (GetOwnerEntity() == NULL)
		return NULL;

	// Must be a player
	if (GetOwnerEntity()->IsPlayer() == false)
		return NULL;

	// We don't care about the time passed in
	return static_cast<CBasePlayer *>(GetOwnerEntity());
}

//-----------------------------------------------------------------------------
// Purpose: Determines whether a physics object is an AR2 combine ball or not
// Input  : *pEntity - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool UTIL_IsAR2WeaponThrow(CBaseEntity *pEntity)
{
	// Must be the correct collision group
	if (pEntity->GetCollisionGroup() != HL2COLLISION_GROUP_COMBINE_BALL)
		return false;

	CWeaponThrowingSkills *pSkWpnThrow = dynamic_cast<CWeaponThrowingSkills *>(pEntity);

	if (pSkWpnThrow && pSkWpnThrow->WasWeaponLaunched())
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Uses a deeper casting check to determine if pEntity is a combine
//			ball. This function exists because the normal (much faster) check
//			in UTIL_IsCombineBall() can never identify a combine ball held by
//			the physcannon because the physcannon changes the held entity's
//			collision group.
// Input  : *pEntity - Entity to check 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
//bool UTIL_IsCombineBallDefinite(CBaseEntity *pEntity)
//{
//	CWeaponThrowingSkills *pSkWpnThrow = dynamic_cast<CWeaponThrowingSkills *>(pEntity);
//
//	return pSkWpnThrow != NULL;
//}

//-----------------------------------------------------------------------------
//
// Spawns combine balls
//
//-----------------------------------------------------------------------------
#define SF_SPAWNER_START_DISABLED 0x1000
#define SF_SPAWNER_POWER_SUPPLY 0x2000



//-----------------------------------------------------------------------------
// Implementation of CWeaponThrowingSkills
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS(skills_weaponthrow, CWeaponThrowingSkills);

//-----------------------------------------------------------------------------
// Save/load: 
//-----------------------------------------------------------------------------
BEGIN_DATADESC(CWeaponThrowingSkills)

DEFINE_FIELD(m_flRadius, FIELD_FLOAT),
DEFINE_FIELD(m_nState, FIELD_CHARACTER),
DEFINE_FIELD(m_bEmit, FIELD_BOOLEAN),
DEFINE_FIELD(m_bHeld, FIELD_BOOLEAN),
DEFINE_FIELD(m_bLaunched, FIELD_BOOLEAN),
DEFINE_FIELD(m_bStruckEntity, FIELD_BOOLEAN),
DEFINE_FIELD(m_bWeaponLaunched, FIELD_BOOLEAN),
DEFINE_FIELD(m_bForward, FIELD_BOOLEAN),
DEFINE_FIELD(m_flSpeed, FIELD_FLOAT),

DEFINE_FIELD(m_flNextDamageTime, FIELD_TIME),
DEFINE_FIELD(m_flLastCaptureTime, FIELD_TIME),
DEFINE_FIELD(m_bCaptureInProgress, FIELD_BOOLEAN),


DEFINE_THINKFUNC(ExplodeThink),
DEFINE_THINKFUNC(WhizSoundThink),
DEFINE_THINKFUNC(DieThink),
DEFINE_THINKFUNC(AnimThink),

DEFINE_INPUTFUNC(FIELD_VOID, "Explode", InputExplode),
DEFINE_INPUTFUNC(FIELD_VOID, "FadeAndRespawn", InputFadeAndRespawn),
DEFINE_INPUTFUNC(FIELD_VOID, "Kill", InputKill),
DEFINE_INPUTFUNC(FIELD_VOID, "Socketed", InputSocketed),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponThrowingSkills, DT_WeaponThrowingSkills)
SendPropBool(SENDINFO(m_bEmit)),
SendPropFloat(SENDINFO(m_flRadius), 0, SPROP_NOSCALE),
SendPropBool(SENDINFO(m_bHeld)),
SendPropBool(SENDINFO(m_bLaunched)),
END_SEND_TABLE()


//-----------------------------------------------------------------------------
// Precache 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::Precache(void)
{
	//NOTENOTE: We don't call into the base class because it chains multiple 
	//			precaches we don't need to incur
	PrecacheModel(lilyss_skill2_throwmodel.GetString());
	
	PrecacheScriptSound("NPC_CombineBall.Launch");
	PrecacheScriptSound("NPC_CombineBall.KillImpact");

	if (hl2_episodic.GetBool())
	{
		PrecacheScriptSound("NPC_CombineBall_Episodic.Explosion");
		PrecacheScriptSound("NPC_CombineBall_Episodic.WhizFlyby");
		PrecacheScriptSound("NPC_CombineBall_Episodic.Impact");
	}
	else
	{
		PrecacheScriptSound("NPC_CombineBall.Explosion");
		PrecacheScriptSound("NPC_CombineBall.WhizFlyby");
		PrecacheScriptSound("NPC_CombineBall.Impact");
	}

	PrecacheScriptSound("NPC_CombineBall.HoldingInPhysCannon");
	PrecacheParticleSystem("aoehint4");
}


//-----------------------------------------------------------------------------
// Spherical vphysics
//-----------------------------------------------------------------------------
bool CWeaponThrowingSkills::OverridePropdata()
{
	return true;
}


//-----------------------------------------------------------------------------
// Spherical vphysics
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::SetState(int state)
{
	if (m_nState != state)
	{
		if (m_nState == STATE_NOT_THROWN)
		{
			m_flLastCaptureTime = gpGlobals->curtime;
		}

		m_nState = state;
	}
}

//-----------------------------------------------------------------------------
// Sets the radius
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::SetRadius(float flRadius)
{
	m_flRadius = clamp(flRadius, 1, MAX_WPNTHROW_RADIUS);
}

//-----------------------------------------------------------------------------
// Create vphysics
//-----------------------------------------------------------------------------
bool CWeaponThrowingSkills::CreateVPhysics()
{
	SetSolid(SOLID_BBOX);

	float flSize = m_flRadius;

	SetCollisionBounds(Vector(-flSize, -flSize, -flSize), Vector(flSize, flSize, flSize));
	objectparams_t params = g_PhysDefaultObjectParams;
	params.pGameData = static_cast<void *>(this);
	int nMaterialIndex = physprops->GetSurfaceIndex("metal_bouncy");
	IPhysicsObject *pPhysicsObject = physenv->CreateSphereObject(flSize, nMaterialIndex, GetAbsOrigin(), GetAbsAngles(), &params, false);
	if (!pPhysicsObject)
		return false;

	VPhysicsSetObject(pPhysicsObject);
	SetMoveType(MOVETYPE_VPHYSICS);
	pPhysicsObject->Wake();

	pPhysicsObject->SetMass(750.0f);
	pPhysicsObject->EnableGravity(false);
	pPhysicsObject->EnableDrag(false);

	float flDamping = 0.0f;
	float flAngDamping = 0.5f;
	pPhysicsObject->SetDamping(&flDamping, &flAngDamping);
	pPhysicsObject->SetInertia(Vector(1e30, 1e30, 1e30));



	if (WasFiredByNPC())
	{
		//PhysSetGameFlags(pPhysicsObject, FVPHYSICS_NO_NPC_IMPACT_DMG);
	}


	return true;
}


//-----------------------------------------------------------------------------
// Spawn: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::Spawn(void)
{
	Precache();

	BaseClass::Spawn();

	SetModel(lilyss_skill2_throwmodel.GetString());

	//if (ShouldHitPlayer())
	//{
	//	// This allows the combine ball to hit the player.
	//	SetCollisionGroup(HL2COLLISION_GROUP_COMBINE_BALL_NPC);
	//}
	//else
	//{
	//	SetCollisionGroup(HL2COLLISION_GROUP_COMBINE_BALL);
	//}

	CreateVPhysics();

	Vector vecAbsVelocity = GetAbsVelocity();
	VPhysicsGetObject()->SetVelocity(&vecAbsVelocity, NULL);

	m_nState = STATE_NOT_THROWN;
	m_bForward = true;
	m_bCaptureInProgress = false;

	// No shadow!
	//AddEffects(EF_NOSHADOW);
	//AddEffects(EF_NODRAW);

	m_bEmit = true;
	m_bHeld = false;
	m_bLaunched = false;
	m_bStruckEntity = false;
	m_bWeaponLaunched = false;

	m_flNextDamageTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::StartAnimating(void)
{
	// Start our animation cycle. Use the random to avoid everything thinking the same frame
	SetContextThink(&CWeaponThrowingSkills::AnimThink, gpGlobals->curtime + random->RandomFloat(0.0f, 0.1f), s_pAnimThinkContext);

	int nSequence = LookupSequence("idle");

	SetCycle(0);
	m_flAnimTime = gpGlobals->curtime;
	ResetSequence(nSequence);
	ResetClientsideFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::StopAnimating(void)
{
	SetContextThink(NULL, gpGlobals->curtime, s_pAnimThinkContext);
}

//-----------------------------------------------------------------------------
// Purpose: Starts the lifetime countdown on the ball
// Input  : flDuration - number of seconds to live before exploding
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::StartLifetime(float flDuration)
{
	SetContextThink(&CWeaponThrowingSkills::ExplodeThink, gpGlobals->curtime + flDuration, s_pExplodeTimerContext);
}

//-----------------------------------------------------------------------------
// Purpose: Stops the lifetime on the ball from expiring
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::ClearLifetime(void)
{
	// Prevent it from exploding
	SetContextThink(NULL, gpGlobals->curtime, s_pExplodeTimerContext);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : mass - 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::SetMass(float mass)
{
	IPhysicsObject *pObj = VPhysicsGetObject();

	if (pObj != NULL)
	{
		pObj->SetMass(mass);
		pObj->SetInertia(Vector(500, 500, 500));
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponThrowingSkills::ShouldHitPlayer() const
{
	if (GetOwnerEntity())
	{
		CAI_BaseNPC *pNPC = GetOwnerEntity()->MyNPCPointer();
		if (pNPC && !pNPC->IsPlayerAlly())
		{
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::InputKill(inputdata_t &inputdata)
{
	// tell owner ( if any ) that we're dead.This is mostly for NPCMaker functionality.
	CBaseEntity *pOwner = GetOwnerEntity();
	if (pOwner)
	{
		pOwner->DeathNotice(this);
		SetOwnerEntity(NULL);
	}

	UTIL_Remove(this);

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::InputSocketed(inputdata_t &inputdata)
{
	// tell owner ( if any ) that we're dead.This is mostly for NPCMaker functionality.
	CBaseEntity *pOwner = GetOwnerEntity();
	if (pOwner)
	{
		pOwner->DeathNotice(this);
		SetOwnerEntity(NULL);
	}

	// if our owner is a player, tell them we were socketed
	CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(pOwner);
	if (pPlayer)
	{
		//pPlayer->CombineBallSocketed(this); !!!!
	}

	UTIL_Remove(this);

}

//-----------------------------------------------------------------------------
// Cleanup. 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::UpdateOnRemove()
{

	//Sigh... this is the only place where I can get a message after the ball is done dissolving.
	if (hl2_episodic.GetBool())
	{
		if (IsDissolving())
		{
		}
	}

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::ExplodeThink(void)
{
	DoExplosion();
}


//-----------------------------------------------------------------------------
// Fade out. 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::DieThink()
{
	UTIL_Remove(this);
}


//-----------------------------------------------------------------------------
// Fade out. 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::FadeOut(float flDuration)
{
	AddSolidFlags(FSOLID_NOT_SOLID);

	SetThink(&CWeaponThrowingSkills::DieThink);
	SetNextThink(gpGlobals->curtime + flDuration);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::StartWhizSoundThink(void)
{
	SetContextThink(&CWeaponThrowingSkills::WhizSoundThink, gpGlobals->curtime + 2.0f * TICK_INTERVAL, s_pWhizThinkContext);
}

//-----------------------------------------------------------------------------
// Danger sounds. 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::WhizSoundThink()
{
	Vector vecPosition, vecVelocity;
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if (pPhysicsObject == NULL)
	{
		//NOTENOTE: We should always have been created at this point
		Assert(0);
		SetContextThink(&CWeaponThrowingSkills::WhizSoundThink, gpGlobals->curtime + 2.0f * TICK_INTERVAL, s_pWhizThinkContext);
		return;
	}

	pPhysicsObject->GetPosition(&vecPosition, NULL);
	pPhysicsObject->GetVelocity(&vecVelocity, NULL);

	if (gpGlobals->maxClients == 1)
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		if (pPlayer)
		{
			Vector vecDelta;
			VectorSubtract(pPlayer->GetAbsOrigin(), vecPosition, vecDelta);
			VectorNormalize(vecDelta);
			if (DotProduct(vecDelta, vecVelocity) > 0.5f)
			{
				Vector vecEndPoint;
				VectorMA(vecPosition, 2.0f * TICK_INTERVAL, vecVelocity, vecEndPoint);
				float flDist = CalcDistanceToLineSegment(pPlayer->GetAbsOrigin(), vecPosition, vecEndPoint);
				if (flDist < 200.0f)
				{
					CPASAttenuationFilter filter(vecPosition, ATTN_NORM);

					EmitSound_t ep;
					ep.m_nChannel = CHAN_STATIC;
					if (hl2_episodic.GetBool())
					{
						ep.m_pSoundName = "NPC_CombineBall_Episodic.WhizFlyby";
					}
					else
					{
						ep.m_pSoundName = "NPC_CombineBall.WhizFlyby";
					}
					ep.m_flVolume = 1.0f;
					ep.m_SoundLevel = SNDLVL_NORM;

					EmitSound(filter, entindex(), ep);

					SetContextThink(&CWeaponThrowingSkills::WhizSoundThink, gpGlobals->curtime + 0.5f, s_pWhizThinkContext);
					return;
				}
			}
		}
	}

	SetContextThink(&CWeaponThrowingSkills::WhizSoundThink, gpGlobals->curtime + 2.0f * TICK_INTERVAL, s_pWhizThinkContext);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::SetBallAsLaunched(void)
{
	// Give the ball a duration
	StartLifetime(WPNTHROW_LIFETIME);

	m_bHeld = false;
	m_bLaunched = true;
	SetState(STATE_THROWN);

	VPhysicsGetObject()->SetMass(750.0f);
	VPhysicsGetObject()->SetInertia(Vector(1e30, 1e30, 1e30));

	EmitSound("NPC_CombineBall.Launch");

	WhizSoundThink();
}


//-----------------------------------------------------------------------------
// Purpose: Reset the ball to be deadly to NPCs after we've picked it up
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::SetPlayerLaunched(CBasePlayer *pOwner)
{
	// Now we own this ball
	SetOwnerEntity(pOwner);
	SetWeaponLaunched(false);

	if (VPhysicsGetObject())
	{
		PhysClearGameFlags(VPhysicsGetObject(), FVPHYSICS_NO_NPC_IMPACT_DMG);
	}
}



//------------------------------------------------------------------------------
// Pow!
//------------------------------------------------------------------------------
void CWeaponThrowingSkills::DoExplosion()
{
	// don't do this twice
	if (GetMoveType() == MOVETYPE_NONE)
		return;

	if (PhysIsInCallback())
	{
		g_PostSimulationQueue.QueueCall(this, &CWeaponThrowingSkills::DoExplosion);
		return;
	}

	//Shockring
	CBroadcastRecipientFilter filter2;
	filter2.AddAllPlayers();

	
	if (hl2_episodic.GetBool())
	{
		CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_EXPLOSION, WorldSpaceCenter(), 180.0f, 0.25, this);
	}

	// Turn us off and wait because we need our trails to finish up properly
	SetAbsVelocity(vec3_origin);
	SetMoveType(MOVETYPE_NONE);
	AddSolidFlags(FSOLID_NOT_SOLID);

	m_bEmit = false;


	if (!m_bStruckEntity && hl2_episodic.GetBool() && GetOwnerEntity() != NULL)
	{
		// Notify the player proxy that this combine ball missed so that it can fire an output.
		CHL2_Player *pPlayer = dynamic_cast<CHL2_Player *>(GetOwnerEntity());
		if (pPlayer)
		{
			pPlayer->MissedAR2AltFire();
		}
	}

	SetContextThink(&CWeaponThrowingSkills::SUB_Remove, gpGlobals->curtime + 0.5f, s_pRemoveContext);
}

//-----------------------------------------------------------------------------
// Enable/disable
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::InputExplode(inputdata_t &inputdata)
{
	DoExplosion();
}

//-----------------------------------------------------------------------------
// Enable/disable
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::InputFadeAndRespawn(inputdata_t &inputdata)
{
	FadeOut(0.1f);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::CollisionEventToTrace(int index, gamevcollisionevent_t *pEvent, trace_t &tr)
{
	UTIL_ClearTrace(tr);
	pEvent->pInternalData->GetSurfaceNormal(tr.plane.normal);
	pEvent->pInternalData->GetContactPoint(tr.endpos);
	tr.plane.dist = DotProduct(tr.plane.normal, tr.endpos);
	VectorMA(tr.endpos, -1.0f, pEvent->preVelocity[index], tr.startpos);
	tr.m_pEnt = pEvent->pEntities[!index];
	tr.fraction = 0.01f;	// spoof!
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::OnHitEntity(CBaseEntity *pHitEntity, float flSpeed, int index, gamevcollisionevent_t *pEvent)
{
	// Detonate on the strider + the bone followers in the strider
	if (FClassnameIs(pHitEntity, "npc_strider") ||
		(pHitEntity->GetOwnerEntity() && FClassnameIs(pHitEntity->GetOwnerEntity(), "npc_strider")))
	{
		DoExplosion();
		return;
	}
	
	//HACK: Stop the weapon from moving
	float flFinalVelocity = 0;
	Vector vecFinalVelocity;

	// Don't slow down when hitting other entities.
	vecFinalVelocity = pEvent->postVelocity[index];
	VectorNormalize(vecFinalVelocity);
	//vecFinalVelocity *= GetSpeed();
	vecFinalVelocity *= flFinalVelocity;
	
	PhysCallbackSetVelocity(pEvent->pObjects[index], vecFinalVelocity);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::DoImpactEffect(const Vector &preVelocity, int index, gamevcollisionevent_t *pEvent)
{
	// Do that crazy impact effect!
	trace_t tr;
	CollisionEventToTrace(!index, pEvent, tr);

	CBaseEntity *pTraceEntity = pEvent->pEntities[index];
	UTIL_TraceLine(tr.startpos - preVelocity * 2.0f, tr.startpos + preVelocity * 2.0f, MASK_SOLID, pTraceEntity, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0f)
	{
		// See if we hit the sky
		if (tr.surface.flags & SURF_SKY)
		{
			DoExplosion();
			return;
		}

		// Send the effect over
		CEffectData	data;

		data.m_flRadius = 16;
		data.m_vNormal = tr.plane.normal;
		data.m_vOrigin = tr.endpos + tr.plane.normal * 1.0f;

		DispatchEffect("cball_bounce", data);
	}

	if (hl2_episodic.GetBool())
	{
		EmitSound("NPC_CombineBall_Episodic.Impact");
	}
	else
	{
		EmitSound("NPC_CombineBall.Impact");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponThrowingSkills::IsHittableEntity(CBaseEntity *pHitEntity)
{
	if (pHitEntity->IsWorld())
		return false;

	if (pHitEntity->GetMoveType() == MOVETYPE_PUSH)
	{
		if (pHitEntity->GetOwnerEntity() && FClassnameIs(pHitEntity->GetOwnerEntity(), "npc_strider"))
		{
			// The Strider's Bone Followers are MOVETYPE_PUSH, and we want the combine ball to hit these.
			return true;
		}

		// If the entity we hit can take damage, we're good
		if (pHitEntity->m_takedamage == DAMAGE_YES)
			return true;

		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::VPhysicsCollision(int index, gamevcollisionevent_t *pEvent)
{
	Vector preVelocity = pEvent->preVelocity[index];
	//float flSpeed = VectorNormalize(preVelocity);
	float flSpeed = 0;

	
		//const surfacedata_t *pHit = physprops->GetSurfaceData(pEvent->surfaceProps[!index]);

		//if (pHit->game.material != CHAR_TEX_FLESH || !hl2_episodic.GetBool())
	//	{
			CBaseEntity *pHitEntity = pEvent->pEntities[!index];
			if (pHitEntity && IsHittableEntity(pHitEntity))
			{
				OnHitEntity(pHitEntity, flSpeed, index, pEvent);
				
			}

			// Remove self without affecting the object that was hit. (Unless it was flesh)
			//NotifySpawnerOfRemoval();
			//PhysCallbackRemove(this->NetworkProp());
			
			// disable dissolve damage so we don't kill off the player when he's the one we hit
			
		//}
			StartSkillsStat();
			PhysClearGameFlags(VPhysicsGetObject(), FVPHYSICS_DMG_DISSOLVE);
			PhysSetGameFlags(VPhysicsGetObject(), FVPHYSICS_CONSTRAINT_STATIC);
			VPhysicsGetObject()->EnableMotion(false);
			PhysCallbackSetVelocity(pEvent->pObjects[index], vec3_origin);
			return;

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::AnimThink(void)
{
	StudioFrameAdvance();
	SetContextThink(&CWeaponThrowingSkills::AnimThink, gpGlobals->curtime + 0.1f, s_pAnimThinkContext);
}

float flKnockbackVelocity = 128.0f*1.5;
Vector dir;
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::StartSkillsStat(void)
{
	AngleVectors(UTIL_GetLocalPlayer()->GetAbsAngles(), &dir);
	dir.z = 0;
	VectorNormalize(dir);
	m_flSkillsRange = 128.0f;
	// Start our animation cycle. Use the random to avoid everything thinking the same frame
	SetContextThink(&CWeaponThrowingSkills::SkillsStatThink, gpGlobals->curtime + random->RandomFloat(0.0f, 0.1f), s_pSkillsStatsThinkContext);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::StopSkillsStat(void)
{
	SetContextThink(NULL, gpGlobals->curtime, s_pSkillsStatsThinkContext);

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponThrowingSkills::SkillsStatThink(void)
{
	
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();
	string_t iszNPCName = AllocPooledString("npc_metropolice");
	Vector playernpcdist;
	Vector WpnThrowdist;

	for (int i = 0; i < nAIs; i++)
	{
		//if (ppAIs[i]->m_iClassname == iszNPCName)
		if (ppAIs[i])
		{
			CBaseEntity *pEntity = NULL;

			while ((pEntity = gEntList.FindEntityByClassname(pEntity, "skills_weaponthrow")) != NULL)
			{
				CWeaponThrowingSkills *pSkWpnThrow = dynamic_cast<CWeaponThrowingSkills *>(pEntity);
				if (pSkWpnThrow->IsAlive())
				{
					WpnThrowdist.x = abs(pSkWpnThrow->GetAbsOrigin().x - ppAIs[i]->GetAbsOrigin().x);
					WpnThrowdist.y = abs(pSkWpnThrow->GetAbsOrigin().y - ppAIs[i]->GetAbsOrigin().y);
				}
			}

			if (WpnThrowdist.x <= m_flSkillsRange && WpnThrowdist.y <= m_flSkillsRange)
			{
				ppAIs[i]->SetRenderMode(kRenderTransColor);
				ppAIs[i]->SetRenderColor(128, 128, 128, 128);
				ppAIs[i]->ApplyAbsVelocityImpulse(dir*flKnockbackVelocity);
				
			}
		}
	}
	CTakeDamageInfo infosk1(this, GetOwnerEntity(), GetAbsVelocity(), GetAbsOrigin(), sk_npc_dmg_wpnthrow.GetFloat(), DMG_SLASH);
	RadiusDamage(infosk1, GetAbsOrigin(), m_flSkillsRange, CLASS_NONE, UTIL_GetLocalPlayer());
	DispatchParticleEffect("aoehint4", GetAbsOrigin(), vec3_angle);


	SetContextThink(&CWeaponThrowingSkills::SkillsStatThink, gpGlobals->curtime + 0.1f, s_pSkillsStatsThinkContext);

}
