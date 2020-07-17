// Emacs style mode select	 -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
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
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>
#include <string.h>
#include <process.h>

#include <stdarg.h>
#include <sys/types.h>
#include <sys/timeb.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "hardware.h"
#include "doomerrors.h"
#include <math.h>

#include "doomtype.h"
#include "version.h"
#include "doomdef.h"
#include "cmdlib.h"
#include "m_argv.h"
//#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"
//#include "i_music.h"
#include "resource.h"

//#include "d_main.h"
//#include "d_net.h"
//#include "g_game.h"
#include "i_input.h"
#include "i_system.h"
#include "c_dispatch.h"
#include "c_cvars.h"

#include "stats.h"
//#include "duke3d.h"

#define TICRATE 120
extern volatile long totalclock;

#ifdef USEASM
extern "C" BOOL STACK_ARGS CheckMMX (char *vendorid);
#endif

extern "C"
{
	BOOL		HaveRDTSC = 0;
	BOOL		HaveCMOV = 0;
	double		SecondsPerCycle = 1e-8;
	double		CyclesPerSecond = 1e8;		// 100 MHz
	byte		CPUFamily, CPUModel, CPUStepping;
}

extern HWND Window, ConWindow;
extern HINSTANCE g_hInst;

BOOL UseMMX;
UINT TimerPeriod;
HANDLE TimerThread;
bool KillTheTicker;
HANDLE NewTicArrived;
void CalculateCPUSpeed ();

os_t OSPlatform;

void I_Tactile (int on, int off, int total)
{
  // UNUSED.
  on = off = total = 0;
}

static DWORD ticinc;
static DWORD ticerr;
static long clockwait;

DWORD WINAPI TimerTicked (LPVOID foo)
{
	DWORD ticint;

	SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	while (!KillTheTicker)
	{
		InterlockedIncrement (&totalclock);
		if (clockwait != 0 && totalclock == clockwait)
		{
			SetEvent (NewTicArrived);
			clockwait = 0;
		}

		ticerr += ticinc;
		ticint = ticerr >> 16;
		ticerr &= 0xFFFF;
		Sleep (ticint);
	}
	return 0;
}

void I_WaitClocks (int clockCount)
{
	clockwait = totalclock + clockCount;
	WaitForSingleObject (NewTicArrived, INFINITE);
}

void I_WaitVBL (int count)
{
	// I_WaitVBL is never used to actually synchronize to the
	// vertical blank. Instead, it's used for delay purposes.
	Sleep (1000 * count / 70);
}

// [RH] Detect the OS the game is running under
void I_DetectOS (void)
{
	OSVERSIONINFO info;
	const char *osname;

	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx (&info);

	switch (info.dwPlatformId)
	{
	case VER_PLATFORM_WIN32s:
		OSPlatform = os_Win32s;
		osname = "3.x";
		break;

	case VER_PLATFORM_WIN32_WINDOWS:
		OSPlatform = os_Win95;
		if (info.dwMinorVersion < 10)
		{
			osname = "95";
		}
		else if (info.dwMinorVersion < 90)
		{
			osname = "98";
		}
		else
		{
			osname = "Me";
		}
		break;

	case VER_PLATFORM_WIN32_NT:
		OSPlatform = info.dwMajorVersion < 5 ? os_WinNT : os_Win2k;
		if (OSPlatform == os_WinNT)
		{
			osname = "NT";
		}
		else if (info.dwMinorVersion == 0)
		{
			osname = "2000";
		}
		else
		{
			osname = "XP";
		}
		break;

	default:
		OSPlatform = os_unknown;
		osname = "Unknown OS";
		break;
	}

	Printf ("OS: Windows %s %lu.%lu (Build %lu)\n",
			osname,
			info.dwMajorVersion, info.dwMinorVersion,
			OSPlatform == os_Win95 ? info.dwBuildNumber & 0xffff : info.dwBuildNumber);
	if (info.szCSDVersion[0])
	{
		Printf ("    %s\n", info.szCSDVersion);
	}

	if (OSPlatform == os_Win32s)
	{
		I_FatalError ("Sorry, Win32s is not supported.\n"
					  "Upgrade to a newer version of Windows.");
	}
	else if (OSPlatform == os_unknown)
	{
		Printf ("(Assuming Windows 95)\n");
		OSPlatform = os_Win95;
	}
}

//
// I_Init
//
void I_Init (void)
{
#if !defined(USEASM) || !WORKINPROGRESS
	UseMMX = 0;
#else
	char vendorid[13];

	vendorid[0] = vendorid[12] = 0;
	UseMMX = CheckMMX (vendorid);
	if (Args.CheckParm ("-nommx"))
		UseMMX = 0;

	if (vendorid[0])
		Printf ("CPUID: %s  (", vendorid);

	if (UseMMX)
		Printf ("using MMX)\n");
	else
		Printf ("not using MMX)\n");

	Printf ("       family %d, model %d, stepping %d\n", CPUFamily, CPUModel, CPUStepping);
	CalculateCPUSpeed ();
#endif

	// Use a timer event if possible
	NewTicArrived = CreateEvent (NULL, FALSE, FALSE, NULL);
	if (NewTicArrived)
	{
		DWORD tid;

		ticinc = 65536000 / TICRATE;
		ticerr = 0;
		TimerThread = CreateThread (NULL, 0, TimerTicked, 0, 0, &tid);
	}
	if (TimerThread == 0)
	{
		if (NewTicArrived)
		{
			CloseHandle (NewTicArrived);
			NewTicArrived = 0;
		}
		I_FatalError ("Could not create timer");
	}

	I_InitSound ();
	I_InitInput (Window);
	I_InitHardware ();
}

void CalculateCPUSpeed ()
{
	LARGE_INTEGER freq;

	QueryPerformanceFrequency (&freq);

	if (freq.QuadPart != 0 && HaveRDTSC)
	{
		LARGE_INTEGER count1, count2;
		DWORD minDiff;
		tsc_t ClockCalibration = 0;

		// Count cycles for at least 55 milliseconds.
		// The performance counter is very low resolution compared to CPU
		// speeds today, so the longer we count, the more accurate our estimate.
		// On the other hand, we don't want to count too long, because we don't
		// want the user to notice us spend time here, since most users will
		// probably never use the performance statistics.
		minDiff = freq.LowPart * 11 / 200;

		// Minimize the chance of task switching during the testing by going very
		// high priority. This is another reason to avoid timing for too long.
		SetPriorityClass (GetCurrentProcess (), REALTIME_PRIORITY_CLASS);
		SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);
		clock (ClockCalibration);
		QueryPerformanceCounter (&count1);
		do
		{
			QueryPerformanceCounter (&count2);
		} while ((DWORD)((unsigned __int64)count2.QuadPart - (unsigned __int64)count1.QuadPart) < minDiff);
		unclock (ClockCalibration);
		QueryPerformanceCounter (&count2);
		SetPriorityClass (GetCurrentProcess (), NORMAL_PRIORITY_CLASS);
		SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_NORMAL);

		CyclesPerSecond = (double)ClockCalibration *
			(double)freq.QuadPart /
			(double)((__int64)count2.QuadPart - (__int64)count1.QuadPart);
		SecondsPerCycle = 1.0 / CyclesPerSecond;
	}
	else
	{
		Printf ("Can't determine CPU speed, so pretending.\n");
	}

	Printf ("CPU Speed: %f MHz\n", CyclesPerSecond / 1e6);
}

//
// I_Quit
//
static int has_exited;

void STACK_ARGS I_Quit (void)
{
	has_exited = 1;		/* Prevent infinitely recursive exits -- killough */

	if (TimerThread)
	{
		KillTheTicker = true;
		WaitForSingleObject (TimerThread, INFINITE);
		CloseHandle (TimerThread);
	}
	if (NewTicArrived)
	{
		CloseHandle (NewTicArrived);
	}

	timeEndPeriod (TimerPeriod);

#if WORKINPROGRESS
	if (demorecording)
		G_CheckDemoStatus();
	G_ClearSnapshots ();
#endif

}


//
// I_Error
//
extern FILE *Logfile;
BOOL gameisdead;

void STACK_ARGS I_FatalError (const char *error, ...)
{
	static BOOL alreadyThrown = false;
	gameisdead = true;

	if (!alreadyThrown)		// ignore all but the first message -- killough
	{
		char errortext[MAX_ERRORTEXT];
		int index;
		va_list argptr;
		va_start (argptr, error);
		index = vsprintf (errortext, error, argptr);
// GetLastError() is usually useless because we don't do a lot of Win32 stuff
//		sprintf (errortext + index, "\nGetLastError = %ld", GetLastError());
		va_end (argptr);

		// Record error to log (if logging)
		if (Logfile)
			fprintf (Logfile, "\n**** DIED WITH FATAL ERROR:\n%s\n", errortext);

		throw CFatalError (errortext);
	}

	if (!has_exited)	// If it hasn't exited yet, exit now -- killough
	{
		has_exited = 1;	// Prevent infinitely recursive exits -- killough
		exit(-1);
	}
}

void STACK_ARGS I_Error (const char *error, ...)
{
	va_list argptr;
	char errortext[MAX_ERRORTEXT];

	va_start (argptr, error);
	vsprintf (errortext, error, argptr);
	va_end (argptr);

	throw CRecoverableError (errortext);
}

char DoomStartupTitle[256] = { 0 };

void I_SetTitleString (const char *title)
{
	int i;

	for (i = 0; title[i]; i++)
		DoomStartupTitle[i] = title[i];
}

void I_PrintStr (const char *cp, bool lineBreak)
{
	if (ConWindow == NULL)
		return;

	static bool newLine = true;
	HWND edit = (HWND)GetWindowLongPtr (ConWindow, GWLP_USERDATA);
	char buf[256];
	int bpos = 0;

	SendMessage (edit, EM_SETSEL, -1, 0);

	if (lineBreak && !newLine)
	{
		buf[0] = '\r';
		buf[1] = '\n';
		bpos = 2;
	}
	while (*cp != NULL)
	{
		if (*cp == 28)
		{ // Skip color changes
			if (*++cp != 0)
				cp++;
			continue;
		}
		if (bpos < 253)
		{
			// Stupid edit controls need CR-LF pairs
			if (*cp == '\n')
			{
				buf[bpos++] = '\r';
			}
		}
		else
		{
			buf[bpos] = 0;
			SendMessage (edit, EM_REPLACESEL, FALSE, (LPARAM)buf);
			newLine = buf[bpos-1] == '\n';
			bpos = 0;
		}
		buf[bpos++] = *cp++;
	}
	if (bpos != 0)
	{
		buf[bpos] = 0;
		SendMessage (edit, EM_REPLACESEL, FALSE, (LPARAM)buf);
		newLine = buf[bpos-1] == '\n';
	}
}

bool STACK_ARGS I_YesNoQuestion (const char *fmt, ...)
{
	char quest[1024];
	va_list argptr;

	va_start (argptr, fmt);
	vsprintf (quest, fmt, argptr);
	va_end (argptr);

	return MessageBox (Window, quest, "ZDuke Question", MB_YESNO|MB_ICONQUESTION) == IDYES;
}

void STACK_ARGS I_OkayMessage (const char *fmt, ...)
{
	char quest[1024];
	va_list argptr;

	va_start (argptr, fmt);
	vsprintf (quest, fmt, argptr);
	va_end (argptr);

	MessageBox (Window, quest, "ZDuke Message", MB_OK|MB_ICONINFORMATION);
}

void *I_FindFirst (const char *filespec, findstate_t *fileinfo)
{
	return FindFirstFileA (filespec, (LPWIN32_FIND_DATAA)fileinfo);
}
int I_FindNext (void *handle, findstate_t *fileinfo)
{
	return !FindNextFileA ((HANDLE)handle, (LPWIN32_FIND_DATAA)fileinfo);
}

int I_FindClose (void *handle)
{
	return FindClose ((HANDLE)handle);
}
