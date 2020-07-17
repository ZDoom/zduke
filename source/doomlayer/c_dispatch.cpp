/*
** c_dispatch.cpp
** Functions for executing console commands and aliases
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "templates.h"
#include "doomtype.h"
#include "cmdlib.h"
#include "c_console.h"
#include "c_dispatch.h"
#include "c_cvars.h"
#include "m_argv.h"
#include "d_gui.h"
//#include "doomstat.h"
#include "m_alloc.h"
//#include "d_player.h"
//#include "configfile.h"
#include "m_crc32.h"
//#include "v_text.h"
#include "duke3d.h"


// WORKINPROGRESS
#define TEXTCOLOR_YELLOW ""
#define TEXTCOLOR_ORANGE ""
static int gamestate;
#define GS_STARTUP 0
// WORKINPROGRESS


static long ParseCommandLine (const char *args, int *argc, char **argv);

CVAR (Bool, lookspring, true, CVAR_ARCHIVE);	// Generate centerview when -mlook encountered?

// The three standard shift keys. These are automatically set before
// each command is dispatched, so trying to set them at the console
// is meaningless.
CVAR (Bool, shiftdown, false, 0);
CVAR (Bool, altdown, false, 0);
CVAR (Bool, ctrldown, false, 0);

struct FActionMap
{
	unsigned int	Key;	// value from passing Name to MakeKey()
	FButtonStatus	*Button;
	char			Name[12];
};

struct FQueuedCommand
{
	FQueuedCommand	*Next;
	long			DispatchClock;	// totalclock to execute this command on
	WORD			KeyNum;			// key binding this command came from (0 for none)
	WORD			bShiftDown:1;
	WORD			bAltDown:1;
	WORD			bCtrlDown:1;
	size_t			MaxCommandSize;	// These structs are reused, so record what size this one holds
	char			Command[1];		// 1+actual command size

	static void QueueCommand (const char *command, int keynum, long time);
};

static FConsoleCommand *FindNameInHashTable (FConsoleCommand **table, const char *name, size_t namelen);
static FConsoleCommand *ScanChainForName (FConsoleCommand *start, const char *name, size_t namelen, FConsoleCommand **prev);

FConsoleCommand *Commands[HASH_SIZE];

FQueuedCommand *CommandQueue;
FQueuedCommand *CommandQueuePool;

FButtonStatus
   gamefunc_Move_Forward,
   gamefunc_Move_Backward,
   gamefunc_Turn_Left,
   gamefunc_Turn_Right,
   gamefunc_Strafe,
   gamefunc_Fire,
   gamefunc_Open,
   gamefunc_Run,
   gamefunc_AutoRun,
   gamefunc_Jump,
   gamefunc_Crouch,
   gamefunc_Look_Up,
   gamefunc_Look_Down,
   gamefunc_Look_Left,
   gamefunc_Look_Right,
   gamefunc_Strafe_Left,
   gamefunc_Strafe_Right,
   gamefunc_Aim_Up,
   gamefunc_Aim_Down,
   gamefunc_Weapon_1,
   gamefunc_Weapon_2,
   gamefunc_Weapon_3,
   gamefunc_Weapon_4,
   gamefunc_Weapon_5,
   gamefunc_Weapon_6,
   gamefunc_Weapon_7,
   gamefunc_Weapon_8,
   gamefunc_Weapon_9,
   gamefunc_Weapon_10,
   gamefunc_Inventory,
   gamefunc_Inventory_Left,
   gamefunc_Inventory_Right,
   gamefunc_Holo_Duke,
   gamefunc_Jetpack,
   gamefunc_NightVision,
   gamefunc_MedKit,
   gamefunc_TurnAround,
   gamefunc_SendMessage,
   gamefunc_Map,
   gamefunc_Shrink_Screen,
   gamefunc_Enlarge_Screen,
   gamefunc_Center_View,
   gamefunc_Holster_Weapon,
   gamefunc_Show_Opponents_Weapon,
   gamefunc_Map_Follow_Mode,
   gamefunc_See_Coop_View,
   gamefunc_Mouse_Aiming,
   gamefunc_Toggle_Crosshair,
   gamefunc_Steroids,
   gamefunc_Quick_Kick,
   gamefunc_Next_Weapon,
   gamefunc_Previous_Weapon;

// To add new actions, go to the console and type "key <action name>".
// This will give you the key value to use in the first column. Then
// insert your new action into this list so that the keys remain sorted
// in ascending order. No two keys can be identical. If yours matches
// an existing key, either modify MakeKey(), or (preferably) change the
// name of your action.

FActionMap ActionMaps[] =
{
	{ 0x0f26fef6, &gamefunc_Run,			"speed" },
	//{ 0x1ccf57bf, &Button_MoveUp,			"moveup" },
	//{ 0x22beba5f, &Button_Klook,			"klook" },
	{ 0x38b09595, &gamefunc_Look_Right,		"lookright" },
	{ 0x47c02d3b, &gamefunc_Fire,			"attack" },
	{ 0x661cb595, &gamefunc_Aim_Up,			"aimup" },
	{ 0x6dcec137, &gamefunc_Move_Backward,	"back" },
	{ 0x7a67e768, &gamefunc_Turn_Left,		"left" },
	{ 0x8076f318, &gamefunc_Crouch,			"crouch" },
	{ 0x84b8789a, &gamefunc_Strafe_Left,	"moveleft" },
	//{ 0x8fd9bf1e, &Button_ShowScores,		"showscores" },
	{ 0x94b1cc4b, &gamefunc_Open,			"use" },
	{ 0x9f03d991, &gamefunc_Shrink_Screen,	"sizeup" },
	{ 0xa45c6e4b, &gamefunc_Quick_Kick,		"quickkick" },
	{ 0xa7b30616, &gamefunc_Jump,			"jump" },
	{ 0xadfe4fff, &gamefunc_Mouse_Aiming,	"mlook" },
	{ 0xb4ca7514, &gamefunc_Turn_Right,		"right" },
	{ 0xb563e265, &gamefunc_Look_Down,		"lookdown" },
	{ 0xb67a0835, &gamefunc_Strafe,			"strafe" },
	{ 0xc34f3a66, &gamefunc_Aim_Down,		"aimdown" },
	{ 0xd3fb9536, &gamefunc_Look_Left,		"lookleft" },
	{ 0xe0857e4c, &gamefunc_Enlarge_Screen,	"sizedown" },
	//{ 0xe2200fc9, &Button_MoveDown,		"movedown" },
	{ 0xe78739bb, &gamefunc_Strafe_Right,	"moveright" },
	{ 0xe7912f86, &gamefunc_Move_Forward,	"forward" },
	{ 0xf01cb105, &gamefunc_Look_Up,		"lookup" },
};

#define NUM_ACTIONS (sizeof(ActionMaps)/sizeof(FActionMap))

static int ListActionCommands (const char *pattern)
{
	char matcher[16];
	unsigned int i;
	int count = 0;

	for (i = 0; i < NUM_ACTIONS; ++i)
	{
		if (pattern == NULL || CheckWildcards (pattern,
			(sprintf (matcher, "+%s", ActionMaps[i].Name), matcher)))
		{
			Printf ("+%s\n", ActionMaps[i].Name);
			count++;
		}
		if (pattern == NULL || CheckWildcards (pattern,
			(sprintf (matcher, "-%s", ActionMaps[i].Name), matcher)))
		{
			Printf ("-%s\n", ActionMaps[i].Name);
			count++;
		}
	}
	return count;
}

unsigned int MakeKey (const char *s)
{
	DWORD key = 0xffffffff;
	const DWORD *table = GetCRCTable ();

	while (*s)
	{
		key = CRC1 (key, tolower (*s++), table);
	}
	return key ^ 0xffffffff;
}

unsigned int MakeKey (const char *s, size_t len)
{
	if (len == 0)
	{
		return 0xffffffff;
	}

	DWORD key = 0xffffffff;
	const DWORD *table = GetCRCTable ();

	while (len > 0)
	{
		key = CRC1 (key, tolower(*s++), table);
		--len;
	}
	return key ^ 0xffffffff;
}

// FindButton scans through the actionbits[] array
// for a matching key and returns an index or -1 if
// the key could not be found. This uses binary search,
// so actionbits[] must be sorted in ascending order.

FButtonStatus *FindButton (unsigned int key)
{
	const FActionMap *bit;

	bit = BinarySearch<FActionMap, unsigned int>
			(ActionMaps, NUM_ACTIONS, &FActionMap::Key, key);
	return bit ? bit->Button : NULL;
}

void FButtonStatus::PressKey (int keynum)
{
	int i, open;

	keynum &= 0x0FFF;

	if (keynum == 0)
	{ // Issued from console instead of a key, so force on
		Keys[0] = 0xffff;
		for (i = MAX_KEYS-1; i > 0; --i)
		{
			Keys[i] = 0;
		}
	}
	else
	{
		for (i = MAX_KEYS-1, open = -1; i >= 0; --i)
		{
			if (Keys[i] == 0)
			{
				open = i;
			}
			else if (Keys[i] == keynum)
			{ // Key is already down; do nothing
				return;
			}
		}
		if (open < 0)
		{ // No free key slots, so do nothing
			Printf ("More than %u keys pressed for a single action!\n", MAX_KEYS);
			return;
		}
		Keys[open] = keynum;
	}
	bDown = bWentDown = true;
}

void FButtonStatus::ReleaseKey (int keynum)
{
	int i, numdown, match;

	keynum &= 0x0FFF;

	if (keynum == 0)
	{ // Issued from console instead of a key, so force off
		for (i = MAX_KEYS-1; i >= 0; --i)
		{
			Keys[i] = 0;
		}
		bWentUp = true;
		bDown = false;
	}
	else
	{
		for (i = MAX_KEYS-1, numdown = 0, match = -1; i >= 0; --i)
		{
			if (Keys[i] != 0)
			{
				++numdown;
				if (Keys[i] == keynum)
				{
					match = i;
				}
			}
		}
		if (match < 0)
		{ // Key was not down; do nothing
			return;
		}
		Keys[match] = 0;
		bWentUp = true;
		if (--numdown == 0)
		{
			bDown = false;
		}
	}
}

void ResetButtonTriggers ()
{
	for (int i = NUM_ACTIONS-1; i >= 0; --i)
	{
		ActionMaps[i].Button->ResetTriggers ();
	}
}

void ResetButtonStates ()
{
	for (int i = NUM_ACTIONS-1; i >= 0; --i)
	{
		FButtonStatus *button = ActionMaps[i].Button;


		if (button != &gamefunc_Mouse_Aiming/* WORKINPROGRESS && button != &Button_Klook*/)
		{
			button->ReleaseKey (0);
		}
		button->ResetTriggers ();
	}
}

void C_DoCommand (const char *cmd, int keynum)
{
	FConsoleCommand *com;
	const char *end;
	const char *beg;

	// Skip any beginning whitespace
	while (*cmd && *cmd <= ' ')
		cmd++;

	// Find end of the command name
	if (*cmd == '\"')
	{
		for (end = beg = cmd+1; *end && *end != '\"'; ++end)
			;
	}
	else
	{
		beg = cmd;
		for (end = cmd+1; *end > ' '; ++end)
			;
	}

	const int len = end - beg;

	// Check if this is an action
	if (*beg == '+' || *beg == '-')
	{
		FButtonStatus *button;

		button = FindButton (MakeKey (beg + 1, end - beg - 1));
		if (button != NULL)
		{
			if (*beg == '+')
			{
				button->PressKey (keynum);
			}
			else
			{
				button->ReleaseKey (keynum);
				if (button == &gamefunc_Mouse_Aiming && lookspring)
				{
					C_DoCommand ("centerview");
				}
			}
			return;
		}
	}
	
	// Parse it as a normal command
	// Checking for matching commands follows this search order:
	//	1. Check the Commands[] hash table
	//	2. Check the CVars list

	if ( (com = FindNameInHashTable (Commands, beg, len)) )
	{/*
		if (gamestate != GS_STARTUP ||
			(len == 3 && strnicmp (beg, "set", 3) == 0) ||
			(len == 7 && strnicmp (beg, "logfile", 7) == 0) ||
			(len == 9 && strnicmp (beg, "unbindall", 9) == 0) ||
			(len == 4 && strnicmp (beg, "bind", 4) == 0) ||
			(len == 4 && strnicmp (beg, "exec", 4) == 0) ||
			(len ==10 && strnicmp (beg, "doublebind", 10) == 0)
			)*/
		{
			FCommandLine args (beg);
			com->Run (args, keynum);
		}
	/*
		else
		{

			new DStoredCommand (com, beg);
		}*/
	}
	else
	{ // Check for any console vars that match the command
		FBaseCVar *var = FindCVarSub (beg, len);

		if (var != NULL)
		{
			FCommandLine args (beg);

			if (args.argc() >= 2)
			{ // Set the variable
				var->CmdSet (args[1]);
			}
			else
			{ // Get the variable's value
				UCVarValue val = var->GetGenericRep (CVAR_String);
				Printf ("\"%s\" is \"%s\"\n", var->GetName(), val.String);
			}
		}
		else
		{ // We don't know how to handle this command
			char cmdname[64];
			int minlen = MIN (len, 63);

			memcpy (cmdname, beg, minlen);
			cmdname[len] = 0;
			Printf ("Unknown command \"%s\"\n", cmdname);
		}
	}
}

void AddCommandString (char *cmd, int keynum, long clockdue)
{
	char *brkpt;
	int more;

	if (clockdue == 0)
	{
		clockdue = totalclock;
	}

	if (cmd)
	{
		while (*cmd)
		{
			brkpt = cmd;
			while (*brkpt != ';' && *brkpt != '\0')
			{
				if (*brkpt == '\"')
				{
					brkpt++;
					while (*brkpt != '\0' && (*brkpt != '\"' || *(brkpt-1) == '\\'))
						brkpt++;
				}
				brkpt++;
			}
			if (*brkpt == ';')
			{
				*brkpt = '\0';
				more = 1;
			}
			else
			{
				more = 0;
			}
			// Intercept wait commands here. Note: wait must be lowercase
			while (*cmd && *cmd <= ' ')
			{
				cmd++;
			}
			if (*cmd)
			{
				if (cmd[0] == 'w' && cmd[1] == 'a' && cmd[2] == 'i' && cmd[3] == 't' &&
					(cmd[4] == 0 || cmd[4] == ' '))
				{
					int tics;

					if (cmd[4] == ' ')
					{
						tics = strtol (cmd + 5, NULL, 0);
					}
					else
					{
						tics = 1;
					}
					clockdue += tics * TICSPERFRAME;
					if (more)
					{
						*brkpt = ';';
					}
				}
				else
				{
					FQueuedCommand::QueueCommand (cmd, keynum, clockdue);
				}
			}
			if (more)
			{
				*brkpt = ';';
			}
			cmd = brkpt + more;
		}
	}
}

void FQueuedCommand::QueueCommand (const char *command, int keynum, long time)
{
	FQueuedCommand *probe = CommandQueuePool, **prev = &CommandQueuePool, *qcmd;
	size_t cmdlen = strlen (command);

	while (probe != NULL)
	{
		if (probe->MaxCommandSize >= cmdlen)
		{
			*prev = probe->Next;
			break;
		}
		prev = &probe->Next;
		probe = probe->Next;
	}
	if (probe == NULL)
	{
		// Allocate a little more than needed to make this more reusable
		qcmd = (FQueuedCommand *)Malloc (sizeof(*probe) + cmdlen + 64);
		qcmd->MaxCommandSize = cmdlen + 64;
	}
	else
	{
		qcmd = probe;
	}

	qcmd->KeyNum = keynum & 0x0FFF;
	qcmd->bShiftDown = (keynum & 0x1000) != 0;
	qcmd->bAltDown = (keynum & 0x2000) != 0;
	qcmd->bCtrlDown = (keynum & 0x4000) != 0;
	qcmd->DispatchClock = time;
	memcpy (qcmd->Command, command, (cmdlen+1)*sizeof(*command));

	// Insert command in order (earliest first)
	probe = CommandQueue;
	prev = &CommandQueue;

	while (probe != NULL && probe->DispatchClock <= time)
	{
		prev = &probe->Next;
		probe = probe->Next;
	}

	qcmd->Next = probe;
	*prev = qcmd;
}

void C_DispatchCommands (long clock)
{
	FQueuedCommand *probe = CommandQueue;

	while (probe != NULL && probe->DispatchClock <= clock)
	{
		FQueuedCommand *next;
		shiftdown = probe->bShiftDown;
		altdown = probe->bAltDown;
		ctrldown = probe->bCtrlDown;
		C_DoCommand (probe->Command, probe->KeyNum);
		next = probe->Next;
		probe->Next = CommandQueuePool;
		CommandQueuePool = probe;
		probe = next;
	}
	CommandQueue = probe;
}

// ParseCommandLine
//
// Parse a command line (passed in args). If argc is non-NULL, it will
// be set to the number of arguments. If argv is non-NULL, it will be
// filled with pointers to each argument; argv[0] should be initialized
// to point to a buffer large enough to hold all the arguments. The
// return value is the necessary size of this buffer.
//
// Special processing: Inside quoted strings, \" becomes just "
// $<cvar> is replaced by the contents of <cvar>

static long ParseCommandLine (const char *args, int *argc, char **argv)
{
	int count;
	char *buffplace;

	count = 0;
	buffplace = NULL;
	if (argv != NULL)
	{
		buffplace = argv[0];
	}

	for (;;)
	{
		while (*args <= ' ' && *args)
		{ // skip white space
			args++;
		}
		if (*args == 0)
		{
			break;
		}
		else if (*args == '\"')
		{ // read quoted string
			char stuff;
			if (argv != NULL)
			{
				argv[count] = buffplace;
			}
			count++;
			args++;
			do
			{
				stuff = *args++;
				if (stuff == '\\' && *args == '\"')
				{
					stuff = '\"', args++;
				}
				else if (stuff == '\"')
				{
					stuff = 0;
				}
				else if (stuff == 0)
				{
					args--;
				}
				if (argv != NULL)
				{
					*buffplace = stuff;
				}
				buffplace++;
			} while (stuff);
		}
		else
		{ // read unquoted string
			const char *start = args++, *end;
			FBaseCVar *var;
			UCVarValue val;

			while (*args && *args > ' ' && *args != '\"')
				args++;
			if (*start == '$' && (var = FindCVarSub (start+1, args-start-1)))
			{
				val = var->GetGenericRep (CVAR_String);
				start = val.String;
				end = start + strlen (start);
			}
			else
			{
				end = args;
			}
			if (argv != NULL)
			{
				argv[count] = buffplace;
				while (start < end)
					*buffplace++ = *start++;
				*buffplace++ = 0;
			}
			else
			{
				buffplace += end - start + 1;
			}
			count++;
		}
	}
	if (argc != NULL)
	{
		*argc = count;
	}
	return (long)(buffplace - (char *)0);
}

FCommandLine::FCommandLine (const char *commandline)
{
	cmd = commandline;
	_argc = -1;
	_argv = NULL;
}

FCommandLine::~FCommandLine ()
{
	if (_argv != NULL)
	{
		delete[] _argv;
	}
}

int FCommandLine::argc ()
{
	if (_argc == -1)
	{
		argsize = ParseCommandLine (cmd, &_argc, NULL);
	}
	return _argc;
}

char *FCommandLine::operator[] (int i)
{
	if (_argv == NULL)
	{
		int count = argc();
		_argv = new char *[count + (argsize+sizeof(char*)-1)/sizeof(char*)];
		_argv[0] = (char *)_argv + count*sizeof(char *);
		ParseCommandLine (cmd, NULL, _argv);
	}
	return _argv[i];
}

static FConsoleCommand *ScanChainForName (FConsoleCommand *start, const char *name, size_t namelen, FConsoleCommand **prev)
{
	int comp;

	*prev = NULL;
	while (start)
	{
		comp = strnicmp (start->m_Name, name, namelen);
		if (comp > 0)
			return NULL;
		else if (comp == 0 && start->m_Name[namelen] == 0)
			return start;

		*prev = start;
		start = start->m_Next;
	}
	return NULL;
}

static FConsoleCommand *FindNameInHashTable (FConsoleCommand **table, const char *name, size_t namelen)
{
	FConsoleCommand *dummy;

	return ScanChainForName (table[MakeKey (name, namelen) % HASH_SIZE], name, namelen, &dummy);
}

bool FConsoleCommand::AddToHash (FConsoleCommand **table)
{
	unsigned int key;
	FConsoleCommand *insert, **bucket;

	key = MakeKey (m_Name);
	bucket = &table[key % HASH_SIZE];

	if (ScanChainForName (*bucket, m_Name, strlen (m_Name), &insert))
	{
		return false;
	}
	else
	{
		if (insert)
		{
			m_Next = insert->m_Next;
			if (m_Next)
				m_Next->m_Prev = &m_Next;
			insert->m_Next = this;
			m_Prev = &insert->m_Next;
		}
		else
		{
			m_Next = *bucket;
			*bucket = this;
			m_Prev = bucket;
			if (m_Next)
				m_Next->m_Prev = &m_Next;
		}
	}
	return true;
}

FConsoleCommand::FConsoleCommand (const char *name, CCmdRun runFunc)
	: m_RunFunc (runFunc)
{
	static bool firstTime = true;

	if (firstTime)
	{
		char tname[16];
		unsigned int i;

		firstTime = false;

		// Add all the action commands for tab completion
		for (i = 0; i < NUM_ACTIONS; i++)
		{
			strcpy (&tname[1], ActionMaps[i].Name);
			tname[0] = '+';
			C_AddTabCommand (tname);
			tname[0] = '-';
			C_AddTabCommand (tname);
		}
	}

	int ag = strcmp (name, "kill");
	if (ag == 0)
		ag=0;
	m_Name = copystring (name);

	if (!AddToHash (Commands))
		Printf ("FConsoleCommand c'tor: %s exists\n", name);
	else
		C_AddTabCommand (name);
}

FConsoleCommand::~FConsoleCommand ()
{
	*m_Prev = m_Next;
	if (m_Next)
		m_Next->m_Prev = m_Prev;
	C_RemoveTabCommand (m_Name);
	delete[] m_Name;
}

void FConsoleCommand::Run (FCommandLine &argv, int key)
{
	m_RunFunc (argv, key);
}

FConsoleAlias::FConsoleAlias (const char *name, const char *command, bool noSave)
	: FConsoleCommand (name, NULL),
	  bRunning (false), bKill (false)
{
	m_Command[noSave] = copystring (command);
	m_Command[!noSave] = NULL;
}

FConsoleAlias::~FConsoleAlias ()
{
	for (int i = 0; i < 2; ++i)
	{
		if (m_Command[i] != NULL)
		{
			delete[] m_Command[i];
			m_Command[i] = NULL;
		}
	}
}

char *BuildString (int argc, char **argv)
{
	char temp[1024];
	char *cur;
	int arg;

	if (argc == 1)
	{
		return copystring (*argv);
	}
	else
	{
		cur = temp;
		for (arg = 0; arg < argc; arg++)
		{
			if (strchr (argv[arg], ' '))
			{
				cur += sprintf (cur, "\"%s\" ", argv[arg]);
			}
			else
			{
				cur += sprintf (cur, "%s ", argv[arg]);
			}
		}
		temp[strlen (temp) - 1] = 0;
		return copystring (temp);
	}
}

static int DumpHash (FConsoleCommand **table, BOOL aliases, const char *pattern=NULL)
{
	int bucket, count;
	FConsoleCommand *cmd;

	for (bucket = count = 0; bucket < HASH_SIZE; bucket++)
	{
		cmd = table[bucket];
		while (cmd)
		{
			if (CheckWildcards (pattern, cmd->m_Name))
			{
				if (cmd->IsAlias())
				{
					if (aliases)
					{
						++count;
						static_cast<FConsoleAlias *>(cmd)->PrintAlias ();
					}
				}
				else if (!aliases)
				{
					++count;
					cmd->PrintCommand ();
				}
			}
			cmd = cmd->m_Next;
		}
	}
	return count;
}

void FConsoleAlias::PrintAlias ()
{
	if (m_Command[0])
	{
		Printf (TEXTCOLOR_YELLOW "%s : %s\n", m_Name, m_Command[0]);
	}
	if (m_Command[1])
	{
		Printf (TEXTCOLOR_ORANGE "%s : %s\n", m_Name, m_Command[1]);
	}
}

void FConsoleAlias::Archive (FConfigFile *f)
{
#if WORKINPROGRESS
	if (f != NULL && m_Command[0] != NULL)
	{
		f->SetValueForKey ("Name", m_Name, true);
		f->SetValueForKey ("Command", m_Command[0], true);
	}
#endif
}

void C_ArchiveAliases (FConfigFile *f)
{
	int bucket;
	FConsoleCommand *alias;

	for (bucket = 0; bucket < HASH_SIZE; bucket++)
	{
		alias = Commands[bucket];
		while (alias)
		{
			if (alias->IsAlias())
				static_cast<FConsoleAlias *>(alias)->Archive (f);
			alias = alias->m_Next;
		}
	}
}

// This is called only by the ini parser.
void C_SetAlias (const char *name, const char *cmd)
{
	FConsoleCommand *prev, *alias, **chain;

	chain = &Commands[MakeKey (name) % HASH_SIZE];
	alias = ScanChainForName (*chain, name, strlen (name), &prev);
	if (alias != NULL)
	{
		if (!alias->IsAlias ())
		{
			//Printf (PRINT_BOLD, "%s is a command and cannot be an alias.\n", name);
			return;
		}
		delete alias;
	}
	new FConsoleAlias (name, cmd, false);
}

CCMD (alias)
{
	FConsoleCommand *prev, *alias, **chain;

	if (argv.argc() == 1)
	{
		Printf ("Current alias commands:\n");
		DumpHash (Commands, true);
	}
	else
	{
		chain = &Commands[MakeKey (argv[1]) % HASH_SIZE];

		if (argv.argc() == 2)
		{ // Remove the alias

			if ( (alias = ScanChainForName (*chain, argv[1], strlen (argv[1]), &prev)))
			{
				if (alias->IsAlias ())
				{
					static_cast<FConsoleAlias *> (alias)->SafeDelete ();
				}
				else
				{
					Printf ("%s is a normal command\n", alias->m_Name);
				}
			}
		}
		else
		{ // Add/change the alias

			alias = ScanChainForName (*chain, argv[1], strlen (argv[1]), &prev);
			if (alias != NULL)
			{
				if (alias->IsAlias ())
				{
					static_cast<FConsoleAlias *> (alias)->Realias (argv[2], false);
				}
				else
				{
					Printf ("%s is a normal command\n", alias->m_Name);
					alias = NULL;
				}
			}
			else
			{
				alias = new FConsoleAlias (argv[1], argv[2], false);
			}
		}
	}
}

CCMD (cmdlist)
{
	int count;
	const char *filter = (argv.argc() == 1 ? NULL : argv[1]);

	count = ListActionCommands (filter);
	count += DumpHash (Commands, false, filter);
	Printf ("%d commands\n", count);
}

CCMD (key)
{
	if (argv.argc() > 1)
	{
		int i;

		for (i = 1; i < argv.argc(); ++i)
		{
			unsigned int key = MakeKey (argv[i]);
			Printf (" 0x%08x\n", key);
		}
	}
}

// Execute any console commands specified on the command line.
// These all begin with '+' as opposed to '-'.
void C_ExecCmdLineParams ()
{
	for (int currArg = 1; currArg < Args.NumArgs(); )
	{
		if (*Args.GetArg (currArg++) == '+')
		{
			char *cmdString;
			int cmdlen = 1;
			int argstart = currArg - 1;

			while (currArg < Args.NumArgs())
			{
				if (*Args.GetArg (currArg) == '-' || *Args.GetArg (currArg) == '+')
					break;
				currArg++;
				cmdlen++;
			}

			if ( (cmdString = BuildString (cmdlen, Args.GetArgList (argstart))) )
			{
				C_DoCommand (cmdString + 1);
				delete[] cmdString;
			}
		}
	}
}

bool FConsoleCommand::IsAlias ()
{
	return false;
}

bool FConsoleAlias::IsAlias ()
{
	return true;
}

void FConsoleAlias::Run (FCommandLine &args, int key)
{
	if (bRunning)
	{
		Printf ("Alias %s tried to recurse.\n", m_Name);
		return;
	}

	int index = m_Command[1] != NULL;
	char *mycommand = m_Command[index];
	m_Command[index] = NULL;
	bRunning = true;
	AddCommandString (mycommand, key);
	bRunning = false;
	if (m_Command[index] != NULL)
	{ // The alias realiased itself, so delete the memory used by this command.
		delete[] mycommand;
	}
	else
	{ // The alias is unchanged, so put the command back so it can be used again.
		m_Command[index] = mycommand;
	}
	if (bKill)
	{ // The alias wants to remove itself
		delete this;
	}
}

void FConsoleAlias::Realias (const char *command, bool noSave)
{
	if (m_Command[1] != NULL)
	{
		noSave = true;
	}
	if (m_Command[noSave] != NULL)
	{
		delete[] m_Command[noSave];
	}
	m_Command[noSave] = copystring (command);
	bKill = false;
}

void FConsoleAlias::SafeDelete ()
{
	if (!bRunning)
	{
		delete this;
	}
	else
	{
		bKill = true;
	}
}

int C_ExecFile (const char *file, bool usePullin)
{
	FILE *f;
	char cmd[4096];
	int retval = 0;

	if ( (f = fopen (file, "r")) )
	{
		while (fgets (cmd, 4095, f))
		{
			// Comments begin with //
			char *stop = cmd + strlen (cmd) - 1;
			char *comment = cmd;
			int inQuote = 0;

			if (*stop == '\n')
				*stop-- = 0;

			while (comment < stop)
			{
				if (*comment == '\"')
				{
					inQuote ^= 1;
				}
				else if (!inQuote && *comment == '/' && *(comment + 1) == '/')
				{
					break;
				}
				comment++;
			}
			if (comment == cmd)
			{ // Comment at line beginning
				continue;
			}
			else if (comment < stop)
			{ // Comment in middle of line
				*comment = 0;
			}

			AddCommandString (cmd);
		}
		if (!feof (f))
		{
			retval = 2;
		}
		fclose (f);
	}
	else
	{
		retval = 1;
	}
	return retval;
}
