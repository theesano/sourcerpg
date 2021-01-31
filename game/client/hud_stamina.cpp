#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_stamina.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "c_basehlplayer.h"


using namespace vgui;

#include "tier0/memdbgon.h" 

DECLARE_HUDELEMENT(CHudStamina);

# define HULL_INIT -1

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudStamina::CHudStamina(const char * pElementName) :
CHudElement(pElementName), BaseClass(NULL, "HUDStamina")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudStamina::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudStamina::Reset(void)
{
	m_flStamina = HULL_INIT;

	SetBgColor(Color(0, 0, 0, 0));
}


//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudStamina::OnThink(void)
{
	float flCurrentPower = 0;
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	flCurrentPower = pPlayer->m_HL2Local.m_flSuitPower;

	m_flStamina = flCurrentPower;

}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudStamina::Paint()
{
	wchar_t sz[64];

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_StaminaColor);
	surface()->DrawSetTextPos(text_xpos, text_ypos);
	surface()->DrawPrintText(L"STAMINA", wcslen(L"STAMINA"));

	// Get bar chunks
	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_flStamina / 100.0f) + 0.5f);

	// Draw the suit power bar MP
	surface()->DrawSetColor(m_StaminaColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_StaminaColor[0], m_StaminaColor[1], m_StaminaColor[2], m_iStaminaDisabledAlpha));

	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	V_swprintf_safe(sz,L"%.0f /100",m_flStamina);
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_StaminaColor);
	surface()->DrawSetTextPos(text2_xpos, text2_ypos);
	surface()->DrawPrintText(sz, wcslen(sz));

}