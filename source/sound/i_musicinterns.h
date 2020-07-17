#define WIN32_LEAN_AND_MEAN
#if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x0400
#undef _WIN32_WINNT
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#include <windows.h>
#include <mmsystem.h>
#include <fmod.h>
//#include "oplsynth/opl_mus_player.h"
#include "c_cvars.h"
#include "mus2midi.h"

void I_InitMusicWin32 ();
void I_ShutdownMusicWin32 ();

extern FSOUND_DSPUNIT *DryUnit;

// The base music class. Everything is derived from this --------------------

class MusInfo
{
public:
	MusInfo () : m_Status(STATE_Stopped), m_LumpMem(0) {}
	virtual ~MusInfo ();
	virtual void SetVolume (float volume) = 0;
	virtual void Play (bool looping) = 0;
	virtual void Pause () = 0;
	virtual void Resume () = 0;
	virtual void Stop () = 0;
	virtual bool IsPlaying () = 0;
	virtual bool IsMIDI () const = 0;
	virtual bool IsValid () const = 0;
	virtual bool SetPosition (int order);

	enum EState
	{
		STATE_Stopped,
		STATE_Playing,
		STATE_Paused
	} m_Status;
	bool m_Looping;
	const void *m_LumpMem;
	int m_LumpFP;
};

// MUS file played with MIDI output messages --------------------------------

class MUSSong2 : public MusInfo
{
public:
	MUSSong2 (const char *fn);
	~MUSSong2 ();

	void SetVolume (float volume);
	void Play (bool looping);
	void Pause ();
	void Resume ();
	void Stop ();
	bool IsPlaying ();
	bool IsMIDI () const;
	bool IsValid () const;

protected:
	static DWORD WINAPI PlayerProc (LPVOID lpParameter);
	void OutputVolume (DWORD volume);
	int SendCommand ();

	enum
	{
		SEND_DONE,
		SEND_WAIT
	};

	HMIDIOUT MidiOut;
	HANDLE PlayerThread;
	HANDLE PauseEvent;
	HANDLE ExitEvent;
	HANDLE VolumeChangeEvent;
	DWORD SavedVolume;
	bool VolumeWorks;

	BYTE *MusBuffer;
	MUSHeader *MusHeader;
	BYTE LastVelocity[16];
	BYTE ChannelVolumes[16];
	size_t MusP, MaxMusP;
};

// MIDI file played with MIDI output messages -------------------------------

class MIDISong2 : public MusInfo
{
public:
	MIDISong2 (const char *fn);
	~MIDISong2 ();

	void SetVolume (float volume);
	void Play (bool looping);
	void Pause ();
	void Resume ();
	void Stop ();
	bool IsPlaying ();
	bool IsMIDI () const;
	bool IsValid () const;

protected:
	struct TrackInfo;

	static DWORD WINAPI PlayerProc (LPVOID lpParameter);
	void OutputVolume (DWORD volume);
	void ProcessInitialMetaEvents ();
	DWORD SendCommands ();
	void SendCommand (TrackInfo *track);
	TrackInfo *FindNextDue ();

	HMIDIOUT MidiOut;
	HANDLE PlayerThread;
	HANDLE PauseEvent;
	HANDLE ExitEvent;
	HANDLE VolumeChangeEvent;
	DWORD SavedVolume;
	bool VolumeWorks;

	BYTE *MusHeader;
	BYTE ChannelVolumes[16];
	TrackInfo *Tracks;
	TrackInfo *TrackDue;
	int NumTracks;
	int Format;
	int Division;
	int Tempo;
	WORD DesignationMask;
};

// MOD file played with FMOD ------------------------------------------------

class MODSong : public MusInfo
{
public:
	MODSong (const char *fn);
	~MODSong ();
	void SetVolume (float volume);
	void Play (bool looping);
	void Pause ();
	void Resume ();
	void Stop ();
	bool IsPlaying ();
	bool IsMIDI () const { return false; }
	bool IsValid () const { return m_Module != NULL; }
	bool SetPosition (int order);

protected:
	MODSong () {}

	void SetMusicReserved (signed char flag);

	FMUSIC_MODULE *m_Module;
};

// OGG/MP3/WAV/other format streamed through FMOD ---------------------------

class StreamSong : public MusInfo
{
public:
	StreamSong (const char *fn);
	~StreamSong ();
	void SetVolume (float volume);
	void Play (bool looping);
	void Pause ();
	void Resume ();
	void Stop ();
	bool IsPlaying ();
	bool IsMIDI () const { return false; }
	bool IsValid () const { return m_Stream != NULL; }

protected:
	StreamSong () : m_Stream (NULL), m_Channel (-1) {}

	void SetStreamReserved (signed char flag);

	FSOUND_STREAM *m_Stream;
	int m_Channel;
	int m_LastPos;
};

// SPC file, rendered with SNESAPU.DLL and streamed through FMOD ------------

typedef void (__stdcall *SNESAPUInfo_TYPE) (DWORD&, DWORD&, DWORD&);
typedef void (__stdcall *GetAPUData_TYPE) (void**, BYTE**, BYTE**, DWORD**, void**, void**, DWORD**, DWORD**);
typedef void (__stdcall *ResetAPU_TYPE) (DWORD);
typedef void (__stdcall *FixAPU_TYPE) (WORD, BYTE, BYTE, BYTE, BYTE, BYTE);
typedef void (__stdcall *SetAPUOpt_TYPE) (DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
typedef void *(__stdcall *EmuAPU_TYPE) (void *, DWORD, BYTE);

class SPCSong : public StreamSong
{
public:
	SPCSong (const char *fn);
	~SPCSong ();
	void Play (bool looping);
	bool IsPlaying ();
	bool IsValid () const;

protected:
	bool LoadEmu ();
	void CloseEmu ();

	static signed char STACK_ARGS FillStream (FSOUND_STREAM *stream, void *buff, int len, int param);

	HINSTANCE HandleAPU;

	SNESAPUInfo_TYPE SNESAPUInfo;
	GetAPUData_TYPE GetAPUData;
	ResetAPU_TYPE ResetAPU;
	FixAPU_TYPE FixAPU;
	SetAPUOpt_TYPE SetAPUOpt;
	EmuAPU_TYPE EmuAPU;
};

// MUS file played by a software OPL2 synth and streamed through FMOD -------

#if 0
class OPLMUSSong : public StreamSong
{
public:
	OPLMUSSong (const void *mem, int len);
	~OPLMUSSong ();
	void Play (bool looping);
	bool IsPlaying ();
	bool IsValid () const;
	void ResetChips ();

protected:
	static signed char STACK_ARGS FillStream (FSOUND_STREAM *stream, void *buff, int len, int param);

	OPLmusicBlock *Music;
};
#endif

// FLAC file streamed through FMOD (You should probably use Vorbis instead) -

class FLACSong : public StreamSong
{
public:
	FLACSong (const char *fn);
	~FLACSong ();
	void Play (bool looping);
	bool IsPlaying ();
	bool IsValid () const;

protected:
	static signed char STACK_ARGS FillStream (FSOUND_STREAM *stream, void *buff, int len, int param);

	class FLACStreamer;

	FLACStreamer *State;
};

// CD track/disk played through the multimedia system -----------------------

class CDSong : public MusInfo
{
public:
	CDSong (int track, int id);
	~CDSong ();
	void SetVolume (float volume) {}
	void Play (bool looping);
	void Pause ();
	void Resume ();
	void Stop ();
	bool IsPlaying ();
	bool IsMIDI () const { return false; }
	bool IsValid () const { return m_Inited; }

protected:
	CDSong () : m_Inited(false) {}

	int m_Track;
	bool m_Inited;
};

// CD track on a specific disk played through the multimedia system ---------

class CDDAFile : public CDSong
{
public:
	CDDAFile (const char *fn);
};

// --------------------------------------------------------------------------

extern MusInfo *currSong;
extern int		nomusic;

EXTERN_CVAR (Float, snd_musicvolume)
EXTERN_CVAR (Bool, opl_enable)
