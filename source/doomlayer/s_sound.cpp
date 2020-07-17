// Emacs style mode select	 -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: s_sound.c,v 1.3 1998/01/05 16:26:08 pekangas Exp $
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
//
// DESCRIPTION:  none
// Dolby Pro Logic code by Gavino.
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include "m_alloc.h"

#include "i_system.h"
#include "i_sound.h"
#include "i_music.h"
#include "i_cd.h"
#include "s_sound.h"
//#include "s_sndseq.h"
#include "s_playlist.h"
#include "c_dispatch.h"
//#include "m_random.h"
//#include "w_wad.h"
#include "doomdef.h"
//#include "p_local.h"
//#include "doomstat.h"
#include "cmdlib.h"
#include "v_video.h"
//#include "v_text.h"
//#include "a_sharedglobal.h"
//#include "gstrings.h"
//#include "vectors.h"
//#include "gi.h"
#include "templates.h"
#include "duke3d.h"

// MACROS ------------------------------------------------------------------

#ifdef NeXT
// NeXT doesn't need a binary flag in open call
#define O_BINARY 0
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define SELECT_ATTEN(a)			((a)==ATTN_NONE ? 0 : (a)==ATTN_SURROUND ? -1 : \
								 (a)==ATTN_STATIC ? 3 : 1)
#ifndef FIXED2FLOAT
#define FIXED2FLOAT(f)			(((float)(f))/(float)65536)
#endif

#define PRIORITY_MAX_ADJUST		10
#define DIST_ADJUST				(MAX_SND_DIST/PRIORITY_MAX_ADJUST)

#define NORM_PITCH				128
#define NORM_PRIORITY			64
#define NORM_SEP				128
#define NONE_SEP				-2

#define S_PITCH_PERTURB 		1
#define S_STEREO_SWING			(96<<14)

/* Sound curve parameters for Doom */

// Maximum sound distance
const int S_CLIPPING_DIST = 255<<6;

// Sounds closer than this to the listener are maxed out.
// Originally: 200.
const int S_CLOSE_DIST =	(255-150)<<6;

const int S_ATTENUATOR =	S_CLIPPING_DIST - S_CLOSE_DIST;

// TYPES -------------------------------------------------------------------

struct MusPlayingInfo
{
	char *name;
	void *handle;
	int   baseorder;
	bool  loop;
};

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

static long P_AproxDistance2 (long *listener, long x, long y);
static void S_StopChannel (int cnum);
static void S_StartSound (long *pt, spritetype *mover, int channel,
	int sound_id, float volume, float attenuation, BOOL looping);
static void S_ActivatePlayList (bool goBack);
static void CalcPosVel (long *pt, spritetype *mover, float pos[3], float vel[3]);

// PRIVATE DATA DEFINITIONS ------------------------------------------------

int MAX_SND_DIST;
static BOOL		mus_paused;			// whether songs are paused
static MusPlayingInfo mus_playing;	// music currently being played
static char		*LastSong;			// last music that was played
static FPlayList *PlayList;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// [RH] Hacks for pitch variance
int sfx_sawup, sfx_sawidl, sfx_sawful, sfx_sawhit;
int sfx_itemup, sfx_tink;

int sfx_empty;

int numChannels;

CVAR (Bool, snd_surround, true, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)	// [RH] Use surround sounds?
FBoolCVar noisedebug ("noise", false, 0);	// [RH] Print sound debugging info?
CVAR (Int, snd_channels, 12, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)	// number of channels available
CVAR (Bool, snd_matrix, false, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)
CVAR (Bool, snd_flipstereo, false, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)

TArray<sfxinfo_t> S_sfx;

// CODE --------------------------------------------------------------------

//==========================================================================
//
// S_NoiseDebug
//
// [RH] Print sound debug info. Called by status bar.
//==========================================================================

void S_NoiseDebug (void)
{
}

//==========================================================================
//
// S_Init
//
// Initializes sound stuff, including volume. Sets channels, SFX and
// music volume, allocates channel buffer, and sets S_sfx lookup.
//==========================================================================

void S_Init ()
{
	Printf ("S_Init\n");

	// Allocating the virtual channels
	numChannels = I_SetChannels (snd_channels);

	// no sounds are playing, and they are not paused
	mus_paused = 0;

	// Note that sounds have not been cached (yet).
//	for (i=1; (size_t)i < S_sfx.Size (); i++)
//		S_sfx[i].usefulness = -1;
}

//==========================================================================
//
// S_ActivatePlayList
//
// Plays the next song in the playlist. If no songs in the playlist can be
// played, then it is deleted.
//==========================================================================

void S_ActivatePlayList (bool goBack)
{
	int startpos, pos;

	startpos = pos = PlayList->GetPosition ();
	S_StopMusic (true);
	while (!S_ChangeMusic (PlayList->GetSong (pos), 0, false, true))
	{
		pos = goBack ? PlayList->Backup () : PlayList->Advance ();
		if (pos == startpos)
		{
			delete PlayList;
			PlayList = NULL;
			Printf ("Cannot play anything in the playlist.\n");
			return;
		}
	}
}

//==========================================================================
//
// S_PauseSound
//
// Stop music, during game PAUSE.
//==========================================================================

void S_PauseSound ()
{
	if (mus_playing.handle && !mus_paused)
	{
		I_PauseSong (mus_playing.handle);
		mus_paused = true;
	}
}

//==========================================================================
//
// S_ResumeSound
//
// Resume music, after game PAUSE.
//==========================================================================

void S_ResumeSound ()
{
	if (mus_playing.handle && mus_paused)
	{
		I_ResumeSong (mus_playing.handle);
		mus_paused = false;
	}
}

//==========================================================================
//
// S_ChangeCDMusic
//
// Starts a CD track as music.
//==========================================================================

bool S_ChangeCDMusic (int track, unsigned int id, bool looping)
{
	char temp[32];

	if (id)
	{
		sprintf (temp, ",CD,%d,%x", track, id);
	}
	else
	{
		sprintf (temp, ",CD,%d", track);
	}
	return S_ChangeMusic (temp, 0, looping);
}

//==========================================================================
//
// S_StartMusic
//
// Starts some music with the given name.
//==========================================================================

bool S_StartMusic (char *m_id)
{
	return S_ChangeMusic (m_id, 0, false);
}

//==========================================================================
//
// S_ChangeMusic
//
// Starts playing a music, possibly looping.
//
// [RH] If music is a MOD, starts it at position order. If name is of the
// format ",CD,<track>,[cd id]" song is a CD track, and if [cd id] is
// specified, it will only be played if the specified CD is in a drive.
//==========================================================================

bool S_ChangeMusic (const char *musicname, int order, bool looping, bool force)
{
	if (!force && PlayList)
	{ // Don't change if a playlist is active
		return false;
	}

	if (musicname == NULL || musicname[0] == 0)
	{
		// Don't choke if the map doesn't have a song attached
		S_StopMusic (true);
		return false;
	}

	if (mus_playing.name && stricmp (mus_playing.name, musicname) == 0)
	{
		if (order != mus_playing.baseorder)
		{
			mus_playing.baseorder =
				(I_SetSongPosition (mus_playing.handle, order) ? order : 0);
		}
		return true;
	}

	if (strnicmp (musicname, ",CD,", 4) == 0)
	{
		int track = strtoul (musicname+4, NULL, 0);
		char *more = strchr (musicname+4, ',');
		unsigned int id = 0;

		if (more != NULL)
		{
			id = strtoul (more+1, NULL, 16);
		}
		S_StopMusic (true);
		if (mus_playing.name)
		{
			delete[] mus_playing.name;
		}
		mus_playing.handle = I_RegisterCDSong (track, id);
	}
	else
	{
		int lumpnum = kopen4load (musicname, 0);

		if (lumpnum == -1)
		{
			Printf ("Music \"%s\" not found\n", musicname);
			return false;
		}
		else
		{
			kclose (lumpnum);
		}

		// shutdown old music
		S_StopMusic (true);

		// load & register it
		if (mus_playing.name)
		{
			delete[] mus_playing.name;
		}
		mus_playing.handle = I_RegisterSong (musicname);
	}

	mus_playing.loop = looping;

	if (mus_playing.handle != 0)
	{ // play it
		mus_playing.name = copystring (musicname);
		I_PlaySong (mus_playing.handle, looping);
		mus_playing.baseorder =
			(I_SetSongPosition (mus_playing.handle, order) ? order : 0);
		return true;
	}
	return false;
}

//==========================================================================
//
// S_RestartMusic
//
// Must only be called from snd_reset in i_sound.cpp!
//==========================================================================

void S_RestartMusic ()
{
	if (LastSong != NULL)
	{
		S_ChangeMusic (LastSong, mus_playing.baseorder, mus_playing.loop, true);
		delete[] LastSong;
		LastSong = NULL;
	}
}

//==========================================================================
//
// S_GetMusic
//
//==========================================================================

int S_GetMusic (char **name)
{
	int order;

	if (mus_playing.name)
	{
		*name = copystring (mus_playing.name);
		order = mus_playing.baseorder;
	}
	else
	{
		*name = NULL;
		order = 0;
	}
	return order;
}

//==========================================================================
//
// S_StopMusic
//
//==========================================================================

void S_StopMusic (bool force)
{
	// [RH] Don't stop if a playlist is active.
	if ((force || PlayList == NULL) && mus_playing.name)
	{
		if (mus_paused)
			I_ResumeSong(mus_playing.handle);

		I_StopSong(mus_playing.handle);
		I_UnRegisterSong(mus_playing.handle);

		if (LastSong)
		{
			delete[] LastSong;
		}

		LastSong = mus_playing.name;
		mus_playing.name = NULL;
		mus_playing.handle = 0;
	}
}

//==========================================================================
//
// CCMD playsound
//
//==========================================================================

CCMD (playsound)
{
	if (argv.argc() > 1)
	{
		for (size_t i = 0; i < S_sfx.Size(); ++i)
		{
			if (stricmp (S_sfx[i].Name, argv[1]) == 0)
			{
				sound (i);
				return;
			}
		}
	}
}

//==========================================================================
//
// CCMD changemus
//
//==========================================================================

CCMD (changemus)
{
	if (argv.argc() > 1)
	{
		if (PlayList)
		{
			delete PlayList;
			PlayList = NULL;
		}
		S_ChangeMusic (argv[1], argv.argc() > 2 ? atoi (argv[2]) : 0);
	}
}

//==========================================================================
//
// CCMD stopmus
//
//==========================================================================

CCMD (stopmus)
{
	if (PlayList)
	{
		delete PlayList;
		PlayList = NULL;
	}
	S_StopMusic (false);
}

//==========================================================================
//
// CCMD cd_play
//
// Plays a specified track, or the entire CD if no track is specified.
//==========================================================================

CCMD (cd_play)
{
	char musname[16];

	if (argv.argc() == 1)
		strcpy (musname, ",CD,");
	else
		sprintf (musname, ",CD,%d", atoi(argv[1]));
	S_ChangeMusic (musname, 0, true);
}

//==========================================================================
//
// CCMD cd_stop
//
//==========================================================================

CCMD (cd_stop)
{
	CD_Stop ();
}

//==========================================================================
//
// CCMD cd_eject
//
//==========================================================================

CCMD (cd_eject)
{
	CD_Eject ();
}

//==========================================================================
//
// CCMD cd_close
//
//==========================================================================

CCMD (cd_close)
{
	CD_UnEject ();
}

//==========================================================================
//
// CCMD cd_pause
//
//==========================================================================

CCMD (cd_pause)
{
	CD_Pause ();
}

//==========================================================================
//
// CCMD cd_resume
//
//==========================================================================

CCMD (cd_resume)
{
	CD_Resume ();
}

//==========================================================================
//
// CCMD playlist
//
//==========================================================================

CCMD (playlist)
{
	int argc = argv.argc();

	if (argc < 2 || argc > 3)
	{
		Printf ("playlist <playlist.m3u> [<position>|shuffle]\n");
	}
	else
	{
		if (PlayList != NULL)
		{
			PlayList->ChangeList (argv[1]);
		}
		else
		{
			PlayList = new FPlayList (argv[1]);
		}
		if (PlayList->GetNumSongs () == 0)
		{
			delete PlayList;
			PlayList = NULL;
		}
		else
		{
			if (argc == 3)
			{
				if (stricmp (argv[2], "shuffle") == 0)
				{
					PlayList->Shuffle ();
				}
				else
				{
					PlayList->SetPosition (atoi (argv[2]));
				}
			}
			S_ActivatePlayList (false);
		}
	}
}

//==========================================================================
//
// CCMD playlistpos
//
//==========================================================================

static bool CheckForPlaylist ()
{
	if (PlayList == NULL)
	{
		Printf ("No playlist is playing.\n");
		return false;
	}
	return true;
}

CCMD (playlistpos)
{
	if (CheckForPlaylist() && argv.argc() > 1)
	{
		PlayList->SetPosition (atoi (argv[1]) - 1);
		S_ActivatePlayList (false);
	}
}

//==========================================================================
//
// CCMD playlistnext
//
//==========================================================================

CCMD (playlistnext)
{
	if (CheckForPlaylist())
	{
		PlayList->Advance ();
		S_ActivatePlayList (false);
	}
}

//==========================================================================
//
// CCMD playlistprev
//
//==========================================================================

CCMD (playlistprev)
{
	if (CheckForPlaylist())
	{
		PlayList->Backup ();
		S_ActivatePlayList (true);
	}
}

//==========================================================================
//
// CCMD playliststatus
//
//==========================================================================

CCMD (playliststatus)
{
	if (CheckForPlaylist ())
	{
		Printf ("Song %d of %d:\n%s\n",
			PlayList->GetPosition () + 1,
			PlayList->GetNumSongs (),
			PlayList->GetSong (PlayList->GetPosition ()));
	}
}
