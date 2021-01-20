#include "cbase.h"
#include "Ihudskillinfo.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>




//CHudSkillInfo class:
class CHudSkillInfo : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CHudSkillInfo, vgui::Frame);
	//CHUdSkillInfo : This class / vgui::Frame : BaseClass

	CHudSkillInfo(vgui::VPANEL parent); //Constructor
	~CHudSkillInfo(){}; //Destructor

protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);

private:
	//Other used VGUI control Elements:
	Label *m_skinfo1;

};

//Constructor: Initialize the Panel
CHudSkillInfo::CHudSkillInfo(vgui::VPANEL parent)
	:BaseClass(NULL, "HudSkillInfo")
{
	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);
	
	SetProportional(false);
	SetTitleBarVisible(false);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false); // I want it to be movable later on
	SetVisible(true);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

	LoadControlSettings("resource/UI/hudskillinfo.res");

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

	DevMsg("HudSkillInfo has been constructed \n");

	m_skinfo1 = new Label(this, "Skill1", "skill1");
	m_skinfo1->SetPos(100, 100);

	m_skinfo1 = new Label(this, "Skill2", "skill2");
	m_skinfo1->SetPos(100, 100);

	
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
	BaseClass::OnTick();
	SetVisible(cl_showskillinfo.GetBool()); //CL_SHOWMYPANEL / 1 BY DEFAULT

}


CON_COMMAND(ToggleSkillInfo, "Toggles myPanel on or off")
{
	cl_showskillinfo.SetValue(!cl_showskillinfo.GetBool());
	hudskillinfo->Activate();

};

void CHudSkillInfo::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);
	if (!Q_stricmp(pcCommand, "turnoff"))
		cl_showskillinfo.SetValue(0);
}