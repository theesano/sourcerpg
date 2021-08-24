#if !defined HUD_SKILLCD_H
#define HUD_SKILLCD_H 

#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>


//-----------------------------------------------------------------------------
// Purpose: Shows the hull bar
//-----------------------------------------------------------------------------

class CHudSkillCooldown : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudSkillCooldown, vgui::Panel);

public:
	CHudSkillCooldown(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void OnThink(void);

protected:
	virtual void Paint();
	int m_nSkillImage1;

private:
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Trebuchet24");
	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "30", "proportional_float");
	CPanelAnimationVar(Color, m_TextColor1, "TextColor1", "255 0 0 255");

	CPanelAnimationVarAliasType(float, text_xpos2, "text_xpos2", "30", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos2, "text_ypos2", "30", "proportional_float");
	CPanelAnimationVar(Color, m_TextColor2, "TextColor2", "255 0 0 255");

	CPanelAnimationVarAliasType(float, text_xpos3, "text_xpos3", "60", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos3, "text_ypos3", "30", "proportional_float");
	CPanelAnimationVar(Color, m_TextColor3, "TextColor3", "255 0 0 255");

	CPanelAnimationVarAliasType(float, text_xpos4, "text_xpos4", "90", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos4, "text_ypos4", "30", "proportional_float");
	CPanelAnimationVar(Color, m_TextColor4, "TextColor4", "255 0 0 255");

	CPanelAnimationVarAliasType(float, text_xpos5, "text_xpos5", "120", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos5, "text_ypos5", "30", "proportional_float");
	CPanelAnimationVar(Color, m_TextColor5, "TextColor5", "255 0 0 255");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "255 255 255 255");

	CPanelAnimationVarAliasType(float, text_xpos6, "text_xpos6", "120", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos6, "text_ypos6", "30", "proportional_float");
	CPanelAnimationVar(Color, m_TextColor6, "TextColor6", "255 0 0 255");
	
	CPanelAnimationVarAliasType(int, m_nIconTextureId0, "icon_texture0", "UI/rage/None", "textureid");

	CPanelAnimationVarAliasType(int, m_iIconX, "icon_xpos", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconY, "icon_ypos", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconWide, "icon_width", "28", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconTall, "icon_height", "28", "proportional_int");
	CPanelAnimationVarAliasType(int, m_nIconTextureId, "icon_texture", "UI/skills/icon_evilslash", "textureid");
	CPanelAnimationVarAliasType(int, m_nIconTextureId_2, "icon_texture_2", "UI/skills/icon_evilslash_onCD", "textureid");
	CPanelAnimationVarAliasType(int, m_nIconTextureId_3, "icon_texture_3", "UI/skills/icon_evilslash_noMP", "textureid");

	CPanelAnimationVarAliasType(int, m_iIconX2, "icon_xpos2", "30", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconY2, "icon_ypos2", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconWide2, "icon_width2", "56", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconTall2, "icon_height2", "28", "proportional_int");
	CPanelAnimationVarAliasType(int, m_nIconTextureId2, "icon_texture2", "UI/skills/icon_deathgrinder", "textureid");

	CPanelAnimationVarAliasType(int, m_iIconX3, "icon_xpos3", "30", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconY3, "icon_ypos3", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconWide3, "icon_width3", "86", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconTall3, "icon_height3", "28", "proportional_int");
	CPanelAnimationVarAliasType(int, m_nIconTextureId3, "icon_texture3", "UI/skills/icon_grimcountdown", "textureid");

	CPanelAnimationVarAliasType(int, m_iIconX4, "icon_xpos4", "90", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconY4, "icon_ypos4", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconWide4, "icon_width4", "116", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconTall4, "icon_height4", "28", "proportional_int");
	CPanelAnimationVarAliasType(int, m_nIconTextureId4, "icon_texture4", "UI/skills/icon_fearchain", "textureid");

	CPanelAnimationVarAliasType(int, m_iIconX5, "icon_xpos5", "120", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconY5, "icon_ypos5", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconWide5, "icon_width5", "146", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconTall5, "icon_height5", "28", "proportional_int");
	CPanelAnimationVarAliasType(int, m_nIconTextureId5, "icon_texture5", "UI/skills/icon_tornado", "textureid");
	CPanelAnimationVar(Color, m_Icon5Color, "Icon5Color", "255 0 0 255");

	CPanelAnimationVarAliasType(int, m_iIconX6, "icon_xpos6", "150", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconY6, "icon_ypos6", "2", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconWide6, "icon_width6", "176", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iIconTall6, "icon_height6", "28", "proportional_int");
	CPanelAnimationVarAliasType(int, m_nIconTextureId6, "icon_texture6", "UI/skills/icon_scythecutter", "textureid");
	CPanelAnimationVar(Color, m_Icon6Color, "Icon6Color", "255 0 0 255");


	int m_flHudSk1Timer;
	int m_flHudSk2Timer;
	int m_flHudSk3Timer;
	int m_flHudSk4Timer;
	int m_flHudSk5Timer;
	int m_flHudSk6Timer;
	int m_flHudSk7Timer;

	int m_iGetPlayerMP;

	int m_iQuickslot1SkillID;
	int m_iQuickslot2SkillID;
	int m_iQuickslot3SkillID;
	int m_iQuickslot4SkillID;
	int m_iQuickslot5SkillID;
	int m_iQuickslot6SkillID;

	vgui::Label *m_pPassiveSkillLabel1;
	vgui::ImagePanel *m_pPassiveSkill1;


};

#endif 