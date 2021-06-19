#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_playerquickstats.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"

using namespace vgui;

#include "tier0/memdbgon.h" 

DECLARE_HUDELEMENT(CHudPlayerQuickStats);

# define HULL_INIT -1

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudPlayerQuickStats::CHudPlayerQuickStats(const char * pElementName) :
CHudElement(pElementName), BaseClass(NULL, "HUDPlayerQuickStats")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudPlayerQuickStats::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudPlayerQuickStats::Reset(void)
{
	m_flHull = HULL_INIT;
	m_nHullLow = -1;
	//SetBgColor(Color(0, 0, 0, 128));
	SetBgColor(Color(0, 0, 0, 0));
}


//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudPlayerQuickStats::OnThink(void)
{
	float newHull = 0;
	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();
	
	if (!local)
		return;

	ConVar *pGetPlayerMP = cvar->FindVar("sk_plr_current_mp");
	m_iGetPlayerMP = max(pGetPlayerMP->GetInt(), 0);

	// Never below zero 
	newHull = max(local->GetHealth(), 0);

	// DevMsg("Sheild at is at: %f\n",newShield);
	// Only update the fade if we've changed health
	//if (newHull == m_flHull)
		//return;

	m_flHull = newHull;


}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudPlayerQuickStats::Paint()
{
	wchar_t sz[64];
	C_BasePlayer * mylocal = C_BasePlayer::GetLocalPlayer();

	// Draw our name
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_HullColor);
	surface()->DrawSetTextPos(text_xpos, text_ypos);
	surface()->DrawPrintText(L"HEALTH", wcslen(L"HEALTH"));


	// Get bar chunks
	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_flHull / 100.0f) + 0.5f);

	// Draw the suit power bar MP
	surface()->DrawSetColor(m_HullColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	
	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor[0], m_HullColor[1], m_HullColor[2], m_iHullDisabledAlpha));

	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	V_swprintf_safe(sz, L"%i", mylocal->GetHealth());

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_HullColor);
	surface()->DrawSetTextPos(text_xposHPNum, text_yposHPNum);
	surface()->DrawPrintText(sz, wcslen(sz));
	
	
	//Draw Name MP
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_HullColor2);
	surface()->DrawSetTextPos(text_xpos2, text_ypos2);
	surface()->DrawPrintText(L"MP", wcslen(L"MP"));
	
	// Get bar chunks MP
	int chunkCount2 = m_flBarWidth2 / (m_flBarChunkWidth2 + m_flBarChunkGap2);
	int enabledChunks2 = (int)((float)chunkCount2* (m_iGetPlayerMP / 100.0f) + 0.5f);

	// Draw the suit power bar MP
	surface()->DrawSetColor(m_HullColor2);

	//Position of the bar on the Hud
	int xpos2 = m_flBarInsetX2, ypos2 = m_flBarInsetY2;

	//reversed

	for (int i = 0; i < enabledChunks2; i++)
	{
		surface()->DrawFilledRect(xpos2, ypos2, xpos2 + m_flBarChunkWidth2, ypos2 + m_flBarHeight2);
		xpos2 += (m_flBarChunkWidth2 + m_flBarChunkGap2);
	}

	// Draw the exhausted portion of the bar. MP
	
	//reversed
	surface()->DrawSetColor(Color(m_HullColor2[0], m_HullColor2[1], m_HullColor2[2], m_iHullDisabledAlpha));
	for (int i = enabledChunks2; i < chunkCount2; i++)
	{
		surface()->DrawFilledRect(xpos2, ypos2, xpos2 + m_flBarChunkWidth2, ypos2 + m_flBarHeight2);
		xpos2 += (m_flBarChunkWidth2 + m_flBarChunkGap2);
	}
	

	V_swprintf_safe(sz, L"%i", m_iGetPlayerMP);

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_HullColor);
	surface()->DrawSetTextPos(text_xpos3, text_ypos3);
	surface()->DrawPrintText(sz, wcslen(sz));

}