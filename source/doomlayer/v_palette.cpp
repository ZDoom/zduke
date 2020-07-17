/*
** v_palette.cpp
** Automatic colormap generation for "colored lights", etc.
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

#include <stddef.h>
#include <string.h>
#include <math.h>

#include "templates.h"
#include "v_video.h"
#include "m_alloc.h"
#include "i_video.h"
#include "c_dispatch.h"

FPalette GPalette;

/* Current color blending values */
int		BlendR, BlendG, BlendB, BlendA;


/**************************/
/* Gamma correction stuff */
/**************************/

byte newgamma[256];
CUSTOM_CVAR (Float, Gamma, 1.f, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
{
	if (self == 0.f)
	{ // Gamma values of 0 are illegal.
		self = 1.f;
		return;
	}

	if (screen != NULL)
	{
		screen->SetGamma (self);
	}
}


/****************************/
/* Palette management stuff */
/****************************/

extern "C"
{
	byte BestColor_MMX (DWORD rgb, const DWORD *pal);
}

int BestColor (const DWORD *pal_in, int r, int g, int b, int first)
{
#if defined(USEASM) && WORKINPROGRESS
	if (UseMMX)
	{
		return BestColor_MMX ((first<<24)|(r<<16)|(g<<8)|b, pal_in);
	}
#endif
	const PalEntry *pal = (const PalEntry *)pal_in;
	int bestcolor = first;
	int bestdist = 257*257+257*257+257*257;

	for (int color = first; color < 256; color++)
	{
		int dist = (r-pal[color].r)*(r-pal[color].r)+
				   (g-pal[color].g)*(g-pal[color].g)+
				   (b-pal[color].b)*(b-pal[color].b);
		if (dist < bestdist)
		{
			if (dist == 0)
				return color;

			bestdist = dist;
			bestcolor = color;
		}
	}
	return bestcolor;
}

FPalette::FPalette ()
{
}

FPalette::FPalette (const BYTE *colors)
{
	SetPalette (colors);
}

void FPalette::SetPalette (const BYTE *colors)
{
	int i;

	for (i = 0; i < 256; i++, colors += 3)
		BaseColors[i] = PalEntry (colors[0], colors[1], colors[2]);

	GammaAdjust ();
}

void InitPalette ()
{
	const BYTE *pal;

#if WORKINPROGRESS
	pal = (BYTE *)W_MapLumpName ("PLAYPAL");
	GPalette.SetPalette (pal);
	W_UnMapLump (pal);

	ColorMatcher.SetPalette ((DWORD *)GPalette.BaseColors);
#endif
}

void FPalette::GammaAdjust ()
{
	int i;

	for (i = 0; i < 256; i++)
	{
		PalEntry color = BaseColors[i];
		Colors[i] = PalEntry (newgamma[color.r], newgamma[color.g], newgamma[color.b]);
	}
}

extern "C"
{
	void STACK_ARGS DoBlending_MMX (const PalEntry *from, PalEntry *to, int count, int r, int g, int b, int a);
}

void DoBlending (const PalEntry *from, PalEntry *to, int count, int r, int g, int b, int a)
{
	if (a == 0)
	{
		if (from != to)
		{
			memcpy (to, from, count * sizeof(DWORD));
		}
	}
	else if (a == 256)
	{
		DWORD t = MAKERGB(r,g,b);
		int i;

		for (i = 0; i < count; i++)
		{
			to[i] = t;
		}
	}
#if defined(USEASM) && WORKINPROGRESS
	else if (UseMMX && !(count & 1))
	{
		DoBlending_MMX (from, to, count, r, g, b, a);
	}
#endif
	else
	{
		int i, ia;

		ia = 256 - a;
		r *= a;
		g *= a;
		b *= a;

		for (i = count; i > 0; i--, to++, from++)
		{
			to->r = (r + from->r*ia) >> 8;
			to->g = (g + from->g*ia) >> 8;
			to->b = (b + from->b*ia) >> 8;
		}
	}

}
