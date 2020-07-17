// Emacs style mode select	 -*- C++ -*- 
//-----------------------------------------------------------------------------
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
//		The not so system specific sound interface.
//
//-----------------------------------------------------------------------------


#ifndef __S_SOUND__
#define __S_SOUND__

#include "tarray.h"
#include "doomtype.h"
#include "build.h"

#define MAX_SNDNAME	23

//
// SoundFX struct.
//
struct sfxinfo_t
{
	union
	{
		char	Name[MAX_SNDNAME+1];
		int		LumpNum;
	};

	// A non-null data means the sound has been loaded.
	void*		data;
	void*		altdata;

	WORD		bHaveLoop:1;
	WORD		bPlayedNormal:1;
	WORD		bPlayedLoop:1;
	WORD		bWadLump:1;
//	WORD		bRandomHeader:1;
//	WORD		bPlayerReserve:1;

	WORD		bForce11025:1;
	WORD		bForce22050:1;
	WORD		bLoadRAW:1;
	WORD		b16bit:1;

	WORD		bRepeat:1;
	WORD		bMusicAndSfx:1;
	WORD		bDukeVoice:1;
	WORD		bLockout:1;
	WORD		bGlobHeard:1;
	WORD		bFullVolume:1;

	WORD		link;

	enum { NO_LINK = 0xffff };

	short		PitchStart, PitchEnd, VolumeAdjust;
	BYTE		Priority;
};

// the complete set of sound effects
extern TArray<sfxinfo_t> S_sfx;

// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//	allocates channel buffer, sets S_sfx lookup.
//
void S_Init ();

// Per level startup code.
// Kills playing sounds at start of level and starts new music.
//
void S_Start ();

// Start sound for thing at <ent>
void S_SoundID (int channel, int sfxid, float volume, int attenuation);
void S_SoundID (spritetype *ent, int channel, int sfxid, float volume, int attenuation);
void S_SoundID (long *pt, int channel, int sfxid, float volume, int attenuation);
void S_SoundID (long x, long y, long z, int channel, int sfxid, float volume, int attenuation);
void S_LoopedSoundID (spritetype *ent, int channel, int sfxid, float volume, int attenuation);
void S_LoopedSoundID (long *pt, int channel, int sfxid, float volume, int attenuation);

// sound channels
// channel 0 never willingly overrides
// other channels (1-7) always override a playing sound on that channel
//
// CHAN_AUTO searches down from channel 7 until it finds a channel not in use
// CHAN_WEAPON is for weapons
// CHAN_VOICE is for oof, sight, or other voice sounds
// CHAN_ITEM is for small things and item pickup
// CHAN_BODY is for generic body sounds
// CHAN_PICKUP is can optionally be set as a local sound only for "compatibility"

#define CHAN_AUTO				0
#define CHAN_WEAPON				1
#define CHAN_VOICE				2
#define CHAN_ITEM				3
#define CHAN_BODY				4
// modifier flags
#define CHAN_LISTENERZ			8
#define CHAN_IMMOBILE			16
#define CHAN_MAYBE_LOCAL		32
#define CHAN_PICKUP				(CHAN_ITEM|CHAN_MAYBE_LOCAL)

// sound attenuation values
#define ATTN_NONE				0	// full volume the entire level
#define ATTN_NORM				1
#define ATTN_IDLE				2
#define ATTN_STATIC				3	// diminish very rapidly with distance
#define ATTN_SURROUND			4	// like ATTN_NONE, but plays in surround sound

int S_PickReplacement (int refid);

// [RH] From Hexen.
//		Prevents too many of the same sound from playing simultaneously.
BOOL S_StopSoundID (int sound_id, int priority, int limit, long x, long y, long z);

// Stops a sound emanating from one of an entity's channels
void S_StopSound (spritetype *ent, int channel);
void S_StopSound (long *pt, int channel);

// Stop sound for all channels
void S_StopAllChannels (void);

// Is the sound playing on one of the entity's channels?
bool S_GetSoundPlayingInfo (spritetype *ent, int sound_id);
bool S_GetSoundPlayingInfo (long *pt, int sound_id);
bool S_IsActorPlayingSomething (spritetype *actor, int channel);

// Moves all sounds from one mobj to another
void S_RelinkSound (spritetype *from, spritetype *to);

// Start music using <music_name>
bool S_StartMusic (char *music_name);

// Start music using <music_name>, and set whether looping
bool S_ChangeMusic (const char *music_name, int order=0, bool looping=true, bool force=false);

// Start playing a cd track as music
bool S_ChangeCDMusic (int track, unsigned int id=0, bool looping=true);

void S_RestartMusic ();

int S_GetMusic (char **name);

// Stops the music for sure.
void S_StopMusic (bool force);

// Stop and resume music, during game PAUSE.
void S_PauseSound ();
void S_ResumeSound ();

//
// Updates music & sounds
//
void S_UpdateSounds ();

// [RH] Prints sound debug info to the screen.
//		Modelled after Hexen's noise cheat.
void S_NoiseDebug ();


#endif
