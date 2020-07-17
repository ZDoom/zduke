/*
** i_sound.cpp
** System interface for sound; uses fmod.dll
**
**---------------------------------------------------------------------------
** Copyright 1998-2003 Randy Heit
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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "resource.h"
extern HWND Window;
extern HINSTANCE g_hInst;
#else
#define FALSE 0
#define TRUE 1
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "templates.h"
#include "doomtype.h"
#include "m_alloc.h"
#include "pitch.h"
#include <math.h>


#include <fmod.h>
#include "sample_flac.h"

#include "stats.h"

#include "c_cvars.h"
#include "c_dispatch.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_music.h"
#include "m_argv.h"
//#include "m_misc.h"
//#include "w_wad.h"
#include "i_video.h"
#include "s_sound.h"
#include "duke3d.h"

#include "doomdef.h"

#define NUM_SOFT_CHANNELS	256

EXTERN_CVAR (Float, snd_musicvolume)

#define DistScale (500.f)

extern void CalcPosVel (fixed_t *pt, AActor *mover, int constz, float pos[3], float vel[3]);

static const char *OutputNames[] =
{
	"No sound",
	"Windows Multimedia",
	"DirectSound",
	"A3D",
	"OSS (Open Sound System)",
	"ESD (Enlightenment Sound Daemon)",
	"ALSA (Advanced Linux Sound Architecture)"
};
static const char *MixerNames[] =
{
	"Auto",
	"Non-MMX blendmode",
	"Pentium MMX",
	"PPro MMX",
	"Quality auto",
	"Quality FPU",
	"Quality Pentium MMX",
	"Quality PPro MMX"
};

EXTERN_CVAR (Float, snd_sfxvolume)
CVAR (Int, snd_samplerate, 44100, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR (Int, snd_buffersize, 0, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR (Int, snd_driver, 0, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR (String, snd_output, "default", CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR (Bool, snd_3d, false, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR (Bool, snd_waterreverb, true, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR (Bool, snd_fpumixer, false, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)

// killough 2/21/98: optionally use varying pitched sounds
CVAR (Bool, snd_pitched, false, CVAR_ARCHIVE)
#define PITCH(f,x) (snd_pitched ? ((f)*(x))/128 : (f))

bool _nosound = 0;
bool Sound3D;

int NumSFXChannels;
static unsigned int DriverCaps;
static int OutputType;

FSOUND_DSPUNIT *EchoUnit, *DryUnit;
void *EchoBuffer;
int EchoLen, EchoPos;
int EchoVolume;

static bool SoundDown = true;

#if 0
static const char *FmodErrors[] =
{
	"No errors",
	"Cannot call this command after FSOUND_Init.  Call FSOUND_Close first.",
	"This command failed because FSOUND_Init was not called",
	"Error initializing output device.",
	"Error initializing output device, but more specifically, the output device is already in use and cannot be reused.",
	"Playing the sound failed.",
	"Soundcard does not support the features needed for this soundsystem (16bit stereo output)",
	"Error setting cooperative level for hardware.",
	"Error creating hardware sound buffer.",
	"File not found",
	"Unknown file format",
	"Error loading file",
	"Not enough memory ",
	"The version number of this file format is not supported",
	"Incorrect mixer selected",
	"An invalid parameter was passed to this function",
	"Tried to use a3d and not an a3d hardware card, or dll didnt exist, try another output type.",
	"Tried to use an EAX command on a non EAX enabled channel or output.",
	"Failed to allocate a new channel"
};
#endif

static unsigned int F_CALLBACKAPI OpenCallback (const char *name)
{
	return kopen4load (name, 0) + 1;
}

static void F_CALLBACKAPI CloseCallback (unsigned int handle)
{
	kclose (handle - 1);
}

static int F_CALLBACKAPI ReadCallback (void *buffer, int size, unsigned int handle)
{
	return kread (handle - 1, buffer, size);
}

static int F_CALLBACKAPI SeekCallback (unsigned int handle, int pos, signed char mode)
{
	return klseek (handle - 1, pos, mode);
}

static int F_CALLBACKAPI TellCallback (unsigned int handle)
{
	return ktell (handle - 1);
}

// FSOUND_Sample_Upload seems to mess up the signedness of sound data when
// uploading to hardware buffers. The pattern is not particularly predictable,
// so this is a replacement for it that loads the data manually. Source data
// is mono, unsigned, 8-bit. Output is mono, signed, 8- or 16-bit.

static int PutSampleData (FSOUND_SAMPLE *sample, const BYTE *data, int len,
	unsigned int mode)
{
	if (!(mode & FSOUND_HW3D))
	{
		return FSOUND_Sample_Upload (sample, const_cast<BYTE *>(data),
			FSOUND_8BITS|FSOUND_MONO|FSOUND_UNSIGNED);
	}
	else if (FSOUND_Sample_GetMode (sample) & FSOUND_8BITS)
	{
		void *ptr1, *ptr2;
		unsigned int len1, len2;

		if (FSOUND_Sample_Lock (sample, 0, len, &ptr1, &ptr2, &len1, &len2))
		{
			int i;
			BYTE *ptr;
			int len;

			for (i = 0, ptr = (BYTE *)ptr1, len = len1;
				 i < 2 && ptr && len;
				i++, ptr = (BYTE *)ptr2, len = len2)
			{
				int j;
				for (j = 0; j < len; j++)
				{
					ptr[j] = *data++ - 128;
				}
			}
			FSOUND_Sample_Unlock (sample, ptr1, ptr2, len1, len2);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		void *ptr1, *ptr2;
		unsigned int len1, len2;

		if (FSOUND_Sample_Lock (sample, 0, len*2, &ptr1, &ptr2, &len1, &len2))
		{
			int i;
			SWORD *ptr;
			int len;

			for (i = 0, ptr = (SWORD *)ptr1, len = len1/2;
				 i < 2 && ptr && len;
				i++, ptr = (SWORD *)ptr2, len = len2/2)
			{
				int j;
				for (j = 0; j < len; j++)
				{
					ptr[j] = ((*data<<8)|(*data)) - 32768;
					data++;
				}
			}
			FSOUND_Sample_Unlock (sample, ptr1, ptr2, len1, len2);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

long readwrap (long handle, void *mem, long len)
{
	return read (handle, mem, len);
}

long lseekwrap (long handle, long offset, long whence)
{
	return lseek (handle, offset, whence);
}

static void DoLoad (void **slot, sfxinfo_t *sfx)
{
	long (*reader)(long,void*,long);
	long (*seeker)(long,long,long);
	BYTE header[8];
	unsigned int frequency, length;
	byte *sfxdata;
	int size;
	int errcount;
	unsigned long samplemode;
	int fp;

	samplemode = Sound3D ? FSOUND_HW3D : 0;
	sfxdata = NULL;

	errcount = 0;
	while (sfx->Name[0] != 0 && errcount < 2)
	{
		if (sfxdata != NULL)
		{
			delete[] sfxdata;
			sfxdata = NULL;
		}

		if (errcount)
		{
			sfx->Name[0] = 0;
			sfx->bWadLump = false;
			continue;
		}

		if (sfx->bWadLump)
		{
			size = RTS_SoundLength (sfx->LumpNum);
			if (size == 0)
			{
				errcount++;
				continue;
			}
			fp = RTS_OpenLump (sfx->LumpNum);
			reader = readwrap;
			seeker = lseekwrap;
		}
		else
		{
			fp = kopen4load (sfx->Name, 0);
			if (fp == -1)
			{
				errcount++;
				continue;
			}

			size = kfilelength (fp);
			if (size == 0)
			{
				kclose (fp);
				errcount++;
				continue;
			}
			reader = kread;
			seeker = klseek;
		}

		reader (fp, header, 8);

		//sfxdata = (const byte *)W_MapLumpNum (sfx->lumpnum);
		SDWORD len = ((SDWORD *)header)[1];
		bool voc = false;

		// If the sound is raw, just load it as such.
		// Otherwise, try the sound as DMX or VOC format.
		// If that fails, let FMOD try and figure it out.
		if (sfx->bLoadRAW ||
			(header[0] == 3 && header[1] == 0 && len <= size - 8) ||
			(strncmp ((const char *)header, "Creative", 8) == 0 && (voc = true /*This must be = and not ==*/)))
		{
			FSOUND_SAMPLE *sample;
			unsigned int bits;

			bits = FSOUND_8BITS;

			if (sfx->bLoadRAW)
			{
				len = size;
				frequency = (sfx->bForce22050 ? 22050 : 11025);
				sfxdata = new BYTE[len];
				memcpy ((void *)sfxdata, header, 8);
				reader (fp, (void *)(sfxdata+8), len);
			}
			else if (voc)
			{
				BYTE block[4];
				DWORD blen;
				DWORD foo4;
				WORD foo2;
				BYTE foo1;

				len = 0;
				sfxdata = new BYTE[size];

				seeker (fp, 26-8, SEEK_CUR);

				while (ktell (fp) < size)
				{
					reader (fp, &block[0], 1);

					if (block[0] == 0)
					{
						break;
					}

					reader (fp, &block[1], 3);
					blen = block[1] + block[2]*256 + block[3]*65536;
					switch (block[0])
					{
					case 9:
						reader (fp, &foo4, 4);
						frequency = foo4;
						reader (fp, &foo1, 1);	// BitsPerSample
						if (foo1 == 16)
						{
							bits = FSOUND_16BITS;
						}
						reader (fp, &foo1, 1);	// NumChannels
						reader (fp, &foo2, 2);	// Format
						if (foo2 == 0 || foo2 == 4)
						{
							seeker (fp, 4, SEEK_CUR);
							reader (fp, (void *)(sfxdata + len), blen - 12);
							len += blen - 12;
						}
						break;

					case 1:
						reader (fp, &foo1, 1);	// time constant
						frequency = 1000000 / (256 - foo1);
//						Printf ("%s: SR = %d, Freq = %d\n", sfx->Name, foo1, frequency);
						reader (fp, &foo1, 1);	// compression type
						if (foo1 == 0)
						{
							reader (fp, (void *)(sfxdata + len), blen - 2);
							len += blen - 2;
						}
						break;

					case 2:
						reader (fp, (void *)(sfxdata + len), blen);
						len += blen;
						break;

					case 3:
						if (blen == 3)
						{
							reader (fp, &foo2, 2);	// pause period
							reader (fp, &foo1, 1);	// time constant
							if (bits == FSOUND_8BITS)
							{
								memset ((void *)(sfxdata + len), 128, foo2);
							}
							else
							{
								memset ((void *)(sfxdata + len), 0, foo2*2);
							}
							len += foo2 << ((bits == FSOUND_16BITS)?1:0);
							break;
						}

					default:
						seeker (fp, blen, SEEK_CUR);
						//Printf ("block %d in %s\n", block[0], sfx->Name);
						break;
					}
				}
			}
			else
			{
				frequency = ((WORD *)header)[1];
				if (frequency == 0)
				{
					frequency = 11025;
				}
				len = min(size-8, len);
				sfxdata = new BYTE[len];
				reader (fp, (void *)sfxdata, len);
			}
			if (reader == kread)
			{
				kclose (fp);
			}
			if (len == 0)
			{
				if (sfxdata != NULL)
				{
					delete[] sfxdata;
					sfxdata = NULL;
				}
				errcount++;
				continue;
			}
			if (bits == FSOUND_16BITS)
			{
				length = len/2;
			}
			else
			{
				length = len;
			}

			do
			{
				sample = FSOUND_Sample_Alloc (FSOUND_FREE, length,
					samplemode|bits|FSOUND_LOOP_OFF|FSOUND_MONO,
					frequency, 255, FSOUND_STEREOPAN, 0);
			} while (sample == NULL && (bits <<= 1) <= FSOUND_16BITS);

			if (sample == NULL)
			{
				DPrintf ("Failed to allocate sample: %d\n", FSOUND_GetError ());
				errcount++;
				continue;
			}

			if (length < (size_t)len)
			{ // 16 bit source
				if (!FSOUND_Sample_Upload (sample, sfxdata,
					FSOUND_16BITS|FSOUND_MONO|FSOUND_SIGNED))
				{
					DPrintf ("Failed to upload sample: %d\n", FSOUND_GetError ());
					FSOUND_Sample_Free (sample);
					sample = NULL;
					errcount++;
					continue;
				}
			}
			else if (!PutSampleData (sample, sfxdata, len, samplemode))
			{
				DPrintf ("Failed to upload sample: %d\n", FSOUND_GetError ());
				FSOUND_Sample_Free (sample);
				sample = NULL;
				errcount++;
				continue;
			}
			FSOUND_Sample_SetDefaults (sample, -1, -1, FSOUND_STEREOPAN, sfx->Priority);
			*slot = sample;
		}
		else if (reader == kread)
		{
			kclose (fp);
			if (header[0] == 'f' && header[1] == 'L' &&
				header[2] == 'a' && header[3] == 'C')
			{
				FLACSampleLoader loader (sfx);
				*slot = loader.LoadSample (samplemode);
				if (*slot == NULL && FSOUND_GetError() == FMOD_ERR_CREATEBUFFER && samplemode == FSOUND_HW3D)
				{
					DPrintf ("Trying to fall back to software sample\n");
					*slot = loader.LoadSample (0);
				}
			}
			else
			{
				*slot = FSOUND_Sample_Load (FSOUND_FREE, sfx->Name, samplemode, size);
				if (*slot == NULL && FSOUND_GetError() == FMOD_ERR_CREATEBUFFER && samplemode == FSOUND_HW3D)
				{
					DPrintf ("Trying to fall back to software sample\n");
					*slot = FSOUND_Sample_Load (FSOUND_FREE, sfx->Name, FSOUND_LOADMEMORY, size);
				}
			}
		}
		break;
	}

	if (sfx->data)
	{
		DPrintf ("sound loaded: %s\n", sfx->Name);

		// Max distance is irrelevant, alas.
		FSOUND_Sample_SetMinMaxDistance ((FSOUND_SAMPLE *)sfx->data,
			/*(float)2.5625f*/6720.f/DistScale/2, (float)5000.f);
	}

	if (sfxdata != NULL)
	{
		delete[] sfxdata;
	}
}

static bool getsfx (sfxinfo_t *sfx)
{
	unsigned int i;

	// Get the sound data from the WAD and register it with sound library

	// If the sound doesn't exist, replace it with the empty sound.
	if (sfx->Name[0] == 0)
	{
		return false;
	}
	
	// See if there is another sound already initialized with this lump. If so,
	// then set this one up as a link, and don't load the sound again.
	for (i = 0; i < S_sfx.Size (); i++)
	{
		if (S_sfx[i].data && S_sfx[i].link == sfxinfo_t::NO_LINK &&
			stricmp (S_sfx[i].Name, sfx->Name) == 0)
		{
			DPrintf ("Linked to %d\n", i);
			sfx->link = i;
			return true;
		}
	}

	sfx->bHaveLoop = false;
	sfx->bPlayedNormal = false;
	sfx->bPlayedLoop = false;
	sfx->altdata = NULL;
	DoLoad (&sfx->data, sfx);

	return sfx->data != NULL;
}


// Right now, FMOD's biggest shortcoming compared to MIDAS is that it does not
// support multiple samples with the same sample data. Thus, if we want to
// play a looped and non-looped version of the same sound, we need to create
// two copies of it. Fortunately, most sounds will either be played looped or
// not, so this really isn't too much of a problem. This function juggles the
// sample between looping and non-looping, creating a copy if necessary. Note
// that once a sample is played normal or looped, a new copy of the sample
// will be created if it needs to be played the other way.
//
// Update: FMOD 3.3 added FSOUND_SetLoopMode to set a channel's looping status,
// but that only works with software channels. So I think I will continue to
// do this even though I don't *have* to anymore.

static FSOUND_SAMPLE *CheckLooping (sfxinfo_t *sfx, BOOL looped)
{
	if (looped)
	{
		if (sfx->bHaveLoop)
		{
			return (FSOUND_SAMPLE *)(sfx->altdata ? sfx->altdata : sfx->data);
		}
		else
		{
			sfx->bPlayedLoop = true;
			if (!sfx->bPlayedNormal)
			{
				sfx->bHaveLoop = true;
				FSOUND_Sample_SetMode ((FSOUND_SAMPLE *)sfx->data, FSOUND_LOOP_NORMAL);
				return (FSOUND_SAMPLE *)sfx->data;
			}
		}
	}
	else
	{
		sfx->bPlayedNormal = true;
		if (sfx->altdata || !sfx->bHaveLoop)
		{
			return (FSOUND_SAMPLE *)sfx->data;
		}
	}

	// If we get here, we need to create an alternate version of the sample.
	FSOUND_Sample_SetMode ((FSOUND_SAMPLE *)sfx->data, FSOUND_LOOP_OFF);
	DoLoad (&sfx->altdata, sfx);
	FSOUND_Sample_SetMode ((FSOUND_SAMPLE *)sfx->altdata, FSOUND_LOOP_NORMAL);
	sfx->bHaveLoop = true;
	return (FSOUND_SAMPLE *)(looped ? sfx->altdata : sfx->data);
}

//
// SFX API
//

//==========================================================================
//
// CVAR snd_sfxvolume
//
// Maximum volume of a sound effect.
//==========================================================================

CUSTOM_CVAR (Float, snd_sfxvolume, 0.863f, CVAR_ARCHIVE|CVAR_GLOBALCONFIG|CVAR_NOINITCALL)
{
	if (self < 0.f)
		self = 0.f;
	else if (self > 1.f)
		self = 1.f;
	else
	{
		FSOUND_SetSFXMasterVolume ((int)(self * 255.f));
		// FMOD apparently resets absolute volume channels when setting master vol
		snd_musicvolume.Callback ();
	}
}

void * F_CALLBACKAPI EchoMixer (void *originalbuffer, void *newbuffer, int length, int param)
{
	int mixer = FSOUND_GetMixer();
	int bits;
	int epos;

	if (EchoVolume == 0)
	{
		return newbuffer;
	}

	if (mixer==FSOUND_MIXER_QUALITY_MMXP5 || mixer==FSOUND_MIXER_QUALITY_MMXP6)
	{
		bits = 2;
	}
	else
	{
		bits = 3;
	}

	epos = EchoPos << bits;

	if (EchoPos + length <= EchoLen)
	{
		FSOUND_DSP_MixBuffers (newbuffer, (void *)((BYTE *)EchoBuffer + epos), length,
			snd_samplerate, EchoVolume, FSOUND_STEREOPAN, FSOUND_STEREO | FSOUND_16BITS);
		memcpy ((void *)((BYTE *)EchoBuffer + epos), newbuffer, length << bits);

		EchoPos += length;
		if (EchoPos == EchoLen)
		{
			EchoPos = 0;
		}
	}
	else
	{
		int l1 = EchoLen - EchoPos;
		int l2 = length - l1;

		FSOUND_DSP_MixBuffers (newbuffer, (void *)((BYTE *)EchoBuffer + epos), l1,
			EchoVolume, snd_samplerate, FSOUND_STEREOPAN, FSOUND_STEREO | FSOUND_16BITS);
		FSOUND_DSP_MixBuffers ((void *)((BYTE *)newbuffer + (l1 << bits)), EchoBuffer, l2,
			EchoVolume, snd_samplerate, FSOUND_STEREOPAN, FSOUND_STEREO | FSOUND_16BITS);
		memcpy ((void *)((BYTE *)EchoBuffer + epos), newbuffer, l1 << bits);
		memcpy (EchoBuffer, (void *)((BYTE *)newbuffer + (l1 << bits)), l2 << bits);

		EchoPos = l2;
	}
	return newbuffer;
}

void SetEAXEcho ()
{
	if (EchoVolume == 0)
	{
		FSOUND_REVERB_PROPERTIES noEcho = FSOUND_PRESET_OFF;
		FSOUND_Reverb_SetProperties (&noEcho);
	}
	else
	{
		FSOUND_REVERB_PROPERTIES *daEcho;
		FSOUND_REVERB_PROPERTIES sewer = FSOUND_PRESET_SEWERPIPE;
		FSOUND_REVERB_PROPERTIES generic = FSOUND_PRESET_GENERIC;
		
		// The sewer pipe sounds much better on my Audigy than on my nForce2
		if (DriverCaps & FSOUND_CAPS_EAX3)
		{
			daEcho = &sewer;
			daEcho->Room = EchoVolume * -2000 / 255 - 500;
		}
		else
		{
			daEcho = &generic;
			daEcho->Reverb = EchoVolume * 2000 / 255;
		}

		FSOUND_Reverb_SetProperties (daEcho);
	}
}

void I_LoadSound (sfxinfo_t *sfx)
{
	if (_nosound)
		return;

	if (!sfx->data && (sfx->Name[0] != 0 || sfx->bWadLump))
	{
		DPrintf ("loading sound \"%s\" (%d)\n", sfx->Name, sfx - &S_sfx[0]);
		getsfx (sfx);
	}
}

#ifdef _WIN32
// [RH] Dialog procedure for the error dialog that appears if FMOD
//		could not be initialized for some reason.
BOOL CALLBACK InitBoxCallback (HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		if (wParam == IDOK ||
			wParam == IDC_NOSOUND ||
			wParam == IDCANCEL)
		{
			EndDialog (hwndDlg, wParam);
			return TRUE;
		}
		break;
	}
	return FALSE;
}
#endif

static char FModLog (char success)
{
	if (success)
	{
		Printf (" succeeded\n");
	}
	else
	{
		Printf (" failed (error %d)\n", FSOUND_GetError());
	}
	return success;
}

void I_InitSound ()
{
#ifdef _WIN32
	static const FSOUND_OUTPUTTYPES outtypes[2] =
	{ FSOUND_OUTPUT_DSOUND, FSOUND_OUTPUT_WINMM };
	const int maxtrynum = 2;
#else
	static const FSOUND_OUTPUTTYPES outtypes[3] =
	{ FSOUND_OUTPUT_ALSA, FSOUND_OUTPUT_OSS, FSOUND_OUTPUT_ESD };
	const int maxtrynum = 3;
#endif
#if 0
	bool trya3d = false;
#endif

	/* Get command line options: */
	_nosound = !!Args.CheckParm ("-nosfx") || !!Args.CheckParm ("-nosound");

	if (_nosound)
	{
		I_InitMusic ();
		return;
	}

	FSOUND_File_SetCallbacks (OpenCallback, CloseCallback, ReadCallback, SeekCallback, TellCallback);

	int outindex;
	int trynum;

	// clamp snd_samplerate to FMOD's limits
	if (snd_samplerate < 4000)
	{
		snd_samplerate = 4000;
	}
	else if (snd_samplerate > 65535)
	{
		snd_samplerate = 65535;
	}
	//PrevEnvironment = DefaultEnvironments[0];

#ifdef _WIN32
	if (stricmp (snd_output, "dsound") == 0 || stricmp (snd_output, "directsound") == 0)
	{
		outindex = 0;
	}
	else if (stricmp (snd_output, "winmm") == 0 || stricmp (snd_output, "waveout") == 0)
	{
		outindex = 1;
	}
	else
	{
		// If snd_3d is true, try for a3d output if snd_output was not recognized above.
		// However, if running under NT 4.0, a3d will only be tried if specifically requested.
		outindex = (OSPlatform == os_WinNT) ? 1 : 0;
#if 0
		// FMOD 3.6 no longer supports a3d. Keep this code here in case support comes back.
		if (stricmp (snd_output, "a3d") == 0 || (outindex == 0 && snd_3d))
		{
			trya3d = true;
		}
#endif
	}
#else
	if (stricmp (snd_output, "oss") == 0)
	{
		outindex = 2;
	}
	else if (stricmp (snd_output, "esd") == 0 ||
			 stricmp (snd_output, "esound") == 0)
	{
		outindex = 1;
	}
	else
	{
		outindex = 0;
	}
#endif
	
	Printf ("I_InitSound: Initializing FMOD\n");
#ifdef _WIN32
	FSOUND_SetHWND (Window);
#endif

	if (snd_fpumixer)
	{
		FSOUND_SetMixer (FSOUND_MIXER_QUALITY_FPU);
	}
	else
	{
		FSOUND_SetMixer (FSOUND_MIXER_AUTODETECT);
	}

#ifdef _WIN32
	if (OSPlatform == os_WinNT)
	{
		// If running Windows NT 4, we need to initialize DirectSound before
		// using WinMM. If we don't, then FSOUND_Close will corrupt a
		// heap. This might just be the Audigy's drivers--I don't know why
		// it happens. At least the fix is simple enough. I only need to
		// initialize DirectSound once, and then I can initialize/close
		// WinMM as many times as I want.
		//
		// Yes, using WinMM under NT 4 is a good idea. I can get latencies as
		// low as 20 ms with WinMM, but with DirectSound I need to have the
		// latency as high as 120 ms to avoid crackling--quite the opposite
		// from the other Windows versions with real DirectSound support.

		static bool initedDSound = false;

		if (!initedDSound)
		{
			FSOUND_SetOutput (FSOUND_OUTPUT_DSOUND);
			if (FSOUND_GetOutput () == FSOUND_OUTPUT_DSOUND)
			{
				if (FSOUND_Init (snd_samplerate, NUM_SOFT_CHANNELS, 0))
				{
					Sleep (50);
					FSOUND_Close ();
					initedDSound = true;
				}
			}
		}
	}
#endif

	while (!_nosound)
	{
		trynum = 0;
		while (trynum < maxtrynum)
		{
			long outtype = /*trya3d ? FSOUND_OUTPUT_A3D :*/
						   outtypes[(outindex+trynum) % maxtrynum];

			Printf ("  Setting %s output", OutputNames[outtype]);
			FModLog (FSOUND_SetOutput (outtype));
			if (FSOUND_GetOutput() != outtype)
			{
				Printf ("  Got %s output instead.\n", OutputNames[FSOUND_GetOutput()]);
#if 0
				if (trya3d)
				{
					trya3d = false;
				}
				else
#endif
				{
					++trynum;
				}
				continue;
			}
			Printf ("  Setting driver %d", *snd_driver);
			FModLog (FSOUND_SetDriver (snd_driver));
			if (FSOUND_GetOutput() != outtype)
			{
				Printf ("   Output changed to %s\n   Trying driver 0",
					OutputNames[FSOUND_GetOutput()]);
				FSOUND_SetOutput (outtype);
				FModLog (FSOUND_SetDriver (0));
			}
			if (snd_buffersize)
			{
				Printf ("  Setting buffer size %d", *snd_buffersize);
				FModLog (FSOUND_SetBufferSize (snd_buffersize));
			}
			FSOUND_GetDriverCaps (FSOUND_GetDriver(), &DriverCaps);
			Printf ("  Initialization");
			if (!FModLog (FSOUND_Init (snd_samplerate, 64, 0)))
			{
#if 0
				if (trya3d)
				{
					trya3d = false;
				}
				else
#endif
				{
					trynum++;
				}
			}
			else
			{
				break;
			}
		}
		if (trynum < 2)
		{ // Initialized successfully
			break;
		}
#ifdef _WIN32
		// If sound cannot be initialized, give the user some options.
		switch (DialogBox (g_hInst,
						   MAKEINTRESOURCE(IDD_FMODINITFAILED),
						   (HWND)Window,
						   (DLGPROC)InitBoxCallback))
		{
		case IDC_NOSOUND:
			_nosound = true;
			break;

		case IDCANCEL:
			exit (0);
			break;
		}
#else
		Printf ("Sound init failed. Using nosound.\n");
		_nosound = true;
#endif
	}

	if (!_nosound)
	{
		OutputType = FSOUND_GetOutput ();
		Sound3D = false;
		if (snd_3d)
		{
			Sound3D = true;
			FSOUND_3D_SetRolloffFactor (1.f);
			FSOUND_3D_SetDopplerFactor (1.f);
			FSOUND_3D_SetDistanceFactor (100.f);	// Distance factor only affects doppler!
		}
		if (!(DriverCaps & FSOUND_CAPS_HARDWARE) || !Sound3D)
		{
			int mixer = FSOUND_GetMixer();
			int sampleSize = (mixer==FSOUND_MIXER_QUALITY_MMXP5||mixer==FSOUND_MIXER_QUALITY_MMXP6)?4:8;
			EchoLen = snd_samplerate * 256 / 11025;
			EchoBuffer = calloc (EchoLen, sampleSize);
			if (EchoBuffer != NULL)
			{
				EchoUnit = FSOUND_DSP_Create (EchoMixer, FSOUND_DSP_DEFAULTPRIORITY_SFXUNIT+1, 0);
				if (EchoUnit == NULL)
				{
					free (EchoBuffer);
					EchoBuffer = NULL;
				}
				else
				{
					FSOUND_DSP_SetActive (EchoUnit, TRUE);
					DryUnit = FSOUND_DSP_Create (NULL, FSOUND_DSP_DEFAULTPRIORITY_MUSICUNIT-1, 0);
					if (DryUnit != NULL)
					{
						FSOUND_DSP_SetActive (DryUnit, TRUE);
					}
				}
			}
		}
		else
		{
			SetEAXEcho ();
		}

		snd_sfxvolume.Callback ();

		static bool didthis = false;
		if (!didthis)
		{
			didthis = true;
			atterm (I_ShutdownSound);
		}
		SoundDown = false;
	}

	I_InitMusic ();

	snd_sfxvolume.Callback ();
}

CCMD (rolloff)
{
	if (argv.argc()>1)
		FSOUND_3D_SetRolloffFactor (atof (argv[1]));
}

int I_SetChannels (int numchannels)
{
	int maxchan;
	int i;

	if (_nosound)
	{
		return numchannels;
	}


	// If using 3D sound, use all the hardware channels available,
	// regardless of what the user sets with snd_channels. If there
	// are fewer than 8 hardware channels, then force software.
	if (Sound3D)
	{
		int hardChans = FSOUND_GetNumHardwareChannels ();

		if (hardChans < 8)
		{
			Sound3D = false;
		}
		else
		{
			numchannels = hardChans;
		}
	}

	maxchan = FSOUND_GetMaxChannels ();
	numchannels = min (numchannels, maxchan);

	if (!Sound3D)
	{
		for (i = 0; i < numchannels; ++i)
		{
			FSOUND_SetReserved (i, FALSE);
		}
		for (; i < maxchan; ++i)
		{
			FSOUND_SetReserved (i, TRUE);
		}
	}

	NumSFXChannels = numchannels;
	NumVoices = numchannels;
	return numchannels;
}

void STACK_ARGS I_ShutdownSound (void)
{
	if (!_nosound && !SoundDown)
	{
		size_t i;

		SoundDown = true;

		FSOUND_StopSound (FSOUND_ALL);

		// Free all loaded samples
		for (i = 0; i < S_sfx.Size (); i++)
		{
			S_sfx[i].data = NULL;
			S_sfx[i].altdata = NULL;
			S_sfx[i].bHaveLoop = false;
			S_sfx[i].link = sfxinfo_t::NO_LINK;
		}

		if (EchoUnit != NULL)
		{
			FSOUND_DSP_Free (EchoUnit);
			free (EchoBuffer);
			EchoBuffer = NULL;
			EchoUnit = NULL;
		}

		if (DryUnit != NULL)
		{
			FSOUND_DSP_Free (DryUnit);
			DryUnit = NULL;
		}

		FSOUND_Close ();
	}
	Sound3D = false;
}

CCMD (snd_status)
{
	if (_nosound)
	{
		Printf ("sound is not active\n");
		return;
	}

	int output = FSOUND_GetOutput ();
	int driver = FSOUND_GetDriver ();
	int mixer = FSOUND_GetMixer ();

	Printf ("Output: %s\n", OutputNames[output]);
	Printf ("Driver: %d (%s)\n", driver,
		FSOUND_GetDriverName (driver));
	Printf ("Mixer: %s\n", MixerNames[mixer]);
	if (Sound3D)
	{
		Printf ("Using 3D sound\n");
	}
	if (DriverCaps)
	{
		Printf ("Driver caps: 0x%x\n", DriverCaps);
		if (DriverCaps & FSOUND_CAPS_HARDWARE)
		{
			Printf ("Hardware channels: %d\n", FSOUND_GetNumHardwareChannels ());
		}
	}
}

void I_MovieDisableSound ()
{
	//OutputDebugString ("away\n");
	I_ShutdownMusic ();
	I_ShutdownSound ();
}

void I_MovieResumeSound ()
{
	//OutputDebugString ("come back\n");
	I_InitSound ();
	S_Init ();
	S_RestartMusic ();
}

CCMD (snd_reset)
{
	I_MovieDisableSound ();
	I_MovieResumeSound ();
}

CCMD (snd_listdrivers)
{
	long numdrivers = FSOUND_GetNumDrivers ();
	long i;

	for (i = 0; i < numdrivers; i++)
	{
		Printf ("%ld. %s\n", i, FSOUND_GetDriverName (i));
	}
}

ADD_STAT (sound, out)
{
	if (_nosound)
	{
		strcpy (out, "no sound");
	}
	else
	{
		sprintf (out, "%d channels, %.2f%% CPU", FSOUND_GetChannelsPlaying(),
			FSOUND_GetCPUUsage());
	}
}

void FX_SetPriority (int handle, int priority)
{
	FSOUND_SetPriority (handle, priority);
}

void FX_SetReverbDelay (int)
{
	// Duke only uses this to set the delay to 0, which ASS clamps
	// to the buffer size, which as far as I can tell is always 256.
}

void FX_SetReverb (int volume)
{
	// ASS reverb is really an echo
	volume = clamp (volume, 0, 255);

	// Clear the echo buffer if we're just starting to echo
	if (EchoVolume == 0 && volume != 0 && EchoBuffer != NULL)
	{
		int mixer = FSOUND_GetMixer();
		int len;

		if (mixer == FSOUND_MIXER_QUALITY_MMXP5 || mixer == FSOUND_MIXER_QUALITY_MMXP6)
		{
			len = EchoLen << 2;
		}
		else
		{
			len = EchoLen << 3;
		}
		memset (EchoBuffer, 0, len);
	}
	EchoVolume = volume;
	if (Sound3D)
	{
		SetEAXEcho ();
	}
}

void FX_SetReverseStereo (int)
{
}

void FX_SetVolume (int)
{
}

char *FX_ErrorString (int)
{
	return "FXBlah";
}

int FX_GetReverseStereo (void)
{
	return 0;
}

int FX_Init (int, int, int, int, unsigned int)
{
	return 0;
}

static void SetPos (float pos[3], long lrelpos[3], int distance, int zeroang)
{
	if (distance == 0)
	{
		pos[0] = sintable[(zeroang+512)&2047] / 65536.f;;
		pos[1] = 0.f;
		pos[2] = sintable[zeroang] / -65536.f;
	}
	else
	{
		float relpos[3] =
		{
			(float)lrelpos[0],
			(float)lrelpos[1],
			(float)lrelpos[2]
		};
		float fdist = distance / DistScale / sqrtf (relpos[0]*relpos[0]+relpos[1]*relpos[1]+relpos[2]*relpos[2]);
		pos[0] = relpos[0]*fdist;
		pos[1] = -relpos[2]*fdist;
		pos[2] = -relpos[1]*fdist;
	}
}

int FX_Pan3D (int channel, long relpos[3], int distance, int zeroang)
{
	float pos[3];

	SetPos (pos, relpos, distance, zeroang);
	FSOUND_3D_SetAttributes (channel, pos, NULL);
	return FX_Ok;
}

int FX_PlaySound3D (long which, int pitchoffset, long vector[3], int distance, int zeroang, int priority, bool looped)
{
	if (_nosound)
	{
		return -1;
	}

	long which1 = which;
	sfxinfo_t *sfx = &S_sfx[which];

	if (sfx->data == NULL)
	{
		I_LoadSound (sfx);
		if (sfx->Name[0] == 0 && sfx->bWadLump == false)
		{
			return -1;
		}
		while (sfx->link != sfxinfo_t::NO_LINK)
		{
			which = sfx->link;
			sfx = &S_sfx[which];
		}
	}

	FSOUND_SAMPLE *sample = CheckLooping (sfx, looped);
	int freq;
	long chan;
	unsigned long scale;

	//Printf ("PlaySound3D %13s %3d %5d %3d %d = %d\n", sfx->Name, pitchoffset, distance, priority, looped, channel);

	scale = PITCH_GetScale (pitchoffset);
	if (scale == 0x10000)
	{
		freq = 0;
	}
	else
	{
		FSOUND_Sample_GetDefaults (sample, &freq, NULL, NULL, NULL);
		freq = mulscale16 (freq, scale);
	}

	chan = FSOUND_PlaySoundEx (FSOUND_FREE, sample, NULL, true);

	if (chan != -1)
	{
		float pos[3];

		if (freq != 0)
		{
			FSOUND_SetFrequency (chan, freq);
		}
		SetPos (pos, vector, distance, zeroang);
		FSOUND_3D_SetAttributes (chan, pos, NULL);
		FSOUND_SetPaused (chan, false);
		return chan;
	};

	return -1;
}

int FX_SetCallBack (void (*cb)(long,long))
{
	//FX_Callback = cb;
	return FX_Ok;
}

int FX_SetupSoundBlaster (fx_blaster_config, int *,int*, int*)
{
	return 0;
}

int FX_Shutdown (void)
{
	return 0;
}

int FX_StopSound (int handle)
{
	if (_nosound)
		return FX_Warning;

	FSOUND_StopSound (handle);
	return FX_Ok;
}


int FX_SoundIsPlaying (int handle)
{
	if (_nosound)
		return 0;

	return FSOUND_IsPlaying (handle);
}

void FX_UpdateSounds (int angle)
{
	float zero[3] = { 0.f, 0.f, 0.f };

	FSOUND_3D_Listener_SetAttributes (zero, NULL,
		sintable[(angle+512)&2047]/16384.f, 0.f, -sintable[angle]/16384.f,		// forward vector
		0.f, 1.f, 0.f);		// up vector
	FSOUND_Update ();
}
