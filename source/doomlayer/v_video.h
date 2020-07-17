/*
** v_video.h
**
**---------------------------------------------------------------------------
** Copyright 1998-2001 Randy Heit
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#ifndef __V_VIDEO_H__
#define __V_VIDEO_H__

#include "doomtype.h"
#include "v_palette.h"
#include "c_cvars.h"

extern int CleanWidth, CleanHeight, CleanXfac, CleanYfac;
extern int DisplayWidth, DisplayHeight, DisplayBits;

//
// VIDEO
//
class DCanvas
{
public:
	DCanvas (int width, int height);
	virtual ~DCanvas ();

	// Member variable access
	inline BYTE *GetBuffer () const { return Buffer; }
	inline int GetWidth () const { return Width; }
	inline int GetHeight () const { return Height; }
	inline int GetPitch () const { return Pitch; }

	virtual bool IsValid ();

	// Access control
	virtual bool Lock () = 0;		// Returns true if the surface was lost since last time
	virtual void Unlock () = 0;

	// Copy blocks from one canvas to another
	virtual void Blit (int srcx, int srcy, int srcwidth, int srcheight, DCanvas *dest, int destx, int desty, int destwidth, int destheight);

	// Set an area to a specified color
	virtual void Clear (int left, int top, int right, int bottom, int color) const;

	// Calculate gamma table
	void CalcGamma (float gamma, BYTE gammalookup[256]);

	// Text drawing functions -----------------------------------------------

protected:
	BYTE *Buffer;
	int Width;
	int Height;
	int Pitch;
	int LockCount;

private:
	// Keep track of canvases, for automatic destruction at exit
	DCanvas *Next;
	static DCanvas *CanvasChain;

	friend void STACK_ARGS FreeCanvasChain ();
};

// A canvas in system memory.

class DSimpleCanvas : public DCanvas
{
public:
	DSimpleCanvas (int width, int height);
	~DSimpleCanvas ();

	bool IsValid ();
	bool Lock ();
	void Unlock ();

protected:
	BYTE *MemBuffer;
};

// A canvas that represents the actual display. The video code is responsible
// for actually implementing this. Built on top of SimpleCanvas, because it
// needs a system memory buffer when buffered output is enabled.

class DFrameBuffer : public DSimpleCanvas
{
public:
	DFrameBuffer (int width, int height);

	// Force the surface to use buffered output if true is passed.
	virtual bool Lock (bool buffered) = 0;

	// Locks the surface, using whatever the previous buffered status was.
	virtual bool Relock () = 0;

	// Make the surface visible. Also implies Unlock().
	virtual void Update () = 0;

	// Return a pointer to 256 palette entries that can be written to.
	virtual PalEntry *GetPalette () = 0;

	// Stores the palette with flash blended in into 256 dwords
	virtual void GetFlashedPalette (PalEntry palette[256]) = 0;

	// Mark the palette as changed. It will be updated on the next Update().
	virtual void UpdatePalette () = 0;

	// Sets the gamma level. Returns false if the hardware does not support
	// gamma changing. (Always true for now, since palettes can always be
	// gamma adjusted.)
	virtual bool SetGamma (float gamma) = 0;

	// Sets a color flash. RGB is the color, and amount is 0-256, with 256
	// being all flash and 0 being no flash. Returns false if the hardware
	// does not support this. (Always true for now, since palettes can always
	// be flashed.)
	virtual bool SetFlash (PalEntry rgb, int amount) = 0;

	// Converse of SetFlash
	virtual void GetFlash (PalEntry &rgb, int &amount) = 0;

	// Returns the number of video pages the frame buffer is using.
	virtual int GetPageCount () = 0;

	// Returns true if running fullscreen.
	virtual bool IsFullscreen () = 0;

#ifdef _WIN32
	virtual int QueryNewPalette () = 0;
#endif

protected:
	void DrawRateStuff ();
	void CopyFromBuff (BYTE *src, int srcPitch, int width, int height, BYTE *dest);

private:
	DWORD LastMS, LastSec, FrameCount, LastCount, LastTic;
};

EXTERN_CVAR (Float, Gamma)

void V_Init ();
bool V_SetResolution (int width, int height, int bits);

// This is the screen updated by I_FinishUpdate.
extern DFrameBuffer *screen;

#define SCREENWIDTH (screen->GetWidth ())
#define SCREENHEIGHT (screen->GetHeight ())
#define SCREENPITCH (screen->GetPitch ())

struct brokenlines_s
{
	short width;
	byte nlterminated;
	byte pad;
	char *string;
};
typedef struct brokenlines_s brokenlines_t;

#define TEXTCOLOR_ESCAPE	'\x1c'

#define TEXTCOLOR_BRICK		"\x1c""A"
#define TEXTCOLOR_TAN		"\x1c""B"
#define TEXTCOLOR_GRAY		"\x1c""C"
#define TEXTCOLOR_GREY		"\x1c""C"
#define TEXTCOLOR_GREEN		"\x1c""D"
#define TEXTCOLOR_BROWN		"\x1c""E"
#define TEXTCOLOR_GOLD		"\x1c""F"
#define TEXTCOLOR_RED		"\x1c""G"
#define TEXTCOLOR_BLUE		"\x1c""H"
#define TEXTCOLOR_ORANGE	"\x1c""I"
#define TEXTCOLOR_WHITE		"\x1c""J"
#define TEXTCOLOR_YELLOW	"\x1c""K"

#define TEXTCOLOR_NORMAL	"\x1c-"
#define TEXTCOLOR_BOLD		"\x1c+"

brokenlines_t *V_BreakLines (int maxwidth, const byte *str, bool keepspace=false);
void V_FreeBrokenLines (brokenlines_t *lines);
inline brokenlines_t *V_BreakLines (int maxwidth, const char *str, bool keepspace=false)
 { return V_BreakLines (maxwidth, (const byte *)str, keepspace); }

#endif // __V_VIDEO_H__
