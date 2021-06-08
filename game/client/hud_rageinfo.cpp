#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_rageinfo.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "c_basehlplayer.h"


using namespace vgui;

#include "tier0/memdbgon.h" 

DECLARE_HUDELEMENT(CHudRageInfo);

# define HULL_INIT -1

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudRageInfo::CHudRageInfo(const char * pElementName) :
CHudElement(pElementName), BaseClass(NULL, "HUDRageInfo")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudRageInfo::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudRageInfo::Reset(void)
{
	m_flRage = HULL_INIT;

	SetBgColor(Color(0, 0, 0, 0));
}


//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudRageInfo::OnThink(void)
{
	ConVar *pRageCurrent = cvar->FindVar("sk_plr_rage_current");
	ConVar *pRageMax = cvar->FindVar("sk_plr_rage_max");

	m_flRage = pRageCurrent->GetFloat();
	m_flRageMax = pRageMax->GetFloat();

}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudRageInfo::Paint()
{
	wchar_t sz[64];

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_RageColor);
	surface()->DrawSetTextPos(text_xpos, text_ypos);
	surface()->DrawPrintText(L"RAGE", wcslen(L"RAGE"));

	V_swprintf_safe(sz, L"%.0f/%.0f", m_flRage,m_flRageMax);
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_RageColor);
	surface()->DrawSetTextPos(text2_xpos, text2_ypos);
	surface()->DrawPrintText(sz, wcslen(sz));

	surface()->DrawSetTextPos(text3_xpos, text3_ypos);
	surface()->DrawPrintText(L"Q:Heal E:MP F1:MoveSpd F2:Stamina+", wcslen(L"Q:heal E:MP F1:movespd F2:stamina+"));



}