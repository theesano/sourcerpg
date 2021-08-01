#include "cbase.h" 
#include "hud.h" 
#include "hud_macros.h" 
#include "hud_menuhint.h"
#include "c_baseplayer.h" 
#include "iclientmode.h" 
#include "vgui/ISurface.h"
#include "vgui/IScheme.h"

using namespace vgui;

#include "tier0/memdbgon.h" 

DECLARE_HUDELEMENT(CHudMenuHint);


# define HULL_INIT -1

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------

CHudMenuHint::CHudMenuHint(const char * pElementName) :
CHudElement(pElementName), BaseClass(NULL, "HUDMenuHint")
{
	
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	pParent->SetProportional(true);

	m_GameMenuIcon = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "HudGameMenuIcon"));
	m_pTestImagePanel2 = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "HudImage2"));
	m_pTestImagePanel3 = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "HudImage3"));
	m_pTestImagePanel4 = vgui::SETUP_PANEL(new vgui::ImagePanel(this, "HudImage4"));

	m_IconLabel1 = vgui::SETUP_PANEL(new vgui::Label(this, "HUDLabel1", "ESC"));
	m_IconLabel2 = vgui::SETUP_PANEL(new vgui::Label(this, "HUDLabel2", "K"));
	m_IconLabel3 = vgui::SETUP_PANEL(new vgui::Label(this, "HUDLabel3", "I"));
	m_IconLabel4 = vgui::SETUP_PANEL(new vgui::Label(this, "HUDLabel4", "P"));

	m_GameMenuIcon->SetPos(0, 0);
	m_GameMenuIcon->SetSize(48,48);
	m_GameMenuIcon->SetImage("OptionsIco");
	m_GameMenuIcon->SetShouldScaleImage(true);

	m_pTestImagePanel2->SetPos(0, 64);
	m_pTestImagePanel2->SetSize(48, 48);
	m_pTestImagePanel2->SetImage("SkillsIco");
	m_pTestImagePanel2->SetShouldScaleImage(true);

	m_pTestImagePanel3->SetPos(0, 128);
	m_pTestImagePanel3->SetSize(48, 48);
	m_pTestImagePanel3->SetImage("InventoryIco");
	m_pTestImagePanel3->SetShouldScaleImage(true);

	m_pTestImagePanel4->SetPos(0, 192);
	m_pTestImagePanel4->SetSize(48, 48);
	m_pTestImagePanel4->SetImage("CharacterPanelIco");
	m_pTestImagePanel4->SetShouldScaleImage(true);

	m_IconLabel1->SetPos(0, 0);
	m_IconLabel1->SetSize(128, 16);
	m_IconLabel1->SetFont(m_hTextFont);
	m_IconLabel1->SetPaintBorderEnabled(false);
	m_IconLabel1->SetPaintBackgroundEnabled(false);

	m_IconLabel2->SetPos(0, 64);
	m_IconLabel2->SetSize(128, 16);
	m_IconLabel2->SetFont(m_hTextFont);
	m_IconLabel2->SetPaintBorderEnabled(false);
	m_IconLabel2->SetPaintBackgroundEnabled(false);

	m_IconLabel3->SetPos(0, 128);
	m_IconLabel3->SetSize(128, 16);
	m_IconLabel3->SetFont(m_hTextFont);
	m_IconLabel3->SetPaintBorderEnabled(false);
	m_IconLabel3->SetPaintBackgroundEnabled(false);

	m_IconLabel4->SetPos(0, 192);
	m_IconLabel4->SetSize(128, 16);
	m_IconLabel4->SetFont(m_hTextFont);
	m_IconLabel4->SetPaintBorderEnabled(false);
	m_IconLabel4->SetPaintBackgroundEnabled(false);



}

//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudMenuHint::Init()
{
	
	Reset();
}

//------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------

void CHudMenuHint::Reset(void)
{
	SetBgColor(Color(0, 0, 0, 0));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudMenuHint::VidInit(void)
{
	Reset();
}


//------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------

void CHudMenuHint::OnThink(void)
{

}


//------------------------------------------------------------------------
// Purpose: draws the power bar
//------------------------------------------------------------------------

void CHudMenuHint::Paint()
{

}

