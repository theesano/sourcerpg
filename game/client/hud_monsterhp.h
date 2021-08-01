#if !defined HUD_STAMINA_H
#define HUD_STAMINA_H 

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


protected:
	virtual void Paint();

private:

	CPanelAnimationVar(Color, m_MonsterHpColor, "MonsterHpColor", "255 255 255 255");
	CPanelAnimationVar(int, m_iMonsterHpDisabledAlpha, "MonsterHpDisabledAlpha", "70");

	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "80", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "10", "proportional_float");

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");

	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "20", "proportional_float");

	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "40", "proportional_float");

	float m_flStaminaHUD;
};

#endif 