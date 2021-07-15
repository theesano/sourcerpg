#include "cbase.h"
#include "hud_macros.h"
#include "ICharacterPanel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include "vgui/ISurface.h"
//#include <vgui_controls/ImagePanel.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>

//CCharacterPanel class:
class CCharacterPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CCharacterPanel, vgui::Frame);
	//CCharacterPanel : This class / vgui::Frame : BaseClass

	CCharacterPanel(vgui::VPANEL parent); //Constructor
	~CCharacterPanel(){}; //Destructor

	virtual void	OnMouseReleased(vgui::MouseCode code);
public:
	void CCharacterPanel::OnKeyCodePressed(vgui::KeyCode code);

protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);
	virtual void Paint();
	virtual void OnThink();

private:
	//Other used VGUI control Elements:
	Label *m_StatsBaseDamage;
	Label *m_StatsCritDamage;
	Label *m_StatsCritPer;
	Label *m_StatsAttackSpeed;
	Label *m_StatsCooldownReduction;
	Label *m_StatsMoveSpeed;
	//ImagePanel* imagePanel = new ImagePanel(this, "myPanel");
	Button *m_pCloseButton;
	Panel *m_TestPanel;
	

	MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);
	CPanelAnimationVarAliasType(int, m_iBgImageX, "BgImageX", "0", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iBgImageY, "BgImageY", "0", "proportional_int");
	CPanelAnimationVar(int, m_iBgImageWide, "BgImageWidth", "512");
	CPanelAnimationVar(int, m_iBgImageTall, "BgImageHeight", "512");
	CPanelAnimationVarAliasType(int, m_BgImage, "bgimage", "vgui/panel", "textureid");
	CPanelAnimationVarAliasType(int, m_iStatsAttackSpeedX, "StatsAttackSpeedX", "64", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iStatsAttackSpeedY, "StatsAttackSpeedY", "96", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iStatsAttackSpeedButtonX, "StatsAttackSpeedButtonX", "72", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iStatsAttackSpeedButtonY, "StatsAttackSpeedButtonY", "96", "proportional_int");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Trebuchet18");
	float m_flGetPlayerAttackSpeedMod;
	float m_flPlayerBaseDamage;
	float m_flWeaponDamage;
	float m_flPlayerCooldownReductionRate;
	float m_flPlayerMovementSpeed;
	float m_flPlayerCritDamage;
	int m_iPlayerCritPer;
};

//Constructor: Initialize the Panel
CCharacterPanel::CCharacterPanel(vgui::VPANEL parent)
	:BaseClass(NULL, "CharacterPanel")
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

	LoadControlSettings("resource/UI/characterpanel.res");

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

	DevMsg("CharacterPanel has been constructed \n");
	
	//imagePanel->SetImage(scheme()->GetImage("panel", false));

	m_StatsBaseDamage = new Label(this, "BaseDamage", "bdmg");
	m_StatsBaseDamage->SetPos(64,64);
	m_StatsBaseDamage->SetFont(m_hTextFont);
	m_StatsBaseDamage->SetWide(128);

	m_StatsCritDamage = new Label(this, "CritDamage", "critdmg");
	m_StatsCritDamage->SetPos(64, 76);
	m_StatsCritDamage->SetFont(m_hTextFont);
	m_StatsCritDamage->SetWide(144);

	m_StatsCritPer = new Label(this, "CritRate", "critrate");
	m_StatsCritPer->SetPos(64, 88);
	m_StatsCritPer->SetFont(m_hTextFont);
	m_StatsCritPer->SetWide(144);

	m_StatsAttackSpeed = new Label(this, "AttackSpeedDisp", "aspd");
	m_StatsAttackSpeed->SetPos(64, 100);
	m_StatsAttackSpeed->SetFont(m_hTextFont);
	m_StatsAttackSpeed->SetWide(128);

	m_StatsMoveSpeed = new Label(this, "MovementSpeedDisp", "move");
	m_StatsMoveSpeed->SetPos(64, 112);
	m_StatsMoveSpeed->SetFont(m_hTextFont);
	m_StatsMoveSpeed->SetWide(144);

	m_StatsCooldownReduction = new Label(this, "CooldownReductionDisp", "cd");
	m_StatsCooldownReduction->SetPos(64, 124);
	m_StatsCooldownReduction->SetFont(m_hTextFont);
	m_StatsCooldownReduction->SetWide(169);



}

//Class: CCharacterPanelInterface Class. Used for construction.
class CCharacterPanelInterface : public ICharacterPanel
{
private:
	CCharacterPanel *CharacterPanel;
public:
	CCharacterPanelInterface()
	{
		CharacterPanel = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		CharacterPanel = new CCharacterPanel(parent);
	}
	void Destroy()
	{
		if (CharacterPanel)
		{
			CharacterPanel->SetParent((vgui::Panel *)NULL);
			delete CharacterPanel;
		}
	}
	void Activate(void)
	{
		if (characterpanel)
		{
			characterpanel->Activate();
		}
	}
};
static CCharacterPanelInterface g_CharacterPanel;
ICharacterPanel* characterpanel = (ICharacterPanel*)&g_CharacterPanel;

ConVar cl_showcharacterpanel("cl_showcharacterpanel", "0", FCVAR_CLIENTDLL, "Sets the state of the character information panel <state>");

void CCharacterPanel::OnTick()
{
	ConVar *pGetPlayerAttackSpeedMod = cvar->FindVar("lilyss_player_attackspeed");
	m_flGetPlayerAttackSpeedMod = pGetPlayerAttackSpeedMod->GetFloat();

	ConVar *pGetPlayerCooldownRate = cvar->FindVar("lilyss_skills_cooldown_timereduction");
	m_flPlayerCooldownReductionRate = pGetPlayerCooldownRate->GetFloat();

	ConVar *pGetPlayerMovementSpeed = cvar->FindVar("hl2_normspeed");
	m_flPlayerMovementSpeed = pGetPlayerMovementSpeed->GetFloat();

	ConVar *pGetPlayerCritDamage = cvar->FindVar("lilyss_player_critdamage");
	m_flPlayerCritDamage = pGetPlayerCritDamage->GetFloat();

	ConVar *pGetPlayerCritPer = cvar->FindVar("lilyss_player_crit_per");
	m_iPlayerCritPer = pGetPlayerCritPer->GetInt();

	BaseClass::OnTick();
	SetVisible(cl_showcharacterpanel.GetBool()); 

}

void CCharacterPanel::OnThink()
{
	//Change to adjust player damage
	ConVar *pGetPlayerBaseDamage = cvar->FindVar("lilyss_player_basedamage");
	m_flPlayerBaseDamage = pGetPlayerBaseDamage->GetFloat();

	ConVar *pGetWeaponDamage = cvar->FindVar("sk_plr_dmg_melee");
	m_flWeaponDamage = pGetWeaponDamage->GetFloat();

	
}

void CCharacterPanel::OnKeyCodePressed(vgui::KeyCode code)
{
	if (code == KEY_P)
	{
		if (cl_showcharacterpanel.GetBool() == true)
		{
			cl_showcharacterpanel.SetValue(0);
		}
		else
		{
			cl_showcharacterpanel.SetValue(1);
		}
	}
}

void CCharacterPanel::OnTextChanged(Panel *panel)
{

}

void CCharacterPanel::Paint()
{
	surface()->DrawSetTexture(m_BgImage);
	surface()->DrawTexturedRect(m_iBgImageX, m_iBgImageY, m_iBgImageWide, m_iBgImageTall);
	
	surface()->DrawSetTextFont(m_hTextFont);

	wchar_t bdmg[64];
	V_swprintf_safe(bdmg, L"Damage: %.0f + %.0f", m_flPlayerBaseDamage,m_flWeaponDamage);
	m_StatsBaseDamage->SetText(bdmg);

	wchar_t critdmg[64];
	V_swprintf_safe(critdmg, L"Critical Damage: %.0f + %.0f", m_flPlayerCritDamage + m_flPlayerBaseDamage, m_flWeaponDamage);
	m_StatsCritDamage->SetText(critdmg);

	wchar_t critrate[64];
	V_swprintf_safe(critrate, L"Critical Rate: %i %%", m_iPlayerCritPer);
	m_StatsCritPer->SetText(critrate);

	wchar_t aspd[64];
	V_swprintf_safe(aspd, L"Attack Speed: %.0f %%", m_flGetPlayerAttackSpeedMod * 100);
	m_StatsAttackSpeed->SetText(aspd);

	wchar_t cdr[64];
	V_swprintf_safe(cdr, L"Cooldown Reduction: %.0f %%", m_flPlayerCooldownReductionRate*100);
	m_StatsCooldownReduction->SetText(cdr);

	wchar_t mvmtspd[64];
	V_swprintf_safe(mvmtspd, L"Movement Speed: %.0f %%", (m_flPlayerMovementSpeed/280)*100);
	m_StatsMoveSpeed->SetText(mvmtspd);



}

void CCharacterPanel::OnMouseReleased(vgui::MouseCode code)
{

}


CON_COMMAND(ToggleCharacterPanel, "Toggles myPanel on or off")
{
	if (cl_showcharacterpanel.GetBool() == true)
	{
		cl_showcharacterpanel.SetValue(0);
	}
	else
	{
		cl_showcharacterpanel.SetValue(1);
	}
	

};

void CCharacterPanel::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);
	if (!Q_stricmp(pcCommand, "turnoff"))
		cl_showcharacterpanel.SetValue(0);
}
