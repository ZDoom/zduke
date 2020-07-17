/*
** c_bind.cpp
** Functions for using and maintaining key bindings
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

#include "doomtype.h"
#include "doomdef.h"
#include "cmdlib.h"
#include "c_dispatch.h"
#include "c_bind.h"
#include "d_event.h"
#include "d_gui.h"
#include "duke3d.h"
//#include "g_level.h"
//#include "hu_stuff.h"
//#include "gi.h"
//#include "configfile.h"

#include <math.h>
#include <stdlib.h>

struct FBinding
{
	const char *Key;
	const char *Bind;
};

/* Default keybindings for Doom (and all other games)
 */
static const FBinding DefBindings[] =
{
	{ "uparrow", "+forward" },	{ "kp8", "+forward" },		{ "mouse3", "+forward" },
	{ "downarrow", "+back" },	{ "kp2", "+back" },
	{ "leftarrow", "+left" },	{ "kp4", "+left" },
	{ "rightarrow", "+right" },	{ "kp6", "+right" },
	{ "alt", "+strafe" },		{ "mouse2", "+strafe" },	{ "joy2", "+strafe" },
	{ "ctrl", "+attack" },		{ "mouse1", "+attack" },	{ "joy1", "+attack" },
	{ "space", "+use" },		{ "joy4", "+use" },
	{ "shift", "+speed" },		{ "joy3", "+speed" },
	{ "capslock", "autorun" },
	{ "a", "+jump" },			{ "/", "+jump" },
	{ "z", "+crouch" },
	{ "pgup", "+lookup" },		{ "kp9", "+lookup" },
	{ "pgdn", "+lookdown" },	{ "kp3", "+lookdown" },
	{ "ins", "+lookleft" },		{ "kp0", "+lookleft" },		{ "joy8", "+lookleft" },
	{ "del", "+lookright" },	{ "kp.", "+lookright" },	{ "joy6", "+lookright" },
	{ ",", "+moveleft" },
	{ ".", "+moveright" },
	{ "home", "+aimup" },		{ "kp7", "+aimup" },		{ "joy5", "+aimdown" },
	{ "end", "+aimdown" },		{ "kp1", "+aimdown" },		{ "joy7", "+aimup" },
	{ "1", "slot 1" },
	{ "2", "slot 2" },
	{ "3", "slot 3" },
	{ "4", "slot 4" },
	{ "5", "slot 5" },
	{ "6", "slot 6" },
	{ "7", "slot 7" },
	{ "8", "slot 8" },
	{ "9", "slot 9" },
	{ "0", "slot 10" },
	{ "[", "invprev" },
	{ "]", "invnext" },
	{ "enter", "invuse" },		{ "kpenter", "invuse" },
	{ "-", "+sizedown" },
	{ "kp-", "+sizedown" },
	{ "=", "+sizeup" },
	{ "kp+", "+sizeup" },
	{ "h", "holoduke" },
	{ "j", "jetpack" },
	{ "n", "nightvision" },
	{ "m", "medkit" },
	{ "backspace", "turnaround" },
	{ "t", "messagemode" },
	{ "tab", "togglemap" },
	{ "kp5", "centerview" },
	{ "scroll", "holsterweapon" },
	{ "w", "showweapons" },
	{ "f", "mapfollow" },
	{ "k", "spynext" },
	{ "u", "+mlook" },
	{ "i", "togglecrosshair" },
	{ "r", "steroids" },
	{ "`", "+quickkick" },
	{ "'", "weapnext" },
	{ ";", "weapprev" },
	{ "pause", "pause" },
	{ "\\", "toggleconsole" },
	{ "f1", "cond $shiftdown \"taunttext 1\" $altdown \"tauntvoice 1\" else helpscreen" },
	{ "f2", "cond $shiftdown \"taunttext 2\" $altdown \"tauntvoice 2\" else menu_save" },
	{ "f3", "cond $shiftdown \"taunttext 3\" $altdown \"tauntvoice 3\" else menu_load" },
	{ "f4", "cond $shiftdown \"taunttext 4\" $altdown \"tauntvoice 4\" else menu_sound" },
	{ "f5", "cond && $shiftdown $musicselectoron musicselector_next $shiftdown \"taunttext 5\" $altdown \"tauntvoice 5\" else musicselector" },
	{ "f6", "cond $shiftdown \"taunttext 6\" $altdown \"tauntvoice 6\" else quicksave" },
	{ "f7", "cond $shiftdown \"taunttext 7\" $altdown \"tauntvoice 7\" else overshoulder" },
	{ "f8", "cond $shiftdown \"taunttext 8\" $altdown \"tauntvoice 8\" else togglemessages" },
	{ "f9", "cond $shiftdown \"taunttext 9\" $altdown \"tauntvoice 9\" else quickload" },
	{ "f10", "cond $shiftdown \"taunttext 10\" $altdown \"tauntvoice 10\" else menu_quit" },
	{ "f11", "bumpgamma" },
	{ "f12", "screenshot" },
	{ "sysrq", "screenshot" },
	{ "capslock", "toggleautorun" },
	/*
	{ "ctrl", "+attack" },
	{ "alt", "+strafe" },
	{ "shift", "+speed" },
	{ "space", "+use" },
	{ "rightarrow", "+right" },
	{ "leftarrow", "+left" },
	{ "uparrow", "+forward" },
	{ "downarrow", "+back" },
	{ ",", "+moveleft" },
	{ ".", "+moveright" },
	{ "mouse1", "+attack" },
	{ "mouse2", "+strafe" },
	{ "mouse3", "+forward" },
	{ "mouse4", "+speed" },
	{ "joy1", "+attack" },
	{ "joy2", "+strafe" },
	{ "joy3", "+speed" },
	{ "joy4", "+use" },
	{ "capslock", "toggle cl_run" },
	{ "f1", "menu_help" },
	{ "f2", "menu_save" },
	{ "f3", "menu_load" },
	{ "f4", "menu_options" },
	{ "f5", "menu_display" },
	{ "f6", "quicksave" },
	{ "f7", "menu_endgame" },
	{ "f8", "togglemessages" },
	{ "f9", "quickload" },
	{ "f11", "bumpgamma" },
	{ "f10", "menu_quit" },
	{ "tab", "togglemap" },
	{ "pause", "pause" },
	{ "sysrq", "screenshot" },
	{ "t", "messagemode" },
	{ "\\", "+showscores" },
	{ "f12", "spynext" },
	{ "mwheeldown", "weapnext" },
	{ "mwheelup", "weapprev" },
	{ "a", "jump" },*/
	{ NULL }
};

const char *KeyNames[NUM_KEYS] =
{
	// This array is dependant on the particular keyboard input
	// codes generated in i_input.c. If they change there, they
	// also need to change here. In this case, we use the
	// DirectInput codes and assume a qwerty keyboard layout.
	// See <dinput.h> for the DIK_* codes

	NULL,		"escape",	"1",		"2",		"3",		"4",		"5",		"6",		//00
	"7",		"8",		"9",		"0",		"-",		"=",		"backspace","tab",		//08
	"q",		"w",		"e",		"r",		"t",		"y",		"u",		"i",		//10
	"o",		"p",		"[",		"]",		"enter",	"ctrl",		"a",		"s",		//18
	"d",		"f",		"g",		"h",		"j",		"k",		"l",		";",		//20
	"'",		"`",		"shift",	"\\",		"z",		"x",		"c",		"v",		//28
	"b",		"n",		"m",		",",		".",		"/",		NULL,		"kp*",		//30
	"alt",		"space",	"capslock",	"f1",		"f2",		"f3",		"f4",		"f5",		//38
	"f6",		"f7",		"f8",		"f9",		"f10",		"numlock",	"scroll",	"kp7",		//40
	"kp8",		"kp9",		"kp-",		"kp4",		"kp5",		"kp6",		"kp+",		"kp1",		//48
	"kp2",		"kp3",		"kp0",		"kp.",		NULL,		NULL,		"oem102",	"f11",		//50
	"f12",		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		//58
	NULL,		NULL,		NULL,		NULL,		"f13",		"f14",		"f15",		NULL,		//60
	NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		//68
	"kana",		NULL,		NULL,		"abnt_c1",	NULL,		NULL,		NULL,		NULL,		//70
	NULL,		"convert",	NULL,		"noconvert",NULL,		"yen",		"abnt_c2",	NULL,		//78
	NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		//80
	NULL,		NULL,		NULL,		NULL,		NULL,		"kp=",		NULL,		NULL,		//88
	"circumflex","@",		":",		"_",		"kanji",	"stop",		"ax",		"unlabeled",//90
	NULL,		"nexttrack",NULL,		NULL,		"kpenter",	NULL,		NULL,		NULL,		//98
	"mute",		"calculator","play",	NULL,		"stop",		NULL,		NULL,		NULL,		//A0
	NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		"voldown",	NULL,		//A8
	"volup",	NULL,		"webhome",	"kp,",		NULL,		"kp/",		NULL,		"sysrq",	//B0
	NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		//B8
	NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		"home",		//C0
	"uparrow",	"pgup",		NULL,		"leftarrow",NULL,		"rightarrow",NULL,		"end",		//C8
	"downarrow","pgdn",		"ins",		"del",		NULL,		NULL,		NULL,		NULL,		//D0
	NULL,		NULL,		NULL,		"lwin",		"rwin",		"apps",		"power",	"sleep",	//D8
	NULL,		NULL,		NULL,		"wake",		NULL,		"search",	"favorites","refresh",	//E0
	"webstop",	"webforward","webback",	"mycomputer","mail",	"mediaselect",NULL,		NULL,		//E8
	NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		//F0
	NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		NULL,		"pause",	//F8

	// non-keyboard buttons that can be bound
	"mouse1",	"mouse2",	"mouse3",	"mouse4",		// 8 mouse buttons
	"mouse5",	"mouse6",	"mouse7",	"mouse8",

	"joy1",		"joy2",		"joy3",		"joy4",			// 128 joystick buttons!
	"joy5",		"joy6",		"joy7",		"joy8",
	"joy9",		"joy10",	"joy11",	"joy12",
	"joy13",	"joy14",	"joy15",	"joy16",
	"joy17",	"joy18",	"joy19",	"joy20",
	"joy21",	"joy22",	"joy23",	"joy24",
	"joy25",	"joy26",	"joy27",	"joy28",
	"joy29",	"joy30",	"joy31",	"joy32",
	"joy33",	"joy34",	"joy35",	"joy36",
	"joy37",	"joy38",	"joy39",	"joy40",
	"joy41",	"joy42",	"joy43",	"joy44",
	"joy45",	"joy46",	"joy47",	"joy48",
	"joy49",	"joy50",	"joy51",	"joy52",
	"joy53",	"joy54",	"joy55",	"joy56",
	"joy57",	"joy58",	"joy59",	"joy60",
	"joy61",	"joy62",	"joy63",	"joy64",
	"joy65",	"joy66",	"joy67",	"joy68",
	"joy69",	"joy70",	"joy71",	"joy72",
	"joy73",	"joy74",	"joy75",	"joy76",
	"joy77",	"joy78",	"joy79",	"joy80",
	"joy81",	"joy82",	"joy83",	"joy84",
	"joy85",	"joy86",	"joy87",	"joy88",
	"joy89",	"joy90",	"joy91",	"joy92",
	"joy93",	"joy94",	"joy95",	"joy96",
	"joy97",	"joy98",	"joy99",	"joy100",
	"joy101",	"joy102",	"joy103",	"joy104",
	"joy105",	"joy106",	"joy107",	"joy108",
	"joy109",	"joy110",	"joy111",	"joy112",
	"joy113",	"joy114",	"joy115",	"joy116",
	"joy117",	"joy118",	"joy119",	"joy120",
	"joy121",	"joy122",	"joy123",	"joy124",
	"joy125",	"joy126",	"joy127",	"joy128",

	"pov1up",	"pov1right","pov1down",	"pov1left",		// First POV hat
	"pov2up",	"pov2right","pov2down",	"pov2left",		// Second POV hat
	"pov3up",	"pov3right","pov3down",	"pov3left",		// Third POV hat
	"pov4up",	"pov4right","pov4down",	"pov4left",		// Fourth POV hat

	"mwheelup",	"mwheeldown",							// the mouse wheel
};

static char *Bindings[NUM_KEYS];
static BYTE SBindings[NUM_KEYS];

static char *DoubleBindings[NUM_KEYS];
static int DClickTime[NUM_KEYS];
static byte DClicked[(NUM_KEYS+7)/8];

static int GetKeyFromName (const char *name)
{
	int i;

	// Names of the form #xxx are translated to key xxx automatically
	if (name[0] == '#' && name[1] != 0)
	{
		return atoi (name + 1);
	}

	// Otherwise, we scan the KeyNames[] array for a matching name
	for (i = 0; i < NUM_KEYS; i++)
	{
		if (KeyNames[i] && !stricmp (KeyNames[i], name))
			return i;
	}
	return 0;
}

static const char *KeyName (int key)
{
	static char name[5];

	if (KeyNames[key])
		return KeyNames[key];

	sprintf (name, "#%d", key);
	return name;
}

void C_UnbindAll ()
{
	for (int i = 0; i < NUM_KEYS; ++i)
	{
		if (Bindings[i])
		{
			free (Bindings[i]);
			Bindings[i] = NULL;
		}
		if (DoubleBindings[i])
		{
			free (DoubleBindings[i]);
			DoubleBindings[i] = NULL;
		}
	}
}

CCMD (unbindall)
{
	C_UnbindAll ();
}

CCMD (unbind)
{
	int i;

	if (argv.argc() > 1)
	{
		if ( (i = GetKeyFromName (argv[1])) )
		{
			if (Bindings[i])
			{
				free (Bindings[i]);
				Bindings[i] = NULL;
			}
		}
		else
		{
			Printf ("Unknown key \"%s\"\n", argv[1]);
			return;
		}

	}
}

CCMD (bind)
{
	int i;

	if (argv.argc() > 1)
	{
		i = GetKeyFromName (argv[1]);
		if (!i)
		{
			Printf ("Unknown key \"%s\"\n", argv[1]);
			return;
		}
		if (argv.argc() == 2)
		{
			Printf ("\"%s\" = \"%s\"\n", argv[1], (Bindings[i] ? Bindings[i] : ""));
		}
		else
		{
			ReplaceString (&Bindings[i], argv[2]);
		}
	}
	else
	{
		Printf ("Current key bindings:\n");
		
		for (i = 0; i < NUM_KEYS; i++)
		{
			if (Bindings[i])
				Printf ("%s \"%s\"\n", KeyName (i), Bindings[i]);
		}
	}
}

//==========================================================================
//
// CCMD defaultbind
//
// Binds a command to a key if that key is not already bound and if
// that command is not already bound to another key.
//
//==========================================================================

CCMD (defaultbind)
{
	if (argv.argc() < 3)
	{
		Printf ("Usage: defaultbind <key> <command>\n");
	}
	else
	{
		int key = GetKeyFromName (argv[1]);
		if (key == 0)
		{
			Printf ("Unknown key \"%s\"\n", argv[1]);
			return;
		}
		if (Bindings[key] != NULL)
		{ // This key is already bound.
			return;
		}
		for (int i = 0; i < NUM_KEYS; ++i)
		{
			if (Bindings[i] != NULL && stricmp (Bindings[i], argv[2]) == 0)
			{ // This command is already bound to a key.
				return;
			}
		}
		// It is safe to do the bind, so do it.
		Bindings[key] = copystring (argv[2]);
	}
}

CCMD (undoublebind)
{
	int i;

	if (argv.argc() > 1)
	{
		if ( (i = GetKeyFromName (argv[1])) )
		{
			if (DoubleBindings[i])
			{
				delete[] DoubleBindings[i];
				DoubleBindings[i] = NULL;
			}
		}
		else
		{
			Printf ("Unknown key \"%s\"\n", argv[1]);
			return;
		}

	}
}

CCMD (doublebind)
{
	int i;

	if (argv.argc() > 1)
	{
		i = GetKeyFromName (argv[1]);
		if (!i)
		{
			Printf ("Unknown key \"%s\"\n", argv[1]);
			return;
		}
		if (argv.argc() == 2)
		{
			Printf ("\"%s\" = \"%s\"\n", argv[1], (DoubleBindings[i] ? DoubleBindings[i] : ""));
		}
		else
		{
			ReplaceString (&DoubleBindings[i], argv[2]);
		}
	}
	else
	{
		Printf ("Current key doublebindings:\n");
		
		for (i = 0; i < NUM_KEYS; i++)
		{
			if (DoubleBindings[i])
				Printf ("%s \"%s\"\n", KeyName (i), DoubleBindings[i]);
		}
	}
}

CCMD (rebind)
{
	char **bindings;

	if (key == 0)
	{
		Printf ("Rebind cannot be used from the console\n");
		return;
	}

	if (key & KEY_DBLCLICKED)
	{
		bindings = DoubleBindings;
		key &= 0x0FFF;
	}
	else
	{
		bindings = Bindings;
	}

	if (argv.argc() > 1)
	{
		ReplaceString (&bindings[key], argv[1]);
	}
}

static void SetBinds (const FBinding *array)
{
	while (array->Key)
	{
		C_DoBind (array->Key, array->Bind, false);
		array++;
	}
}

void C_BindDefaults ()
{
	SetBinds (DefBindings);
	//C_DoBind ("mouse2", "+use", true);
	C_DoBind ("joy2", "invuse", true);
	C_DoBind ("joy3", "+jump", true);
	C_DoBind ("joy4", "+crouch", true);
}

CCMD(binddefaults)
{
	C_BindDefaults ();
}

void C_SetDefaultBindings ()
{
	C_UnbindAll ();
	C_BindDefaults ();
}

bool C_DoKey (const event_t *ev)
{
	char *binding = NULL;
	bool dclick;
	int dclickspot;
	byte dclickmask;

	if (ev->type != EV_KeyDown && ev->type != EV_KeyUp)
		return false;

	dclickspot = ev->data1 >> 3;
	dclickmask = 1 << (ev->data1 & 7);
	dclick = false;

	if (DClickTime[ev->data1] > totalclock && ev->type == EV_KeyDown)
	{
		// Key pressed for a double click
		binding = DoubleBindings[ev->data1];
		DClicked[dclickspot] |= dclickmask;
		dclick = true;
	}
	else
	{
		if (ev->type == EV_KeyDown)
		{ // Key pressed for a normal press
			binding = Bindings[ev->data1];
			DClickTime[ev->data1] = totalclock + 68;
		}
		else if (DClicked[dclickspot] & dclickmask)
		{ // Key released from a double click
			binding = DoubleBindings[ev->data1];
			DClicked[dclickspot] &= ~dclickmask;
			DClickTime[ev->data1] = 0;
			dclick = true;
		}
		else
		{ // Key released from a normal press
			binding = Bindings[ev->data1];
		}
	}

	if (binding == NULL)
	{
		binding = Bindings[ev->data1];
		dclick = false;
	}

	if (binding != NULL && (!(ps[myconnectindex].gm & MODE_TYPE) || ev->data1 < 256))
	{
		if (ev->type == EV_KeyUp)
		{
			if (binding[0] != '+')
			{
				return false;
			}
			binding[0] = '-';
		}

		int keyval = ev->data1;

		if (dclick)					keyval |= KEY_DBLCLICKED;
		if (ev->data3 & GKM_SHIFT)	keyval |= 0x1000;
		if (ev->data3 & GKM_ALT)	keyval |= 0x2000;
		if (ev->data3 & GKM_CTRL)	keyval |= 0x4000;

		AddCommandString (binding, keyval);

		if (ev->type == EV_KeyUp)
		{
			binding[0] = '+';
		}
		return true;
	}
	return false;
}

#if 0 //WORKINPROGRESS
void C_ArchiveBindings (FConfigFile *f, bool dodouble, const char *matchcmd)
{
	char **bindings;
	const char *name;
	int i;

	bindings = dodouble ? DoubleBindings : Bindings;

	for (i = 0; i < NUM_KEYS; i++)
	{
		if (bindings[i] && (matchcmd==NULL || stricmp(bindings[i], matchcmd)==0))
		{
			name = KeyName (i);
			if (name[1] == 0)	// Make sure given name is config-safe
			{
				if (name[0] == '[')
					name = "LeftBracket";
				else if (name[0] == ']')
					name = "RightBracket";
				else if (name[0] == '=')
					name = "Equals";
				else if (strcmp (name, "kp=") == 0)
					name = "KP-Equals";
			}
			f->SetValueForKey (name, bindings[i]);
			if (matchcmd != NULL)
			{ // If saving a specific command, remove the old binding
			  // so it does not get saved in the general binding list
				delete[] Bindings[i];
				Bindings[i] = NULL;
			}
		}
	}
}
#endif

void C_DoBind (const char *key, const char *bind, bool dodouble)
{
	int keynum = GetKeyFromName (key);
	if (keynum == 0)
	{
		if (stricmp (key, "LeftBracket") == 0)
		{
			keynum = GetKeyFromName ("[");
		}
		else if (stricmp (key, "RightBracket") == 0)
		{
			keynum = GetKeyFromName ("]");
		}
		else if (stricmp (key, "Equals") == 0)
		{
			keynum = GetKeyFromName ("=");
		}
		else if (stricmp (key, "KP-Equals") == 0)
		{
			keynum = GetKeyFromName ("kp=");
		}
	}
	if (keynum != 0)
	{
		ReplaceString ((dodouble ? DoubleBindings : Bindings) + keynum, bind);
		if (!dodouble)
		{
			if (stricmp (bind, "toggleconsole") == 0)
			{
				SBindings[keynum] = SBIND_ToggleConsole;
			}
			else if (stricmp (bind, "screenshot") == 0)
			{
				SBindings[keynum] = SBIND_Screenshot;
			}
			else
			{
				SBindings[keynum] = SBIND_None;
			}
		}
	}
}

int C_GetKeysForCommand (char *cmd, int *first, int *second)
{
	int c, i;

	*first = *second = c = i = 0;

	while (i < NUM_KEYS && c < 2)
	{
		if (Bindings[i] && stricmp (cmd, Bindings[i]) == 0)
		{
			if (c++ == 0)
				*first = i;
			else
				*second = i;
		}
		i++;
	}
	return c;
}

void C_NameKeys (char *str, int first, int second)
{
	int c = 0;

	*str = 0;
	if (first)
	{
		c++;
		strcpy (str, KeyName (first));
		if (second)
			strcat (str, " or ");
	}

	if (second)
	{
		c++;
		strcat (str, KeyName (second));
	}

	if (!c)
		strcpy (str, "???");
}

void C_UnbindACommand (char *str)
{
	int i;

	for (i = 0; i < NUM_KEYS; i++)
	{
		if (Bindings[i] && !stricmp (str, Bindings[i]))
		{
			delete[] Bindings[i];
			Bindings[i] = NULL;
		}
	}
}

void C_ChangeBinding (const char *str, int newone)
{
	if (Bindings[newone])
		delete[] Bindings[newone];

	Bindings[newone] = copystring (str);
}

char *C_GetBinding (int key)
{
	return Bindings[key];
}

ESpecialBinding C_GetSpecialBinding (int key)
{
	return (ESpecialBinding)SBindings[key];
}
