#if !defined HUD_MONSTERHP_H
#define HUD_MONSTERHP_H 

#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>

//-----------------------------------------------------------------------------
// Purpose: Shows the hull bar
//-----------------------------------------------------------------------------

class CHudMonsterHP : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudMonsterHP, vgui::Panel);

public:
	CHudMonsterHP(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);
	bool			ShouldDraw(void);

	void MsgFunc_MonsterHP(bf_read &msg);
	void MsgFunc_MonsterArmor(bf_read &msg);
	void MsgFunc_MonsterHPMax(bf_read &msg);
	void MsgFunc_MonsterName(bf_read &msg);

protected:
	virtual void Paint();

private:

	CPanelAnimationVar(Color, m_MonsterHpColor, "MonsterHpColor", "255 255 255 255");
	CPanelAnimationVar(Color, m_MonsterArmorColor, "MonsterArmorColor", "255 191 0 255");
	CPanelAnimationVar(int, m_iMonsterHpDisabledAlpha, "MonsterHpDisabledAlpha", "70");

	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "80", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "10", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flBar2InsetX, "Bar2InsetX", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBar2InsetY, "Bar2InsetY", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBar2Width, "Bar2Width", "80", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBar2Height, "Bar2Height", "10", "proportional_float");

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "DefaultSmall");

	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "20", "proportional_float");

	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "40", "proportional_float");

	CPanelAnimationVarAliasType(float, text3_xpos, "text3_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text3_ypos, "text3_ypos", "20", "proportional_float")

	CPanelAnimationVarAliasType(float, text4_xpos, "text4_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text4_ypos, "text4_ypos", "20", "proportional_float")

	float m_flStaminaHUD;
	float	m_iHealth;
	float	m_iHealthMax;
	float	m_iArmor;
	char	monstername;
};

#endif 