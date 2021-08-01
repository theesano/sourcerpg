#if !defined HUD_PLAYERQUICKSTATS_H
#define HUD_PLAYERQUICKSTATS_H 

#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Label.h>

//-----------------------------------------------------------------------------
// Purpose: Shows the hull bar
//-----------------------------------------------------------------------------

class CHudPlayerQuickStats : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudPlayerQuickStats, vgui::Panel);

public:
	CHudPlayerQuickStats(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);
	void VidInit(void);


	void MsgFunc_Battery(bf_read &msg);
	void MsgFunc_AS(bf_read &msg);


protected:
	virtual void Paint();

private:
	CPanelAnimationVar(Color, m_HullColor, "HullColor", "255 255 255 255");
	CPanelAnimationVar(Color, m_HullColor2, "HullColor2", "255 255 255 255");
	CPanelAnimationVar(Color, m_HullColor3, "HullColor3", "255 255 0 255");
	CPanelAnimationVar(int, m_iHullDisabledAlpha, "HullDisabledAlpha", "50");

	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "26", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "84", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "4", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth, "BarChunkWidth", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap, "BarChunkGap", "0", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Trebuchet18");
	CPanelAnimationVar(vgui::HFont, m_hTextFont2, "TextFont", "Default");
	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flBarInsetX2, "BarInsetX2", "26", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY2, "BarInsetY2", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth2, "BarWidth2", "84", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight2, "BarHeight2", "4", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth2, "BarChunkWidth2", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap2, "BarChunkGap2", "0", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flBarInsetX3, "BarInsetX3", "26", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY3, "BarInsetY3", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth3, "BarWidth3", "84", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight3, "BarHeight3", "4", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth3, "BarChunkWidth3", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap3, "BarChunkGap3", "0", "proportional_float");


	CPanelAnimationVarAliasType(float, text_xpos2, "text_xpos2", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos2, "text_ypos2", "2", "proportional_float");

	CPanelAnimationVarAliasType(float, text_xpos3, "text_xpos3", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos3, "text_ypos3", "2", "proportional_float");

	CPanelAnimationVarAliasType(float, text_xposHPNum, "text_xposHPNum", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_yposHPNum, "text_yposHPNum", "2", "proportional_float");

	CPanelAnimationVarAliasType(float, text_xposArmorNum, "text_xposArmorNum", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_yposArmorNum, "text_yposArmorNum", "2", "proportional_float");
	
	
	float m_flHull;

	int m_nHullLow;

	int m_iGetPlayerMP;

	int m_flPlayerArmor;

	float		m_flAttackSpeedBuffDuration;

	int		m_iBat;
	int		m_iNewBat;
	vgui::Label *m_ASPD;

};

#endif // HUD_SUITPOWER_H