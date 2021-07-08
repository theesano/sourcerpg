#if !defined HUD_MENUHINT_H
#define HUD_MENUHINT_H 

#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>


//-----------------------------------------------------------------------------
// Purpose: Shows 
//-----------------------------------------------------------------------------

class CHudMenuHint : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudMenuHint, vgui::Panel);

public:
	CHudMenuHint(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);
	void VidInit(void);

protected:
	virtual void Paint();

private:
	CPanelAnimationVar(Color, m_Color, "HullColor", "255 255 255 255");

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2", "proportional_float");

	vgui::ImagePanel *m_GameMenuIcon;
	vgui::ImagePanel *m_pTestImagePanel2;
	vgui::ImagePanel *m_pTestImagePanel3;
	vgui::ImagePanel *m_pTestImagePanel4;

	vgui::Label *m_IconLabel1;
	vgui::Label *m_IconLabel2;
	vgui::Label *m_IconLabel3;
	vgui::Label *m_IconLabel4;




};
#endif 