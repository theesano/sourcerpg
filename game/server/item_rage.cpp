#include "cbase.h"
#include "items.h"
#include "hl2_player.h"

#include "tier0/memdbgon.h"

#define	PICKUP_MODEL "models/items/dummy.mdl"
#define PICKUP_MIN_HEIGHT 50

//RAGE ENERGY PICKUP

ConVar sk_item_rage_selfremovaltime("sk_item_rage_selfremovaltime", "20");

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
	void	Think();
	void	Remove();
	
	//virtual void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent);
	//virtual bool CreateVPhysics();

	bool	MyTouch(CBasePlayer *pPlayer);

	int m_iRageToGive;
	float m_flSelfRemovalTime;
	CNetworkVar(bool, m_bRespawning);

private:
	Vector MdlTop;

};

LINK_ENTITY_TO_CLASS(item_rage, CRagePickup);

PRECACHE_REGISTER(item_rage);

BEGIN_DATADESC(CRagePickup)
	DEFINE_KEYFIELD(m_iRageToGive, FIELD_INTEGER, "giverage"),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CRagePickup, DT_RagePickup)
	SendPropBool(SENDINFO(m_bRespawning)),
	SendPropExclude("DT_BaseEntity", "m_angRotation"),
END_SEND_TABLE()

CRagePickup::CRagePickup()
{
	if (m_iRageToGive <= 0)
		m_iRageToGive = 12;

}

void CRagePickup::Spawn()
{
	// CItem is designed for Vphys objects, so we need to undo a couple of things its spawn() does
	Vector OriginalLocation = GetAbsOrigin();
	BaseClass::Spawn();
	//CreateVPhysics(); // TODO: 
	VPhysicsDestroyObject();
	SetAbsOrigin(OriginalLocation);

	UseClientSideAnimation();
	SetModel(PICKUP_MODEL);

	SetMoveType(MOVETYPE_NONE);

	// Grab the highest point on the model before we change the bounding box
	MdlTop = GetAbsOrigin();
	MdlTop.z += GetModelPtr()->hull_max().z;

	SetSolid(SOLID_NONE);
	//SetSolid(SOLID_BBOX); //TODO
	CollisionProp()->UseTriggerBounds(true, 8); // Reign in the volume added to the trigger collision box
	Vector OBBSize = Vector(CollisionProp()->OBBSize().Length() / 2); // need to use length as the model will be rotated at 45 degrees on clients
	SetSize(-OBBSize, OBBSize); // Resize the bounding box

	AddEffects(EF_NOSHADOW);

	SetNextThink(gpGlobals->curtime + sk_item_rage_selfremovaltime.GetInt()); //Set time to run Think(); 
}

void CRagePickup::Activate()
{
	BaseClass::Activate();

	// Ensure minimum distance above a standable surfare
	trace_t tr;
	UTIL_TraceLine(MdlTop, MdlTop + Vector(0, 0, -PICKUP_MIN_HEIGHT), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr); // measuring from MdlTop
	if (tr.DidHit())
	{
		SetAbsOrigin(GetAbsOrigin() + (Vector(0, 0, PICKUP_MIN_HEIGHT*(1 - tr.fraction))));
	}
	
}

void CRagePickup::Precache()
{
	PrecacheModel(PICKUP_MODEL);

}

//-----------------------------------------------------------------------------
// Create vphysics !!TODO
//-----------------------------------------------------------------------------
//bool CRagePickup::CreateVPhysics()
//{
//	SetSolid(SOLID_BBOX);
//
//	float flSize = 8.0f;
//
//	SetCollisionBounds(Vector(-flSize, -flSize, -flSize), Vector(flSize, flSize, flSize));
//	objectparams_t params = g_PhysDefaultObjectParams;
//	params.pGameData = static_cast<void *>(this);
//	int nMaterialIndex = physprops->GetSurfaceIndex("metal_bouncy");
//	IPhysicsObject *pPhysicsObject = physenv->CreateSphereObject(flSize, nMaterialIndex, GetAbsOrigin(), GetAbsAngles(), &params, false);
//	if (!pPhysicsObject)
//		return false;
//
//	VPhysicsSetObject(pPhysicsObject);
//	SetMoveType(MOVETYPE_VPHYSICS);
//	pPhysicsObject->Wake();
//
//	pPhysicsObject->SetMass(750.0f);
//	pPhysicsObject->EnableGravity(true);
//
//	float flDamping = 0.0f;
//	float flAngDamping = 0.5f;
//	pPhysicsObject->SetDamping(&flDamping, &flAngDamping);
//	pPhysicsObject->SetInertia(Vector(1e30, 1e30, 1e30));
//
//	return true;
//}

//!!TODO
//void CRagePickup::VPhysicsCollision(int index, gamevcollisionevent_t *pEvent)
//{
//	Vector OriginalLocation = GetAbsOrigin();
//	VPhysicsDestroyObject();
//	SetAbsOrigin(OriginalLocation);
//
//	UseClientSideAnimation();
//	SetMoveType(MOVETYPE_NONE);
//
//	// Grab the highest point on the model before we change the bounding box
//	MdlTop = GetAbsOrigin();
//	MdlTop.z += GetModelPtr()->hull_max().z;
//
//	SetSolid(SOLID_NONE);
//
//}

void CRagePickup::Think()
{
	BaseClass::Think();
	Remove();
	//SetNextThink(gpGlobals->curtime + 1); // Think again in 1 second

}

// Called from CItem::ItemTouch()
bool CRagePickup::MyTouch(CBasePlayer *pPlayer)
{		
	
	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player *>(pPlayer);
	return (pHL2Player && pHL2Player->ApplyRagePower());

}

void CRagePickup::Remove()
{
	UTIL_Remove(this);
	//Msg("My World Position Is %f %f %f.\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
	//Msg("Removed \n");

}


