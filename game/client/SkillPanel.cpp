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
	//Other used VGUI control Elements:
	Button *m_pCloseButton;
	ImagePanel *bgImagePanel = new ImagePanel(this, "icon_tornado");
	Label *m_pUtilSlotLabel;
	ComboBox *m_pUtilSlot1OptionIDCombo;
	ComboBox *m_pUtilSlot2OptionIDCombo;
	ComboBox *m_pUtilSlot3OptionIDCombo;
	ComboBox *m_pUtilSlot4OptionIDCombo;

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
	bgImagePanel->SetPos(32, 48);
	bgImagePanel->SetSize(48, 48);
	bgImagePanel->SetShouldScaleImage(true);

	m_pCloseButton = new Button(this, "Button", "", this, "turnoff");
	m_pCloseButton->SetPos(476, 12);
	m_pCloseButton->SetSize(32, 24);
	m_pCloseButton->SetDepressedSound("common/bugreporter_succeeded.wav");
	m_pCloseButton->SetReleasedSound("ui/buttonclick.wav");

	m_pUtilSlotLabel = new Label(this, "UtilSlotGeneral", "uslot");
	m_pUtilSlotLabel->SetPos(64, 180);
	m_pUtilSlotLabel->SetFont(m_hTextFont);
	m_pUtilSlotLabel->SetWide(96);

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