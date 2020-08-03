//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef IN_BUTTONS_H
#define IN_BUTTONS_H
#ifdef _WIN32
#pragma once
#endif

#define IN_ATTACK		(1 << 0)
#define IN_JUMP			(1 << 1)
#define IN_DUCK			(1 << 2)
#define IN_FORWARD		(1 << 3)
#define IN_BACK			(1 << 4)
#define IN_USE			(1 << 5)
#define IN_CANCEL		(1 << 6)
#define IN_LEFT			(1 << 7)
#define IN_RIGHT		(1 << 8)
#define IN_MOVELEFT		(1 << 9)
#define IN_MOVERIGHT	(1 << 10)
#define IN_ATTACK2		(1 << 11)
//#define IN_RUN			(1 << 12) //
#define IN_RELOAD		(1 << 12)
//#define IN_ALT1			(1 << 14) //
//#define IN_ALT2			(1 << 15) //
#define IN_SCORE		(1 << 13)   // Used by client.dll for when scoreboard is held down
#define IN_SPEED		(1 << 14)	// Player is holding the speed key
#define IN_WALK			(1 << 15)	// Player holding walk key
#define IN_ZOOM			(1 << 16)	// Zoom key for HUD zoom
#define IN_WEAPON1		(1 << 17)	// weapon defines these bits
#define IN_WEAPON2		(1 << 18)	// weapon defines these bits
//#define IN_BULLRUSH		(1 << 22)//
//#define IN_GRENADE1		(1 << 23)	// grenade 1 //
//#define IN_GRENADE2		(1 << 24)	// grenade 2 //
#define	IN_ATTACK3		(1 << 19)
#define IN_THROWGRENADE (1 << 20)	//would probably be changed for quickaction, or not
#define IN_SLOT1		(1 << 21)	//mmorpg style quickslot , from 1-6
#define IN_SLOT2		(1 << 22)
#define IN_SLOT3		(1 << 23)
#define IN_SLOT4		(1 << 24)
#define IN_SLOT5		(1 << 25)
#define IN_SLOT6		(1 << 26)
#endif // IN_BUTTONS_H
