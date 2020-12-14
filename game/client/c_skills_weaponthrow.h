//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef CSKILLSWEAPONTHROW_H_
#define CSKILLSWEAPONTHROW_H_

#ifdef _WIN32
#pragma once
#endif

class C_WeaponThrowingSkills : public C_BaseAnimating
{
	DECLARE_CLASS(C_WeaponThrowingSkills, C_BaseAnimating);
	DECLARE_CLIENTCLASS();
public:

	C_WeaponThrowingSkills(void);

	virtual RenderGroup_t GetRenderGroup(void);

	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual int		DrawModel(int flags);

protected:

	void	DrawMotionBlur(void);
	void	DrawFlicker(void);
	virtual bool	InitMaterials(void);

	Vector	m_vecLastOrigin;
	bool	m_bEmit;
	float	m_flRadius;
	bool	m_bHeld;
	bool	m_bLaunched;

	IMaterial	*m_pFlickerMaterial;
	IMaterial	*m_pBodyMaterial;
	IMaterial	*m_pBlurMaterial;
};


#endif