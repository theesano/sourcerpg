#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_rageinfo.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "c_basehlplayer.h"
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/EditablePanel.h>



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
	m_pTestImagePanel = new ImagePanel(this, "TestIcon1");
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


	//SetBgColor(Color(0, 0, 0, 0));
}


//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudRageInfo::OnThink(void)
{
	ConVar *pRageCurrent = cvar->FindVar("sk_plr_rage_current");
	ConVar *pRageMax = cvar->FindVar("sk_plr_rage_max");

	ConVar *pUtilSlot1OptionID = cvar->FindVar("sk_plr_utilslot1_option_id");
	ConVar *pUtilSlot2OptionID = cvar->FindVar("sk_plr_utilslot2_option_id");
	ConVar *pUtilSlot3OptionID = cvar->FindVar("sk_plr_utilslot3_option_id");
	ConVar *pUtilSlot4OptionID = cvar->FindVar("sk_plr_utilslot4_option_id");

	m_flRage = pRageCurrent->GetFloat();
	m_flRageMax = pRageMax->GetFloat();

	m_iUtilSlot1OptionID = pUtilSlot1OptionID->GetInt();
	m_iUtilSlot2OptionID = pUtilSlot2OptionID->GetInt();
	m_iUtilSlot3OptionID = pUtilSlot3OptionID->GetInt();
	m_iUtilSlot4OptionID = pUtilSlot4OptionID->GetInt();

	//if (m_iUtilSlot1OptionID == 1)
	//{
	//	/*g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("UtilSlot1Icon1");*/

	//}
	//else if (m_iUtilSlot1OptionID == 2)
	//{
	//	/*g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("UtilSlot1Icon2");*/
	//}
	//else if (m_iUtilSlot1OptionID == 3)
	//{
	//	/*g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("UtilSlot1Icon3");*/
	//}
	//else if (m_iUtilSlot1OptionID == 4)
	//{
	//	/*g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("UtilSlot1Icon4");*/
	//}
	//else
	//{
	//	/*g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("UtilSlot1Icon0");*/
	//}

}


//------------------------------------------------------------------------
// Purpose: Draw the GUI
//------------------------------------------------------------------------
void CHudRageInfo::Paint()
{
	wchar_t sz[64];
//Texture block
	if (m_iUtilSlot1OptionID == 1)	
		surface()->DrawSetTexture(m_nIconTextureId1);
	else if (m_iUtilSlot1OptionID == 2)
		surface()->DrawSetTexture(m_nIconTextureId2);
	else if (m_iUtilSlot1OptionID == 3)
		surface()->DrawSetTexture(m_nIconTextureId3);
	else if (m_iUtilSlot1OptionID == 4)
		surface()->DrawSetTexture(m_nIconTextureId4);
	else
		surface()->DrawSetTexture(m_nIconTextureId0);

	surface()->DrawTexturedRect(m_iIconX, m_iIconY, m_iIconWide, m_iIconTall);

	if (m_iUtilSlot2OptionID == 1)
	{
		surface()->DrawSetTexture(m_nIconTextureId1);
	}
	else if (m_iUtilSlot2OptionID == 2)
	{
		surface()->DrawSetTexture(m_nIconTextureId2);
	}
	else if (m_iUtilSlot2OptionID == 3)
	{
		surface()->DrawSetTexture(m_nIconTextureId3);
	}
	else if (m_iUtilSlot2OptionID == 4)
	{
		surface()->DrawSetTexture(m_nIconTextureId4);
	}
	else
		surface()->DrawSetTexture(m_nIconTextureId0);

	surface()->DrawTexturedRect(m_iIcon2X, m_iIcon2Y, m_iIcon2Wide, m_iIcon2Tall);

	if (m_iUtilSlot3OptionID == 1)
	{
		surface()->DrawSetTexture(m_nIconTextureId1);
	}
	else if (m_iUtilSlot3OptionID == 2)
	{
		surface()->DrawSetTexture(m_nIconTextureId2);
	}
	else if (m_iUtilSlot3OptionID == 3)
	{
		surface()->DrawSetTexture(m_nIconTextureId3);
	}
	else if (m_iUtilSlot3OptionID == 4)
	{
		surface()->DrawSetTexture(m_nIconTextureId4);
	}
	else
		surface()->DrawSetTexture(m_nIconTextureId0);

	surface()->DrawTexturedRect(m_iIcon3X, m_iIcon3Y, m_iIcon3Wide, m_iIcon3Tall);

	if (m_iUtilSlot4OptionID == 1)
	{
		surface()->DrawSetTexture(m_nIconTextureId1);
	}
	else if (m_iUtilSlot4OptionID == 2)
	{
		surface()->DrawSetTexture(m_nIconTextureId2);
	}
	else if (m_iUtilSlot4OptionID == 3)
	{
		surface()->DrawSetTexture(m_nIconTextureId3);
	}
	else if (m_iUtilSlot4OptionID == 4)
	{
		surface()->DrawSetTexture(m_nIconTextureId4);
	}
	else
		surface()->DrawSetTexture(m_nIconTextureId0);

	surface()->DrawTexturedRect(m_iIcon4X, m_iIcon4Y, m_iIcon4Wide, m_iIcon4Tall);

//Text Block
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_RageColor);
	surface()->DrawSetTextPos(text_xpos, text_ypos);
	surface()->DrawPrintText(L"RAGE", wcslen(L"RAGE"));

	V_swprintf_safe(sz, L"%.0f/%.0f", m_flRage,m_flRageMax);
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_RageColor);
	surface()->DrawSetTextPos(text2_xpos, text2_ypos);
	surface()->DrawPrintText(sz, wcslen(sz));

	V_swprintf_safe(sz, L"Q:%i", m_iUtilSlot1OptionID);
	surface()->DrawSetTextPos(text3_xpos, text3_ypos);
	surface()->DrawPrintText(sz, wcslen(sz));

	V_swprintf_safe(sz, L"E:%i", m_iUtilSlot2OptionID);
	surface()->DrawSetTextPos(text4_xpos, text4_ypos);
	surface()->DrawPrintText(sz, wcslen(sz));

	V_swprintf_safe(sz, L"F1:%i", m_iUtilSlot3OptionID);
	surface()->DrawSetTextPos(text5_xpos, text5_ypos);
	surface()->DrawPrintText(sz, wcslen(sz));

	V_swprintf_safe(sz, L"F2:%i", m_iUtilSlot4OptionID);
	surface()->DrawSetTextPos(text6_xpos, text6_ypos);
	surface()->DrawPrintText(sz, wcslen(sz));


}