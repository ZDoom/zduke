/*
** c_console.cpp
** Implements the console itself
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

#include "duke3d.h"
#include "m_alloc.h"
#include "templates.h"
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#include "version.h"
//#include "g_game.h"
#include "d_event.h"
#include "d_gui.h"
#include "c_console.h"
#include "c_cvars.h"
#include "c_dispatch.h"
#include "c_bind.h"
//#include "hu_stuff.h"
#include "i_system.h"
#include "i_video.h"
#include "i_input.h"
//#include "m_swap.h"
#include "v_palette.h"
#include "v_video.h"
#include "cmdlib.h"
#include "build.h"
//#include "v_text.h"
//#include "w_wad.h"
//#include "r_main.h"
//#include "r_draw.h"
//#include "sbar.h"
//#include "s_sound.h"
//#include "s_sndseq.h"
//#include "doomstat.h"

// WORKINPROGRESS
#define CR_GOLD 0
#define CR_GRAY 0
#define CR_GREEN 0
#define CR_RED 0
#define CR_TAN 0
#define CR_UNTRANSLATED 0
#define NUM_TEXT_COLORS 1
static int gamestate = 2;
#define GS_STARTUP 0
#define GS_FULLCONSOLE 1
#define GS_LEVEL 2
gameaction_t gameaction;
// WORKINPROGRESS

extern volatile long totalclock;

#define CONSOLESIZE	16384	// Number of characters to store in console
#define CONSOLELINES 256	// Max number of lines of console text
#define LINEMASK (CONSOLELINES-1)

#define LEFTMARGIN 8
#define RIGHTMARGIN 8
#define BOTTOMARGIN 12

static void C_TabComplete (bool goForward);
static BOOL TabbedLast;		// Last key pressed was tab


extern bool		automapactive;	// in AM_map.c

int			ConCols, PhysRows;
BOOL		vidactive = false;
BOOL		cursoron = false;
int			ConBottom, ConScroll, RowAdjust;
int			CursorTicker;
constate_e	ConsoleState = c_up;
char		VersionString[32];

static char ConsoleBuffer[CONSOLESIZE];
static char *Lines[CONSOLELINES];
static bool LineJoins[CONSOLELINES];

static int TopLine, InsertLine;
static char *BufferRover = ConsoleBuffer;

static void ClearConsole ();

CVAR (Bool, developer, false, 0)

#define SCROLLUP 1
#define SCROLLDN 2
#define SCROLLNO 0

EXTERN_CVAR (Bool, show_messages)

static unsigned int TickerAt, TickerMax;
static const char *TickerLabel;

static bool TickerVisible;

struct History
{
	struct History *Older;
	struct History *Newer;
	char String[1];
};

// CmdLine[0]  = # of chars on command line
// CmdLine[1]  = cursor position
// CmdLine[2+] = command line (max 255 chars + NULL)
// CmdLine[259]= offset from beginning of cmdline to display
static byte CmdLine[260];

#define MAXHISTSIZE 50
static struct History *HistHead = NULL, *HistTail = NULL, *HistPos = NULL;
static int HistSize;

#define NUMNOTIFIES 4

CVAR (Float, con_notifytime, 3.34f, CVAR_ARCHIVE)
CVAR (Bool, con_centernotify, true, CVAR_ARCHIVE)
CVAR (Bool, con_scaletext, false, CVAR_ARCHIVE)		// Scale notify text at high resolutions?

// Command to run when Ctrl-D is pressed at start of line
CVAR (String, con_ctrl_d, "", CVAR_ARCHIVE|CVAR_GLOBALCONFIG)

static struct NotifyText
{
	long timeout;
	short printlevel, quote;
	byte text[256];
} NotifyStrings[NUMNOTIFIES];

static int NotifyTop, NotifyTopGoal;

#define PRINTLEVELS 5
int PrintColors[PRINTLEVELS+2] = { CR_RED, CR_GOLD, CR_GRAY, CR_GREEN, CR_GREEN, CR_GOLD };

static void setmsgcolor (int index, int color);

FILE *Logfile = NULL;


FIntCVar msglevel ("msg", 0, CVAR_ARCHIVE);

CUSTOM_CVAR (Int, msg0color, 6, CVAR_ARCHIVE)
{
	setmsgcolor (0, self);
}

CUSTOM_CVAR (Int, msg1color, 5, CVAR_ARCHIVE)
{
	setmsgcolor (1, self);
}

CUSTOM_CVAR (Int, msg2color, 2, CVAR_ARCHIVE)
{
	setmsgcolor (2, self);
}

CUSTOM_CVAR (Int, msg3color, 3, CVAR_ARCHIVE)
{
	setmsgcolor (3, self);
}

CUSTOM_CVAR (Int, msg4color, 3, CVAR_ARCHIVE)
{
	setmsgcolor (4, self);
}

CUSTOM_CVAR (Int, msgmidcolor, 5, CVAR_ARCHIVE)
{
	setmsgcolor (PRINTLEVELS, self);
}

CUSTOM_CVAR (Int, msgmidcolor2, 4, CVAR_ARCHIVE)
{
	setmsgcolor (PRINTLEVELS+1, self);
}

static void maybedrawnow (bool tick, bool force)
{
	if (vidactive &&
		(((tick || gameaction != ga_nothing) && ConsoleState == c_down)
		|| gamestate == GS_STARTUP))
	{
		static long lastprinttime = 0;
		long nowtime = totalclock;

		if (nowtime - lastprinttime > 4 || force)
		{
			screen->Lock (false);
			C_DrawConsole ();
			screen->Update ();
			lastprinttime = nowtime;
		}
	}
}

void C_InitConsole (int width, int height, BOOL ingame)
{
	sprintf (VersionString, "v%s", VERSION);
	ConCols = (width - LEFTMARGIN - RIGHTMARGIN) / 8;
	PhysRows = height / 8;

	// If there is some text in the console buffer, reformat it
	// for the new resolution.
	if (TopLine != InsertLine)
	{
		// Note: Don't use new here, because we attach a handler to new in
		// i_main.cpp that calls I_FatalError if the allocation fails,
		// but we can gracefully handle such a condition here by just
		// clearing the console buffer.

		char *fmtBuff = (char *)malloc (CONSOLESIZE);
		char **fmtLines = (char **)malloc (CONSOLELINES*sizeof(char*)*4);
		int out = 0;

		if (fmtBuff && fmtLines)
		{
			int in;
			char *fmtpos;
			bool newline = true;

			fmtpos = fmtBuff;
			memset (fmtBuff, 0, CONSOLESIZE);

			for (in = TopLine; in != InsertLine; in = (in + 1) & LINEMASK)
			{
				size_t len = strlen (Lines[in]);

				if (fmtpos + len + 2 - fmtBuff > CONSOLESIZE)
				{
					break;
				}
				if (newline)
				{
					newline = false;
					fmtLines[out++] = fmtpos;
				}
				strcpy (fmtpos, Lines[in]);
				fmtpos += len;
				if (!LineJoins[in])
				{
					*fmtpos++ = '\n';
					fmtpos++;
					if (out == CONSOLELINES*4)
					{
						break;
					}
					newline = true;
				}
			}
		}

		ClearConsole ();

		if (fmtBuff && fmtLines)
		{
			int i;

			for (i = 0; i < out; i++)
			{
				AddToConsole (-1, fmtLines[i], false);
			}
		}

		if (fmtBuff)
			free (fmtBuff);
		if (fmtLines)
			free (fmtLines);
	}

	if (ingame && gamestate == GS_STARTUP)
	{
		C_FullConsole ();
	}
	vidactive = ingame;
}

static void ClearConsole ()
{
	RowAdjust = 0;
	TopLine = InsertLine = 0;
	BufferRover = ConsoleBuffer;
	memset (ConsoleBuffer, 0, CONSOLESIZE);
	memset (Lines, 0, sizeof(Lines));
	memset (LineJoins, 0, sizeof(LineJoins));
}

static void setmsgcolor (int index, int color)
{
	if ((unsigned)color >= (unsigned)NUM_TEXT_COLORS)
		color = 0;
	PrintColors[index] = color;
}

extern int DisplayWidth;

void ResetNotifyTopGoal ()
{
	if (ud.coop != 1 && ud.screen_size > 0 && ud.multimode > 1)
	{ // In dukematch, quotes need to be bumped down to make room for the frag bar.
		int j = 0; NotifyTopGoal = 8;
		for(int i=connecthead;i>=0;i=connectpoint2[i])
			if (i > j) j = i;

		if (j >= 4 && j <= 8) NotifyTopGoal += 8;
		else if (j > 8 && j <= 12) NotifyTopGoal += 16;
		else if (j > 12) NotifyTopGoal += 24;
	}
	else NotifyTopGoal = 0;
}

void C_AddNotifyString (int printlevel, const char *source, bool lineBreak)
{
	static enum
	{
		NEWLINE,
		APPENDLINE,
		REPLACELINE
	} addtype = NEWLINE;

	char *work;
	brokenlines_t *lines;
	int i, len, width;

	if (lineBreak)
		addtype = NEWLINE;

	if ((printlevel != 128 && !ud.fta_on) ||
		!(len = (int)strlen (source)) ||
		!(ps[myconnectindex].gm & MODE_GAME))
		return;

	//width = con_scaletext ? DisplayWidth / CleanXfac : DisplayWidth;
	width = 320;

	if (addtype == APPENDLINE && NotifyStrings[NUMNOTIFIES-1].printlevel == printlevel
		&& (work = (char *)malloc (strlen ((char *)NotifyStrings[NUMNOTIFIES-1].text)
									+ strlen (source) + 1)) )
	{
		sprintf (work, "%s%s", NotifyStrings[NUMNOTIFIES-1].text, source);
		lines = V_BreakLines (width, work);
		free (work);
	}
	else
	{
		lines = V_BreakLines (width, source);
		if (addtype == APPENDLINE)
		{
			addtype = NEWLINE;
		}
	}

	if (lines == NULL)
		return;

	for (i = 0; lines[i].width != -1; i++)
	{
		if (addtype == NEWLINE)
			memmove (&NotifyStrings[0], &NotifyStrings[1], sizeof(struct NotifyText) * (NUMNOTIFIES-1));
		strcpy ((char *)NotifyStrings[NUMNOTIFIES-1].text, lines[i].string);
		NotifyStrings[NUMNOTIFIES-1].timeout = totalclock + (long)(con_notifytime * TICRATE);
		NotifyStrings[NUMNOTIFIES-1].printlevel = printlevel;
		NotifyStrings[NUMNOTIFIES-1].quote = 0;
		addtype = NEWLINE;
	}

	V_FreeBrokenLines (lines);

	switch (source[len-1])
	{
	case '\r':	addtype = REPLACELINE;	break;
	case '\n':	addtype = NEWLINE;		break;
	default:	addtype = APPENDLINE;	break;
	}

	if (lineBreak)
		addtype = NEWLINE;

	ResetNotifyTopGoal ();
}

void C_AddQuote (int q, bool noDups)
{
	if ((noDups || q == 13) &&
		NotifyStrings[NUMNOTIFIES-1].timeout != 0 &&
		NotifyStrings[NUMNOTIFIES-1].quote == q)
	{
		NotifyStrings[NUMNOTIFIES-1].timeout = totalclock + (long)(con_notifytime * TICRATE);
		return;
	}

	AddToConsole (PRINT_MEDIUM, fta_quotes[q], true);
	C_AddNotifyString (PRINT_MEDIUM, fta_quotes[q], true);
	NotifyStrings[NUMNOTIFIES-1].quote = q;
}

bool QuoteVisible (int q)
{
	for (int i = 0; i < NUMNOTIFIES; ++i)
	{
		if (NotifyStrings[i].timeout != 0 &&
			NotifyStrings[i].quote == q)
		{
			return true;
		}
	}
	return false;
}

static int FlushLines (const char *start, const char *stop)
{
	int i;

	for (i = TopLine; i != InsertLine; i = (i + 1) & LINEMASK)
	{
		if (Lines[i] < stop && Lines[i] + strlen (Lines[i]) > start)
		{
			Lines[i] = NULL;
		}
		else
		{
			break;
		}
	}
	if (i != TopLine)
		i = i;
	return i;
}

static void AddLine (const char *text, bool more, int len)
{
	if (BufferRover + len + 1 - ConsoleBuffer > CONSOLESIZE)
	{
		TopLine = FlushLines (BufferRover, ConsoleBuffer + CONSOLESIZE);
		BufferRover = ConsoleBuffer;
	}
	TopLine = FlushLines (BufferRover, BufferRover + len + 1);
	memcpy (BufferRover, text, len);
	BufferRover[len] = 0;
	Lines[InsertLine] = BufferRover;
	BufferRover += len + 1;
	LineJoins[InsertLine] = more;
	InsertLine = (InsertLine + 1) & LINEMASK;
	if (InsertLine == TopLine)
	{
		TopLine = (TopLine + 1) & LINEMASK;
	}
}

void AddToConsole (int printlevel, const char *text, bool lineBreak)
{
	static enum
	{
		NEWLINE,
		APPENDLINE,
		REPLACELINE
	} addtype = NEWLINE;
	static char *work = NULL;
	static int worklen = 0;

	char *work_p;
	char *linestart;
	int cc = CR_TAN;
	int size, len;


	if (lineBreak)
		addtype = NEWLINE;

	len = (int)strlen (text);
	size = len + 3;

	if (addtype != NEWLINE)
	{
		InsertLine = (InsertLine - 1) & LINEMASK;
		if (Lines[InsertLine] == NULL)
		{
			InsertLine = (InsertLine + 1) & LINEMASK;
			addtype = NEWLINE;
		}
		else
		{
			BufferRover = Lines[InsertLine];
		}
	}
	if (addtype == APPENDLINE)
	{
		size += (int)strlen (Lines[InsertLine]);
	}
	if (size > worklen)
	{
		work = (char *)Realloc (work, size);
		worklen = size;
	}
	if (work == NULL)
	{
		work = TEXTCOLOR_RED "*** OUT OF MEMORY ***";
		worklen = 0;
	}
	else
	{
		if (addtype == APPENDLINE)
		{
			strcpy (work, Lines[InsertLine]);
			strcat (work, text);
		}
		else if (printlevel >= 0)
		{
			work[0] = TEXTCOLOR_ESCAPE;
			work[1] = 'A' + (printlevel == PRINT_HIGH ? CR_TAN :
						printlevel == 200 ? CR_GREEN :
						printlevel < PRINTLEVELS ? PrintColors[printlevel] :
						CR_TAN);
			cc = work[1];
			strcpy (work + 2, text);
		}
		else
		{
			strcpy (work, text);
		}
	}

	work_p = linestart = work;

	if (screen != NULL)
	{
		int x = 0;
		int maxwidth = screen->GetWidth() - LEFTMARGIN - RIGHTMARGIN;

		while (*work_p)
		{
			if (*work_p == TEXTCOLOR_ESCAPE)
			{
				work_p++;
				if (*work_p)
					cc = *work_p++;
				continue;
			}
			int w = gamecharwidth (*work_p);
			if (*work_p == '\n' || x + w > maxwidth)
			{
				AddLine (linestart, *work_p != '\n', work_p - linestart);
				if (*work_p == '\n')
				{
					x = 0;
					work_p++;
				}
				else
				{
					x = w;
				}
				if (*work_p)
				{
					linestart = work_p - 2;
					linestart[0] = TEXTCOLOR_ESCAPE;
					linestart[1] = cc;
				}
				else
				{
					linestart = work_p;
				}
			}
			else
			{
				x += w;
				work_p++;
			}
		}

		if (*linestart)
		{
			AddLine (linestart, !lineBreak, work_p - linestart);
		}
	}
	else
	{
		while (*work_p)
		{
			if (*work_p++ == '\n')
			{
				AddLine (linestart, false, work_p - linestart - 1);
				linestart = work_p;
			}
		}
		if (*linestart)
		{
			AddLine (linestart, true, work_p - linestart);
		}
	}

	switch (text[len-1])
	{
	case '\r':	addtype = REPLACELINE;	break;
	case '\n':	addtype = NEWLINE;		break;
	default:	addtype = APPENDLINE;	break;
	}

	if (lineBreak)
	{
		addtype = NEWLINE;
	}
}

/* Adds a string to the console and also to the notify buffer */
int PrintString (int printlevel, const char *outline)
{
	if (printlevel < msglevel || *outline == '\0')
	{
		return 0;
	}

	if (Logfile)
	{
		fputs (outline, Logfile);
//#ifdef _DEBUG
		fflush (Logfile);
//#endif
	}

	I_PrintStr (outline, false);

	AddToConsole (printlevel, outline, false);
	if (vidactive && screen)
	{
		C_AddNotifyString (printlevel, outline, false);
		maybedrawnow (false, false);
	}
	return (int)strlen (outline);
}

extern BOOL gameisdead;

int VPrintf (int printlevel, const char *format, va_list parms)
{
	char outline[8192];

	if (gameisdead)
		return 0;

	vsprintf (outline, format, parms);
	return PrintString (printlevel, outline);
}

int STACK_ARGS Printf (int printlevel, const char *format, ...)
{
	va_list argptr;
	int count;

	va_start (argptr, format);
	count = VPrintf (printlevel, format, argptr);
	va_end (argptr);

	return count;
}

int STACK_ARGS Printf (const char *format, ...)
{
	va_list argptr;
	int count;

	va_start (argptr, format);
	count = VPrintf (PRINT_HIGH, format, argptr);
	va_end (argptr);

	return count;
}

int STACK_ARGS DPrintf (const char *format, ...)
{
	va_list argptr;
	int count;

	if (developer)
	{
		va_start (argptr, format);
		count = VPrintf (PRINT_HIGH, format, argptr);
		va_end (argptr);
		return count;
	}
	else
	{
		return 0;
	}
}

void C_FlushDisplay ()
{
	int i;

	for (i = 0; i < NUMNOTIFIES; i++)
		NotifyStrings[i].timeout = 0;
}

void C_AdjustBottom ()
{
	if (gamestate == GS_FULLCONSOLE || gamestate == GS_STARTUP)
		ConBottom = SCREENHEIGHT;
	else if (ConBottom > SCREENHEIGHT / 2 || ConsoleState == c_down)
		ConBottom = SCREENHEIGHT / 2;
}

void C_NewModeAdjust ()
{
	C_InitConsole (SCREENWIDTH, SCREENHEIGHT, true);
	C_FlushDisplay ();
	C_AdjustBottom ();
}

void C_Ticker ()
{
	static long lasttic = 0;

	if (lasttic == 0)
		lasttic = totalclock - 1;

	if (ConsoleState != c_up)
	{
		if (ConsoleState == c_falling)
		{
			ConBottom += (totalclock - lasttic) * (SCREENHEIGHT*2/25);
			if (ConBottom >= SCREENHEIGHT / 2)
			{
				ConBottom = SCREENHEIGHT / 2;
				ConsoleState = c_down;
			}
		}
		else if (ConsoleState == c_rising)
		{
			ConBottom -= (totalclock - lasttic) * (SCREENHEIGHT*2/25);
			if (ConBottom <= 0)
			{
				ConsoleState = c_up;
				ConBottom = 0;
			}
		}
	}

	if (--CursorTicker <= 0)
	{
		cursoron ^= 1;
		CursorTicker = C_BLINKRATE;
	}

	lasttic = totalclock;

	NotifyTop += ksgn (NotifyTopGoal - NotifyTop);
}

static void C_DrawNotifyText ()
{
	bool center = (con_centernotify != 0.f);
	int i, line, color, j, skip;
	bool canskip;
	
	if (!(ps[myconnectindex].gm & MODE_GAME) || (ps[myconnectindex].gm & MODE_MENU))
		return;

	line = NotifyTop;
	skip = 0;
	canskip = true;

	for (i = 0; i < NUMNOTIFIES; i++)
	{
		if (NotifyStrings[i].timeout == 0)
			continue;

		j = (NotifyStrings[i].timeout - totalclock) / TICSPERFRAME;
		if (j > 0)
		{
			if (!ud.fta_on && NotifyStrings[i].printlevel != 128)
				continue;

			short bits = 2+8+16;

			if (j <= 2)
			{
				bits += 1+32;
			}
			else if (j <= 4)
			{
				bits += 1;
			}

			if (NotifyStrings[i].printlevel >= PRINTLEVELS)
				color = 0;//CR_UNTRANSLATED;
			else
				color = PrintColors[NotifyStrings[i].printlevel];

			/*
			if (con_scaletext)
			{
				if (!center)
					screen->DrawTextClean (color, 0, line, NotifyStrings[i].text);
				else
					screen->DrawTextClean (color, (SCREENWIDTH -
						screen->StringWidth (NotifyStrings[i].text)*CleanXfac)/2,
						line, NotifyStrings[i].text);
				line += 8 * CleanYfac;
			}
			else
			*/
			{
				gametext (center ? 160 : 0, line, (char *)NotifyStrings[i].text, color, bits);
				line += 8;
			}
			canskip = false;
		}
		else
		{
			if (canskip)
			{
				NotifyTop += 8;
				line += 8;
				skip++;
			}
			NotifyStrings[i].timeout = 0;
		}
	}
	if (canskip)
	{
		NotifyTop = NotifyTopGoal;
	}
	else
	{
		pus = pub = NUMPAGES;
	}
}

void C_InitTicker (const char *label, unsigned int max)
{
	TickerMax = max;
	TickerLabel = label;
	TickerAt = 0;
	maybedrawnow (true, false);
}

void C_SetTicker (unsigned int at, bool forceUpdate)
{
	TickerAt = at > TickerMax ? TickerMax : at;
	maybedrawnow (true, TickerVisible ? forceUpdate : false);
}

CVAR (Int, con_shade, 4, 0)
CVAR (Int, con_pal, 0, 0)

int consoletext(int x,int y,const char *t,char s,char p)
{
	short ac,newx;
	const char *oldt;
	long zoom = (320<<16) / xdim;
	long xmul = (320<<16) / xdim;
	long ymul = (200<<16) / ydim;

	newx = 0;
	oldt = t;

	while(*t)
	{
		if(*t == TEXTCOLOR_ESCAPE)
		{
			t++;
			if (*t == 0)
				return x;
			else
				p = p;//clamp (*t-'A',0,255);
			t++;
			continue;
		}
		else if (*t == '\t')
		{
			x = (x+47)/48*48;
			t++;
			continue;
		}
		else if(*t == 32) {x+=5;t++;continue;}
		else ac = *t - '!' + STARTALPHANUM;

		if(ac < STARTALPHANUM || ac > ENDALPHANUM) {x+=5;t++;continue;}

		rotatesprite(x<<16,y<<16,65536,0,ac,s,p,8+16,0,0,xdim-1,ydim-1);
		if(*t >= '0' && *t <= '9')
			x += 8;
		else x += tilesizx[ac];

		t++;
	}

	return (x);
}

int gametextwidth (const char *t, int max)
{
	const char *str = t;
	int x;
	short ac;

	for (x = 0; (max < 0 || (t-str) < max) && *t; ++t)
	{
		if(*t == TEXTCOLOR_ESCAPE)
		{
			t++;
			if (*t == 0)
				return x;
		}
		else
		{
			if(*t == 32) {x+=5;continue;}
			else ac = *t - '!' + STARTALPHANUM;

			if(ac < STARTALPHANUM || ac > ENDALPHANUM) {x+=5;continue;}

			if(*t >= '0' && *t <= '9')
				x += 8;
			else
				x += tilesizx[ac];
		}
	}
	return x;
}

int gamecharwidth (int ch)
{
	if (ch == ' ') return 5;
	if (ch >= 0 && ch <= '9') return 8;
	if (ch < '!' || ch > '!' + ENDALPHANUM - STARTALPHANUM) return 5;
	return tilesizx[ch - '!' + STARTALPHANUM];
}

void C_DrawConsole ()
{
	const int fontHeight = tilesizy[STARTALPHANUM] + 1;
	static int oldbottom = 0;
	int lines, left, offset;

	left = 8;
	lines = (ConBottom-16)/fontHeight;
	if (-8 + lines*8 > ConBottom - 28)
	{
		offset = -4;
		lines--;
	}
	else
	{
		offset = -8;
	}

	if (ConBottom < oldbottom)
	{
		pub = NUMPAGES;
		pus = NUMPAGES;
	}

	oldbottom = ConBottom;

 	if (ConsoleState == c_up)
	{
		C_DrawNotifyText ();
		return;
	}
	else if (ConBottom)
	{
		int visheight, realheight;

		visheight = ConBottom;
		realheight = (visheight * 200) / ydim;

//		conback->Blit (0, conback->GetHeight() - realheight, conback->GetWidth(), realheight,
//			screen, 0, 0, SCREENWIDTH, visheight);
        rotatesprite(320<<15,(ConBottom-ydim/2)<<15,65536L,0,LOADSCREEN,
			con_shade,con_pal,
			2+8+64,0,0,xdim-1,ydim-1);

		if (ConBottom >= 12)
		{
			minitext (xdim-strlen(VersionString)*4, (ConBottom-6), VersionString, 2, 8+16+128);
		}
		if (lines > 0)
		{
			int bottomline = ConBottom - fontHeight*2 - 4;
			int pos = (InsertLine - 1) & LINEMASK;
			int i;

			for (i = RowAdjust; i; i--)
			{
				if (pos == TopLine)
				{
					RowAdjust = RowAdjust - i;
					break;
				}
				else
				{
					pos = (pos - 1) & LINEMASK;
				}
			}
			pos++;
			int pal = 18;
			do
			{
				pos = (pos - 1) & LINEMASK;
				if (Lines[pos] != NULL)
				{
					consoletext (8, offset + lines * fontHeight, Lines[pos], 0, pal);
				}
				lines--;
			} while (pos != TopLine && lines > 0);

			if (ConBottom >= 20)
			{
				if (RowAdjust && ConBottom >= 28)
				{
					// Indicate that the view has been scrolled up
					// and if we can scroll no further
					consoletext (0, bottomline+8, "^", 0, 6);
					if (pos == TopLine)
					{
						consoletext (0, bottomline, "*", 0, 6);
					}
				}
				CmdLine[2+CmdLine[0]] = 0;
				left = consoletext (left, bottomline, "]", 0, 2);
				consoletext (left, bottomline, (char *)&CmdLine[2+CmdLine[259]], 0, 2);
				if (cursoron)
				{
					left += gametextwidth ((char *)&CmdLine[2+CmdLine[259]],
						CmdLine[1] - CmdLine[259]);
					consoletext (left, bottomline+2, "_", 0, 2);
				}
			}
		}
	}
}

void C_FullConsole ()
{
#if WORKINPROGRESS
	if (demoplayback)
		G_CheckDemoStatus ();
	advancedemo = false;
#endif
	ConsoleState = c_down;
	HistPos = NULL;
	TabbedLast = false;
	if (gamestate != GS_STARTUP)
	{
#if WORKINPROGRESS
		gamestate = GS_FULLCONSOLE;
		level.music = NULL;
		S_Start ();
		SN_StopAllSequences ();
		V_SetBlend (0,0,0,0);
#endif
	}
	else
	{
		C_AdjustBottom ();
	}
}

void C_ToggleConsole ()
{
	/* WORKINPROGRESS
	if (gamestate == GS_DEMOSCREEN || demoplayback)
	{
		gameaction = ga_fullconsole;
	}
	else */
		if (!(ps[myconnectindex].gm & MODE_TYPE) && (ConsoleState == c_up || ConsoleState == c_rising))
	{
		ConsoleState = c_falling;
		HistPos = NULL;
		TabbedLast = false;
		if (ud.multimode < 2 && ud.recstat == 0)
		{
			ud.pause_on |= 2;
		}
	}
	else /* WORKINPROGRESS if (gamestate != GS_FULLCONSOLE && gamestate != GS_STARTUP) */
	{
		ConsoleState = c_rising;
		C_FlushDisplay ();
		ud.pause_on &= ~2;
	}
}

CCMD (toggleconsole)
{
	C_ToggleConsole();
}

void C_HideConsole ()
{
	if (gamestate != GS_FULLCONSOLE &&
		gamestate != GS_STARTUP &&
		ConsoleState != c_up)
	{
		ConsoleState = c_up;
		ConBottom = 0;
		HistPos = NULL;
	}
}

static void makestartposgood ()
{
	int n;
	int pos = CmdLine[259];
	int curs = CmdLine[1];
	int len = CmdLine[0];

	n = pos;

	if (pos >= len)
	{ // Start of visible line is beyond end of line
		n = curs - ConCols + 2;
	}
	if ((curs - pos) >= ConCols - 2)
	{ // The cursor is beyond the visible part of the line
		n = curs - ConCols + 2;
	}
	if (pos > curs)
	{ // The cursor is in front of the visible part of the line
		n = curs;
	}
	if (n < 0)
		n = 0;
	CmdLine[259] = n;
}

static bool C_HandleKey (const event_t *ev, byte *buffer, int len)
{
	int i;

	if (ev->subtype == EV_GUI_Char)
	{ // Add keypress to command line
		if (buffer[0] < len)
		{
			if (buffer[1] == buffer[0])
			{
				buffer[buffer[0] + 2] = ev->data1;
			}
			else
			{
				char *c, *e;

				e = (char *)&buffer[buffer[0] + 1];
				c = (char *)&buffer[buffer[1] + 2];

				for (; e >= c; e--)
					*(e + 1) = *e;

				*c = ev->data1;
			}
			buffer[0]++;
			buffer[1]++;
			makestartposgood ();
			HistPos = NULL;
		}
		TabbedLast = false;
	}
	else
	{
		switch (ev->data1)
		{
		case '\t':
			// Try to do tab-completion
			C_TabComplete ((ev->data3 & GKM_SHIFT) ? false : true);
			break;

		case GK_PGUP:
			if (ev->data3 & (GKM_SHIFT|GKM_CTRL))
			{ // Scroll console buffer up one page
				RowAdjust += (SCREENHEIGHT-4) /
					((gamestate == GS_FULLCONSOLE || gamestate == GS_STARTUP) ? 8 : 16) - 3;
			}
			else if (RowAdjust < CONSOLELINES)
			{ // Scroll console buffer up
				RowAdjust++;
			}
			break;

		case GK_PGDN:
			if (ev->data3 & (GKM_SHIFT|GKM_CTRL))
			{ // Scroll console buffer down one page
				const int scrollamt = (SCREENHEIGHT-4) /
					((gamestate == GS_FULLCONSOLE || gamestate == GS_STARTUP) ? 8 : 16) - 3;
				if (RowAdjust < scrollamt)
				{
					RowAdjust = 0;
				}
				else
				{
					RowAdjust -= scrollamt;
				}
			}
			else if (RowAdjust > 0)
			{ // Scroll console buffer down
				RowAdjust--;
			}
			break;

		case GK_HOME:
			if (ev->data3 & GKM_CTRL)
			{ // Move to top of console buffer
				RowAdjust = CONSOLELINES;
			}
			else
			{ // Move cursor to start of line
				buffer[1] = buffer[len+4] = 0;
			}
			break;

		case GK_END:
			if (ev->data3 & GKM_CTRL)
			{ // Move to bottom of console buffer
				RowAdjust = 0;
			}
			else
			{ // Move cursor to end of line
				buffer[1] = buffer[0];
				makestartposgood ();
			}
			break;

		case GK_LEFT:
			// Move cursor left one character
			if (buffer[1])
			{
				buffer[1]--;
				makestartposgood ();
			}
			break;

		case GK_RIGHT:
			// Move cursor right one character
			if (buffer[1] < buffer[0])
			{
				buffer[1]++;
				makestartposgood ();
			}
			break;

		case '\b':
			// Erase character to left of cursor
			if (buffer[0] && buffer[1])
			{
				char *c, *e;

				e = (char *)&buffer[buffer[0] + 2];
				c = (char *)&buffer[buffer[1] + 2];

				for (; c < e; c++)
					*(c - 1) = *c;
				
				buffer[0]--;
				buffer[1]--;
				if (buffer[len+4])
					buffer[len+4]--;
				makestartposgood ();
			}
			TabbedLast = false;
			break;

		case GK_DEL:
			// Erase charater under cursor
			if (buffer[1] < buffer[0])
			{
				char *c, *e;

				e = (char *)&buffer[buffer[0] + 2];
				c = (char *)&buffer[buffer[1] + 3];

				for (; c < e; c++)
					*(c - 1) = *c;

				buffer[0]--;
				makestartposgood ();
			}
			TabbedLast = false;
			break;

		case GK_UP:
			// Move to previous entry in the command history
			if (HistPos == NULL)
			{
				HistPos = HistHead;
			}
			else if (HistPos->Older)
			{
				HistPos = HistPos->Older;
			}

			if (HistPos)
			{
				strcpy ((char *)&buffer[2], HistPos->String);
				buffer[0] = buffer[1] = (BYTE)strlen ((char *)&buffer[2]);
				buffer[len+4] = 0;
				makestartposgood();
			}

			TabbedLast = false;
			break;

		case GK_DOWN:
			// Move to next entry in the command history
			if (HistPos && HistPos->Newer)
			{
				HistPos = HistPos->Newer;
			
				strcpy ((char *)&buffer[2], HistPos->String);
				buffer[0] = buffer[1] = (BYTE)strlen ((char *)&buffer[2]);
			}
			else
			{
				HistPos = NULL;
				buffer[0] = buffer[1] = 0;
			}
			buffer[len+4] = 0;
			makestartposgood();
			TabbedLast = false;
			break;

		case 'D':
			if (ev->data3 & GKM_CTRL && buffer[0] == 0)
			{ // Control-D pressed on an empty line
				int replen = (int)strlen (con_ctrl_d);

				if (replen == 0)
					break;	// Replacement is empty, so do nothing

				if (replen > len)
					replen = len;

				memcpy (&buffer[2], con_ctrl_d, replen);
				buffer[0] = buffer[1] = replen;
			}
			else
			{
				break;
			}
			// Intentional fall-through for command(s) added with Ctrl-D

		case '\r':
			// Execute command line (ENTER)

			buffer[2 + buffer[0]] = 0;

			for (i = 0; i < buffer[0] && isspace(buffer[2+i]); ++i)
			{
			}
			if (i == buffer[0])
			{
				 // Command line is empty, so do nothing to the history
			}
			else if (HistHead && stricmp (HistHead->String, (char *)&buffer[2]) == 0)
			{
				// Command line was the same as the previous one,
				// so leave the history list alone
			}
			else
			{
				// Command line is different from last command line,
				// or there is nothing in the history list,
				// so add it to the history list.

				History *temp = (History *)Malloc (sizeof(struct History) + buffer[0]);

				strcpy (temp->String, (char *)&buffer[2]);
				temp->Older = HistHead;
				if (HistHead)
				{
					HistHead->Newer = temp;
				}
				temp->Newer = NULL;
				HistHead = temp;

				if (!HistTail)
				{
					HistTail = temp;
				}

				if (HistSize == MAXHISTSIZE)
				{
					HistTail = HistTail->Newer;
					free (HistTail->Older);
					HistTail->Older = NULL;
				}
				else
				{
					HistSize++;
				}
			}
			HistPos = NULL;
			Printf (127, "]%s\n", &buffer[2]);
			buffer[0] = buffer[1] = buffer[len+4] = 0;
			AddCommandString ((char *)&buffer[2]);
			TabbedLast = false;
			break;
		
		case '`':
		case GK_ESCAPE:
			// Close console and clear command line. But if we're in the
			// fullscreen console mode, there's nothing to fall back on
			// if it's closed, so open the main menu instead.
			if (gamestate == GS_STARTUP)
			{
				return false;
			}
			else if (gamestate == GS_FULLCONSOLE)
			{
				C_DoCommand ("menu_main");
			}
			else
			{
				buffer[0] = buffer[1] = buffer[len+4] = 0;
				HistPos = NULL;
				C_ToggleConsole ();
			}
			break;

		case 'C':
		case 'V':
			TabbedLast = false;
			if (ev->data3 & GKM_CTRL)
			{
				if (ev->data1 == 'C')
				{ // copy to clipboard
					if (buffer[0] > 0)
					{
						buffer[2 + buffer[0]] = 0;
						I_PutInClipboard ((char *)&buffer[2]);
					}
					break;
				}
				else
				{ // paste from clipboard
					char *clip = I_GetFromClipboard ();
					if (clip != NULL)
					{
						strtok (clip, "\r\n\b");
						int cliplen = (int)strlen (clip);

						cliplen = MIN(len, cliplen);
						if (buffer[0] + cliplen > len)
						{
							cliplen = len - buffer[0];
						}

						if (cliplen > 0)
						{
							if (buffer[1] < buffer[0])
							{
								memmove (&buffer[2 + buffer[1] + cliplen],
										 &buffer[2 + buffer[1]], buffer[0] - buffer[1]);
							}
							memcpy (&buffer[2 + buffer[1]], clip, cliplen);
							buffer[0] += cliplen;
							buffer[1] += cliplen;
							makestartposgood ();
							HistPos = NULL;
						}
						delete[] clip;
					}
					break;
				}
			}
			break;
		}
	}
	CursorTicker = C_BLINKRATE;
	cursoron = 1;
	return true;
}

bool C_Responder (const event_t *ev)
{
	if (ps[myconnectindex].gm & MODE_MENU)
	{
		return false;
	}

	if (ev->type == EV_KeyDown &&
		C_GetSpecialBinding (ev->data1) == SBIND_ToggleConsole)
	{
		C_ToggleConsole ();
		return true;
	}

	if (ev->type != EV_GUI_Event ||
		ConsoleState == c_up ||
		ConsoleState == c_rising)
	{
		return false;
	}

	if (ev->subtype == EV_GUI_KeyDown ||
		ev->subtype == EV_GUI_KeyRepeat ||
		ev->subtype == EV_GUI_Char)
	{
		return C_HandleKey (ev, CmdLine, 255);
	}

	return false;
}

CCMD (history)
{
	struct History *hist = HistTail;

	while (hist)
	{
		Printf ("   %s\n", hist->String);
		hist = hist->Newer;
	}
}

CCMD (clear)
{
	C_FlushDisplay ();
	ClearConsole ();
}

CCMD (echo)
{
	int last = argv.argc()-1;
	for (int i = 1; i <= last; ++i)
	{
		Printf ("%s%s", argv[i], i!=last ? " " : "\n");
	}
}

/* Printing in the middle of the screen */

CVAR (Float, con_midtime, 3.f, CVAR_ARCHIVE)

void C_MidPrint (const char *msg)
{
	if (msg)
	{
		char buff[1024];
		sprintf (buff, TEXTCOLOR_RED
			"\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36"
			"\36\36\36\36\36\36\36\36\36\36\36\36\37" TEXTCOLOR_TAN
			"\n\n%s\n" TEXTCOLOR_RED
			"\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36"
			"\36\36\36\36\36\36\36\36\36\36\36\36\37" TEXTCOLOR_NORMAL "\n\n" ,
			msg);

		AddToConsole (-1, buff, false);
#if WORKINPROGRESS
		StatusBar->AttachMessage (new DHUDMessage (msg, 1.5f, 0.375f,
			(EColorRange)PrintColors[PRINTLEVELS], con_midtime), 'CNTR');
#endif
	}
	else
	{
#if WORKINPROGRESS
		StatusBar->DetachMessage ('CNTR');
#endif
	}
}

void C_MidPrintBold (const char *msg)
{
	if (msg)
	{
		char buff[1024];
		sprintf (buff, TEXTCOLOR_RED
			"\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36"
			"\36\36\36\36\36\36\36\36\36\36\36\36\37" TEXTCOLOR_GREEN
			"\n\n%s\n" TEXTCOLOR_RED
			"\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36"
			"\36\36\36\36\36\36\36\36\36\36\36\36\37" TEXTCOLOR_NORMAL "\n\n" ,
			msg);

		AddToConsole (-1, buff, false);

#if WORKINPROGRESS
		StatusBar->AttachMessage (new DHUDMessage (msg, 1.5f, 0.375f,
			(EColorRange)PrintColors[PRINTLEVELS+1], con_midtime), 'CNTR');
#endif
	}
	else
	{
#if WORKINPROGRESS
		StatusBar->DetachMessage ('CNTR');
#endif
	}
}

/****** Tab completion code ******/

static struct TabData
{
	int UseCount;
	char *Name;
} *TabCommands = NULL;
static int NumTabCommands = 0;
static int TabPos;				// Last TabCommand tabbed to
static int TabStart;			// First char in CmdLine to use for tab completion
static int TabSize;				// Size of tab string

static BOOL FindTabCommand (const char *name, int *stoppos, int len)
{
	int i, cval = 1;

	for (i = 0; i < NumTabCommands; i++)
	{
		cval = strnicmp (TabCommands[i].Name, name, len);
		if (cval >= 0)
			break;
	}

	*stoppos = i;

	return (cval == 0);
}

void C_AddTabCommand (const char *name)
{
	int pos;

	if (FindTabCommand (name, &pos, INT_MAX))
	{
		TabCommands[pos].UseCount++;
	}
	else
	{
		NumTabCommands++;
		TabCommands = (TabData *)Realloc (TabCommands, sizeof(struct TabData) * NumTabCommands);
		if (pos < NumTabCommands - 1)
		{
			memmove (TabCommands + pos + 1, TabCommands + pos,
					 (NumTabCommands - pos - 1) * sizeof(struct TabData));
		}
		TabCommands[pos].Name = copystring (name);
		TabCommands[pos].UseCount = 1;
	}
}

void C_RemoveTabCommand (const char *name)
{
	int pos;

	if (FindTabCommand (name, &pos, INT_MAX))
	{
		if (--TabCommands[pos].UseCount == 0)
		{
			NumTabCommands--;
			delete[] TabCommands[pos].Name;
			if (pos < NumTabCommands - 1)
			{
				memmove (TabCommands + pos, TabCommands + pos + 1,
						 (NumTabCommands - pos - 1) * sizeof(struct TabData));
			}
		}
	}
}

static int FindDiffPoint (const char *str1, const char *str2)
{
	int i;

	for (i = 0; tolower(str1[i]) == tolower(str2[i]); i++)
		if (str1[i] == 0 || str2[i] == 0)
			break;

	return i;
}

static void C_TabComplete (bool goForward)
{
	int i;
	int diffpoint;

	if (!TabbedLast)
	{
		// Skip any spaces at beginning of command line
		if (CmdLine[2] == ' ')
		{
			for (i = 0; i < CmdLine[0]; i++)
				if (CmdLine[2+i] != ' ')
					break;

			TabStart = i + 2;
		}
		else
		{
			TabStart = 2;
		}

		if (TabStart == CmdLine[0] + 2)
			return;		// Line was nothing but spaces

		TabSize = CmdLine[0] - TabStart + 2;

		if (!FindTabCommand ((char *)(CmdLine + TabStart), &TabPos, TabSize))
			return;		// No initial matches

		if (goForward)
		{ // Position just before the list of completions so that when TabPos
		  // gets advanced below, it will be at the first one.
			--TabPos;
		}
		else
		{ // Find the last matching tab, then go one past it.
			while (++TabPos < NumTabCommands)
			{
				if (FindDiffPoint (TabCommands[TabPos].Name, (char *)(CmdLine + TabStart)) < TabSize)
				{
					break;
				}
			}
		}
		TabbedLast = true;
	}

	if ((goForward && ++TabPos == NumTabCommands) ||
		(!goForward && --TabPos < 0))
	{
		TabbedLast = false;
		CmdLine[0] = CmdLine[1] = TabSize;
	}
	else
	{
		diffpoint = FindDiffPoint (TabCommands[TabPos].Name, (char *)(CmdLine + TabStart));

		if (diffpoint < TabSize)
		{
			// No more matches
			TabbedLast = false;
			CmdLine[0] = CmdLine[1] = TabSize + TabStart - 2;
		}
		else
		{		
			strcpy ((char *)(CmdLine + TabStart), TabCommands[TabPos].Name);
			CmdLine[0] = CmdLine[1] = (BYTE)strlen ((char *)(CmdLine + 2)) + 1;
			CmdLine[CmdLine[0] + 1] = ' ';
		}
	}

	makestartposgood ();
}
