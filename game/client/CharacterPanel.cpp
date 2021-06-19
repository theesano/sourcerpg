#include "cbase.h"
#include "ICharacterPanel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include "vgui/ISurface.h"
//#include <vgui_controls/ImagePanel.h>
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
	Label *m_StatsInfo;
	Label *m_StatsBaseDamage;
	Label *m_pUtilSlotLabel;
	//ImagePanel* imagePanel = new ImagePanel(this, "myPanel");
	Button *m_pCloseButton;
	Button *m_pAspdUpButton;
	Button *m_pAspdDownButton;
	Button *m_pHardModeButton;
	Button *m_pNormalModeButton;
	ComboBox *m_pUtilSlot1OptionIDCombo;
	ComboBox *m_pUtilSlot2OptionIDCombo;
	ComboBox *m_pUtilSlot3OptionIDCombo;
	ComboBox *m_pUtilSlot4OptionIDCombo;
	Panel *m_TestPanel;
	

	MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);
	CPanelAnimationVarAliasType(int, m_iBgImageX, "BgImageX", "0", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iBgImageY, "BgImageY", "0", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iBgImageWide, "BgImageWidth", "312", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iBgImageTall, "BgImageHeight", "312", "proportional_int");
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

	m_StatsInfo = new Label(this, "AttackSpeedDisp", "aspd");
	m_StatsInfo->SetPos(64,76);
	m_StatsInfo->SetFont(m_hTextFont);
	m_StatsInfo->SetWide(128);

	m_StatsBaseDamage = new Label(this, "BaseDamage", "bdmg");
	m_StatsBaseDamage->SetPos(64,64);
	m_StatsBaseDamage->SetFont(m_hTextFont);
	m_StatsBaseDamage->SetWide(128);

	m_pUtilSlotLabel = new Label(this, "UtilSlotGeneral", "uslot");
	m_pUtilSlotLabel->SetPos(64, 180);
	m_pUtilSlotLabel->SetFont(m_hTextFont);
	m_pUtilSlotLabel->SetWide(96);

	m_pAspdUpButton = new Button(this, "ButtonIncreaseASPD", "", this);
	m_pAspdUpButton->SetPos(175,75);
	m_pAspdUpButton->SetText("Increase");

	m_pAspdDownButton = new Button(this, "ButtonDecreaseASPD", "", this);
	m_pAspdDownButton->SetPos(175,100);
	m_pAspdDownButton->SetText("Decrease");

	m_pNormalModeButton = new Button(this, "ButtonNormalMode", "", this);
	m_pNormalModeButton->SetPos(175, 130);
	m_pNormalModeButton->SetText("Normal Difficulty");
	m_pNormalModeButton->SetDepressedSound("common/bugreporter_succeeded.wav");
	m_pNormalModeButton->SetReleasedSound("ui/buttonclick.wav");
	m_pNormalModeButton->SetWide(112);

	m_pHardModeButton = new Button(this, "ButtonHardMode", "", this);
	m_pHardModeButton->SetPos(175, 155);
	m_pHardModeButton->SetText("Very Hard Difficulty");
	m_pHardModeButton->SetDepressedSound("common/bugreporter_succeeded.wav");
	m_pHardModeButton->SetReleasedSound("ui/buttonclick.wav");
	m_pHardModeButton->SetWide(150);


//Utility slot
//1
	m_pUtilSlot1OptionIDCombo = new ComboBox(this, "UtilSlot1OptionIDCombo", 5, false);
	m_pUtilSlot1OptionIDCombo->SetPos(150, 190);
	m_pUtilSlot1OptionIDCombo->SetSize(116, 18);

	int defaultItem = m_pUtilSlot1OptionIDCombo->AddItem("None", NULL);

	m_pUtilSlot1OptionIDCombo->AddItem("1", NULL);
	m_pUtilSlot1OptionIDCombo->AddItem("2", NULL);
	m_pUtilSlot1OptionIDCombo->AddItem("3", NULL);
	m_pUtilSlot1OptionIDCombo->AddItem("4", NULL);

	m_pUtilSlot1OptionIDCombo->ActivateItem(defaultItem);
//2
	m_pUtilSlot2OptionIDCombo = new ComboBox(this, "UtilSlot2OptionIDCombo", 5, false);
	m_pUtilSlot2OptionIDCombo->SetPos(150, 220);
	m_pUtilSlot2OptionIDCombo->SetSize(116, 18);

	int defaultItemSlot2 = m_pUtilSlot2OptionIDCombo->AddItem("None", NULL);

	m_pUtilSlot2OptionIDCombo->AddItem("1", NULL);
	m_pUtilSlot2OptionIDCombo->AddItem("2", NULL);
	m_pUtilSlot2OptionIDCombo->AddItem("3", NULL);
	m_pUtilSlot2OptionIDCombo->AddItem("4", NULL);

	m_pUtilSlot2OptionIDCombo->ActivateItem(defaultItemSlot2);
//3
	m_pUtilSlot3OptionIDCombo = new ComboBox(this, "UtilSlot3OptionIDCombo", 5, false);
	m_pUtilSlot3OptionIDCombo->SetPos(150, 250);
	m_pUtilSlot3OptionIDCombo->SetSize(116, 18);

	int defaultItemSlot3 = m_pUtilSlot3OptionIDCombo->AddItem("None", NULL);

	m_pUtilSlot3OptionIDCombo->AddItem("1", NULL);
	m_pUtilSlot3OptionIDCombo->AddItem("2", NULL);
	m_pUtilSlot3OptionIDCombo->AddItem("3", NULL);
	m_pUtilSlot3OptionIDCombo->AddItem("4", NULL);

	m_pUtilSlot3OptionIDCombo->ActivateItem(defaultItemSlot3);

//4
	m_pUtilSlot4OptionIDCombo = new ComboBox(this, "UtilSlot4OptionIDCombo", 5, false);
	m_pUtilSlot4OptionIDCombo->SetPos(150, 280);
	m_pUtilSlot4OptionIDCombo->SetSize(116, 18);

	int defaultItemSlot4 = m_pUtilSlot4OptionIDCombo->AddItem("None", NULL);

	m_pUtilSlot4OptionIDCombo->AddItem("1", NULL);
	m_pUtilSlot4OptionIDCombo->AddItem("2", NULL);
	m_pUtilSlot4OptionIDCombo->AddItem("3", NULL);
	m_pUtilSlot4OptionIDCombo->AddItem("4", NULL);

	m_pUtilSlot4OptionIDCombo->ActivateItem(defaultItemSlot4);

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

ConVar cl_showcharacterpanel("cl_showcharacterpanel", "0", FCVAR_CLIENTDLL, "Sets the state of the skills information panel <state>");

void CCharacterPanel::OnTick()
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
	SetVisible(cl_showcharacterpanel.GetBool()); 

}

void CCharacterPanel::OnThink()
{
	ConVar *pGetMetropoliceStats = cvar->FindVar("metropolice_move_and_melee");

	ConVar *pGetNPCHpKnockback = cvar->FindVar("sk_npcknockbackathealth");

	if (m_pNormalModeButton->IsDepressed())
	{
		pGetMetropoliceStats->SetValue("0");
		pGetNPCHpKnockback->SetValue("100");
	}

	if (m_pHardModeButton->IsDepressed())
	{
		pGetMetropoliceStats->SetValue("1");
		pGetNPCHpKnockback->SetValue("50");
	}

	ConVar *pGetPlayerBaseDamage = cvar->FindVar("sk_plr_dmg_melee");
	m_flPlayerBaseDamage = pGetPlayerBaseDamage->GetFloat();
	
}

void CCharacterPanel::OnKeyCodePressed(vgui::KeyCode code)
{
	if (code == KEY_K)
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
	ConVar *pUtilSlot1OptionID = cvar->FindVar("sk_plr_utilslot1_option_id");
	ConVar *pUtilSlot2OptionID = cvar->FindVar("sk_plr_utilslot2_option_id");
	ConVar *pUtilSlot3OptionID = cvar->FindVar("sk_plr_utilslot3_option_id");
	ConVar *pUtilSlot4OptionID = cvar->FindVar("sk_plr_utilslot4_option_id");
//combobox control
//1
	if (panel == m_pUtilSlot1OptionIDCombo)
	{
		char buf[40];
		m_pUtilSlot1OptionIDCombo->GetText(buf, 40);
		if (stricmp(buf, "None") != 0)
		{
			Msg("Utility Slot 1 Changed \n");
			pUtilSlot1OptionID->SetValue(buf);
		}
		else
		{
			m_pUtilSlot1OptionIDCombo->ActivateItemByRow(0);
			Msg("Utility Slot 1 Option NONE \n");
			pUtilSlot1OptionID->SetValue(0);
		}

	}
//2
	if (panel == m_pUtilSlot2OptionIDCombo)
	{
		char buf[40];
		m_pUtilSlot2OptionIDCombo->GetText(buf, 40);
		if (stricmp(buf, "None") != 0)
		{
			Msg("Utility Slot 2 Changed \n");
			pUtilSlot2OptionID->SetValue(buf);
		}
		else
		{
			m_pUtilSlot2OptionIDCombo->ActivateItemByRow(0);
			Msg("Utility Slot 2 Option NONE \n");
			pUtilSlot2OptionID->SetValue(0);
		}

	}
//3
	if (panel == m_pUtilSlot3OptionIDCombo)
	{
		char buf[40];
		m_pUtilSlot3OptionIDCombo->GetText(buf, 40);
		if (stricmp(buf, "None") != 0)
		{
			Msg("Utility Slot 3 Changed \n");
			pUtilSlot3OptionID->SetValue(buf);
		}
		else
		{
			m_pUtilSlot3OptionIDCombo->ActivateItemByRow(0);
			Msg("Utility Slot 3 Option NONE \n");
			pUtilSlot3OptionID->SetValue(0);
		}

	}
//4
	if (panel == m_pUtilSlot4OptionIDCombo)
	{
		char buf[40];
		m_pUtilSlot4OptionIDCombo->GetText(buf, 40);
		if (stricmp(buf, "None") != 0)
		{
			Msg("Utility Slot 4 Changed \n");
			pUtilSlot4OptionID->SetValue(buf);
		}
		else
		{
			m_pUtilSlot4OptionIDCombo->ActivateItemByRow(0);
			Msg("Utility Slot 4 Option NONE \n");
			pUtilSlot4OptionID->SetValue(0);
		}

	}

}

void CCharacterPanel::Paint()
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

	m_pUtilSlotLabel->SetText("Utility Slot");

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