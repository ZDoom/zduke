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
//		System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __I_SYSTEM__
#define __I_SYSTEM__

//#include "d_main.h"
//#include "d_ticcmd.h"
//#include "d_event.h"
#include "doomtype.h"

// [RH] Detects the OS the game is running under.
void I_DetectOS (void);

typedef enum {
	os_unknown,
	os_Win95,
	os_WinNT,
	os_Win2k,
	os_Win32s
} os_t;

extern os_t OSPlatform;

extern "C" {
	extern byte CPUFamily, CPUModel, CPUStepping;
}

// Called by DoomMain.
void I_Init (void);


//
// Called by D_DoomLoop,
// called before processing any tics in a frame
// (just after displaying a frame).
// Time consuming syncronous operations
// are performed here (joystick reading).
// Can call D_PostEvent.
//
void I_StartFrame (void);


//
// Called by D_DoomLoop,
// called before processing each tic in a frame.
// Quick syncronous operations are performed here.
// Can call D_PostEvent.
void I_StartTic ();

void I_WaitClocks (int clocksToWait);

// Asynchronous interrupt functions should maintain private queues
// that are read by the synchronous functions
// to be converted into events.

// Either returns a null ticcmd,
// or calls a loadable driver to build it.
// This ticcmd will then be modified by the gameloop
// for normal input.
//ticcmd_t *I_BaseTiccmd (void);


// Called by M_Responder when quit is selected.
// Clean exit, displays sell blurb.
void STACK_ARGS I_Quit (void);


void I_Tactile (int on, int off, int total);

void STACK_ARGS I_Error (const char *error, ...) GCCPRINTF(1,2);
void STACK_ARGS I_FatalError (const char *error, ...) GCCPRINTF(1,2);

void atterm (void (STACK_ARGS *func)(void));
void popterm ();

// Repaint the pre-game console
void I_PaintConsole (void);

// Print a console string
void I_PrintStr (const char *str, bool lineBreak);
void I_HideLogWindow ();

// Set the title string of the startup window
void I_SetTitleString (const char *title);

// Pick from multiple IWADs to use
//int I_PickIWad (WadStuff *wads, int numwads);

// [RH] Title string to display at bottom of console during startup
extern char DoomStartupTitle[256];

bool STACK_ARGS I_YesNoQuestion (const char *fmt, ...) GCCPRINTF(1,2);
void STACK_ARGS I_OkayMessage (const char *fmt, ...) GCCPRINTF(1,2);

// Directory searching routines

// Mirror WIN32_FIND_DATAA in <winbase.h>
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef PATH_MAX
#define PATH_MAX 260
#endif

struct findstate_t
{
	DWORD Attribs;
	DWORD Times[3*2];
	DWORD Size[2];
	DWORD Reserved[2];
	char Name[MAX_PATH];
	char AltName[14];
};

void *I_FindFirst (const char *filespec, findstate_t *fileinfo);
int I_FindNext (void *handle, findstate_t *fileinfo);
int I_FindClose (void *handle);

#define I_FindName(a)	((a)->Name)
#define I_FindAttr(a)	((a)->Attribs)

#define FA_RDONLY	0x00000001
#define FA_HIDDEN	0x00000002
#define FA_SYSTEM	0x00000004
#define FA_DIREC	0x00000010
#define FA_ARCH		0x00000020

#endif
