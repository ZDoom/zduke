/*
** v_palette.h
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

#ifndef __V_PALETTE_H__
#define __V_PALETTE_H__

#include "doomtype.h"
//#include "r_main.h"

#define MAKERGB(r,g,b)		(((r)<<16)|((g)<<8)|(b))
#define MAKEARGB(a,r,g,b)	(((a)<<24)|((r)<<16)|((g)<<8)|(b))

#define APART(c)			(((c)>>24)&0xff)
#define RPART(c)			(((c)>>16)&0xff)
#define GPART(c)			(((c)>>8)&0xff)
#define BPART(c)			((c)&0xff)

struct PalEntry
{
	PalEntry () {}
	PalEntry (DWORD argb) { *(DWORD *)this = argb; }
	operator DWORD () { return *(DWORD *)this; }
	PalEntry &operator= (DWORD other) { *(DWORD *)this = other; return *this; }

#ifdef __BIG_ENDIAN__
	PalEntry (BYTE ir, BYTE ig, BYTE ib) : a(0), r(ir), g(ig), b(ib) {}
	PalEntry (BYTE ia, BYTE ir, BYTE ig, BYTE ib) : a(ia), r(ir), g(ig), b(ib) {}
	BYTE a,r,g,b;
#else
	PalEntry (BYTE ir, BYTE ig, BYTE ib) : b(ib), g(ig), r(ir), a(0) {}
	PalEntry (BYTE ia, BYTE ir, BYTE ig, BYTE ib) : b(ib), g(ig), r(ir), a(ia) {}
	BYTE b,g,r,a;
#endif
};

struct FPalette
{
	FPalette ();
	FPalette (const BYTE *colors);

	void SetPalette (const BYTE *colors);
	void GammaAdjust ();

	PalEntry	Colors[256];		// gamma corrected palette
	PalEntry	BaseColors[256];	// non-gamma corrected palette
};

struct FDynamicColormap
{
	void ChangeFade (PalEntry fadecolor);
	void ChangeColor (PalEntry lightcolor);
	void ChangeColorFade (PalEntry lightcolor, PalEntry fadecolor);
	void BuildLights ();

	BYTE *Maps;
	PalEntry Color;
	PalEntry Fade;
	FDynamicColormap *Next;
};

extern BYTE *InvulnerabilityColormap;
extern FPalette GPalette;
extern "C" {
extern FDynamicColormap NormalLight;
}
extern int Near0;		// A color near 0 in appearance, but not 0

int BestColor (const DWORD *pal, int r, int g, int b, int first = 0);

void InitPalette ();

// V_SetBlend()
//	input: blendr: red component of blend
//		   blendg: green component of blend
//		   blendb: blue component of blend
//		   blenda: alpha component of blend
//
// Applies the blend to all palettes with PALETTEF_BLEND flag
void V_SetBlend (int blendr, int blendg, int blendb, int blenda);

// V_ForceBlend()
//
// Normally, V_SetBlend() does nothing if the new blend is the
// same as the old. This function will performing the blending
// even if the blend hasn't changed.
void V_ForceBlend (int blendr, int blendg, int blendb, int blenda);


// Colorspace conversion RGB <-> HSV
void RGBtoHSV (float r, float g, float b, float *h, float *s, float *v);
void HSVtoRGB (float *r, float *g, float *b, float h, float s, float v);

FDynamicColormap *GetSpecialLights (PalEntry lightcolor, PalEntry fadecolor);

#endif //__V_PALETTE_H__
