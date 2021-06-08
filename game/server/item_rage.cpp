#include "cbase.h"
#include "items.h"
#include "basemeleeweapon.h"

#include "tier0/memdbgon.h"

#define PICKUP_DECAL  "decals/item_base"
#define	PICKUP_MODEL "models/items/healthkit.mdl"
#define PICKUP_MIN_HEIGHT 50
int PickupDecalIndex;  //set by CRagePickup::Precache()

#define SF_SUPPRESS_PICKUP_DECAL 0x00000002

//RAGE ENERGY PICKUP

class CRagePickup : public CItem
{
	DECLARE_CLASS(CRagePickup, CItem);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
public:

	CRagePickup();

	void	Spawn();
	void	Activate();
	void	Precache();

	bool	MyTouch(CBasePlayer *pPlayer);

	CBaseEntity*	Respawn();
	void			Materialize();

	int m_iHealthToGive;
	float m_flRageToGive;
	float m_fRespawnTime;
	CNetworkVar(bool, m_bRespawning);

private:
	Vector MdlTop;

};

LINK_ENTITY_TO_CLASS(item_rage, CRagePickup);

PRECACHE_REGISTER(item_rage);

BEGIN_DATADESC(CRagePickup)
	DEFINE_KEYFIELD(m_iHealthToGive, FIELD_INTEGER, "givehealth"),
	DEFINE_KEYFIELD(m_flRageToGive, FIELD_FLOAT, "giverage"),
	DEFINE_KEYFIELD(m_fRespawnTime, FIELD_FLOAT, "respawntime"),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CRagePickup, DT_RagePickup)
	SendPropBool(SENDINFO(m_bRespawning)),
	SendPropExclude("DT_BaseEntity", "m_angRotation"),
END_SEND_TABLE()

CRagePickup::CRagePickup()
{
	if (m_iHealthToGive <= 0)
		m_iHealthToGive = 25;

	if (m_flRageToGive <= 0)
		m_flRageToGive = 12;

	if (m_fRespawnTime <= 0)
		m_fRespawnTime = 20;
}

void CRagePickup::Spawn()
{
	// CItem is designed for Vphys objects, so we need to undo a couple of things its spawn() does
	Vector OriginalLocation = GetAbsOrigin();
	BaseClass::Spawn();
	VPhysicsDestroyObject();
	SetAbsOrigin(OriginalLocation);

	UseClientSideAnimation();
	SetModel(PICKUP_MODEL);

	SetMoveType(MOVETYPE_NONE);

	// Grab the highest point on the model before we change the bounding box
	MdlTop = GetAbsOrigin();
	MdlTop.z += GetModelPtr()->hull_max().z;

	SetSolid(SOLID_NONE);
	CollisionProp()->UseTriggerBounds(true, 6); // Reign in the volume added to the trigger collision box
	Vector OBBSize = Vector(CollisionProp()->OBBSize().Length() / 2); // need to use length as the model will be rotated at 45 degrees on clients
	SetSize(-OBBSize, OBBSize); // Resize the bounding box

	AddEffects(EF_NOSHADOW);
}

void CRagePickup::Activate()
{
	BaseClass::Activate();

	// Ensure minimum distance above a standable surfare
	trace_t tr;
	UTIL_TraceLine(MdlTop, MdlTop + Vector(0, 0, -PICKUP_MIN_HEIGHT), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr); // measuring from MdlTop
	if (tr.DidHit())
	{
		if (!HasSpawnFlags(SF_SUPPRESS_PICKUP_DECAL))
			engine->StaticDecal(tr.endpos, PickupDecalIndex, 0, 0, false); // mark the location of the pickup
		SetAbsOrigin(GetAbsOrigin() + (Vector(0, 0, PICKUP_MIN_HEIGHT*(1 - tr.fraction))));
	}

}

void CRagePickup::Precache()
{
	PrecacheModel(PICKUP_MODEL);
	PrecacheScriptSound("HealthKit.Touch");
	PrecacheScriptSound("AlyxEmp.Charge");
	PickupDecalIndex = UTIL_PrecacheDecal(PICKUP_DECAL, true);

}

// Called from CItem::ItemTouch()
bool CRagePickup::MyTouch(CBasePlayer *pPlayer)
{		
	//if (pPlayer->GetActiveWeapon()->GetName() == "weapon_melee")
	//{
	//	CSingleUserRecipientFilter PlayerFilter(pPlayer);
	//	PlayerFilter.MakeReliable();

	//	UserMessageBegin(PlayerFilter, "ItemPickup");
	//	WRITE_STRING(GetClassname());
	//	MessageEnd();
	//	EmitSound(PlayerFilter, pPlayer->entindex(), "HealthKit.Touch"); // this should be done by the HUD really

	//	//Respawn();
	//	return true;
	//}

	//return false;
	
	CBaseMeleeWeapon *pWeapon = dynamic_cast<CBaseMeleeWeapon *>(pPlayer->GetActiveWeapon());
		return (pWeapon && pWeapon->ApplyRagePower());

}

//Disappear
CBaseEntity* CRagePickup::Respawn()
{
	SetTouch(NULL);
	m_bRespawning = true;

	SetThink(&CRagePickup::Materialize);
	SetNextThink(gpGlobals->curtime + m_fRespawnTime);

	return this;

}

//Reappear
void CRagePickup::Materialize()
{
	EmitSound("AlyxEmp.Charge");
	m_bRespawning = false;
	SetTouch(&CItem::ItemTouch);

}