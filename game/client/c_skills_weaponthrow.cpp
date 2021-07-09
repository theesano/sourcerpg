//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_skills_weaponthrow.h"
#include "materialsystem/imaterial.h"
#include "model_types.h"
#include "c_physicsprop.h"
#include "c_te_effect_dispatch.h"
#include "fx_quad.h"
#include "fx.h"
#include "clienteffectprecachesystem.h"
#include "view.h"
#include "view_scene.h"
#include "beamdraw.h"

// Precache our effects
CLIENTEFFECT_REGISTER_BEGIN(PrecacheEffectWeaponThrowingSkills)
CLIENTEFFECT_MATERIAL("effects/ar2_altfire1")
CLIENTEFFECT_MATERIAL("effects/ar2_altfire1b")
CLIENTEFFECT_MATERIAL("effects/combinemuzzle1_nocull")
CLIENTEFFECT_MATERIAL("effects/combinemuzzle2_nocull")
CLIENTEFFECT_MATERIAL("effects/combinemuzzle1")
CLIENTEFFECT_MATERIAL("effects/ar2_altfire1")
CLIENTEFFECT_MATERIAL("effects/ar2_altfire1b")
CLIENTEFFECT_REGISTER_END()

IMPLEMENT_CLIENTCLASS_DT(C_WeaponThrowingSkills, DT_WeaponThrowingSkills, CWeaponThrowingSkills )
RecvPropBool(RECVINFO(m_bEmit)),
RecvPropFloat(RECVINFO(m_flRadius)),
RecvPropBool(RECVINFO(m_bHeld)),
RecvPropBool(RECVINFO(m_bLaunched)),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_WeaponThrowingSkills::C_WeaponThrowingSkills(void)
{
	m_pBodyMaterial = NULL;
	m_pBlurMaterial = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_WeaponThrowingSkills::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED)
	{
		m_vecLastOrigin = GetAbsOrigin();
		InitMaterials();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : RenderGroup_t
//-----------------------------------------------------------------------------
RenderGroup_t C_WeaponThrowingSkills::GetRenderGroup(void)
{
	return RENDER_GROUP_TRANSLUCENT_ENTITY;
}

//-----------------------------------------------------------------------------
// Purpose: Cache the material handles
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_WeaponThrowingSkills::InitMaterials(void)
{
	// Motion blur
	if (m_pBlurMaterial == NULL)
	{
		m_pBlurMaterial = materials->FindMaterial("effects/ar2_altfire1b", NULL, false);

		if (m_pBlurMaterial == NULL)
			return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_WeaponThrowingSkills::DrawMotionBlur(void)
{
	float color[3];

	Vector	vecDir = GetAbsOrigin() - m_vecLastOrigin;
	float	speed = VectorNormalize(vecDir);

	speed = clamp(speed, 0, 32);

	float	stepSize = MIN((speed * 0.5f), 4.0f);

	Vector	spawnPos = GetAbsOrigin();
	Vector	spawnStep = -vecDir * stepSize;

	float base = RemapValClamped(speed, 4, 32, 0.0f, 1.0f);

	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind(m_pBlurMaterial);

	// Draw the motion blurred trail
	for (int i = 0; i < 8; i++)
	{
		spawnPos += spawnStep;

		color[0] = color[1] = color[2] = base * (1.0f - ((float)i / 12.0f));

		DrawHalo(m_pBlurMaterial, spawnPos, m_flRadius, color);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flags - 
// Output : int
//-----------------------------------------------------------------------------
int C_WeaponThrowingSkills::DrawModel(int flags)
{
	//if (!m_bEmit)
	//	return 0;

	//// Make sure our materials are cached
	//if (!InitMaterials())
	//{
	//	//NOTENOTE: This means that a material was not found for the combine ball, so it may not render!
	//	AssertOnce(0);
	//	return 0;
	//}


	//// Draw the motion blur from movement
	if (m_bHeld || m_bLaunched)
	{
		DrawMotionBlur();
	}

	//// Draw the model if we're being held
	//if (m_bHeld)
	//{
	//	QAngle	angles;
	//	VectorAngles(-CurrentViewForward(), angles);

	//	// Always orient towards the camera!
	//	SetAbsAngles(angles);

	//	BaseClass::DrawModel(flags);
	//}
	//else
	//{
	//	float color[3];
	//	color[0] = color[1] = color[2] = 1.0f;

	//	float sinOffs = 1.0f * sin(gpGlobals->curtime * 25);

	//	float roll = SpawnTime();

		// Draw the main ball body
		//CMatRenderContextPtr pRenderContext(materials);
		//pRenderContext->Bind(m_pBodyMaterial, (C_BaseEntity*) this);
		//DrawHaloOrientedWpnThrow(GetAbsOrigin(), m_flRadius + sinOffs, color, roll);
		BaseClass::DrawModel(flags);
	//}

	m_vecLastOrigin = GetAbsOrigin();

	return 1;
}
