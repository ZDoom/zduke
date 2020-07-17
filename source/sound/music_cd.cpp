#include "i_musicinterns.h"
#include "i_cd.h"
#include "build.h"

void CDSong::Play (bool looping)
{
	m_Status = STATE_Stopped;
	m_Looping = looping;
	if (m_Track != 0 ? CD_Play (m_Track, looping) : CD_PlayCD (looping))
	{
		m_Status = STATE_Playing;
	}
}

void CDSong::Pause ()
{
	if (m_Status == STATE_Playing)
	{
		CD_Pause ();
		m_Status = STATE_Paused;
	}
}

void CDSong::Resume ()
{
	if (m_Status == STATE_Paused)
	{
		if (CD_Resume ())
			m_Status = STATE_Playing;
	}
}

void CDSong::Stop ()
{
	if (m_Status != STATE_Stopped)
	{
		m_Status = STATE_Stopped;
		CD_Stop ();
	}
}

CDSong::~CDSong ()
{
	Stop ();
	m_Inited = false;
}

CDSong::CDSong (int track, int id)
{
	bool success;

	m_Inited = false;

	if (id != 0)
	{
		success = CD_InitID (id);
	}
	else
	{
		success = CD_Init ();
	}

	if (success && track == 0 || CD_CheckTrack (track))
	{
		m_Inited = true;
		m_Track = track;
	}
}

bool CDSong::IsPlaying ()
{
	if (m_Status == STATE_Playing)
	{
		if (CD_GetMode () != CDMode_Play)
		{
			Stop ();
		}
	}
	return m_Status != STATE_Stopped;
}

CDDAFile::CDDAFile (const char *fn)
	: CDSong ()
{
	long fp = kopen4load (fn, 0);
	DWORD chunk;
	WORD track;
	DWORD discid;
	int cursor = 12;

	if (fp == -1)
		return;

	klseek (fp, 12, SEEK_SET);

	// I_RegisterSong already identified this as a CDDA file, so we
	// just need to check the contents we're interested in.

	while (kread (fp, &chunk, 4) == 4)
	{
		if (chunk != (('f')|(('m')<<8)|(('t')<<16)|((' ')<<24)))
		{
			kread (fp, &chunk, 4);
			klseek (fp, SEEK_CUR, chunk);
		}
		else
		{
			klseek (fp, SEEK_CUR, 2);
			if (kread (fp, &track, 2) != 2 || kread (fp, &discid, 4) != 4)
			{
				kclose (fp);
				return;
			}

			kclose (fp);

			if (CD_InitID (discid) && CD_CheckTrack (track))
			{
				m_Inited = true;
				m_Track = track;
			}
			return;
		}
	}
	kclose (fp);
}


