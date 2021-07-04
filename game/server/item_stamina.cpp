//=============================================================================//
//
// Purpose: Handling for the suit aux power.
//
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "gamerules.h"
#include "items.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CItemStamina : public CItem
{
public:
	DECLARE_CLASS(CItemStamina, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/items/battery.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/items/battery.mdl");

		PrecacheScriptSound("ItemBattery.Touch");

	}

	bool MyTouch(CBasePlayer *pPlayer) //Gives player Stamina
	{
		CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player *>(pPlayer);
		return (pHL2Player && pHL2Player->ApplyStamina());
	}
};

LINK_ENTITY_TO_CLASS(item_stamina, CItemStamina);
PRECACHE_REGISTER(item_stamina);
