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
// $Log:$
//
// DESCRIPTION:
//		Functions to draw patches (by post) directly to screen->
//		Functions to blit a block to the screen->
//
//-----------------------------------------------------------------------------



#include <stdio.h>
#include <math.h>

#include "m_alloc.h"
#include "duke3d.h"

#include "i_system.h"
#include "i_video.h"
//#include "r_local.h"
//#include "r_draw.h"
//#include "r_plane.h"
//#include "r_state.h"

#include "doomdef.h"
//#include "doomdata.h"
//#include "doomstat.h"

#include "c_console.h"
//#include "hu_stuff.h"

#include "m_argv.h"
//#include "m_bbox.h"
//#include "m_swap.h"
//#include "m_menu.h"

#include "i_video.h"
#include "v_video.h"
//#include "v_text.h"

//#include "w_wad.h"

#include "c_cvars.h"
#include "c_dispatch.h"
#include "cmdlib.h"
//#include "gi.h"
#include "templates.h"

int CleanWidth, CleanHeight, CleanXfac, CleanYfac;
int DisplayWidth, DisplayHeight, DisplayBits;

DFrameBuffer *screen;

CVAR (Int, vid_defwidth, 320, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR (Int, vid_defheight, 200, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR (Bool, vid_fps, false, 0)
CVAR (Bool, ticker, false, 0)

CVAR (Float, dimamount, 0.2f, CVAR_ARCHIVE)
CVAR (Color, dimcolor, 0xffd700, CVAR_ARCHIVE)

// [RH] Set true when vid_setmode command has been executed
bool	setmodeneeded = false;
// [RH] Resolution to change to when setmodeneeded is true
int		NewWidth, NewHeight, NewBits;


DCanvas *DCanvas::CanvasChain = NULL;

DCanvas::DCanvas (int _width, int _height)
{
	// Init member vars
	Buffer = NULL;
	LockCount = 0;
	Width = _width;
	Height = _height;

	// Add to list of active canvases
	Next = CanvasChain;
	CanvasChain = this;
}

DCanvas::~DCanvas ()
{
	// Remove from list of active canvases
	DCanvas *probe = CanvasChain, **prev;

	prev = &CanvasChain;
	probe = CanvasChain;

	while (probe != NULL)
	{
		if (probe == this)
		{
			*prev = probe->Next;
			break;
		}
		prev = &probe->Next;
		probe = probe->Next;
	}
}

bool DCanvas::IsValid ()
{
	// A nun-subclassed DCanvas is never valid
	return false;
}

// [RH] Set an area to a specified color
void DCanvas::Clear (int left, int top, int right, int bottom, int color) const
{
	int x, y;
	byte *dest;

	dest = Buffer + top * Pitch + left;
	x = right - left;
	for (y = top; y < bottom; y++)
	{
		memset (dest, color, x);
		dest += Pitch;
	}
}

void DCanvas::Blit (int srcx, int srcy, int srcwidth, int srcheight,
			 DCanvas *dest, int destx, int desty, int destwidth, int destheight)
{
	fixed_t fracxstep, fracystep;
	fixed_t fracx, fracy;
	int x, y;
	bool lockthis, lockdest;

	if ( (lockthis = (LockCount == 0)) )
	{
		if (Lock ())
		{ // Surface was lost, so nothing to blit
			Unlock ();
			return;
		}
	}

	if ( (lockdest = (dest->LockCount == 0)) )
	{
		dest->Lock ();
	}

	fracy = srcy << FRACBITS;
	fracystep = (srcheight << FRACBITS) / destheight;
	fracxstep = (srcwidth << FRACBITS) / destwidth;

	byte *destline, *srcline;
	byte *destbuffer = dest->Buffer;
	byte *srcbuffer = Buffer;

	if (fracxstep == FRACUNIT)
	{
		for (y = desty; y < desty + destheight; y++, fracy += fracystep)
		{
			memcpy (destbuffer + y * dest->Pitch + destx,
					srcbuffer + (fracy >> FRACBITS) * Pitch + srcx,
					destwidth);
		}
	}
	else
	{
		for (y = desty; y < desty + destheight; y++, fracy += fracystep)
		{
			srcline = srcbuffer + (fracy >> FRACBITS) * Pitch + srcx;
			destline = destbuffer + y * dest->Pitch + destx;
			for (x = fracx = 0; x < destwidth; x++, fracx += fracxstep)
			{
				destline[x] = srcline[fracx >> FRACBITS];
			}
		}
	}

	if (lockthis)
	{
		Unlock ();
	}
	if (lockdest)
	{
		Unlock ();
	}
}

void DCanvas::CalcGamma (float gamma, BYTE gammalookup[256])
{
	// I found this formula on the web at
	// <http://panda.mostang.com/sane/sane-gamma.html>

	double invgamma = 1.f / gamma;
	int i;

	for (i = 0; i < 256; i++)
	{
		gammalookup[i] = (BYTE)(255.0 * pow (i / 255.0, invgamma));
	}
}

DSimpleCanvas::DSimpleCanvas (int width, int height)
	: DCanvas (width, height)
{
	// Making the pitch a power of 2 is very bad for performance
	if (width == 512 || width == 1024 || width == 2048)
	{
		Pitch = width + 8;
	}
	else
	{
		Pitch = width;
	}
	MemBuffer = new BYTE[Pitch * height];
}

DSimpleCanvas::~DSimpleCanvas ()
{
	if (MemBuffer != NULL)
	{
		delete[] MemBuffer;
		MemBuffer = NULL;
	}
}

bool DSimpleCanvas::IsValid ()
{
	return (MemBuffer != NULL);
}

bool DSimpleCanvas::Lock ()
{
	if (LockCount == 0)
	{
		LockCount++;
		Buffer = MemBuffer;
	}
	return false;		// System surfaces are never lost
}

void DSimpleCanvas::Unlock ()
{
	if (LockCount <= 1)
	{
		LockCount = 0;
		Buffer = NULL;	// Enforce buffer access only between Lock/Unlock
	}
}

DFrameBuffer::DFrameBuffer (int width, int height)
	: DSimpleCanvas (width, height)
{
	LastMS = LastSec = FrameCount = LastCount = LastTic = 0;
}

void DFrameBuffer::DrawRateStuff ()
{
#if WORKINPROGRESS
	// Draws frame time and cumulative fps
	if (vid_fps)
	{
		QWORD ms = I_MSTime ();
		QWORD howlong = ms - LastMS;
		if (howlong > 0)
		{
			char fpsbuff[40];
			int chars;

#if _MSC_VER
			chars = sprintf (fpsbuff, "%I64d ms (%ld fps)", howlong, LastCount);
#else
			chars = sprintf (fpsbuff, "%Ld ms (%ld fps)", howlong, LastCount);
#endif
			Clear (0, screen->GetHeight() - 8, chars * 8, screen->GetHeight(), 0);
			SetFont (ConFont);
			DrawText (CR_WHITE, 0, screen->GetHeight() - 8, (char *)&fpsbuff[0]);
			SetFont (SmallFont);

			DWORD thisSec = ms/1000;
			if (LastSec < thisSec)
			{
				LastCount = FrameCount / (thisSec - LastSec);
				LastSec = thisSec;
				FrameCount = 0;
			}
			FrameCount++;
		}
		LastMS = ms;
	}
#endif
}

void DFrameBuffer::CopyFromBuff (BYTE *src, int srcPitch, int width, int height, BYTE *dest)
{
	if (Pitch == width && Pitch == Width && srcPitch == width)
	{
		memcpy (dest, src, Width * Height);
	}
	else
	{
		for (int y = 0; y < height; y++)
		{
			memcpy (dest, src, width);
			dest += Pitch;
			src += srcPitch;
		}
	}
}

//
// V_SetResolution
//
bool V_DoModeSetup (int width, int height, int bits)
{
	DFrameBuffer *buff = I_SetMode (width, height, screen);

	if (buff == NULL)
	{
		return false;
	}

	screen = buff;
	screen->SetGamma (Gamma);

	CleanXfac = width / 320;
	CleanYfac = height / 200;

	if (CleanXfac > 1 && CleanYfac > 1 && CleanXfac != CleanYfac)
	{
		if (CleanXfac < CleanYfac)
			CleanYfac = CleanXfac;
		else
			CleanXfac = CleanYfac;
	}

	CleanWidth = width / CleanXfac;
	CleanHeight = height / CleanYfac;

	DisplayWidth = width;
	DisplayHeight = height;
	DisplayBits = bits;

#if WORKINPROGRESS
	M_RefreshModesList ();
#endif

	return true;
}

bool V_SetResolution (int width, int height, int bits)
{
	int oldwidth, oldheight;
	int oldbits;

	if (screen)
	{
		oldwidth = SCREENWIDTH;
		oldheight = SCREENHEIGHT;
		oldbits = DisplayBits;
	}
	else
	{ // Harmless if screen wasn't allocated
		oldwidth = width;
		oldheight = height;
		oldbits = bits;
	}

	I_ClosestResolution (&width, &height, bits);
	if (!I_CheckResolution (width, height, bits))
	{ // Try specified resolution
		if (!I_CheckResolution (oldwidth, oldheight, oldbits))
		{ // Try previous resolution (if any)
	   		return false;
		}
		else
		{
			width = oldwidth;
			height = oldheight;
			bits = oldbits;
		}
	}
	return V_DoModeSetup (width, height, bits);
}

CCMD (vid_setmode)
{
	BOOL	goodmode = false;
	int		width = 0, height = SCREENHEIGHT;
	int		bits = DisplayBits;

	if (argv.argc() > 1)
	{
		width = atoi (argv[1]);
		if (argv.argc() > 2)
		{
			height = atoi (argv[2]);
			if (argv.argc() > 3)
			{
				bits = atoi (argv[3]);
			}
		}
	}

	if (width && I_CheckResolution (width, height, bits))
	{
		goodmode = true;
	}

	if (goodmode)
	{
		// The actual change of resolution will take place
		// near the beginning of D_Display().
#if WORKINPROGRESS
		if (gamestate != GS_STARTUP)
#endif
		{
			setmodeneeded = true;
			NewWidth = width;
			NewHeight = height;
			NewBits = bits;
		}
	}
	else if (width)
	{
		Printf ("Unknown resolution %d x %d x %d\n", width, height, bits);
	}
	else
	{
		Printf ("Usage: vid_setmode <width> <height> <mode>\n");
	}
}

//
// V_Init
//

void V_Init (void) 
{ 
	char *i;
	int width, height, bits;

	width = height = bits = 0;

	if ( (i = Args.CheckValue ("-width")) )
		width = atoi (i);

	if ( (i = Args.CheckValue ("-height")) )
		height = atoi (i);


	if (width == 0)
	{
		if (height == 0)
		{
			width = vid_defwidth;
			height = vid_defheight;
		}
		else
		{
			width = (height * 8) / 6;
		}
	}
	else if (height == 0)
	{
		height = (width * 6) / 8;
	}

	bits = 8;

	atterm (FreeCanvasChain);

	I_ClosestResolution (&width, &height, bits);

	if (!V_SetResolution (width, height, bits))
		I_FatalError ("Could not set resolution to %d x %d x %d", width, height, bits);
	else
		Printf ("Resolution: %d x %d\n", SCREENWIDTH, SCREENHEIGHT);

	FBaseCVar::ResetColors ();
#if WORKINPROGRESS
	ConFont = new FSingleLumpFont ("ConsoleFont", W_GetNumForName ("CONFONT"));
#endif
	C_InitConsole (SCREENWIDTH, SCREENHEIGHT, true);
}

void STACK_ARGS FreeCanvasChain ()
{
	while (DCanvas::CanvasChain != NULL)
	{
		delete DCanvas::CanvasChain;
	}
	screen = NULL;
}
//
// Break long lines of text into multiple lines no longer than maxwidth pixels
//
static void breakit (brokenlines_t *line, const byte *start, const byte *string, bool keepspace)
{
	// Leave out trailing white space
	if (!keepspace)
	{
		while (string > start && isspace (*(string - 1)))
			string--;
	}

	line->string = new char[string - start + 1];
	strncpy (line->string, (char *)start, string - start);
	line->string[string - start] = 0;
	line->width = gametextwidth (line->string, -1);
}

brokenlines_t *V_BreakLines (int maxwidth, const byte *string, bool keepspace)
{
	brokenlines_t lines[128];	// Support up to 128 lines (should be plenty)

	const byte *space = NULL, *start = string;
	int i, c, w, nw;
	BOOL lastWasSpace = false;

	i = w = 0;

	while ( (c = *string++) )
	{
		if (c == TEXTCOLOR_ESCAPE)
		{
			if (*string)
				string++;
			continue;
		}

		if (isspace(c)) 
		{
			if (!lastWasSpace)
			{
				space = string - 1;
				lastWasSpace = true;
			}
		}
		else
		{
			lastWasSpace = false;
		}

		nw = gamecharwidth (c);

		if ((w > 0 && w + nw > maxwidth) || c == '\n')
		{ // Time to break the line
			if (!space)
				space = string - 1;

			lines[i].nlterminated = (c == '\n');
			breakit (&lines[i], start, space, keepspace);

			i++;
			w = 0;
			lastWasSpace = false;
			start = space;
			space = NULL;

			while (*start && isspace (*start) && *start != '\n')
				start++;
			if (*start == '\n')
				start++;
			else
				while (*start && isspace (*start))
					start++;
			string = start;
		}
		else
		{
			w += nw;
		}
	}

	if (string - start > 1)
	{
		const byte *s = start;

		while (s < string)
		{
			if (keepspace || !isspace (*s))
			{
				lines[i].nlterminated = (*s == '\n');
				s++;
				breakit (&lines[i++], start, string, keepspace);
				break;
			}
			s++;
		}
	}

	{
		// Make a copy of the broken lines and return them
		brokenlines_t *broken = new brokenlines_t[i+1];

		memcpy (broken, lines, sizeof(brokenlines_t) * i);
		broken[i].string = NULL;
		broken[i].width = -1;

		return broken;
	}
}

void V_FreeBrokenLines (brokenlines_t *lines)
{
	if (lines)
	{
		int i = 0;

		while (lines[i].width != -1)
		{
			delete[] lines[i].string;
			lines[i].string = NULL;
			i++;
		}
		delete[] lines;
	}
}
