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

class CHudRageInfo : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudRageInfo, vgui::Panel);

public:
	CHudRageInfo(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

protected:
	virtual void Paint();

private:

	CPanelAnimationVar(Color, m_RageColor, "RageColor", "255 255 255 255");
	CPanelAnimationVar(int, m_iRageDisabledAlpha, "RageDisabledAlpha", "70");

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");

	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "20", "proportional_float");

	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "40", "proportional_float");

	CPanelAnimationVarAliasType(float, text3_xpos, "text3_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text3_ypos, "text3_ypos", "60", "proportional_float");

	float m_flRage;
	float m_flRageMax;
};

#endif 