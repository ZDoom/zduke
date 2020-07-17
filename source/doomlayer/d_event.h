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
//
//	  
//-----------------------------------------------------------------------------


#ifndef __D_EVENT_H__
#define __D_EVENT_H__


#include "doomtype.h"


//
// Event handling.
//

// Input event types.
enum EGenericEvent
{
	EV_None,
	EV_KeyDown,		// data1: scan code, data2: Qwerty ASCII code, data3: modifiers
	EV_KeyUp,		// same
	EV_Mouse,		// x, y: mouse movement deltas
	EV_GUI_Event	// subtype specifies actual event
};

// Event structure.
struct event_t
{
	BYTE		type;
	BYTE		subtype;
	SWORD 		data1;		// keys / mouse/joystick buttons
	SWORD		data2;
	SWORD		data3;
	int 		x;			// mouse/joystick x move
	int 		y;			// mouse/joystick y move
};

 
typedef enum
{
	ga_nothing,
	ga_loadlevel,
	ga_newgame,
	ga_newgame2,
	ga_loadgame,
	ga_savegame,
	ga_autosave,
	ga_playdemo,
	ga_completed,
	ga_victory,
	ga_worlddone,
	ga_screenshot,
	ga_fullconsole
} gameaction_t;


void D_PostEvent (const event_t *ev);
void D_ProcessEvents ();
void D_AddResponder (bool (*responder)(const event_t *ev), int priority);
void D_RemoveResponder (bool (*responder)(const event_t *ev));

//
// GLOBAL VARIABLES
//
#define MAXEVENTS		128

extern	event_t 		events[MAXEVENTS];
extern	int 			eventhead;
extern	int 			eventtail;

extern	gameaction_t	gameaction;

#define MAX_RESPONDERS 20

typedef bool (*responder_p)(const event_t *);

extern responder_p Responders[MAX_RESPONDERS];
extern int ResponderPriorities[MAX_RESPONDERS];

#endif
