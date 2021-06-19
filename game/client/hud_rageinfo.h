#if !defined HUD_STAMINA_H
#define HUD_STAMINA_H 

#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>
#include <vgui_controls/EditablePanel.h>


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

	CPanelAnimationVarAliasType(float, text4_xpos, "text4_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text4_ypos, "text4_ypos", "60", "proportional_float");

	CPanelAnimationVarAliasType(float, text5_xpos, "text5_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text5_ypos, "text5_ypos", "60", "proportional_float");

	CPanelAnimationVarAliasType(float, text6_xpos, "text6_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text6_ypos, "text6_ypos", "60", "proportional_float");

	CPanelAnimationVarAliasType(int, m_iIconX, "icon_xpos", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconY, "icon_ypos", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconWide, "icon_width", "28", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconTall, "icon_height", "28", "proportional_int");

	CPanelAnimationVarAliasType(int, m_iIcon2X, "icon2_xpos", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIcon2Y, "icon2_ypos", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIcon2Wide, "icon2_width", "28", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIcon2Tall, "icon2_height", "28", "proportional_int");

	CPanelAnimationVarAliasType(int, m_iIcon3X, "icon3_xpos", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIcon3Y, "icon3_ypos", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIcon3Wide, "icon3_width", "28", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIcon3Tall, "icon3_height", "28", "proportional_int");

	CPanelAnimationVarAliasType(int, m_iIcon4X, "icon4_xpos", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIcon4Y, "icon4_ypos", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIcon4Wide, "icon4_width", "28", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIcon4Tall, "icon4_height", "28", "proportional_int");

	CPanelAnimationVarAliasType(int, m_nIconTextureId0, "icon_texture", "UI/rage/None", "textureid");
	CPanelAnimationVarAliasType(int, m_nIconTextureId1, "icon_texture1", "UI/rage/ItemHeal", "textureid");
	CPanelAnimationVarAliasType(int, m_nIconTextureId2, "icon_texture2", "UI/rage/ItemMP", "textureid");
	CPanelAnimationVarAliasType(int, m_nIconTextureId3, "icon_texture3", "UI/rage/ItemStamina", "textureid");
	CPanelAnimationVarAliasType(int, m_nIconTextureId4, "icon_texture4", "UI/rage/ItemMoveSpeed", "textureid");

	float m_flRage;
	float m_flRageMax;

	int m_iUtilSlot1OptionID;
	int m_iUtilSlot2OptionID;
	int m_iUtilSlot3OptionID;
	int m_iUtilSlot4OptionID;

	vgui::ImagePanel *m_pTestImagePanel;

};

#endif 