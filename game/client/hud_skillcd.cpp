#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "c_baseplayer.h" 
#include "hud_skillcd.h" 
#include "iclientmode.h" 
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"
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

	SetHiddenBits(HIDEHUD_PLAYERDEAD);

	m_pPassiveSkill1 = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "HudPassiveSkill1"));
	m_pPassiveSkillLabel1 = vgui::SETUP_PANEL(new vgui::Label(this, "HUDPassiveSkillLabel1", "0"));

	m_pPassiveSkill1->SetPos(2, 64);
	m_pPassiveSkill1->SetSize(48, 48);
	m_pPassiveSkill1->SetImage("icon_spevade");
	m_pPassiveSkill1->SetShouldScaleImage(true);

	m_pPassiveSkillLabel1->SetPos(2, 72);
	m_pPassiveSkillLabel1->SetSize(48, 48);
	m_pPassiveSkillLabel1->SetFont(m_hTextFont);
	m_pPassiveSkillLabel1->SetPaintBorderEnabled(false);
	m_pPassiveSkillLabel1->SetPaintBackgroundEnabled(false);

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

	ConVar *pSkill1cdtimer = cvar->FindVar("sk_plr_skills_1_cd");
	m_flHudSk1Timer = pSkill1cdtimer->GetInt();

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

	ConVar *pSkill7cdtimer = cvar->FindVar("sk_plr_skills_7_cd");
	m_flHudSk7Timer = pSkill7cdtimer->GetInt();

	ConVar *pQuickSlot1SkillID = cvar->FindVar("sk_plr_quickslot1_skill_id");
	ConVar *pQuickSlot2SkillID = cvar->FindVar("sk_plr_quickslot2_skill_id");
	ConVar *pQuickSlot3SkillID = cvar->FindVar("sk_plr_quickslot3_skill_id");
	ConVar *pQuickSlot4SkillID = cvar->FindVar("sk_plr_quickslot4_skill_id");
	ConVar *pQuickSlot5SkillID = cvar->FindVar("sk_plr_quickslot5_skill_id");
	ConVar *pQuickSlot6SkillID = cvar->FindVar("sk_plr_quickslot6_skill_id");

	m_iQuickslot1SkillID = pQuickSlot1SkillID->GetInt();
	m_iQuickslot2SkillID = pQuickSlot2SkillID->GetInt();
	m_iQuickslot3SkillID = pQuickSlot3SkillID->GetInt();
	m_iQuickslot4SkillID = pQuickSlot4SkillID->GetInt();
	m_iQuickslot5SkillID = pQuickSlot5SkillID->GetInt();
	m_iQuickslot6SkillID = pQuickSlot6SkillID->GetInt();

	//Skill 2 cooldown has been defined using a different method below in Paint()

	if (local->GetActiveWeapon() != NULL)
	{

		if (m_flHudSk1Timer >= 0)
		{
			wchar_t sz[64];
			V_swprintf_safe(sz, L"%i", m_flHudSk1Timer);

			m_pPassiveSkill1->SetAlpha(255);

			m_pPassiveSkillLabel1->SetText(sz);
			m_pPassiveSkillLabel1->SetAlpha(255);


		}
		else
		{
			m_pPassiveSkill1->SetAlpha(0);
			m_pPassiveSkillLabel1->SetAlpha(0);

		}
		//Active Skills
		if (m_flHudSk2Timer >= 0)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon1onCD");
		else if (m_iGetPlayerMP < 30)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon1NoMP");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon1");

		}

		if (m_flHudSk3Timer >= 0)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon2onCD");
		else if (m_iGetPlayerMP < 25)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon2NoMP");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon2");

		}

		if (m_flHudSk4Timer >= 0)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon3onCD");
		else if (m_iGetPlayerMP < 50)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon3NoMP");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon3");

		}

		if (m_flHudSk5Timer >= 0)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon4onCD");
		else if (m_iGetPlayerMP < 30)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon4NoMP");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon4");

		}

		if (m_flHudSk6Timer >= 0)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon5onCD");
		else if (m_iGetPlayerMP < 50)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon5NoMP");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon5");

		}

		if (m_flHudSk7Timer >= 0)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon6onCD");
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon6");
		}
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon1onCD");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon2onCD");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon3onCD");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon4onCD");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon5onCD");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Icon6onCD");

		m_pPassiveSkill1->SetAlpha(0);
		m_pPassiveSkillLabel1->SetAlpha(0);

	}


}


//------------------------------------------------------------------------
// Purpose: draws
//------------------------------------------------------------------------

void CHudSkillCooldown::Paint()
{
	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();

	SetPaintBorderEnabled(false);

	//Draw actual textures
	switch (m_iQuickslot1SkillID)
	{

	case 0:
		surface()->DrawSetTexture(m_nIconTextureId0);
		break;
	case 2:
		surface()->DrawSetTexture(m_nIconTextureId);
		break;
	case 3:
		surface()->DrawSetTexture(m_nIconTextureId2);
		break;
	case 4:
		surface()->DrawSetTexture(m_nIconTextureId3);
		break;
	case 5:
		surface()->DrawSetTexture(m_nIconTextureId4);
		break;
	case 6:
		surface()->DrawSetTexture(m_nIconTextureId5);
		break;
	case 7:
		surface()->DrawSetTexture(m_nIconTextureId6);
		break;
	default: surface()->DrawSetTexture(m_nIconTextureId0);

	}

	surface()->DrawTexturedRect(m_iIconX, m_iIconY, m_iIconWide, m_iIconTall);

	//surface()->DrawSetTexture(m_nIconTextureId2);
	switch (m_iQuickslot2SkillID)
	{

	case 0:
		surface()->DrawSetTexture(m_nIconTextureId0);
		break;
	case 2:
		surface()->DrawSetTexture(m_nIconTextureId);
		break;
	case 3:
		surface()->DrawSetTexture(m_nIconTextureId2);
		break;
	case 4:
		surface()->DrawSetTexture(m_nIconTextureId3);
		break;
	case 5:
		surface()->DrawSetTexture(m_nIconTextureId4);
		break;
	case 6:
		surface()->DrawSetTexture(m_nIconTextureId5);
		break;
	case 7:
		surface()->DrawSetTexture(m_nIconTextureId6);
		break;
	default: surface()->DrawSetTexture(m_nIconTextureId0);

	}
	surface()->DrawTexturedRect(m_iIconX2, m_iIconY2, m_iIconWide2, m_iIconTall2);

	switch (m_iQuickslot3SkillID)
	{

	case 0:
		surface()->DrawSetTexture(m_nIconTextureId0);
		break;
	case 2:
		surface()->DrawSetTexture(m_nIconTextureId);
		break;
	case 3:
		surface()->DrawSetTexture(m_nIconTextureId2);
		break;
	case 4:
		surface()->DrawSetTexture(m_nIconTextureId3);
		break;
	case 5:
		surface()->DrawSetTexture(m_nIconTextureId4);
		break;
	case 6:
		surface()->DrawSetTexture(m_nIconTextureId5);
		break;
	case 7:
		surface()->DrawSetTexture(m_nIconTextureId6);
		break;
	default: surface()->DrawSetTexture(m_nIconTextureId0);

	}
	//surface()->DrawSetTexture(m_nIconTextureId3);
	surface()->DrawTexturedRect(m_iIconX3, m_iIconY3, m_iIconWide3, m_iIconTall3);

	switch (m_iQuickslot4SkillID)
	{

	case 0:
		surface()->DrawSetTexture(m_nIconTextureId0);
		break;
	case 2:
		surface()->DrawSetTexture(m_nIconTextureId);
		break;
	case 3:
		surface()->DrawSetTexture(m_nIconTextureId2);
		break;
	case 4:
		surface()->DrawSetTexture(m_nIconTextureId3);
		break;
	case 5:
		surface()->DrawSetTexture(m_nIconTextureId4);
		break;
	case 6:
		surface()->DrawSetTexture(m_nIconTextureId5);
		break;
	case 7:
		surface()->DrawSetTexture(m_nIconTextureId6);
		break;
	default: surface()->DrawSetTexture(m_nIconTextureId0);

	}
	//surface()->DrawSetTexture(m_nIconTextureId4);
	surface()->DrawTexturedRect(m_iIconX4, m_iIconY4, m_iIconWide4, m_iIconTall4);
	
	switch (m_iQuickslot5SkillID)
	{

	case 0:
		surface()->DrawSetTexture(m_nIconTextureId0);
		break;
	case 2:
		surface()->DrawSetTexture(m_nIconTextureId);
		break;
	case 3:
		surface()->DrawSetTexture(m_nIconTextureId2);
		break;
	case 4:
		surface()->DrawSetTexture(m_nIconTextureId3);
		break;
	case 5:
		surface()->DrawSetTexture(m_nIconTextureId4);
		break;
	case 6:
		surface()->DrawSetTexture(m_nIconTextureId5);
		break;
	case 7:
		surface()->DrawSetTexture(m_nIconTextureId6);
		break;
	default: surface()->DrawSetTexture(m_nIconTextureId0);

	}
	surface()->DrawSetColor(m_Icon5Color);
	//surface()->DrawSetTexture(m_nIconTextureId5);
	surface()->DrawTexturedRect(m_iIconX5, m_iIconY5, m_iIconWide5, m_iIconTall5);

	switch (m_iQuickslot6SkillID)
	{

	case 0:
		surface()->DrawSetTexture(m_nIconTextureId0);
		break;
	case 2:
		surface()->DrawSetTexture(m_nIconTextureId);
		break;
	case 3:
		surface()->DrawSetTexture(m_nIconTextureId2);
		break;
	case 4:
		surface()->DrawSetTexture(m_nIconTextureId3);
		break;
	case 5:
		surface()->DrawSetTexture(m_nIconTextureId4);
		break;
	case 6:
		surface()->DrawSetTexture(m_nIconTextureId5);
		break;
	case 7:
		surface()->DrawSetTexture(m_nIconTextureId6);
		break;
	default: surface()->DrawSetTexture(m_nIconTextureId0);

	}
	//surface()->DrawSetTexture(m_nIconTextureId6);
	surface()->DrawTexturedRect(m_iIconX6, m_iIconY6, m_iIconWide6, m_iIconTall6);

	//TODO: read the keybind directly from the keyboard list.
	wchar_t sz[64];
	wchar_t sz3[64];
	wchar_t sz4[64];
	wchar_t sz5[64];
	wchar_t sz6[64];
	wchar_t sz7[64];
	V_swprintf_safe(sz, L"%i",m_flHudSk2Timer);
	V_swprintf_safe(sz3, L"%i", m_flHudSk3Timer);
	V_swprintf_safe(sz4, L"%i", m_flHudSk4Timer);
	V_swprintf_safe(sz5, L"%i", m_flHudSk5Timer);
	V_swprintf_safe(sz6, L"%i", m_flHudSk6Timer);
	V_swprintf_safe(sz7, L"%i", m_flHudSk7Timer);

	//bug , skill 2 timer doesnt display correctly 
	//Skill #2
	surface()->DrawSetTextFont(m_hTextFont);
	if ((m_flHudSk2Timer >= 0) && (m_iQuickslot1SkillID == 2)) //???
	{
		surface()->DrawSetTextColor(m_TextColor1);
		surface()->DrawSetTextPos(text_xpos, text_ypos);
		surface()->DrawPrintText(sz, wcslen(sz));
		
	}
	else if ((m_flHudSk3Timer >= 0) && (m_iQuickslot1SkillID == 3)) //???
	{
		surface()->DrawSetTextColor(m_TextColor1);
		surface()->DrawSetTextPos(text_xpos, text_ypos);
		surface()->DrawPrintText(sz3, wcslen(sz3));

	}
	else if ((m_flHudSk4Timer >= 0) && (m_iQuickslot1SkillID == 4)) //???
	{
		surface()->DrawSetTextColor(m_TextColor1);
		surface()->DrawSetTextPos(text_xpos, text_ypos);
		surface()->DrawPrintText(sz4, wcslen(sz4));

	}
	else if ((m_flHudSk5Timer >= 0) && (m_iQuickslot1SkillID == 5)) //???
	{
		surface()->DrawSetTextColor(m_TextColor1);
		surface()->DrawSetTextPos(text_xpos, text_ypos);
		surface()->DrawPrintText(sz5, wcslen(sz5));

	}
	else if ((m_flHudSk6Timer >= 0) && (m_iQuickslot1SkillID == 6)) //???
	{
		surface()->DrawSetTextColor(m_TextColor1);
		surface()->DrawSetTextPos(text_xpos, text_ypos);
		surface()->DrawPrintText(sz6, wcslen(sz6));

	}
	else if ((m_flHudSk7Timer >= 0) && (m_iQuickslot1SkillID == 7)) //???
	{
		surface()->DrawSetTextColor(m_TextColor1);
		surface()->DrawSetTextPos(text_xpos, text_ypos);
		surface()->DrawPrintText(sz7, wcslen(sz7));

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
	//V_swprintf_safe(sz, L"%i", m_flHudSk3Timer);
	if ((m_flHudSk2Timer >= 0) && (m_iQuickslot2SkillID == 2))
	{
		surface()->DrawSetTextColor(m_TextColor2);
		surface()->DrawSetTextPos(text_xpos2, text_ypos2);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	else if ((m_flHudSk3Timer >= 0) && (m_iQuickslot2SkillID == 3)) //???
	{
		surface()->DrawSetTextColor(m_TextColor2);
		surface()->DrawSetTextPos(text_xpos2, text_ypos2);
		surface()->DrawPrintText(sz3, wcslen(sz3));

	}
	else if ((m_flHudSk4Timer >= 0) && (m_iQuickslot2SkillID == 4)) //???
	{
		surface()->DrawSetTextColor(m_TextColor2);
		surface()->DrawSetTextPos(text_xpos2, text_ypos2);
		surface()->DrawPrintText(sz4, wcslen(sz4));

	}
	else if ((m_flHudSk5Timer >= 0) && (m_iQuickslot2SkillID == 5)) //???
	{
		surface()->DrawSetTextColor(m_TextColor2);
		surface()->DrawSetTextPos(text_xpos2, text_ypos2);
		surface()->DrawPrintText(sz5, wcslen(sz5));

	}
	else if ((m_flHudSk6Timer >= 0) && (m_iQuickslot2SkillID == 6)) //???
	{
		surface()->DrawSetTextColor(m_TextColor2);
		surface()->DrawSetTextPos(text_xpos2, text_ypos2);
		surface()->DrawPrintText(sz6, wcslen(sz6));

	}
	else if ((m_flHudSk7Timer >= 0) && (m_iQuickslot2SkillID == 7)) //???
	{
		surface()->DrawSetTextColor(m_TextColor2);
		surface()->DrawSetTextPos(text_xpos2, text_ypos2);
		surface()->DrawPrintText(sz7, wcslen(sz7));

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
	//V_swprintf_safe(sz, L"%i", m_flHudSk4Timer);
	if ((m_flHudSk2Timer >= 0) && (m_iQuickslot3SkillID == 2))
	{
		surface()->DrawSetTextColor(m_TextColor3);
		surface()->DrawSetTextPos(text_xpos3, text_ypos3);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	else if ((m_flHudSk3Timer >= 0) && (m_iQuickslot3SkillID == 3)) //???
	{
		surface()->DrawSetTextColor(m_TextColor3);
		surface()->DrawSetTextPos(text_xpos3, text_ypos3);
		surface()->DrawPrintText(sz3, wcslen(sz3));

	}
	else if ((m_flHudSk4Timer >= 0) && (m_iQuickslot3SkillID == 4)) //???
	{
		surface()->DrawSetTextColor(m_TextColor3);
		surface()->DrawSetTextPos(text_xpos3, text_ypos3);
		surface()->DrawPrintText(sz4, wcslen(sz4));

	}
	else if ((m_flHudSk5Timer >= 0) && (m_iQuickslot3SkillID == 5)) //???
	{
		surface()->DrawSetTextColor(m_TextColor3);
		surface()->DrawSetTextPos(text_xpos3, text_ypos3);
		surface()->DrawPrintText(sz5, wcslen(sz5));

	}
	else if ((m_flHudSk6Timer >= 0) && (m_iQuickslot3SkillID == 6)) //???
	{
		surface()->DrawSetTextColor(m_TextColor3);
		surface()->DrawSetTextPos(text_xpos3, text_ypos3);
		surface()->DrawPrintText(sz6, wcslen(sz6));

	}
	else if ((m_flHudSk7Timer >= 0) && (m_iQuickslot3SkillID == 7)) //???
	{
		surface()->DrawSetTextColor(m_TextColor3);
		surface()->DrawSetTextPos(text_xpos3, text_ypos3);
		surface()->DrawPrintText(sz7, wcslen(sz7));

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
	//V_swprintf_safe(sz, L"%i", m_flHudSk5Timer);
	if ((m_flHudSk2Timer >= 0) && (m_iQuickslot4SkillID == 2))
	{
		surface()->DrawSetTextColor(m_TextColor4);
		surface()->DrawSetTextPos(text_xpos4, text_ypos4);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	else if ((m_flHudSk3Timer >= 0) && (m_iQuickslot4SkillID == 3)) //???
	{
		surface()->DrawSetTextColor(m_TextColor4);
		surface()->DrawSetTextPos(text_xpos4, text_ypos4);
		surface()->DrawPrintText(sz3, wcslen(sz3));

	}
	else if ((m_flHudSk4Timer >= 0) && (m_iQuickslot4SkillID == 4)) //???
	{
		surface()->DrawSetTextColor(m_TextColor4);
		surface()->DrawSetTextPos(text_xpos4, text_ypos4);
		surface()->DrawPrintText(sz4, wcslen(sz4));

	}
	else if ((m_flHudSk5Timer >= 0) && (m_iQuickslot4SkillID == 5)) //???
	{
		surface()->DrawSetTextColor(m_TextColor4);
		surface()->DrawSetTextPos(text_xpos4, text_ypos4);
		surface()->DrawPrintText(sz5, wcslen(sz5));

	}
	else if ((m_flHudSk6Timer >= 0) && (m_iQuickslot4SkillID == 6)) //???
	{
		surface()->DrawSetTextColor(m_TextColor4);
		surface()->DrawSetTextPos(text_xpos4, text_ypos4);
		surface()->DrawPrintText(sz6, wcslen(sz6));

	}
	else if ((m_flHudSk7Timer >= 0) && (m_iQuickslot4SkillID == 7)) //???
	{
		surface()->DrawSetTextColor(m_TextColor4);
		surface()->DrawSetTextPos(text_xpos4, text_ypos4);
		surface()->DrawPrintText(sz7, wcslen(sz7));

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
	//V_swprintf_safe(sz, L"%i", m_flHudSk6Timer);
	if ((m_flHudSk2Timer >= 0) && (m_iQuickslot5SkillID == 2))
	{
		surface()->DrawSetTextColor(m_TextColor5);
		surface()->DrawSetTextPos(text_xpos5, text_ypos5);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	else if ((m_flHudSk3Timer >= 0) && (m_iQuickslot5SkillID == 3)) //???
	{
		surface()->DrawSetTextColor(m_TextColor5);
		surface()->DrawSetTextPos(text_xpos5, text_ypos5);
		surface()->DrawPrintText(sz3, wcslen(sz3));

	}
	else if ((m_flHudSk4Timer >= 0) && (m_iQuickslot5SkillID == 4)) //???
	{
		surface()->DrawSetTextColor(m_TextColor5);
		surface()->DrawSetTextPos(text_xpos5, text_ypos5);
		surface()->DrawPrintText(sz4, wcslen(sz4));

	}
	else if ((m_flHudSk5Timer >= 0) && (m_iQuickslot5SkillID == 5)) //???
	{
		surface()->DrawSetTextColor(m_TextColor5);
		surface()->DrawSetTextPos(text_xpos5, text_ypos5);
		surface()->DrawPrintText(sz5, wcslen(sz5));

	}
	else if ((m_flHudSk6Timer >= 0) && (m_iQuickslot5SkillID == 6)) //???
	{
		surface()->DrawSetTextColor(m_TextColor5);
		surface()->DrawSetTextPos(text_xpos5, text_ypos5);
		surface()->DrawPrintText(sz6, wcslen(sz6));

	}
	else if ((m_flHudSk7Timer >= 0) && (m_iQuickslot5SkillID == 7)) //???
	{
		surface()->DrawSetTextColor(m_TextColor5);
		surface()->DrawSetTextPos(text_xpos5, text_ypos5);
		surface()->DrawPrintText(sz7, wcslen(sz7));

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
	//Skill #7
	//V_swprintf_safe(sz, L"%i", m_flHudSk7Timer);
	if ((m_flHudSk2Timer >= 0) && (m_iQuickslot6SkillID == 2))
	{
		surface()->DrawSetTextColor(m_TextColor6);
		surface()->DrawSetTextPos(text_xpos6, text_ypos6);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	else if ((m_flHudSk3Timer >= 0) && (m_iQuickslot6SkillID == 3)) //???
	{
		surface()->DrawSetTextColor(m_TextColor6);
		surface()->DrawSetTextPos(text_xpos6, text_ypos6);
		surface()->DrawPrintText(sz3, wcslen(sz3));

	}
	else if ((m_flHudSk4Timer >= 0) && (m_iQuickslot6SkillID == 4)) //???
	{
		surface()->DrawSetTextColor(m_TextColor6);
		surface()->DrawSetTextPos(text_xpos6, text_ypos6);
		surface()->DrawPrintText(sz4, wcslen(sz4));

	}
	else if ((m_flHudSk5Timer >= 0) && (m_iQuickslot6SkillID == 5)) //???
	{
		surface()->DrawSetTextColor(m_TextColor6);
		surface()->DrawSetTextPos(text_xpos6, text_ypos6);
		surface()->DrawPrintText(sz5, wcslen(sz5));

	}
	else if ((m_flHudSk6Timer >= 0) && (m_iQuickslot6SkillID == 6)) //???
	{
		surface()->DrawSetTextColor(m_TextColor6);
		surface()->DrawSetTextPos(text_xpos6, text_ypos6);
		surface()->DrawPrintText(sz6, wcslen(sz6));

	}
	else if ((m_flHudSk7Timer >= 0) && (m_iQuickslot6SkillID == 7)) //???
	{
		surface()->DrawSetTextColor(m_TextColor6);
		surface()->DrawSetTextPos(text_xpos6, text_ypos6);
		surface()->DrawPrintText(sz7, wcslen(sz7));

	}
	else
	{
		//Initialize the buttoncode to store the binding 
		ButtonCode_t keyname6;
		keyname6 = gameuifuncs->GetButtonCodeForBind("quickslot6"); //search for the key belongs to that binding

		//convert the key stored in the button code to string
		const char *keyNameSlot6 = g_pInputSystem->ButtonCodeToString(keyname6);
		V_swprintf_safe(sz, L"%s", keyNameSlot6); //convert the string to wchar_t for DrawPrintText 

		surface()->DrawSetTextColor(m_TextColor);
		surface()->DrawSetTextPos(text_xpos6, text_ypos6);
		surface()->DrawPrintText(sz, wcslen(sz));
	}
	
}