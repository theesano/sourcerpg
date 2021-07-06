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

void CSkillPanel::Paint()
{
	surface()->DrawSetTexture(m_BgImage);
	surface()->DrawTexturedRect(m_iBgImageX, m_iBgImageY, m_iBgImageWide, m_iBgImageTall);

	surface()->DrawSetTextFont(m_hTextFont);

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