#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_gameobjective.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "c_basehlplayer.h"
#include "usermessages.h"


using namespace vgui;

#include "tier0/memdbgon.h" 

DECLARE_HUDELEMENT(CHudGameObjective);
DECLARE_HUD_MESSAGE(CHudGameObjective, HudGameObjShouldDraw);
DECLARE_HUD_MESSAGE(CHudGameObjective, HudGameObjCount);
DECLARE_HUD_MESSAGE(CHudGameObjective, HudGameObjTargetCount);
DECLARE_HUD_MESSAGE(CHudGameObjective, HudGameObjOnHitTarget);
# define HULL_INIT -1


//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudGameObjective::CHudGameObjective(const char * pElementName) :
CHudElement(pElementName), BaseClass(NULL, "HUDGameObjective")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudGameObjective::Init()
{
	HOOK_HUD_MESSAGE(CHudGameObjective, HudGameObjShouldDraw);
	HOOK_HUD_MESSAGE(CHudGameObjective, HudGameObjCount);
	HOOK_HUD_MESSAGE(CHudGameObjective, HudGameObjTargetCount);
	HOOK_HUD_MESSAGE(CHudGameObjective, HudGameObjOnHitTarget);
	//m_bDrawHUD = false;
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudGameObjective::Reset(void)
{
	
}

bool CHudGameObjective::ShouldDraw(void)
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	//return (bNeedsDraw && CHudElement::ShouldDraw());
	return (m_bDrawHUD);
}


//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudGameObjective::OnThink(void)
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

}

//Purpose::
void CHudGameObjective::Paint()
{

	wchar_t txt[512];

	V_swprintf_safe(txt,L"Kill count %i / %i",m_iKillCount,m_iTargetKillCount);
	surface()->DrawSetTextColor(m_GameObjColor);
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextPos(text_xpos, text_ypos);
	surface()->DrawPrintText(txt, wcslen(txt));

	if (m_bHitTarget)
	{
		surface()->DrawSetTextPos(text2_xpos, text2_ypos);
		surface()->DrawPrintText(L"Objective: Eliminate the Boss", wcslen(L"Objective: Eliminate the Boss"));

	}
	else
	{
		surface()->DrawSetTextPos(text2_xpos, text2_ypos);
		surface()->DrawPrintText(L"Objective: Clear the elite-type", wcslen(L"Objective: Clear the elite-type"));
	}

}

void CHudGameObjective::MsgFunc_HudGameObjShouldDraw(bf_read &msg)
{
	m_bDrawHUD = msg.ReadByte();
	//DevMsg("ShouldDrawHudbool: %i", m_bDrawHUD);
}

void CHudGameObjective::MsgFunc_HudGameObjCount(bf_read &msg)
{
	m_iKillCount = msg.ReadShort();
}

void CHudGameObjective::MsgFunc_HudGameObjTargetCount(bf_read &msg)
{
	m_iTargetKillCount = msg.ReadShort();
}

void CHudGameObjective::MsgFunc_HudGameObjOnHitTarget(bf_read &msg)
{
	m_bHitTarget = msg.ReadByte();
}


