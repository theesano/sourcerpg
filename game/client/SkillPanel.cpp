#include "cbase.h"
#include "ISkillPanel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include "vgui/ISurface.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>


//CSkillPanel class:
class CSkillPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CSkillPanel, vgui::Frame);
	//CSkillPanel : This class / vgui::Frame : BaseClass

	CSkillPanel(vgui::VPANEL parent); //Constructor
	~CSkillPanel(){}; //Destructor

	virtual void	OnMouseReleased(vgui::MouseCode code);
public:
	void CSkillPanel::OnKeyCodePressed(vgui::KeyCode code);

protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);
	virtual void Paint();
	virtual void OnThink();

private:
	
	//int m_iQuickSlot1SkillID;
	bool m_bQSCombo1SetDefault;
	bool m_bQSCombo2SetDefault;
	bool m_bQSCombo3SetDefault;
	bool m_bQSCombo4SetDefault;
	bool m_bQSCombo5SetDefault;
	bool m_bQSCombo6SetDefault;

	//Other used VGUI control Elements:
	Button *m_pCloseButton;
	ImagePanel *bgImagePanel = new ImagePanel(this, "icon_tornado");
	Label *m_pUtilSlotLabel;
	Label *m_pActiveSkillsLabel;
	Label *m_pPassiveSkillsLabel;
	ComboBox *m_pUtilSlot1OptionIDCombo;
	ComboBox *m_pUtilSlot2OptionIDCombo;
	ComboBox *m_pUtilSlot3OptionIDCombo;
	ComboBox *m_pUtilSlot4OptionIDCombo;
	ComboBox *m_pQuickslot1Combo;
	ComboBox *m_pQuickslot2Combo;
	ComboBox *m_pQuickslot3Combo;
	ComboBox *m_pQuickslot4Combo;
	ComboBox *m_pQuickslot5Combo;
	ComboBox *m_pQuickslot6Combo;
	ImagePanel *m_pReceiver = new ImagePanel(this, "icon_test");

	MESSAGE_FUNC_PTR(OnTextChanged, "TextChanged", panel);
	CPanelAnimationVarAliasType(int, m_iBgImageX, "BgImageX", "0", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iBgImageY, "BgImageY", "0", "proportional_int");
	CPanelAnimationVar(int, m_iBgImageWide, "BgImageWidth", "512");
	CPanelAnimationVar(int, m_iBgImageTall, "BgImageHeight", "512");
	CPanelAnimationVarAliasType(int, m_BgImage, "bgimage", "vgui/skillspanel", "textureid");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Trebuchet18");

};

//Constructor: Initialize the Panel
CSkillPanel::CSkillPanel(vgui::VPANEL parent)
	:BaseClass(NULL, "SkillPanel")
{
	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);
	SetPaintBackgroundEnabled(false);
	SetProportional(false);
	SetTitleBarVisible(true);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(true); // I want it to be movable later on
	SetVisible(true);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

	LoadControlSettings("resource/UI/skillpanel.res");

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

	bgImagePanel->SetImage(scheme()->GetImage("icon_tornado", false));
	bgImagePanel->SetPos(32, 240);
	bgImagePanel->SetSize(48, 48);
	bgImagePanel->SetShouldScaleImage(true);
	bgImagePanel->SetDragEnabled(true);
	
	m_pReceiver->SetImage(scheme()->GetImage("None", false));
	m_pReceiver->SetPos(96, 240);
	m_pReceiver->SetSize(48, 48);
	m_pReceiver->SetShouldScaleImage(true);
	m_pReceiver->SetDropEnabled(true, 0.0f);
//Close button
	m_pCloseButton = new Button(this, "Button", "", this, "turnoff");
	m_pCloseButton->SetPos(476, 12);
	m_pCloseButton->SetSize(32, 24);
	m_pCloseButton->SetDepressedSound("common/bugreporter_succeeded.wav");
	m_pCloseButton->SetReleasedSound("ui/buttonclick.wav");

//Labels
	m_pUtilSlotLabel = new Label(this, "UtilSlotGeneral", "uslot");
	m_pUtilSlotLabel->SetPos(375, 32);
	m_pUtilSlotLabel->SetFont(m_hTextFont);
	m_pUtilSlotLabel->SetWide(96);

	m_pActiveSkillsLabel = new Label(this, "ActiveSkillGeneral", "Active Skills");
	m_pActiveSkillsLabel->SetPos(32, 32);
	m_pActiveSkillsLabel->SetFont(m_hTextFont);
	m_pActiveSkillsLabel->SetWide(128);

	m_pPassiveSkillsLabel = new Label(this, "PassiveSkillGeneral", "Passive Skills");
	m_pPassiveSkillsLabel->SetPos(32, 300);
	m_pPassiveSkillsLabel->SetFont(m_hTextFont);
	m_pPassiveSkillsLabel->SetWide(128);


//Utility slot
//1
	m_pUtilSlot1OptionIDCombo = new ComboBox(this, "UtilSlot1OptionIDCombo", 5, false);
	m_pUtilSlot1OptionIDCombo->SetPos(375, 96);
	m_pUtilSlot1OptionIDCombo->SetSize(116, 18);

	int defaultItem = m_pUtilSlot1OptionIDCombo->AddItem("None", NULL);

	m_pUtilSlot1OptionIDCombo->AddItem("1", NULL);
	m_pUtilSlot1OptionIDCombo->AddItem("2", NULL);
	m_pUtilSlot1OptionIDCombo->AddItem("3", NULL);
	m_pUtilSlot1OptionIDCombo->AddItem("4", NULL);

	m_pUtilSlot1OptionIDCombo->ActivateItem(defaultItem);
//2
	m_pUtilSlot2OptionIDCombo = new ComboBox(this, "UtilSlot2OptionIDCombo", 5, false);
	m_pUtilSlot2OptionIDCombo->SetPos(375, 126);
	m_pUtilSlot2OptionIDCombo->SetSize(116, 18);

	int defaultItemSlot2 = m_pUtilSlot2OptionIDCombo->AddItem("None", NULL);

	m_pUtilSlot2OptionIDCombo->AddItem("1", NULL);
	m_pUtilSlot2OptionIDCombo->AddItem("2", NULL);
	m_pUtilSlot2OptionIDCombo->AddItem("3", NULL);
	m_pUtilSlot2OptionIDCombo->AddItem("4", NULL);

	m_pUtilSlot2OptionIDCombo->ActivateItem(defaultItemSlot2);
//3
	m_pUtilSlot3OptionIDCombo = new ComboBox(this, "UtilSlot3OptionIDCombo", 5, false);
	m_pUtilSlot3OptionIDCombo->SetPos(375, 156);
	m_pUtilSlot3OptionIDCombo->SetSize(116, 18);

	int defaultItemSlot3 = m_pUtilSlot3OptionIDCombo->AddItem("None", NULL);

	m_pUtilSlot3OptionIDCombo->AddItem("1", NULL);
	m_pUtilSlot3OptionIDCombo->AddItem("2", NULL);
	m_pUtilSlot3OptionIDCombo->AddItem("3", NULL);
	m_pUtilSlot3OptionIDCombo->AddItem("4", NULL);

	m_pUtilSlot3OptionIDCombo->ActivateItem(defaultItemSlot3);

//4
	m_pUtilSlot4OptionIDCombo = new ComboBox(this, "UtilSlot4OptionIDCombo", 5, false);
	m_pUtilSlot4OptionIDCombo->SetPos(375, 186);
	m_pUtilSlot4OptionIDCombo->SetSize(116, 18);

	int defaultItemSlot4 = m_pUtilSlot4OptionIDCombo->AddItem("None", NULL);

	m_pUtilSlot4OptionIDCombo->AddItem("1", NULL);
	m_pUtilSlot4OptionIDCombo->AddItem("2", NULL);
	m_pUtilSlot4OptionIDCombo->AddItem("3", NULL);
	m_pUtilSlot4OptionIDCombo->AddItem("4", NULL);

	m_pUtilSlot4OptionIDCombo->ActivateItem(defaultItemSlot4);

//Quickslot 
	
//1
	m_pQuickslot1Combo = new ComboBox(this, "Quickslot1Combo", 7, false);
	m_pQuickslot1Combo->SetPos(32, 64);
	m_pQuickslot1Combo->SetSize(56, 16);
	
	m_pQuickslot1Combo->AddItem("None",NULL);
	m_pQuickslot1Combo->AddItem("2", NULL);
	m_pQuickslot1Combo->AddItem("3", NULL);
	m_pQuickslot1Combo->AddItem("4", NULL);
	m_pQuickslot1Combo->AddItem("5", NULL);
	m_pQuickslot1Combo->AddItem("6", NULL);
	m_pQuickslot1Combo->AddItem("7", NULL);


//2
	m_pQuickslot2Combo = new ComboBox(this, "Quickslot2Combo", 7, false);
	m_pQuickslot2Combo->SetPos(96, 64);
	m_pQuickslot2Combo->SetSize(56, 16);

	m_pQuickslot2Combo->AddItem("None", NULL);
	m_pQuickslot2Combo->AddItem("2", NULL);
	m_pQuickslot2Combo->AddItem("3", NULL);
	m_pQuickslot2Combo->AddItem("4", NULL);
	m_pQuickslot2Combo->AddItem("5", NULL);
	m_pQuickslot2Combo->AddItem("6", NULL);
	m_pQuickslot2Combo->AddItem("7", NULL);

//3

	m_pQuickslot3Combo = new ComboBox(this, "Quickslot3Combo", 7, false);
	m_pQuickslot3Combo->SetPos(160, 64);
	m_pQuickslot3Combo->SetSize(56, 16);

	m_pQuickslot3Combo->AddItem("None", NULL);
	m_pQuickslot3Combo->AddItem("2", NULL);
	m_pQuickslot3Combo->AddItem("3", NULL);
	m_pQuickslot3Combo->AddItem("4", NULL);
	m_pQuickslot3Combo->AddItem("5", NULL);
	m_pQuickslot3Combo->AddItem("6", NULL);
	m_pQuickslot3Combo->AddItem("7", NULL);

	//1
	m_pQuickslot1Combo = new ComboBox(this, "Quickslot1Combo", 7, false);
	m_pQuickslot1Combo->SetPos(32, 64);
	m_pQuickslot1Combo->SetSize(56, 16);
	
	m_pQuickslot1Combo->AddItem("None",NULL);
	m_pQuickslot1Combo->AddItem("2", NULL);
	m_pQuickslot1Combo->AddItem("3", NULL);
	m_pQuickslot1Combo->AddItem("4", NULL);
	m_pQuickslot1Combo->AddItem("5", NULL);
	m_pQuickslot1Combo->AddItem("6", NULL);
	m_pQuickslot1Combo->AddItem("7", NULL);


//2
	m_pQuickslot2Combo = new ComboBox(this, "Quickslot2Combo", 7, false);
	m_pQuickslot2Combo->SetPos(96, 64);
	m_pQuickslot2Combo->SetSize(56, 16);

	m_pQuickslot2Combo->AddItem("None", NULL);
	m_pQuickslot2Combo->AddItem("2", NULL);
	m_pQuickslot2Combo->AddItem("3", NULL);
	m_pQuickslot2Combo->AddItem("4", NULL);
	m_pQuickslot2Combo->AddItem("5", NULL);
	m_pQuickslot2Combo->AddItem("6", NULL);
	m_pQuickslot2Combo->AddItem("7", NULL);

//3

	m_pQuickslot3Combo = new ComboBox(this, "Quickslot3Combo", 7, false);
	m_pQuickslot3Combo->SetPos(160, 64);
	m_pQuickslot3Combo->SetSize(56, 16);

	m_pQuickslot3Combo->AddItem("None", NULL);
	m_pQuickslot3Combo->AddItem("2", NULL);
	m_pQuickslot3Combo->AddItem("3", NULL);
	m_pQuickslot3Combo->AddItem("4", NULL);
	m_pQuickslot3Combo->AddItem("5", NULL);
	m_pQuickslot3Combo->AddItem("6", NULL);
	m_pQuickslot3Combo->AddItem("7", NULL);

//1
	m_pQuickslot4Combo = new ComboBox(this, "Quickslot4Combo", 7, false);
	m_pQuickslot4Combo->SetPos(224, 64);
	m_pQuickslot4Combo->SetSize(56, 16);

	m_pQuickslot4Combo->AddItem("None", NULL);
	m_pQuickslot4Combo->AddItem("2", NULL);
	m_pQuickslot4Combo->AddItem("3", NULL);
	m_pQuickslot4Combo->AddItem("4", NULL);
	m_pQuickslot4Combo->AddItem("5", NULL);
	m_pQuickslot4Combo->AddItem("6", NULL);
	m_pQuickslot4Combo->AddItem("7", NULL);


//2
	m_pQuickslot5Combo = new ComboBox(this, "Quickslot5Combo", 7, false);
	m_pQuickslot5Combo->SetPos(288, 64);
	m_pQuickslot5Combo->SetSize(56, 16);

	m_pQuickslot5Combo->AddItem("None", NULL);
	m_pQuickslot5Combo->AddItem("2", NULL);
	m_pQuickslot5Combo->AddItem("3", NULL);
	m_pQuickslot5Combo->AddItem("4", NULL);
	m_pQuickslot5Combo->AddItem("5", NULL);
	m_pQuickslot5Combo->AddItem("6", NULL);
	m_pQuickslot5Combo->AddItem("7", NULL);

//3

	m_pQuickslot6Combo = new ComboBox(this, "Quickslot6Combo", 7, false);
	m_pQuickslot6Combo->SetPos(352, 64);
	m_pQuickslot6Combo->SetSize(56, 16);

	m_pQuickslot6Combo->AddItem("None", NULL);
	m_pQuickslot6Combo->AddItem("2", NULL);
	m_pQuickslot6Combo->AddItem("3", NULL);
	m_pQuickslot6Combo->AddItem("4", NULL);
	m_pQuickslot6Combo->AddItem("5", NULL);
	m_pQuickslot6Combo->AddItem("6", NULL);
	m_pQuickslot6Combo->AddItem("7", NULL);
}

//Class: CSkillPanelInterface Class. Used for construction.
class CSkillPanelInterface : public ISkillPanel
{
private:
	CSkillPanel *SkillPanel;
public:
	CSkillPanelInterface()
	{
		SkillPanel = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		SkillPanel = new CSkillPanel(parent);
	}
	void Destroy()
	{
		if (SkillPanel)
		{
			SkillPanel->SetParent((vgui::Panel *)NULL);
			delete SkillPanel;
		}
	}
	void Activate(void)
	{
		if (skillpanel)
		{
			skillpanel->Activate();
		}
	}
};
static CSkillPanelInterface g_SkillPanel;
ISkillPanel* skillpanel = (ISkillPanel*)&g_SkillPanel;

ConVar cl_showskillpanel("cl_showskillpanel", "0", FCVAR_CLIENTDLL, "Sets the state of the skills information panel <state>");

void CSkillPanel::OnTick()
{	
	BaseClass::OnTick();
	SetVisible(cl_showskillpanel.GetBool());

}

void CSkillPanel::OnThink()
{
	if ((m_bQSCombo1SetDefault) && (engine->IsConnected()))
	{
		m_bQSCombo1SetDefault = false;

		ConVar *pQuickslot1SkillID = cvar->FindVar("sk_plr_quickslot1_skill_id");

		switch (pQuickslot1SkillID->GetInt())
		{
			case 0:
				m_pQuickslot1Combo->ActivateItem(0);
				break;
			case 2:
				m_pQuickslot1Combo->ActivateItem(1);
				break;
			case 3:
				m_pQuickslot1Combo->ActivateItem(2);
				break;
			case 4:
				m_pQuickslot1Combo->ActivateItem(3);
				break;
			case 5:
				m_pQuickslot1Combo->ActivateItem(4);
				break;
			case 6:
				m_pQuickslot1Combo->ActivateItem(5);
				break;
			case 7:
				m_pQuickslot1Combo->ActivateItem(6);
				break;
			default:	m_pQuickslot1Combo->ActivateItem(0);
				
		}

		//DevMsg("current quickslot 1 item %i \n", m_pQuickslot1Combo->GetActiveItem());
	}

	if ((m_bQSCombo2SetDefault) && (engine->IsConnected()))
	{
		m_bQSCombo2SetDefault = false;

		ConVar *pQuickslot2SkillID = cvar->FindVar("sk_plr_quickslot2_skill_id");

		switch (pQuickslot2SkillID->GetInt())
		{
		case 0:
			m_pQuickslot2Combo->ActivateItem(0);
			break;
		case 2:
			m_pQuickslot2Combo->ActivateItem(1);
			break;
		case 3:
			m_pQuickslot2Combo->ActivateItem(2);
			break;
		case 4:
			m_pQuickslot2Combo->ActivateItem(3);
			break;
		case 5:
			m_pQuickslot2Combo->ActivateItem(4);
			break;
		case 6:
			m_pQuickslot2Combo->ActivateItem(5);
			break;
		case 7:
			m_pQuickslot2Combo->ActivateItem(6);
			break;
		default:	m_pQuickslot2Combo->ActivateItem(0);

		}

		//DevMsg("current quickslot 1 item %i \n", m_pQuickslot1Combo->GetActiveItem());
	}

	if ((m_bQSCombo3SetDefault) && (engine->IsConnected()))
	{
		m_bQSCombo3SetDefault = false;

		ConVar *pQuickslot3SkillID = cvar->FindVar("sk_plr_quickslot3_skill_id");

		switch (pQuickslot3SkillID->GetInt())
		{
		case 0:
			m_pQuickslot3Combo->ActivateItem(0);
			break;
		case 2:
			m_pQuickslot3Combo->ActivateItem(1);
			break;
		case 3:
			m_pQuickslot3Combo->ActivateItem(2);
			break;
		case 4:
			m_pQuickslot3Combo->ActivateItem(3);
			break;
		case 5:
			m_pQuickslot3Combo->ActivateItem(4);
			break;
		case 6:
			m_pQuickslot3Combo->ActivateItem(5);
			break;
		case 7:
			m_pQuickslot3Combo->ActivateItem(6);
			break;
		default:	m_pQuickslot3Combo->ActivateItem(0);

		}

		//DevMsg("current quickslot 1 item %i \n", m_pQuickslot1Combo->GetActiveItem());


	}

	if ((m_bQSCombo4SetDefault) && (engine->IsConnected()))
	{
		m_bQSCombo4SetDefault = false;

		ConVar *pQuickslot4SkillID = cvar->FindVar("sk_plr_quickslot4_skill_id");

		switch (pQuickslot4SkillID->GetInt())
		{
		case 0:
			m_pQuickslot4Combo->ActivateItem(0);
			break;
		case 2:
			m_pQuickslot4Combo->ActivateItem(1);
			break;
		case 3:
			m_pQuickslot4Combo->ActivateItem(2);
			break;
		case 4:
			m_pQuickslot4Combo->ActivateItem(3);
			break;
		case 5:
			m_pQuickslot4Combo->ActivateItem(4);
			break;
		case 6:
			m_pQuickslot4Combo->ActivateItem(5);
			break;
		case 7:
			m_pQuickslot4Combo->ActivateItem(6);
			break;
		default:	m_pQuickslot4Combo->ActivateItem(0);

		}

		//DevMsg("current quickslot 1 item %i \n", m_pQuickslot1Combo->GetActiveItem());


	}

	if ((m_bQSCombo5SetDefault) && (engine->IsConnected()))
	{
		m_bQSCombo5SetDefault = false;

		ConVar *pQuickslot5SkillID = cvar->FindVar("sk_plr_quickslot5_skill_id");

		switch (pQuickslot5SkillID->GetInt())
		{
		case 0:
			m_pQuickslot5Combo->ActivateItem(0);
			break;
		case 2:
			m_pQuickslot5Combo->ActivateItem(1);
			break;
		case 3:
			m_pQuickslot5Combo->ActivateItem(2);
			break;
		case 4:
			m_pQuickslot5Combo->ActivateItem(3);
			break;
		case 5:
			m_pQuickslot5Combo->ActivateItem(4);
			break;
		case 6:
			m_pQuickslot5Combo->ActivateItem(5);
			break;
		case 7:
			m_pQuickslot5Combo->ActivateItem(6);
			break;
		default:	m_pQuickslot5Combo->ActivateItem(0);

		}

		//DevMsg("current quickslot 1 item %i \n", m_pQuickslot1Combo->GetActiveItem());


	}

	if ((m_bQSCombo6SetDefault) && (engine->IsConnected()))
	{
		m_bQSCombo6SetDefault = false;

		ConVar *pQuickslot6SkillID = cvar->FindVar("sk_plr_quickslot6_skill_id");

		switch (pQuickslot6SkillID->GetInt())
		{
		case 0:
			m_pQuickslot6Combo->ActivateItem(0);
			break;
		case 2:
			m_pQuickslot6Combo->ActivateItem(1);
			break;
		case 3:
			m_pQuickslot6Combo->ActivateItem(2);
			break;
		case 4:
			m_pQuickslot6Combo->ActivateItem(3);
			break;
		case 5:
			m_pQuickslot6Combo->ActivateItem(4);
			break;
		case 6:
			m_pQuickslot6Combo->ActivateItem(5);
			break;
		case 7:
			m_pQuickslot6Combo->ActivateItem(6);
			break;
		default:	m_pQuickslot6Combo->ActivateItem(0);

		}

		//DevMsg("current quickslot 1 item %i \n", m_pQuickslot1Combo->GetActiveItem());


	}
}

void CSkillPanel::OnKeyCodePressed(vgui::KeyCode code)
{
	if (code == KEY_K)
	{
		if (cl_showskillpanel.GetBool() == true)
		{
			cl_showskillpanel.SetValue(0);
		}
		else
		{
			cl_showskillpanel.SetValue(1);
		}
	}

	if (code == KEY_P)
	{
		ConVar *pCharacterPanel = cvar->FindVar("cl_showcharacterpanel");
		if (pCharacterPanel->GetInt() == 1)
		{
			pCharacterPanel->SetValue(0);
		}
		else
		{
			pCharacterPanel->SetValue(1);
		}
	}

}

void CSkillPanel::OnTextChanged(Panel *panel)
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

	}

//Quickslot 
	ConVar *pQuickslot1SkillID = cvar->FindVar("sk_plr_quickslot1_skill_id");
	ConVar *pQuickslot2SkillID = cvar->FindVar("sk_plr_quickslot2_skill_id");
	ConVar *pQuickslot3SkillID = cvar->FindVar("sk_plr_quickslot3_skill_id");
	ConVar *pQuickslot4SkillID = cvar->FindVar("sk_plr_quickslot4_skill_id");
	ConVar *pQuickslot5SkillID = cvar->FindVar("sk_plr_quickslot5_skill_id");
	ConVar *pQuickslot6SkillID = cvar->FindVar("sk_plr_quickslot6_skill_id");

	//m_iQuickSlot1SkillID = pQuickslot1SkillID->GetInt();
//1
	if (engine->IsConnected())
	{
		if (panel == m_pQuickslot1Combo)
		{
			char buf[40];
			m_pQuickslot1Combo->GetText(buf, 40);
			if (stricmp(buf, "None") != 0)
			{
				Msg("QuickSlot 1 Changed \n");
				Msg("current quickslot 1 item %i \n", m_pQuickslot1Combo->GetActiveItem());
				pQuickslot1SkillID->SetValue(buf);
			}
			else
			{
				pQuickslot1SkillID->SetValue(0);
			}


		}

		//2
		if (panel == m_pQuickslot2Combo)
		{
			char buf[40];
			m_pQuickslot2Combo->GetText(buf, 40);
			if (stricmp(buf, "None") != 0)
			{
				Msg("QuickSlot 2 Changed \n");
				Msg("current quickslot 1 item %i \n", m_pQuickslot2Combo->GetActiveItem());
				pQuickslot2SkillID->SetValue(buf);
			}
			else
			{
				pQuickslot2SkillID->SetValue(0);
			}


		}
		//3

		if (panel == m_pQuickslot3Combo)
		{
			char buf[40];
			m_pQuickslot3Combo->GetText(buf, 40);
			if (stricmp(buf, "None") != 0)
			{
				Msg("QuickSlot 3 Changed \n");
				Msg("current quickslot 3 item %i \n", m_pQuickslot3Combo->GetActiveItem());
				pQuickslot3SkillID->SetValue(buf);
			}
			else
			{
				pQuickslot3SkillID->SetValue(0);
			}


		}

//4
		if (panel == m_pQuickslot4Combo)
		{
			char buf[40];
			m_pQuickslot4Combo->GetText(buf, 40);
			if (stricmp(buf, "None") != 0)
			{
				Msg("QuickSlot 4 Changed \n");
				Msg("current quickslot 4 item %i \n", m_pQuickslot4Combo->GetActiveItem());
				pQuickslot4SkillID->SetValue(buf);
			}
			else
			{
				pQuickslot4SkillID->SetValue(0);
			}


		}
//5
		if (panel == m_pQuickslot5Combo)
		{
			char buf[40];
			m_pQuickslot5Combo->GetText(buf, 40);
			if (stricmp(buf, "None") != 0)
			{
				Msg("QuickSlot 5 Changed \n");
				Msg("current quickslot 5 item %i \n", m_pQuickslot5Combo->GetActiveItem());
				pQuickslot5SkillID->SetValue(buf);
			}
			else
			{
				pQuickslot5SkillID->SetValue(0);
			}


		}
//6		
		if (panel == m_pQuickslot6Combo)
		{
			char buf[40];
			m_pQuickslot6Combo->GetText(buf, 40);
			if (stricmp(buf, "None") != 0)
			{
				Msg("QuickSlot 6 Changed \n");
				Msg("current quickslot 6 item %i \n", m_pQuickslot6Combo->GetActiveItem());
				pQuickslot6SkillID->SetValue(buf);
			}
			else
			{
				pQuickslot6SkillID->SetValue(0);
			}


		}

	}

}

void CSkillPanel::Paint()
{
	surface()->DrawSetTexture(m_BgImage);
	surface()->DrawTexturedRect(m_iBgImageX, m_iBgImageY, m_iBgImageWide, m_iBgImageTall);

	surface()->DrawSetTextFont(m_hTextFont);
	m_pUtilSlotLabel->SetText("Utility Slot");

}

void CSkillPanel::OnMouseReleased(vgui::MouseCode code)
{

}


CON_COMMAND(ToggleSkillPanel, "Toggles myPanel on or off")
{
	if (cl_showskillpanel.GetBool() == true)
	{
		cl_showskillpanel.SetValue(0);
	}
	else
	{
		cl_showskillpanel.SetValue(1);
	}


};

void CSkillPanel::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);
	if (!Q_stricmp(pcCommand, "turnoff"))
		cl_showskillpanel.SetValue(0);
}