#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_skillcd.h" 
#include "iclientmode.h" 
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"
#include <vgui_controls/Label.h>
#include "IGameUIFuncs.h"
#include <vgui/IInput.h>
#include <vgui_controls/AnimationController.h>
#include <vgui/KeyCode.h>
#include "inputsystem/iinputsystem.h"



using namespace vgui;

#include "tier0/memdbgon.h" 

DECLARE_HUDELEMENT(CHudSkillCooldown);

# define HULL_INIT 80 

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudSkillCooldown::CHudSkillCooldown(const char * pElementName) :
CHudElement(pElementName), BaseClass(NULL, "HudSkillCooldown")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	//m_nSkillImage1 = surface()->CreateNewTextureID();
	//surface()->DrawSetTextureFile(m_nSkillImage1, "UI/skills/icon_tornado", true, true);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD);


}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudSkillCooldown::Init()
{
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudSkillCooldown::Reset(void)
{
	SetBgColor(Color(0, 0, 0, 128));
}


//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudSkillCooldown::OnThink(void)
{
	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();

	if (!local)
		return;
	ConVar *pGetPlayerMP = cvar->FindVar("sk_plr_current_mp");
	m_iGetPlayerMP = pGetPlayerMP->GetInt();

	ConVar *pSkill2cdtimer = cvar->FindVar("sk_plr_skills_2_cd");
	m_flHudSk2Timer = pSkill2cdtimer->GetInt();

	ConVar *pSkill3cdtimer = cvar->FindVar("sk_plr_skills_3_cd");
	m_flHudSk3Timer = pSkill3cdtimer->GetInt();

	ConVar *pSkill4cdtimer = cvar->FindVar("sk_plr_skills_4_cd");
	m_flHudSk4Timer = pSkill4cdtimer->GetInt();

	ConVar *pSkill5cdtimer = cvar->FindVar("sk_plr_skills_5_cd");
	m_flHudSk5Timer = pSkill5cdtimer->GetInt();

	ConVar *pSkill6cdtimer = cvar->FindVar("sk_plr_skills_6_cd");
	m_flHudSk6Timer = pSkill6cdtimer->GetInt();

	//Skill 2 cooldown has been defined using a different method below in Paint()

	if (m_flHudSk3Timer > 0)
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon2onCD");
	else if (m_iGetPlayerMP < 25)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon2NoMP");
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon2");

	}

	if (m_flHudSk4Timer > 0)
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon3onCD");
	else if (m_iGetPlayerMP < 50)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon3NoMP");
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon3");

	}
	
	if (m_flHudSk5Timer > 0)
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon4onCD");
	else if (m_iGetPlayerMP < 30)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon4NoMP");
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon4");

	}

	if (m_flHudSk6Timer > 0)
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon5onCD");
	else if (m_iGetPlayerMP < 50)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon5NoMP");
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon5");

	}


}


//------------------------------------------------------------------------
// Purpose: draws
//------------------------------------------------------------------------

void CHudSkillCooldown::Paint()
{

	SetPaintBorderEnabled(false);

	if (m_flHudSk2Timer > 0)
	{
		surface()->DrawSetTexture(m_nIconTextureId_2);
		surface()->DrawTexturedRect(m_iIconX, m_iIconY, m_iIconWide, m_iIconTall);
	}
	else if (m_iGetPlayerMP < 30)
	{
		surface()->DrawSetTexture(m_nIconTextureId_3);
		surface()->DrawTexturedRect(m_iIconX, m_iIconY, m_iIconWide, m_iIconTall);
	}
	else
	{
		surface()->DrawSetTexture(m_nIconTextureId);
		surface()->DrawTexturedRect(m_iIconX, m_iIconY, m_iIconWide, m_iIconTall);
	}

	
	surface()->DrawSetTexture(m_nIconTextureId2);
	surface()->DrawTexturedRect(m_iIconX2, m_iIconY2, m_iIconWide2, m_iIconTall2);

	surface()->DrawSetTexture(m_nIconTextureId3);
	surface()->DrawTexturedRect(m_iIconX3, m_iIconY3, m_iIconWide3, m_iIconTall3);

	surface()->DrawSetTexture(m_nIconTextureId4);
	surface()->DrawTexturedRect(m_iIconX4, m_iIconY4, m_iIconWide4, m_iIconTall4);
	
	surface()->DrawSetColor(m_Icon5Color);
	surface()->DrawSetTexture(m_nIconTextureId5);
	surface()->DrawTexturedRect(m_iIconX5, m_iIconY5, m_iIconWide5, m_iIconTall5);


	//TODO: read the keybind directly from the keyboard list.
	wchar_t sz[64];
	V_swprintf_safe(sz, L"%i",m_flHudSk2Timer);
	//Skill #2
	surface()->DrawSetTextFont(m_hTextFont);
	if (m_flHudSk2Timer > 0)
	{
		surface()->DrawSetTextColor(m_TextColor1);
		surface()->DrawSetTextPos(text_xpos, text_ypos);
		surface()->DrawPrintText(sz, wcslen(sz));

	}
	else
	{
		//Initialize the buttoncode to store the binding 
		ButtonCode_t keyname1;
		keyname1 = gameuifuncs->GetButtonCodeForBind("quickslot1"); //search for the key belongs to that binding

		//convert the key stored in the button code to string
		const char *keyNameSlot1 = g_pInputSystem->ButtonCodeToString(keyname1);
		V_swprintf_safe(sz, L"%s", keyNameSlot1); //convert the string to wchar_t for DrawPrintText 

		surface()->DrawSetTextColor(m_TextColor);
		surface()->DrawSetTextPos(text_xpos, text_ypos);
		surface()->DrawPrintText(sz, wcslen(sz));
	}

	//Skill #3
	V_swprintf_safe(sz, L"%i", m_flHudSk3Timer);
	if (m_flHudSk3Timer > 0)
	{
		surface()->DrawSetTextColor(m_TextColor2);
		surface()->DrawSetTextPos(text_xpos2, text_ypos2);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	else
	{
		//Initialize the buttoncode to store the binding 
		ButtonCode_t keyname2;
		keyname2 = gameuifuncs->GetButtonCodeForBind("quickslot2"); //search for the key belongs to that binding

		//convert the key stored in the button code to string
		const char *keyNameSlot2 = g_pInputSystem->ButtonCodeToString(keyname2);
		V_swprintf_safe(sz, L"%s", keyNameSlot2); //convert the string to wchar_t for DrawPrintText 

		surface()->DrawSetTextColor(m_TextColor);
		surface()->DrawSetTextPos(text_xpos2, text_ypos2);
		surface()->DrawPrintText(sz, wcslen(sz));
	}

	//Skill #4
	V_swprintf_safe(sz, L"%i", m_flHudSk4Timer);
	if (m_flHudSk4Timer > 0)
	{
		surface()->DrawSetTextColor(m_TextColor3);
		surface()->DrawSetTextPos(text_xpos3, text_ypos3);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	else
	{
		//Initialize the buttoncode to store the binding 
		ButtonCode_t keyname3;
		keyname3 = gameuifuncs->GetButtonCodeForBind("quickslot3"); //search for the key belongs to that binding

		//convert the key stored in the button code to string
		const char *keyNameSlot3 = g_pInputSystem->ButtonCodeToString(keyname3);
		V_swprintf_safe(sz, L"%s", keyNameSlot3); //convert the string to wchar_t for DrawPrintText 

		surface()->DrawSetTextColor(m_TextColor);
		surface()->DrawSetTextPos(text_xpos3, text_ypos3);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	//Skill #5
	V_swprintf_safe(sz, L"%i", m_flHudSk5Timer);
	if (m_flHudSk5Timer > 0)
	{
		surface()->DrawSetTextColor(m_TextColor4);
		surface()->DrawSetTextPos(text_xpos4, text_ypos4);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	else
	{
		//Initialize the buttoncode to store the binding 
		ButtonCode_t keyname4;
		keyname4 = gameuifuncs->GetButtonCodeForBind("quickslot4"); //search for the key belongs to that binding

		//convert the key stored in the button code to string
		const char *keyNameSlot4 = g_pInputSystem->ButtonCodeToString(keyname4);
		V_swprintf_safe(sz, L"%s", keyNameSlot4); //convert the string to wchar_t for DrawPrintText 

		surface()->DrawSetTextColor(m_TextColor);
		surface()->DrawSetTextPos(text_xpos4, text_ypos4);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	//Skill #6
	V_swprintf_safe(sz, L"%i", m_flHudSk6Timer);
	if (m_flHudSk6Timer > 0)
	{
		surface()->DrawSetTextColor(m_TextColor5);
		surface()->DrawSetTextPos(text_xpos5, text_ypos5);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	else
	{
		//Initialize the buttoncode to store the binding 
		ButtonCode_t keyname5;
		keyname5 = gameuifuncs->GetButtonCodeForBind("quickslot5"); //search for the key belongs to that binding

		//convert the key stored in the button code to string
		const char *keyNameSlot5 = g_pInputSystem->ButtonCodeToString(keyname5);
		V_swprintf_safe(sz, L"%s", keyNameSlot5); //convert the string to wchar_t for DrawPrintText 

		surface()->DrawSetTextColor(m_TextColor);
		surface()->DrawSetTextPos(text_xpos5, text_ypos5);
		surface()->DrawPrintText(sz, wcslen(sz));
	}

	
}