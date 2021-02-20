#include "cbase.h"
#include "Ihudskillinfo.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include "vgui/ISurface.h"
//#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Button.h>

//CHudSkillInfo class:
class CHudSkillInfo : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CHudSkillInfo, vgui::Frame);
	//CHUdSkillInfo : This class / vgui::Frame : BaseClass

	CHudSkillInfo(vgui::VPANEL parent); //Constructor
	~CHudSkillInfo(){}; //Destructor

	virtual void	OnMouseReleased(vgui::MouseCode code);

protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);
	virtual void Paint();
	virtual void OnThink();

private:
	//Other used VGUI control Elements:
	Label *m_StatsInfo;
	Label *m_StatsBaseDamage;
	//ImagePanel* imagePanel = new ImagePanel(this, "myPanel");
	Button *m_pCloseButton;
	Button *m_pAspdUpButton;
	Button *m_pAspdDownButton;

	CPanelAnimationVarAliasType(int, m_iBgImageX, "BgImageX", "0", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iBgImageY, "BgImageY", "0", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iBgImageWide, "BgImageWidth", "308", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iBgImageTall, "BgImageHeight", "308", "proportional_int");
	CPanelAnimationVarAliasType(int, m_BgImage, "bgimage", "vgui/panel", "textureid");
	CPanelAnimationVarAliasType(int, m_iStatsAttackSpeedX, "StatsAttackSpeedX", "64", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iStatsAttackSpeedY, "StatsAttackSpeedY", "96", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iStatsAttackSpeedButtonX, "StatsAttackSpeedButtonX", "72", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iStatsAttackSpeedButtonY, "StatsAttackSpeedButtonY", "96", "proportional_int");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Trebuchet18");
	float m_flGetPlayerAttackSpeedMod;
	float m_flPlayerBaseDamage;
};

//Constructor: Initialize the Panel
CHudSkillInfo::CHudSkillInfo(vgui::VPANEL parent)
	:BaseClass(NULL, "HudSkillInfo")
{
	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);
	SetPaintBackgroundEnabled(false);
	SetProportional(false);
	SetTitleBarVisible(true);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(true);
	SetSizeable(false);
	SetMoveable(true); // I want it to be movable later on
	SetVisible(true);
	
	
	//Button done
	m_pCloseButton = new Button(this, "ButtonClose", "", this, "turnoff");
	m_pCloseButton->SetDepressedSound("common/bugreporter_succeeded.wav");
	m_pCloseButton->SetReleasedSound("ui/buttonclick.wav");

	//surface()->DrawSetTexture(m_BgImage);
	//surface()->DrawTexturedRect(m_iBgImageX, m_iBgImageY, m_iBgImageWide, m_iBgImageTall);
	
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

	LoadControlSettings("resource/UI/hudskillinfo.res");

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

	DevMsg("HudSkillInfo has been constructed \n");
	
	//imagePanel->SetImage(scheme()->GetImage("panel", false));

	m_StatsInfo = new Label(this, "AttackSpeedDisp", "aspd");
	m_StatsInfo->SetPos(64,76);
	m_StatsInfo->SetFont(m_hTextFont);
	m_StatsInfo->SetWide(128);

	m_StatsBaseDamage = new Label(this, "BaseDamage", "bdmg");
	m_StatsBaseDamage->SetPos(64,64);
	m_StatsBaseDamage->SetFont(m_hTextFont);
	m_StatsBaseDamage->SetWide(128);


	m_pAspdUpButton = new Button(this, "ButtonIncreaseASPD", "", this);
	m_pAspdUpButton->SetPos(175,75);
	m_pAspdUpButton->SetText("Increase");

	m_pAspdDownButton = new Button(this, "ButtonDecreaseASPD", "", this);
	m_pAspdDownButton->SetPos(175,100);
	m_pAspdDownButton->SetText("Decrease");
	
	//m_skinfo1 = new Label(this, "Skill2", "skill2");
	//m_skinfo1->SetPos(100, 100);
	
}

//Class: CHudSkillInfoInterface Class. Used for construction.
class CHudSkillInfoInterface : public IHudSkillInfo
{
private:
	CHudSkillInfo *HudSkillInfo;
public:
	CHudSkillInfoInterface()
	{
		HudSkillInfo = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		HudSkillInfo = new CHudSkillInfo(parent);
	}
	void Destroy()
	{
		if (HudSkillInfo)
		{
			HudSkillInfo->SetParent((vgui::Panel *)NULL);
			delete HudSkillInfo;
		}
	}
	void Activate(void)
	{
		if (hudskillinfo)
		{
			hudskillinfo->Activate();
		}
	}
};
static CHudSkillInfoInterface g_HudSkillInfo;
IHudSkillInfo* hudskillinfo = (IHudSkillInfo*)&g_HudSkillInfo;

ConVar cl_showskillinfo("cl_showskillinfo", "0", FCVAR_CLIENTDLL, "Sets the state of the skills information panel <state>");

void CHudSkillInfo::OnTick()
{
	ConVar *pGetPlayerAttackSpeedMod = cvar->FindVar("sk_plr_attackspeedmod");
	m_flGetPlayerAttackSpeedMod = pGetPlayerAttackSpeedMod->GetFloat();

	if (m_pAspdUpButton->IsDepressed())
	{
		pGetPlayerAttackSpeedMod->SetValue(pGetPlayerAttackSpeedMod->GetFloat() + 0.1f);
	}

	if (m_pAspdDownButton->IsDepressed())
	{
		pGetPlayerAttackSpeedMod->SetValue(pGetPlayerAttackSpeedMod->GetFloat() - 0.1f);
	}

	BaseClass::OnTick();
	SetVisible(cl_showskillinfo.GetBool()); 

}

void CHudSkillInfo::OnThink()
{
	

	ConVar *pGetPlayerBaseDamage = cvar->FindVar("sk_plr_dmg_melee");
	m_flPlayerBaseDamage = pGetPlayerBaseDamage->GetFloat();

	

}

void CHudSkillInfo::Paint()
{
	surface()->DrawSetTexture(m_BgImage);
	surface()->DrawTexturedRect(m_iBgImageX, m_iBgImageY, m_iBgImageWide, m_iBgImageTall);

	surface()->DrawSetTextFont(m_hTextFont);

	wchar_t aspd[64];
	V_swprintf_safe(aspd,L"Attack Speed: %.0f", m_flGetPlayerAttackSpeedMod*100);
	m_StatsInfo->SetText(aspd);

	wchar_t bdmg[64];
	V_swprintf_safe(bdmg, L"Base Damage: %.0f", m_flPlayerBaseDamage);
	m_StatsBaseDamage->SetText(bdmg);

}

void CHudSkillInfo::OnMouseReleased(vgui::MouseCode code)
{

}


CON_COMMAND(ToggleSkillInfo, "Toggles myPanel on or off")
{
	if (cl_showskillinfo.GetBool() == true)
	{
		cl_showskillinfo.SetValue(0);
	}
	else
	{
		cl_showskillinfo.SetValue(1);
	}
	

};

void CHudSkillInfo::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);
	if (!Q_stricmp(pcCommand, "turnoff"))
		cl_showskillinfo.SetValue(0);
}