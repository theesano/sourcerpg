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
DECLARE_HUD_MESSAGE(CHudMonsterHP, MonsterHP);
DECLARE_HUD_MESSAGE(CHudMonsterHP, MonsterArmor);
DECLARE_HUD_MESSAGE(CHudMonsterHP, MonsterHPMax);
DECLARE_HUD_MESSAGE(CHudMonsterHP, MonsterName);


# define HULL_INIT -1

ConVar hud_showenemyhp("hud_showenemyhp", "1"); //easy convar to disable or enable enemy hp bar

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
	if (hud_showenemyhp.GetInt() == 0)
		return;

	HOOK_HUD_MESSAGE(CHudMonsterHP, MonsterHP);
	HOOK_HUD_MESSAGE(CHudMonsterHP, MonsterArmor);
	HOOK_HUD_MESSAGE(CHudMonsterHP, MonsterHPMax);
	HOOK_HUD_MESSAGE(CHudMonsterHP, MonsterName);

	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudMonsterHP::Reset(void)
{
	m_flStaminaHUD = HULL_INIT;

	SetBgColor(Color(0, 0, 0, 0));
}


//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudMonsterHP::OnThink(void)
{
	if (hud_showenemyhp.GetInt() == 0)
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

	if (hud_showenemyhp.GetInt() == 0)
		return false;

	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	//return (bNeedsDraw && CHudElement::ShouldDraw());
	return (1);
}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudMonsterHP::Paint()
{
	if (!hud_showenemyhp.GetInt() == 0)
	{
		wchar_t sz[64];

		surface()->DrawSetTextFont(m_hTextFont);
		surface()->DrawSetTextColor(m_MonsterHpColor);
		surface()->DrawSetTextPos(text_xpos, text_ypos);
		surface()->DrawPrintText(L"name",wcslen(L"name"));

		V_swprintf_safe(sz, L"%.0f / %.0f", m_iHealth,m_iHealthMax);
		surface()->DrawSetTextFont(m_hTextFont);
		surface()->DrawSetTextPos(text2_xpos, text2_ypos);
		surface()->DrawPrintText(sz, wcslen(sz));

		// Draw the actual health portion 
		surface()->DrawSetColor(m_MonsterHpColor);

		int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

		float flBarRate = m_iHealth / m_iHealthMax;
		surface()->DrawFilledRect(xpos, ypos, xpos + (m_flBarWidth*flBarRate), ypos + m_flBarHeight);
		
		// Draw the exhausted portion of the bar.
		//surface()->DrawSetColor(Color(m_StaminaColor[0], m_StaminaColor[1], m_StaminaColor[2], m_iStaminaDisabledAlpha));

		//surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		//xpos += (m_flBarChunkWidth + m_flBarChunkGap);

		//pseudocode 
		// bar max width = monster max hp
		// bar current = monster current hp/monster max hp = current to max ratio
//Armor bar
		float flBarRate2 = m_iArmor / 100;

		surface()->DrawSetColor(m_MonsterArmorColor);

		int xpos2 = m_flBar2InsetX, ypos2 = m_flBar2InsetY;

		surface()->DrawFilledRect(xpos2, ypos2, xpos2 + (m_flBar2Width*flBarRate2), ypos2 + m_flBar2Height);
		
	}
}

void CHudMonsterHP::MsgFunc_MonsterHP(bf_read &msg)
{
	m_iHealth = msg.ReadShort();
}

void CHudMonsterHP::MsgFunc_MonsterArmor(bf_read &msg)
{
	m_iArmor = msg.ReadShort();
}

void CHudMonsterHP::MsgFunc_MonsterHPMax(bf_read &msg)
{
	m_iHealthMax = msg.ReadShort();
}


void CHudMonsterHP::MsgFunc_MonsterName(bf_read &msg)
{
	char monmon[32];
	msg.ReadString(monmon,32);
}
