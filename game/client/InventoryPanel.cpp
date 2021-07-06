#include "cbase.h"
#include "IInventoryPanel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include "vgui/ISurface.h"
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>

class CInventoryPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CInventoryPanel, vgui::Frame);
	//CInventoryPanel : This class / vgui::Frame : BaseClass

	CInventoryPanel(vgui::VPANEL parent); //Constructor
	~CInventoryPanel(){}; //Destructor

	virtual void	OnMouseReleased(vgui::MouseCode code);
public:
	void CInventoryPanel::OnKeyCodePressed(vgui::KeyCode code);

protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);
	virtual void Paint();
	virtual void OnThink();

private:
	//Other used VGUI control Elements:
	Button *m_pCloseButton;
	//ImagePanel *bgImagePanel = new ImagePanel(this, "icon_tornado");

	CPanelAnimationVarAliasType(int, m_iBgImageX, "BgImageX", "0", "proportional_int");
	CPanelAnimationVarAliasType(int, m_iBgImageY, "BgImageY", "0", "proportional_int");
	CPanelAnimationVar(int, m_iBgImageWide, "BgImageWidth", "256");
	CPanelAnimationVar(int, m_iBgImageTall, "BgImageHeight", "512");
	CPanelAnimationVarAliasType(int, m_BgImage, "bgimage", "vgui/inventorypanel", "textureid");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Trebuchet18");

};

//Constructor: Initialize the Panel
CInventoryPanel::CInventoryPanel(vgui::VPANEL parent)
	:BaseClass(NULL, "InventoryPanel")
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

	LoadControlSettings("resource/UI/inventorypanel.res");

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

	//bgImagePanel->SetImage(scheme()->GetImage("icon_tornado", false));
	//bgImagePanel->SetPos(32, 48);
	//bgImagePanel->SetSize(48, 48);
	//bgImagePanel->SetShouldScaleImage(true);

	m_pCloseButton = new Button(this, "Button", "", this, "turnoff");
	m_pCloseButton->SetPos(216, 12);
	m_pCloseButton->SetSize(32, 24);
	m_pCloseButton->SetDepressedSound("common/bugreporter_succeeded.wav");
	m_pCloseButton->SetReleasedSound("ui/buttonclick.wav");
}

//Class: CInventoryPanelInterface Class. Used for construction.
class CInventoryPanelInterface : public IInventoryPanel
{
private:
	CInventoryPanel *InventoryPanel;
public:
	CInventoryPanelInterface()
	{
		InventoryPanel = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		InventoryPanel = new CInventoryPanel(parent);
	}
	void Destroy()
	{
		if (InventoryPanel)
		{
			InventoryPanel->SetParent((vgui::Panel *)NULL);
			delete InventoryPanel;
		}
	}
	void Activate(void)
	{
		if (inventorypanel)
		{
			inventorypanel->Activate();
		}
	}
};
static CInventoryPanelInterface g_InventoryPanel;
IInventoryPanel* inventorypanel = (IInventoryPanel*)&g_InventoryPanel;

ConVar cl_showinventorypanel("cl_showinventorypanel", "0", FCVAR_CLIENTDLL, "Sets the state of the skills information panel <state>");

void CInventoryPanel::OnTick()
{

	BaseClass::OnTick();
	SetVisible(cl_showinventorypanel.GetBool());

}

void CInventoryPanel::OnThink()
{

}

void CInventoryPanel::OnKeyCodePressed(vgui::KeyCode code)
{
	if (code == KEY_I)
	{
		if (cl_showinventorypanel.GetBool() == true)
		{
			cl_showinventorypanel.SetValue(0);
		}
		else
		{
			cl_showinventorypanel.SetValue(1);
		}
	}

	if (code == KEY_K)
	{
		ConVar *pSkillPanel = cvar->FindVar("cl_showskillpanel");
		if (pSkillPanel->GetInt() == 1)
		{
			pSkillPanel->SetValue(0);
		}
		else
		{
			pSkillPanel->SetValue(1);
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

void CInventoryPanel::Paint()
{
	surface()->DrawSetTexture(m_BgImage);
	surface()->DrawTexturedRect(m_iBgImageX, m_iBgImageY, m_iBgImageWide, m_iBgImageTall);

	surface()->DrawSetTextFont(m_hTextFont);

}

void CInventoryPanel::OnMouseReleased(vgui::MouseCode code)
{

}


CON_COMMAND(ToggleInventoryPanel, "Toggles Inventory on or off")
{
	if (cl_showinventorypanel.GetBool() == true)
	{
		cl_showinventorypanel.SetValue(0);
	}
	else
	{
		cl_showinventorypanel.SetValue(1);
	}


};

void CInventoryPanel::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);
	if (!Q_stricmp(pcCommand, "turnoff"))
		cl_showinventorypanel.SetValue(0);
}