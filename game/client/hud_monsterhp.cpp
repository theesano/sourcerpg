#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_monsterhp.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "c_basehlplayer.h"


using namespace vgui;

#include "tier0/memdbgon.h" 

DECLARE_HUDELEMENT(CHudMonsterHP);

# define HULL_INIT -1

ConVar cl_showenemyhp("cl_showenemyhp","0"); //easy convar to disable or enable enemy hp bar

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudMonsterHP::CHudMonsterHP(const char * pElementName) :
CHudElement(pElementName), BaseClass(NULL, "HUDMonsterHP")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudMonsterHP::Init()
{
	if (cl_showenemyhp.GetInt() == 0)
		return;

	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudMonsterHP::Reset(void)
{
	m_flStaminaHUD = HULL_INIT;

	//SetBgColor(Color(0, 0, 0, 0));
}


//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudMonsterHP::OnThink(void)
{
	if (cl_showenemyhp.GetInt() == 0)
		return;

	float flCurrentPower = 0;
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;


	flCurrentPower = pPlayer->m_HL2Local.m_flStamina;

	m_flStaminaHUD = flCurrentPower;

}
//!! might cause the hud not to draw during experiment
bool CHudMonsterHP::ShouldDraw(void)
{
	bool bNeedsDraw = false;

	if (cl_showenemyhp.GetInt() == 0)
		return false;

	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	return (bNeedsDraw && CHudElement::ShouldDraw());
}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudMonsterHP::Paint()
{
	if (!cl_showenemyhp.GetInt() == 0)
	{
		wchar_t sz[64];

		surface()->DrawSetTextFont(m_hTextFont);
		surface()->DrawSetTextColor(m_MonsterHpColor);
		surface()->DrawSetTextPos(text_xpos, text_ypos);
		surface()->DrawPrintText(L"HEALTH", wcslen(L"HEALTH"));

		// Draw the actual health portion 
		surface()->DrawSetColor(m_MonsterHpColor);

		int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarWidth, ypos + m_flBarHeight);
		xpos += m_flStaminaHUD;

		// Draw the exhausted portion of the bar.
		//surface()->DrawSetColor(Color(m_StaminaColor[0], m_StaminaColor[1], m_StaminaColor[2], m_iStaminaDisabledAlpha));

		//surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		//xpos += (m_flBarChunkWidth + m_flBarChunkGap);

		//pseudocode 
		// bar max width = monster max hp
		// bar current = monster current hp/monster max hp = current to max ratio

		V_swprintf_safe(sz, L"%.0f /100", m_flStaminaHUD);
		surface()->DrawSetTextFont(m_hTextFont);
		surface()->DrawSetTextColor(m_MonsterHpColor);
		surface()->DrawSetTextPos(text2_xpos, text2_ypos);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
}