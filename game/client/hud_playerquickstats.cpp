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
DECLARE_HUD_MESSAGE(CHudPlayerQuickStats, Battery);
DECLARE_HUD_MESSAGE(CHudPlayerQuickStats, AS);
DECLARE_HUD_MESSAGE(CHudPlayerQuickStats, RageBuffTime);


# define HULL_INIT -1
# define ARMOR_INIT -1

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudPlayerQuickStats::CHudPlayerQuickStats(const char * pElementName) :
CHudElement(pElementName), BaseClass(NULL, "HUDPlayerQuickStats")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	
	m_ASPD = vgui::SETUP_PANEL(new vgui::Label(this, "HUDASPD", ""));
	m_pRageBuffTime = vgui::SETUP_PANEL(new vgui::Label(this, "HUDRageBuffTime", ""));
	SetParent(pParent);

	m_ASPD->SetPos(0, 0);
	m_ASPD->SetSize(16, 16);
	m_ASPD->SetContentAlignment(vgui::Label::a_center);
	m_ASPD->SetPaintBorderEnabled(false);
	m_ASPD->SetPaintBackgroundEnabled(false);

	m_pRageBuffTime->SetPos(16, 16);
	m_pRageBuffTime->SetSize(16, 16);
	m_pRageBuffTime->SetContentAlignment(vgui::Label::a_center);
	m_pRageBuffTime->SetPaintBorderEnabled(false);
	m_pRageBuffTime->SetPaintBackgroundEnabled(false);

}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudPlayerQuickStats::Init()
{
	HOOK_HUD_MESSAGE(CHudPlayerQuickStats, Battery);
	HOOK_HUD_MESSAGE(CHudPlayerQuickStats, AS);
	HOOK_HUD_MESSAGE(CHudPlayerQuickStats, RageBuffTime);


	Reset();
	m_iBat		= ARMOR_INIT;
	m_iNewBat	= 0;
	m_flAttackSpeedBuffDuration = 0;
	m_flRageBuffDuration = 0;


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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudPlayerQuickStats::VidInit(void)
{
	Reset();
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

	m_flPlayerArmor;


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

	// Get bar chunks SA
	int chunkCount3 = m_flBarWidth3 / (m_flBarChunkWidth3 + m_flBarChunkGap3);
	int enabledChunks3 = (int)((float)chunkCount3* (m_iNewBat / 100.0f) + 0.5f);

	// Draw Armor bar
	surface()->DrawSetColor(m_HullColor3);

	V_swprintf_safe(sz, L"SA		%i/100", m_iNewBat);

	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(m_HullColor3);
	surface()->DrawSetTextPos(text_xposArmorNum, text_yposArmorNum);
	surface()->DrawPrintText(sz, wcslen(sz));

	int xpos3 = m_flBarInsetX3, ypos3 = m_flBarInsetY3;

	//reversed

	for (int i = 0; i < enabledChunks3; i++)
	{
		surface()->DrawFilledRect(xpos3, ypos3, xpos3 + m_flBarChunkWidth3, ypos3 + m_flBarHeight3);
		xpos3 += (m_flBarChunkWidth3 + m_flBarChunkGap3);
	}

	// Draw the exhausted portion of the bar. MP

	//reversed
	surface()->DrawSetColor(Color(m_HullColor3[0], m_HullColor3[1], m_HullColor3[2], m_iHullDisabledAlpha));
	for (int i = enabledChunks3; i < chunkCount3; i++)
	{
		surface()->DrawFilledRect(xpos3, ypos3, xpos3 + m_flBarChunkWidth3, ypos3 + m_flBarHeight3);
		xpos3 += (m_flBarChunkWidth3 + m_flBarChunkGap3);
	}

	// Draw Buff Duration
	if (m_flAttackSpeedBuffDuration > 0)
	{
		V_swprintf_safe(sz, L"%.0f", m_flAttackSpeedBuffDuration);
		surface()->DrawSetTextColor(0,255,255,255);
		surface()->DrawSetTextPos(text_xposArmorNum, text_yposArmorNum + 20);
		surface()->DrawPrintText(sz, wcslen(sz));
		//m_ASPD->SetVisible(true);
		//m_ASPD->SetText(sz);
		m_ASPD->SetVisible(false);

	}
	else
	{
		surface()->DrawSetTextColor(0, 255, 255, 0);
	}

	// Draw Buff Duration
	if (m_flRageBuffDuration > 0)
	{
		V_swprintf_safe(sz, L"%.0f", m_flRageBuffDuration);
		surface()->DrawSetTextColor(255, 0, 255, 255);
		surface()->DrawSetTextPos(text_xposArmorNum, text_yposArmorNum + 40);
		surface()->DrawPrintText(sz, wcslen(sz));
		//m_ASPD->SetVisible(true);
		//m_ASPD->SetText(sz);
		m_pRageBuffTime->SetVisible(false);

	}
	else
	{
		surface()->DrawSetTextColor(0, 255, 255, 0);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudPlayerQuickStats::MsgFunc_Battery(bf_read &msg)
{
	m_iNewBat = msg.ReadShort();
}

void CHudPlayerQuickStats::MsgFunc_AS(bf_read &msg)
{
	m_flAttackSpeedBuffDuration = msg.ReadShort();
}

void CHudPlayerQuickStats::MsgFunc_RageBuffTime(bf_read &msg)
{
	m_flRageBuffDuration = msg.ReadShort();
}
