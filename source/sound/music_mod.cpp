#include "i_musicinterns.h"

extern bool Sound3D;
extern int NumSFXChannels;

void MODSong::SetVolume (float volume)
{
	if (m_Module)
	{
		FMUSIC_SetMasterVolume (m_Module, (int)(volume * 256));
	}
}

void MODSong::Play (bool looping)
{
	m_Status = STATE_Stopped;
	m_Looping = looping;

	if (FMUSIC_PlaySong (m_Module))
	{
		FMUSIC_SetMasterVolume (m_Module, (int)(snd_musicvolume * 256));
		m_Status = STATE_Playing;
	}
}

void MODSong::Pause ()
{
	if (m_Status == STATE_Playing)
	{
		if (FMUSIC_SetPaused (m_Module, TRUE))
			m_Status = STATE_Paused;
	}
}

void MODSong::Resume ()
{
	if (m_Status == STATE_Paused)
	{
		if (FMUSIC_SetPaused (m_Module, FALSE))
			m_Status = STATE_Playing;
	}
}

void MODSong::Stop ()
{
	if (m_Status != STATE_Stopped && m_Module)
	{
		FMUSIC_StopSong (m_Module);
	}
	m_Status = STATE_Stopped;
}

void MODSong::SetMusicReserved (signed char flag)
{
	if (Sound3D)
		return;

	int mchans = FMUSIC_GetNumChannels (m_Module);
	int i;

	if (FMUSIC_GetType (m_Module) == FMUSIC_TYPE_IT)
	{
		// An IT can use more channels than it has data for
		mchans += mchans >> 2;
	}

	i = FSOUND_GetMaxChannels ();
	i -= FSOUND_GetNumHardwareChannels ();

	for (i--; i >= 0 && i >= NumSFXChannels; --i)
	{
		FSOUND_SetReserved (i, flag);
	}
}

MODSong::~MODSong ()
{
	Stop ();
	if (m_Module != NULL)
	{
		SetMusicReserved (TRUE);
		FMUSIC_FreeSong (m_Module);
		m_Module = NULL;
	}
}

MODSong::MODSong (const char *fn)
{
	m_Module = FMUSIC_LoadSong (fn);
	if (m_Module != NULL)
	{
		// Modules' samples have priority 255 by default. To avoid having SFX
		// steal channels from a playing module, I clamp their maximum priority
		// to 254 when the get defined in gamedef.cpp.

		// Unblock some channels for the MOD to use. Note that this also makes
		// them available to sound effects.
		SetMusicReserved (FALSE);
	}
}

bool MODSong::IsPlaying ()
{
	if (m_Status != STATE_Stopped)
	{
		if (FMUSIC_IsPlaying (m_Module))
		{
			if (!m_Looping && FMUSIC_IsFinished (m_Module))
			{
				Stop ();
				return false;
			}
			return true;
		}
		else if (m_Looping)
		{
			Play (true);
			return m_Status != STATE_Stopped;
		}
	}
	return false;
}

bool MODSong::SetPosition (int order)
{
	return !!FMUSIC_SetOrder (m_Module, order);
}

