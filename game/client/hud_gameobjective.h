#if !defined HUD_GAMEOBJECTIVE_H
#define HUD_GAMEOBJECTIVE_H 

#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>

//-----------------------------------------------------------------------------
// Purpose: Shows the hull bar
//-----------------------------------------------------------------------------

class CHudGameObjective : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudGameObjective, vgui::Panel);

public:
	CHudGameObjective(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);
	bool		ShouldDraw(void);

	void MsgFunc_HudGameObjShouldDraw(bf_read &msg);
	void MsgFunc_HudGameObjCount(bf_read &msg);
	void MsgFunc_HudGameObjTargetCount(bf_read &msg);
	void MsgFunc_HudGameObjOnHitTarget(bf_read &msg);


protected:
	virtual void Paint();

private:

	CPanelAnimationVar(Color, m_GameObjColor, "GameObjColor", "255 255 255 255");

	CPanelAnimationVar(int, m_iGameObjDisabledAlpha, "GameObjDisabledAlpha", "70");

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "DefaultSmall");

	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "20", "proportional_float");

	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "20", "proportional_float");


	bool m_bDrawHUD;
	bool m_bHitTarget;
	int m_iKillCount;
	int m_iTargetKillCount;
};

#endif 