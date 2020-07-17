//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2003 - 3D Realms Entertainment

This file is part of Duke Nukem 3D version 1.5 - Atomic Edition

Duke Nukem 3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Todd Replogle
Prepared for public release: 03/21/2003 - Charlie Wiederhold, 3D Realms
*/
//-------------------------------------------------------------------------

#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "sndcards.h"
#include "fx_man.h"
#include "music.h"
#include "util_lib.h"
#include "duke3d.h"
#include "s_sound.h"
#include "i_sound.h"

#define MAXCHANNELS 255
#define LOUDESTVOLUME 150

struct FVoiceRecord
{
	WORD Voice;
	WORD Sprite;
	WORD Sound;
	BYTE IsXYZ;
	BYTE Next;
};

FVoiceRecord Voices[MAXCHANNELS];

static BYTE PlayingVoices;
static BYTE FreeVoices;

/*
===================
=
= SoundStartup
=
===================
*/

void SoundStartup( void )
{
	PlayingVoices = MAXCHANNELS;
	FreeVoices = 0;

	for (int i = 0; i < MAXCHANNELS; ++i)
	{
		Voices[i].Next = i+1;
	}
}

/*
===================
=
= SoundShutdown
=
===================
*/

void SoundShutdown( void )
   {
   int32 status;

   // if they chose None lets return
   if (FXDevice == NumSoundCards)
      return;

   status = FX_Shutdown();
   if ( status != FX_Ok )
      {
      I_FatalError( FX_ErrorString( FX_Error ));
      }
   }

/*
===================
=
= MusicStartup
=
===================
*/

void MusicStartup( void )
   {
   int32 status;

   // if they chose None lets return
   if (MusicDevice == NumSoundCards)
      return;

   // satisfy AWE32 and WAVEBLASTER stuff
   BlasterConfig.Midi = MidiPort;

   // Do special Sound Blaster, AWE32 stuff
   if (
         ( FXDevice == SoundBlaster ) ||
         ( FXDevice == Awe32 )
      )
      {
      int MaxVoices;
      int MaxBits;
      int MaxChannels;

      FX_SetupSoundBlaster
                  (
                  BlasterConfig, (int *)&MaxVoices, (int *)&MaxBits, (int *)&MaxChannels
                  );
      }
   status = MUSIC_Init( MusicDevice, MidiPort );

   if ( status == MUSIC_Ok )
      {
      MUSIC_SetVolume( MusicVolume );
      }
   else
   {
      Printf("Couldn't find selected sound card, or, error w/ sound card itself.\n");

      SoundShutdown();
      uninitengine();
      CONTROL_Shutdown();
      CONFIG_WriteSetup();
      uninitgroupfile();
      unlink("duke3d.tmp");
      exit(-1);
   }
}

/*
===================
=
= MusicShutdown
=
===================
*/

void MusicShutdown( void )
   {
   int32 status;

   // if they chose None lets return
   if (MusicDevice == NumSoundCards)
      return;

   status = MUSIC_Shutdown();
   if ( status != MUSIC_Ok )
      {
      I_FatalError( MUSIC_ErrorString( MUSIC_ErrorCode ));
      }
   }

int USRHOOKS_GetMem(char **ptr, unsigned long size )
{
   *ptr = (char*)malloc(size);

   if (*ptr == NULL)
      return(USRHOOKS_Error);

   return( USRHOOKS_Ok);

}

int USRHOOKS_FreeMem(char *ptr)
{
   free(ptr);
   return( USRHOOKS_Ok);
}

char menunum=0;

void intomenusounds(void)
{
    short menusnds[] =
    {
        LASERTRIP_EXPLODE,
        DUKE_GRUNT,
        DUKE_LAND_HURT,
        CHAINGUN_FIRE,
        SQUISHED,
        KICK_HIT,
        PISTOL_RICOCHET,
        PISTOL_BODYHIT,
        PISTOL_FIRE,
        SHOTGUN_FIRE,
        BOS1_WALK,
        RPG_EXPLODE,
        PIPEBOMB_BOUNCE,
        PIPEBOMB_EXPLODE,
        NITEVISION_ONOFF,
        RPG_SHOOT,
        SELECT_WEAPON
    };
    sound(menusnds[menunum++]);
    menunum %= 17;
}

void playmusic(char *fn)
{
    if(MusicToggle == 0) return;
    if(MusicDevice == NumSoundCards) return;

	S_ChangeMusic (fn, 0, MUSIC_LoopSong, false);
}

void AddVoice (int sprite, int voice, int num, bool xyz)
{
	int slot = FreeVoices;

	if (slot == MAXCHANNELS)
		return;

	FreeVoices = Voices[slot].Next;
	Voices[slot].Voice = voice;
	Voices[slot].Sprite = sprite;
	Voices[slot].Sound = num;
	Voices[slot].IsXYZ = xyz;
	Voices[slot].Next = PlayingVoices;
	PlayingVoices = slot;
}

void StopAVoice (int thisone, int prevone)
{
	FX_StopSound (Voices[thisone].Voice);
	if (prevone >= 0)
	{
		Voices[prevone].Next = Voices[thisone].Next;
	}
	else
	{
		PlayingVoices = Voices[thisone].Next;
	}
	Voices[thisone].Next = FreeVoices;
	FreeVoices = thisone;
}

int xyzsound(short num,short i,long x,long y,long z)
{
	return xyzsound2(num,i,x,y,z,false);
}

int xyzsound2(short num,short i,long x,long y,long z,bool forceLoop)
{
	sfxinfo_t *sfx = &S_sfx[num];
	long relpos[3];
    long sndist, cx, cy, cz, j,k;
    short pitche,pitchs,cs;
    int voice, ca, pitch;

    if( (size_t)num >= S_sfx.Size() ||
        FXDevice == NumSoundCards ||
        ( (S_sfx[num].bLockout) && ud.lockout ) ||
        SoundToggle == 0 ||
        (ps[myconnectindex].timebeforeexit > 0 && ps[myconnectindex].timebeforeexit <= 26*3) ||
        ps[myconnectindex].gm&MODE_MENU) return -1;

    if( sfx->bFullVolume )
    {
        return sound(num);
    }

    if( sfx->bDukeVoice )
    {
        if(VoiceToggle==0 || (ud.multimode > 1 && sprite[i].picnum == APLAYER && sprite[i].yvel != screenpeek && ud.coop != 1) ) return -1;

		for (j = PlayingVoices; j != MAXCHANNELS; j = Voices[j].Next)
		{
			if (S_sfx[Voices[j].Sound].bDukeVoice)
				return -1;
		}
    }

    cx = ps[screenpeek].oposx;
    cy = ps[screenpeek].oposy;
    cz = ps[screenpeek].oposz;
    cs = ps[screenpeek].cursectnum;
    ca = ps[screenpeek].ang+ps[screenpeek].look_ang;

    sndist = FindDistance3D((cx-x),(cy-y),(cz-z)>>4);

    if( i >= 0 && (sfx->bGlobHeard) == 0 && sprite[i].picnum == MUSICANDSFX && sprite[i].lotag < 999 && (sector[sprite[i].sectnum].lotag&0xff) < 9 )
        sndist = divscale14(sndist,(sprite[i].hitag+1));

    pitchs = sfx->PitchStart;
    pitche = sfx->PitchEnd;
    cx = klabs(pitche-pitchs);

    if(cx)
    {
        if( pitchs < pitche )
             pitch = pitchs + ( rand()%cx );
        else pitch = pitche + ( rand()%cx );
    }
    else pitch = pitchs;

    sndist += sfx->VolumeAdjust;
    if(sndist < 0) sndist = 0;
    if( sndist && sprite[i].picnum != MUSICANDSFX && !cansee(cx,cy,cz-(24<<8),cs,sprite[i].x,sprite[i].y,sprite[i].z-(24<<8),sprite[i].sectnum) )
        sndist += sndist>>5;

    switch(num)
    {
        case PIPEBOMB_EXPLODE:
        case LASERTRIP_EXPLODE:
        case RPG_EXPLODE:
            if(sndist > (6144) )
                sndist = 6144;
            if(sector[ps[screenpeek].cursectnum].lotag == 2)
                pitch -= 1024;
            break;
        default:
            if(sector[ps[screenpeek].cursectnum].lotag == 2 && (sfx->bDukeVoice) == 0)
                pitch = -768;
            if( sndist > 31444 && sprite[i].picnum != MUSICANDSFX)
                return -1;
            break;
    }

    if( sprite[i].picnum != MUSICANDSFX )
    {
		// If the specified actor is already playing this sound, stop the old sound.
		stopsound (num, i);

		// If the actor is a bad guy and dead, I'm not sure what the old code
		// thought it was doing.

		// If the maximum copies of this sound is already playing, stop the oldest one
		// (but only if it doesn't belong to the local player.
		int pk = -1, pj = -1;
		int count = 0;

		for (k = -1, j = PlayingVoices; j != MAXCHANNELS; k = j, j = Voices[j].Next)
		{
			if (Voices[j].Sound == num && Voices[j].IsXYZ)
			{
				count++;
				if (sprite[Voices[j].Sprite].picnum != APLAYER ||
					sprite[Voices[j].Sprite].yvel != screenpeek)
				{
					pk = k;
					pj = j;
					count++;
				}
			}
		}
		if (count > 3)
		{
			if (pj >= 0)
			{
				StopAVoice (pj, pk);
			}
			else
			{ // If an older sound couldn't be stopped, don't play the new one.
				return -1;
			}
		}
    }

	relpos[0] = x-cx;
	relpos[1] = y-cy;
	relpos[2] = (z-cz)>>4;

    if( sprite[i].picnum == APLAYER && sprite[i].yvel == screenpeek )
    {
        sndist = 0;
    }

    if( sfx->bGlobHeard ) sndist = 0;

    //if(sndist < ((255-LOUDESTVOLUME)<<6) )
    //    sndist = ((255-LOUDESTVOLUME)<<6);
	if (sndist < 0)
		sndist = 0;

    voice = FX_PlaySound3D( num,pitch,relpos,sndist, ca, sfx->Priority,
		sfx->bRepeat || forceLoop);

    if ( voice >= 0 )
    {
		AddVoice (i, voice, num, true);
    }
    return (voice);
}

int sound(short num)
{
	sfxinfo_t *sfx = &S_sfx[num];
	long zero[3] = { 0, 0, 0 };
    short pitch,pitche,pitchs,cx;
    int voice;

    if (FXDevice == NumSoundCards) return -1;
    if(SoundToggle==0) return -1;
    if(VoiceToggle==0 && (sfx->bDukeVoice) ) return -1;
    if( (sfx->bLockout) && ud.lockout ) return -1;

	pitchs = sfx->PitchStart;
    pitche = sfx->PitchEnd;
    cx = klabs(pitche-pitchs);

    if(cx)
    {
        if( pitchs < pitche )
             pitch = pitchs + ( rand()%cx );
        else pitch = pitche + ( rand()%cx );
    }
    else pitch = pitchs;

    voice = FX_PlaySound3D( num, pitch,zero,0,ps[screenpeek].ang+ps[screenpeek].look_ang,
		sfx->Priority, sfx->bRepeat );

    if(voice >= 0)
	{
		AddVoice (-1, voice, num, false);
	}
	return voice;
}

int spritesound(unsigned short num, short i)
{
    if(num >= NUM_SOUNDS) return -1;
    return xyzsound2(num,i,sprite[i].x,sprite[i].y,sprite[i].z,false);
}

void stopsound(short num,short i)
{
    int j, k;

	for (k = -1, j = PlayingVoices; j != MAXCHANNELS; k = j, j = Voices[j].Next)
	{
		if (Voices[j].Sound == num && Voices[j].Sprite == i)
		{
			StopAVoice (j, k);
			break;
		}
	}
}

void pan3dsound(void)
{
	long relpos[3];
    long sndist, sx, sy, sz, cx, cy, cz;
    int ca,j,k,i,cs;
	int next;

    if(ud.camerasprite == -1)
    {
        cx = ps[screenpeek].oposx;
        cy = ps[screenpeek].oposy;
        cz = ps[screenpeek].oposz;
        cs = ps[screenpeek].cursectnum;
        ca = ps[screenpeek].ang+ps[screenpeek].look_ang;
    }
    else
    {
        cx = sprite[ud.camerasprite].x;
        cy = sprite[ud.camerasprite].y;
        cz = sprite[ud.camerasprite].z;
        cs = sprite[ud.camerasprite].sectnum;
        ca = sprite[ud.camerasprite].ang;
    }

	for (k = -1, j = PlayingVoices; j != MAXCHANNELS; j = next)
	{
		next = Voices[j].Next;

		if (!FX_SoundIsPlaying (Voices[j].Voice))
		{
			StopAVoice (j, k);
		}
		else if (Voices[j].IsXYZ)
		{
			i = Voices[j].Sprite;

			sx = sprite[i].x;
			sy = sprite[i].y;
			sz = sprite[i].z;

			if( sprite[i].picnum == APLAYER && sprite[i].yvel == screenpeek)
			{
				sndist = 0;
			}
			else
			{
				sndist = FindDistance3D((cx-sx),(cy-sy),(cz-sz)>>4);
				if( i >= 0 && (S_sfx[j].bGlobHeard) == 0 && sprite[i].picnum == MUSICANDSFX && sprite[i].lotag < 999 && (sector[sprite[i].sectnum].lotag&0xff) < 9 )
					sndist = divscale14(sndist,(sprite[i].hitag+1));
			}

			relpos[0] = sx-cx;
			relpos[1] = sy-cy;
			relpos[2] = (sz-cz)>>4;

			sndist += S_sfx[j].VolumeAdjust;
			if(sndist < 0) sndist = 0;

			if( sndist && sprite[i].picnum != MUSICANDSFX && !cansee(cx,cy,cz-(24<<8),cs,sx,sy,sz-(24<<8),sprite[i].sectnum) )
				sndist += sndist>>5;

			switch(j)
			{
			case PIPEBOMB_EXPLODE:
			case LASERTRIP_EXPLODE:
			case RPG_EXPLODE:
				if(sndist > (6144)) sndist = (6144);
				break;
			default:
				if( sndist > 31444 && sprite[i].picnum != MUSICANDSFX)
				{
					StopAVoice (j, k);
					continue;
				}
			}

			if( S_sfx[j].bGlobHeard ) sndist = 0;

			//if(sndist < ((255-LOUDESTVOLUME)<<6) )
			//    sndist = ((255-LOUDESTVOLUME)<<6);
			if (sndist < 0)
				sndist = 0;

			FX_Pan3D(Voices[j].Voice,relpos,sndist,ca);
			k = j;
		}
	}

	FX_UpdateSounds (ca);
}

bool ActorIsPlaying (int sprite, int sound)
{
	int j, k;

	for (k = -1, j = PlayingVoices; j != MAXCHANNELS; k = j, j = Voices[j].Next)
	{
		if (Voices[j].Sound == sound && Voices[j].Sprite == sprite)
		{
			if (!FX_SoundIsPlaying (Voices[j].Voice))
			{
				StopAVoice (j, k);
				return false;
			}
			return true;
		}
	}
	return false;
}

bool ActorIsMakingNoise (int sprite)
{
	int j, k;
	int next;

	for (k = -1, j = PlayingVoices; j != MAXCHANNELS; j = next)
	{
		next = Voices[j].Next;

		if (Voices[j].Sprite == sprite)
		{
			if (!FX_SoundIsPlaying (Voices[j].Voice))
			{
				StopAVoice (j, k);
			}
			else
			{
				return true;
			}
		}
		else
		{
			k = j;
		}
	}
	return false;
}

void clearsoundlocks(void)
{
}

int FX_StopAllSounds (void)
{
	int j;

	// Do not use FSOUND_StopSound (FSOUND_ALL), because that will also stop
	// any playing music streams.

	for (j = PlayingVoices; j != MAXCHANNELS; j = Voices[j].Next)
	{
		FX_StopSound (Voices[j].Voice);
	}
	FreeVoices = 0;
	PlayingVoices = MAXCHANNELS;
	for (j = 0; j < MAXCHANNELS; ++j)
	{
		Voices[j].Next = j + 1;
	}
	return FX_Ok;
}
