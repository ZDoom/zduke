// Emacs style mode select	 -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//	Internally used data structures for virtually everything,
//	 key definitions, lots of other stuff.
//
//-----------------------------------------------------------------------------

#ifndef __DOOMDEF_H__
#define __DOOMDEF_H__

#include <stdio.h>
#include <string.h>

//
// DOOM keyboard definition. Everything below 0x100 matches
// a DirectInput key code.
//
#define KEY_RIGHTARROW			0xcd	// DIK_RIGHT
#define KEY_LEFTARROW			0xcb	// DIK_LEFT
#define KEY_UPARROW 			0xc8	// DIK_UP
#define KEY_DOWNARROW			0xd0	// DIK_DOWN
#define KEY_ESCAPE				0x01	// DIK_ESCAPE
#define KEY_ENTER				0x1c	// DIK_RETURN
#define KEY_KPENTER				0x9c	// DIK_NUMPADENTER
#define KEY_SPACE				0x39	// DIK_SPACE
#define KEY_TAB 				0x0f	// DIK_TAB
#define KEY_F1					0x3b	// DIK_F1
#define KEY_F2					0x3c	// DIK_F2
#define KEY_F3					0x3d	// DIK_F3
#define KEY_F4					0x3e	// DIK_F4
#define KEY_F5					0x3f	// DIK_F5
#define KEY_F6					0x40	// DIK_F6
#define KEY_F7					0x41	// DIK_F7
#define KEY_F8					0x42	// DIK_F8
#define KEY_F9					0x43	// DIK_F9
#define KEY_F10 				0x44	// DIK_F10
#define KEY_F11 				0x57	// DIK_F11
#define KEY_F12 				0x58	// DIK_F12

#define KEY_BACKSPACE			0x0e	// DIK_BACK
#define KEY_PAUSE				0xff

#define KEY_EQUALS				0x0d	// DIK_EQUALS
#define KEY_MINUS				0x0c	// DIK_MINUS

#define KEY_LSHIFT				0x2A	// DIK_LSHIFT
#define KEY_LCTRL				0x1d	// DIK_LCONTROL
#define KEY_LALT				0x38	// DIK_LMENU

#define	KEY_RSHIFT				KEY_LSHIFT
#define KEY_RCTRL				KEY_LCTRL
#define KEY_RALT				KEY_LALT

#define KEY_INS 				0xd2	// DIK_INSERT
#define KEY_DEL 				0xd3	// DIK_DELETE
#define KEY_END 				0xcf	// DIK_END
#define KEY_HOME				0xc7	// DIK_HOME
#define KEY_PGUP				0xc9	// DIK_PRIOR
#define KEY_PGDN				0xd1	// DIK_NEXT

#define KEY_MOUSE1				0x100
#define KEY_MOUSE2				0x101
#define KEY_MOUSE3				0x102
#define KEY_MOUSE4				0x103
#define KEY_MOUSE5				0x104
#define KEY_MOUSE6				0x105
#define KEY_MOUSE7				0x106
#define KEY_MOUSE8				0x107

#define KEY_FIRSTJOYBUTTON		0x108
#define KEY_LASTJOYBUTTON		0x187
#define KEY_JOYPOV1_UP			0x188
#define KEY_JOYPOV1_RIGHT		0x189
#define KEY_JOYPOV1_DOWN		0x18a
#define KEY_JOYPOV1_LEFT		0x18b
#define KEY_JOYPOV2_UP			0x18c
#define KEY_JOYPOV3_UP			0x190
#define KEY_JOYPOV4_UP			0x194

#define KEY_MWHEELUP			0x198
#define KEY_MWHEELDOWN			0x199

#define NUM_KEYS				0x19A

#define JOYAXIS_NONE			0
#define JOYAXIS_YAW				1
#define JOYAXIS_PITCH			2
#define JOYAXIS_FORWARD			3
#define JOYAXIS_SIDE			4
#define JOYAXIS_UP				5
//#define JOYAXIS_ROLL			6		// Ha ha. No roll for you.

#ifndef __BIG_ENDIAN__
#define MAKE_ID(a,b,c,d)	((a)|((b)<<8)|((c)<<16)|((d)<<24))
#else
#define MAKE_ID(a,b,c,d)	((d)|((c)<<8)|((b)<<16)|((a)<<24))
#endif

#endif	// __DOOMDEF_H__
