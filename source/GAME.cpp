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

#include "types.h"
#include "develop.h"
#include "scriplib.h"
#include "file_lib.h"
#include "gamedefs.h"
#include "keyboard.h"
#include "util_lib.h"
#include "function.h"
#include "control.h"
#include "fx_man.h"
#include "sounds.h"
#include "config.h"
#include "sndcards.h"
#include "v_video.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_gui.h"
#include "i_video.h"
#include "c_console.h"
#include "c_bind.h"
#include "c_dispatch.h"
#include "s_sound.h"

#include "duke3d.h"


#define HEAD   "Duke Nukem 3D Unregistered Shareware v"VERSION" "
#define HEAD2P "Duke Nukem 3D v"VERSION" - Atomic Edition"
#define HEAD2  "Duke Nukem 3D Full Version v"VERSION
#define HEADA  "Duke Nukem 3D AUSSIE Unregistered Shareware v"VERSION
#define HEAD2A "Duke Nukem 3D AUSSIE Full Version v"VERSION

#define IDFSIZE 479985668
// #define IDFSIZE 9961476
// #define IDFSIZE 16384
#define IDFILENAME "DUKE3D.IDF"


#define TIMERUPDATESIZ 32

bool dukever13;

long cameradist = 0, cameraclock = 0;
char playerswhenstarted;
char qe;

int32 CommandSoundToggleOff = 0;
int32 CommandMusicToggleOff = 0;

char confilename[128] = {"GAME.CON"},boardfilename[128] = {0};
char waterpal[768], slimepal[768], titlepal[768], drealms[768], endingpal[768];
char firstdemofile[80] = { '\0' };

bool VOLUMEONE, PLUTOPAK, VOLUMEALL, ONELEVELDEMO;

#define patchstatusbar(x1,y1,x2,y2)                                        \
{                                                                          \
	rotatesprite(0,(200-34)<<16,65536L,0,BOTTOMSTATUSBAR,4,0,10+16+64+128, \
	scale(x1,xdim,320),scale(y1,ydim,200),                             \
	scale(x2,xdim,320)-1,scale(y2,ydim,200)-1);                        \
}

//void __interrupt __far newint24( int errval, int ax, int bp, int si );

int recfilep,totalreccnt;
char debug_on = 0,actor_tog = 0,memorycheckoveride=0;



extern BYTE syncstate;
extern int32 numlumps;

FILE *frecfilep = (FILE *)NULL;
void pitch_test( void );

char restorepalette,screencapt,nomorelogohack;
int sendmessagecommand = -1;

extern long lastvisinc;

int gametext(int x,int y,const char *t,char s,short dabits)
{
	short ac,newx;
	char centre;
	const char *oldt;

	centre = ( x == (320>>1) );
	newx = 0;
	oldt = t;

	if(centre)
	{
		while(*t)
		{
			if(*t == 32) {newx+=5;t++;continue;}
			else ac = *t - '!' + STARTALPHANUM;

			if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

			if(*t >= '0' && *t <= '9')
				newx += 8;
			else newx += tilesizx[ac];
			t++;
		}

		t = oldt;
		x = (320>>1)-(newx>>1);
	}

	while(*t)
	{
		if(*t == 32) {x+=5;t++;continue;}
		else ac = *t - '!' + STARTALPHANUM;

		if( ac < STARTALPHANUM || ac > ENDALPHANUM )
			break;

		rotatesprite(x<<16,y<<16,65536L,0,ac,s,0,dabits,0,0,xdim-1,ydim-1);

		if(*t >= '0' && *t <= '9')
			x += 8;
		else x += tilesizx[ac];

		t++;
	}

	return (x);
}

int gametextpal(int x,int y,const char *t,char s,char p)
{
	short ac,newx;
	char centre;
	const char *oldt;

	centre = ( x == (320>>1) );
	newx = 0;
	oldt = t;

	if(centre)
	{
		while(*t)
		{
			if(*t == 32) {newx+=5;t++;continue;}
			else ac = *t - '!' + STARTALPHANUM;

			if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

			if(*t >= '0' && *t <= '9')
				newx += 8;
			else newx += tilesizx[ac];
			t++;
		}

		t = oldt;
		x = (320>>1)-(newx>>1);
	}

	while(*t)
	{
		if(*t == 32) {x+=5;t++;continue;}
		else ac = *t - '!' + STARTALPHANUM;

		if( ac < STARTALPHANUM || ac > ENDALPHANUM )
			break;

		rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,2+8+16,0,0,xdim-1,ydim-1);
		if(*t >= '0' && *t <= '9')
			x += 8;
		else x += tilesizx[ac];

		t++;
	}

	return (x);
}

int gametextpart(int x,int y,const char *t,char s,short p)
{
	short ac,newx, cnt;
	char centre;
	const char *oldt;

	centre = ( x == (320>>1) );
	newx = 0;
	oldt = t;
	cnt = 0;

	if(centre)
	{
		while(*t)
		{
			if(cnt == p) break;

			if(*t == 32) {newx+=5;t++;continue;}
			else ac = *t - '!' + STARTALPHANUM;

			if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

			newx += tilesizx[ac];
			t++;
			cnt++;

		}

		t = oldt;
		x = (320>>1)-(newx>>1);
	}

	cnt = 0;
	while(*t)
	{
		if(*t == 32) {x+=5;t++;continue;}
		else ac = *t - '!' + STARTALPHANUM;

		if( ac < STARTALPHANUM || ac > ENDALPHANUM ) break;

		if(cnt == p)
		{
			rotatesprite(x<<16,y<<16,65536L,0,ac,s,1,2+8+16,0,0,xdim-1,ydim-1);
			break;
		}
		else
			rotatesprite(x<<16,y<<16,65536L,0,ac,s,0,2+8+16,0,0,xdim-1,ydim-1);

		x += tilesizx[ac];

		t++;
		cnt++;
	}

	return (x);
}

int minitext(int x,int y,const char *t,char p,char sb)
{
	short ac;

	while(*t)
	{
		char ct = toupper(*t);
		if(ct == ' ') {x+=5;t++;continue;}
		else ac = ct - '!' + MINIFONT;

		rotatesprite(x<<16,y<<16,65536L,0,ac,0,p,sb,0,0,xdim-1,ydim-1);
		x += 4; // tilesizx[ac]+1;

		t++;
	}
	return (x);
}

int minitextshade(int x,int y,const char *t,char s,char p,char sb)
{
	short ac;

	while(*t)
	{
		char ct = toupper(*t);
		if(ct == ' ') {x+=5;t++;continue;}
		else ac = ct - '!' + MINIFONT;

		rotatesprite(x<<16,y<<16,65536L,0,ac,s,p,sb,0,0,xdim-1,ydim-1);
		x += 4; // tilesizx[ac]+1;

		t++;
	}
	return (x);
}

void gamenumber(long x,long y,long n,char s)
{
	char b[10];
	ltoa(n,b,10);
	gametext(x,y,b,s,2+8+16);
}

bool GotEscape;

bool Escape_Grabber (const event_t *ev)
{
	if ((ev->type == EV_KeyDown && ev->data1 == KEY_ESCAPE) ||
		(ev->type == EV_GUI_Event &&
		 (ev->subtype == EV_GUI_KeyDown || ev->subtype == EV_GUI_KeyRepeat) &&
		 ev->data1 == GK_ESCAPE))
	{
		GotEscape = true;
	}
	return false;
}

char recbuf[80];
void allowtimetocorrecterrorswhenquitting(void)
{
	long i, j, oldtotalclock;

	ready2send = 0;

	D_AddResponder (Escape_Grabber, 1);
	for(j=0;j<8;j++)
	{
		oldtotalclock = totalclock;

		while (totalclock < oldtotalclock+TICSPERFRAME)
			getpackets();

		I_StartTic ();
		D_ProcessEvents ();
		if (GotEscape)
		{
			break;
		}

		packbuf[0] = 127;
		for(i=connecthead;i>=0;i=connectpoint2[i])
			if (i != myconnectindex)
				sendpacket(i,packbuf,1);
	}
	GotEscape = false;
	D_RemoveResponder (Escape_Grabber);
}

#define MAXUSERQUOTES 4
long quotebot, quotebotgoal;
short user_quote_time[MAXUSERQUOTES];
char user_quote[MAXUSERQUOTES][128];
// char typebuflen,typebuf[41];

void adduserquote(char *daquote)
{
	long i;

	for(i=MAXUSERQUOTES-1;i>0;i--)
	{
		strcpy(user_quote[i],user_quote[i-1]);
		user_quote_time[i] = user_quote_time[i-1];
	}
	strcpy(user_quote[0],daquote);
	user_quote_time[0] = 180;
	pub = NUMPAGES;
}


void getpackets(void)
{
	long i, j, k, l;
	short other, packbufleng;
	input *osyn, *nsyn;

#if 0
	if(qe == 0 && KB_KeyPressed(sc_LeftControl) && KB_KeyPressed(sc_LeftAlt) && KB_KeyPressed(sc_Delete))
	{
		qe = 1;
		gameexit("Quick Exit.");
	}
#endif

	if (numplayers < 2) return;
	while ((packbufleng = getpacket(&other,packbuf)) >= 0)
	{
		// [RH] Allow 0-length packets (they could be errors, so retry right away)
		if (packbufleng == 0)
			continue;

		switch(packbuf[0])
		{
		case 125:
			//cp = 0;
			break;

		case 126:
			multiflag = 2;
			multiwhat = 0;
			multiwho = other;
			multipos = packbuf[1];
			loadplayer( multipos );
			multiflag = 0;
			break;
		case 0:  //[0] (receive master sync buffer)
			j = 1;

			if ((movefifoend[other]&(TIMERUPDATESIZ-1)) == 0)
			{
				for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
				{
					if (playerquitflag[i] == 0) continue;
					if (i == myconnectindex)
						otherminlag = (long)((signed char)packbuf[j]);
					j++;
				}
			}

			osyn = (input *)&inputfifo[(movefifoend[connecthead]-1)&(MOVEFIFOSIZ-1)][0];
			nsyn = (input *)&inputfifo[(movefifoend[connecthead])&(MOVEFIFOSIZ-1)][0];

			k = j;
			for(i=connecthead;i>=0;i=connectpoint2[i])
			{
				j += playerquitflag[i];
			}
			for(i=connecthead;i>=0;i=connectpoint2[i])
			{
				if (playerquitflag[i] == 0) continue;

				l = packbuf[k++];
				if (i == myconnectindex)
				{ j += ((l&1)<<1)+(l&2)+((l&4)>>2)+((l&8)>>3)+((l&16)>>4)+((l&32)>>5)+((l&64)>>6)+((l&128)>>7); continue; }

				copybufbyte(&osyn[i],&nsyn[i],sizeof(input));
				if (l&1)   nsyn[i].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
				if (l&2)   nsyn[i].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
				if (l&4)   nsyn[i].avel = (signed char)packbuf[j++];
				if (l&8)   nsyn[i].bits = ((nsyn[i].bits&0xffffff00)|((long)packbuf[j++]));
				if (l&16)  nsyn[i].bits = ((nsyn[i].bits&0xffff00ff)|((long)packbuf[j++])<<8);
				if (l&32)  nsyn[i].bits = ((nsyn[i].bits&0xff00ffff)|((long)packbuf[j++])<<16);
				if (l&64)  nsyn[i].bits = ((nsyn[i].bits&0x00ffffff)|((long)packbuf[j++])<<24);
				if (l&128) nsyn[i].horz = (signed char)packbuf[j++];

				if (nsyn[i].bits&(1<<26)) playerquitflag[i] = 0;
				movefifoend[i]++;
			}

			while (j != packbufleng)
			{
				for(i=connecthead;i>=0;i=connectpoint2[i])
				{
					if(i != myconnectindex)
					{
						syncval[i][syncvalhead[i]&(MOVEFIFOSIZ-1)] = packbuf[j];
						syncvalhead[i]++;
					}
				}
				j++;
			}

			for(i=connecthead;i>=0;i=connectpoint2[i])
			{
				if (i != myconnectindex)
				{
					for(j=1;j<movesperpacket;j++)
					{
						copybufbyte(&nsyn[i],&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i],sizeof(input));
						movefifoend[i]++;
					}
				}
			}
			movefifosendplc += movesperpacket;

			break;
		case 1:  //[1] (receive slave sync buffer)
			j = 2; k = packbuf[1];

			osyn = (input *)&inputfifo[(movefifoend[other]-1)&(MOVEFIFOSIZ-1)][0];
			nsyn = (input *)&inputfifo[(movefifoend[other])&(MOVEFIFOSIZ-1)][0];

			copybufbyte(&osyn[other],&nsyn[other],sizeof(input));
			if (k&1)   nsyn[other].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
			if (k&2)   nsyn[other].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
			if (k&4)   nsyn[other].avel = (signed char)packbuf[j++];
			if (k&8)   nsyn[other].bits = ((nsyn[other].bits&0xffffff00)|((long)packbuf[j++]));
			if (k&16)  nsyn[other].bits = ((nsyn[other].bits&0xffff00ff)|((long)packbuf[j++])<<8);
			if (k&32)  nsyn[other].bits = ((nsyn[other].bits&0xff00ffff)|((long)packbuf[j++])<<16);
			if (k&64)  nsyn[other].bits = ((nsyn[other].bits&0x00ffffff)|((long)packbuf[j++])<<24);
			if (k&128) nsyn[other].horz = (signed char)packbuf[j++];
			movefifoend[other]++;

			while (j != packbufleng)
			{
				syncval[other][syncvalhead[other]&(MOVEFIFOSIZ-1)] = packbuf[j++];
				syncvalhead[other]++;
			}

			for(i=1;i<movesperpacket;i++)
			{
				copybufbyte(&nsyn[other],&inputfifo[movefifoend[other]&(MOVEFIFOSIZ-1)][other],sizeof(input));
				movefifoend[other]++;
			}

			break;

		case 4:
			strcpy(recbuf,(char *)packbuf+1);
			recbuf[packbufleng-1] = 0;

			adduserquote(recbuf);
			sound(EXITMENUSOUND);

			pus = NUMPAGES;
			pub = NUMPAGES;

			break;

		case 5:
			ud.m_level_number = ud.level_number = packbuf[1];
			ud.m_volume_number = ud.volume_number = packbuf[2];
			ud.m_player_skill = ud.player_skill = packbuf[3];
			ud.m_monsters_off = ud.monsters_off = packbuf[4];
			ud.m_respawn_monsters = ud.respawn_monsters = packbuf[5];
			ud.m_respawn_items = ud.respawn_items = packbuf[6];
			ud.m_respawn_inventory = ud.respawn_inventory = packbuf[7];
			ud.m_coop = packbuf[8];
			ud.m_marker = ud.marker = packbuf[9];
			ud.m_ffire = ud.ffire = packbuf[10];

			for(i=connecthead;i>=0;i=connectpoint2[i])
			{
				resetweapons(i);
				resetinventory(i);
			}

			newgame(ud.volume_number,ud.level_number,ud.player_skill);
			ud.coop = ud.m_coop;

			enterlevel(MODE_GAME);

			break;
		case 6:
			if (packbuf[1] != BYTEVERSION)
				gameexit("\nYou cannot play Duke with different versions.");
			for (i=2;packbuf[i];i++)
				ud.user_name[other][i-2] = packbuf[i];
			ud.user_name[other][i-2] = 0;
			break;
		case 9:
			for (i=1;i<packbufleng;i++)
				ud.wchoice[other][i-1] = packbuf[i];
			break;
		case 7:

			if(numlumps == 0) break;

			if (SoundToggle == 0 || ud.lockout == 1 || FXDevice == NumSoundCards)
				break;
            sound(RTS_FirstSound + packbuf[1]);
			rtsplaying = 7;
			break;
		case 8:
			ud.m_level_number = ud.level_number = packbuf[1];
			ud.m_volume_number = ud.volume_number = packbuf[2];
			ud.m_player_skill = ud.player_skill = packbuf[3];
			ud.m_monsters_off = ud.monsters_off = packbuf[4];
			ud.m_respawn_monsters = ud.respawn_monsters = packbuf[5];
			ud.m_respawn_items = ud.respawn_items = packbuf[6];
			ud.m_respawn_inventory = ud.respawn_inventory = packbuf[7];
			ud.m_coop = ud.coop = packbuf[8];
			ud.m_marker = ud.marker = packbuf[9];
			ud.m_ffire = ud.ffire = packbuf[10];

			copybufbyte(packbuf+10,boardfilename,packbufleng-11);
			boardfilename[packbufleng-11] = 0;

			for(i=connecthead;i>=0;i=connectpoint2[i])
			{
				resetweapons(i);
				resetinventory(i);
			}

			newgame(ud.volume_number,ud.level_number,ud.player_skill);
			enterlevel(MODE_GAME);
			break;

		case 16:
			movefifoend[other] = movefifoplc = movefifosendplc = fakemovefifoplc = 0;
			syncvalhead[other] = syncvaltottail = 0L;
		case 17:
			j = 1;

			if ((movefifoend[other]&(TIMERUPDATESIZ-1)) == 0)
			{
				if (other == connecthead)
				{
					for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
					{
						if (i == myconnectindex)
							otherminlag = (long)((signed char)packbuf[j]);
						j++;
					}
				}
			}

			osyn = (input *)&inputfifo[(movefifoend[other]-1)&(MOVEFIFOSIZ-1)][0];
			nsyn = (input *)&inputfifo[(movefifoend[other])&(MOVEFIFOSIZ-1)][0];

			copybufbyte(&osyn[other],&nsyn[other],sizeof(input));
			k = packbuf[j++];
			if (k&1)   nsyn[other].fvel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
			if (k&2)   nsyn[other].svel = packbuf[j]+((short)packbuf[j+1]<<8), j += 2;
			if (k&4)   nsyn[other].avel = (signed char)packbuf[j++];
			if (k&8)   nsyn[other].bits = ((nsyn[other].bits&0xffffff00)|((long)packbuf[j++]));
			if (k&16)  nsyn[other].bits = ((nsyn[other].bits&0xffff00ff)|((long)packbuf[j++])<<8);
			if (k&32)  nsyn[other].bits = ((nsyn[other].bits&0xff00ffff)|((long)packbuf[j++])<<16);
			if (k&64)  nsyn[other].bits = ((nsyn[other].bits&0x00ffffff)|((long)packbuf[j++])<<24);
			if (k&128) nsyn[other].horz = (signed char)packbuf[j++];
			movefifoend[other]++;

			for(i=1;i<movesperpacket;i++)
			{
				copybufbyte(&nsyn[other],&inputfifo[movefifoend[other]&(MOVEFIFOSIZ-1)][other],sizeof(input));
				movefifoend[other]++;
			}

			if (j > packbufleng)
			{
				Printf("INVALID GAME PACKET!!! (%ld too many bytes)\n",j-packbufleng);
			}
			else
			{
				while (j != packbufleng)
				{
					syncval[other][syncvalhead[other]&(MOVEFIFOSIZ-1)] = packbuf[j++];
					syncvalhead[other]++;
				}
			}
			break;

		case 127:
			break;

		case 250:
			playerreadyflag[other]++;
			break;
		case 255:
			gameexit(" ");
			break;
		}
	}
}

void faketimerhandler()
{
	long i, j, k;
	//    short who;
	input *osyn, *nsyn;

#if 0
	if(qe == 0 && KB_KeyPressed(sc_LeftControl) && KB_KeyPressed(sc_LeftAlt) && KB_KeyPressed(sc_Delete))
	{
		qe = 1;
		gameexit("Quick Exit.");
	}
#endif

	if ((totalclock < ototalclock+TICSPERFRAME) || (ready2send == 0)) return;
	ototalclock += TICSPERFRAME;

	getpackets(); if (getoutputcirclesize() >= 16) return;

	for(i=connecthead;i>=0;i=connectpoint2[i])
		if (i != myconnectindex)
			if (movefifoend[i] < movefifoend[myconnectindex]-200) return;

	getinput(myconnectindex);

	avgfvel += loc.fvel;
	avgsvel += loc.svel;
	avgavel += loc.avel;
	avghorz += loc.horz;
	avgbits |= loc.bits;
	if (movefifoend[myconnectindex]&(movesperpacket-1))
	{
		copybufbyte(&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex],
			&inputfifo[movefifoend[myconnectindex]&(MOVEFIFOSIZ-1)][myconnectindex],sizeof(input));
		movefifoend[myconnectindex]++;
		return;
	}
	nsyn = &inputfifo[movefifoend[myconnectindex]&(MOVEFIFOSIZ-1)][myconnectindex];
	nsyn[0].fvel = avgfvel/movesperpacket;
	nsyn[0].svel = avgsvel/movesperpacket;
	nsyn[0].avel = avgavel/movesperpacket;
	nsyn[0].horz = avghorz/movesperpacket;
	nsyn[0].bits = avgbits;
	avgfvel = avgsvel = avgavel = avghorz = avgbits = 0;
	movefifoend[myconnectindex]++;

	if (numplayers < 2)
	{
		if (ud.multimode > 1 && ud.playerai)
		{
			for(i=connecthead;i>=0;i=connectpoint2[i])
			{
				if(i != myconnectindex)
				{
					//clearbufbyte(&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i],sizeof(input),0L);
					computergetinput(i,&inputfifo[movefifoend[i]&(MOVEFIFOSIZ-1)][i]);
					movefifoend[i]++;
				}
			}
		}
		return;
	}

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		if (i != myconnectindex)
		{
			k = (movefifoend[myconnectindex]-1)-movefifoend[i];
			myminlag[i] = min(myminlag[i],k);
			mymaxlag = max(mymaxlag,k);
		}
	}

	if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
	{
		i = mymaxlag-bufferjitter; mymaxlag = 0;
		if (i > 0) bufferjitter += ((3+i)>>2);
		else if (i < 0) bufferjitter -= ((1-i)>>2);
	}

	if (networkmode == 1)
	{
		packbuf[0] = 17;
		if ((movefifoend[myconnectindex]-1) == 0) packbuf[0] = 16;
		j = 1;

		//Fix timers and buffer/jitter value
		if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
		{
			if (myconnectindex != connecthead)
			{
				i = myminlag[connecthead]-otherminlag;
				if (klabs(i) > 8) i >>= 1;
				else if (klabs(i) > 2) i = ksgn(i);
				else i = 0;

				totalclock -= TICSPERFRAME*i;
				myminlag[connecthead] -= i; otherminlag += i;
			}

			if (myconnectindex == connecthead)
				for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
					packbuf[j++] = min(max(myminlag[i],-128),127);

			for(i=connecthead;i>=0;i=connectpoint2[i])
				myminlag[i] = 0x7fffffff;
		}

		osyn = (input *)&inputfifo[(movefifoend[myconnectindex]-2)&(MOVEFIFOSIZ-1)][myconnectindex];
		nsyn = (input *)&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex];

		k = j;
		packbuf[j++] = 0;

		if (nsyn[0].fvel != osyn[0].fvel)
		{
			packbuf[j++] = (char)nsyn[0].fvel;
			packbuf[j++] = (char)(nsyn[0].fvel>>8);
			packbuf[k] |= 1;
		}
		if (nsyn[0].svel != osyn[0].svel)
		{
			packbuf[j++] = (char)nsyn[0].svel;
			packbuf[j++] = (char)(nsyn[0].svel>>8);
			packbuf[k] |= 2;
		}
		if (nsyn[0].avel != osyn[0].avel)
		{
			packbuf[j++] = (signed char)nsyn[0].avel;
			packbuf[k] |= 4;
		}
		if ((nsyn[0].bits^osyn[0].bits)&0x000000ff) packbuf[j++] = (nsyn[0].bits&255), packbuf[k] |= 8;
		if ((nsyn[0].bits^osyn[0].bits)&0x0000ff00) packbuf[j++] = ((nsyn[0].bits>>8)&255), packbuf[k] |= 16;
		if ((nsyn[0].bits^osyn[0].bits)&0x00ff0000) packbuf[j++] = ((nsyn[0].bits>>16)&255), packbuf[k] |= 32;
		if ((nsyn[0].bits^osyn[0].bits)&0xff000000) packbuf[j++] = ((nsyn[0].bits>>24)&255), packbuf[k] |= 64;
		if (nsyn[0].horz != osyn[0].horz)
		{
			packbuf[j++] = (char)nsyn[0].horz;
			packbuf[k] |= 128;
		}

		while (syncvalhead[myconnectindex] != syncvaltail)
		{
			packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
			syncvaltail++;
		}

		for(i=connecthead;i>=0;i=connectpoint2[i])
			if (i != myconnectindex)
				sendpacket(i,packbuf,j);

		return;
	}
	if (myconnectindex != connecthead)   //Slave
	{
		//Fix timers and buffer/jitter value
		if (((movefifoend[myconnectindex]-1)&(TIMERUPDATESIZ-1)) == 0)
		{
			i = myminlag[connecthead]-otherminlag;
			if (klabs(i) > 8) i >>= 1;
			else if (klabs(i) > 2) i = ksgn(i);
			else i = 0;

			totalclock -= TICSPERFRAME*i;
			myminlag[connecthead] -= i; otherminlag += i;

			for(i=connecthead;i>=0;i=connectpoint2[i])
				myminlag[i] = 0x7fffffff;
		}

		packbuf[0] = 1; packbuf[1] = 0; j = 2;

		osyn = (input *)&inputfifo[(movefifoend[myconnectindex]-2)&(MOVEFIFOSIZ-1)][myconnectindex];
		nsyn = (input *)&inputfifo[(movefifoend[myconnectindex]-1)&(MOVEFIFOSIZ-1)][myconnectindex];

		if (nsyn[0].fvel != osyn[0].fvel)
		{
			packbuf[j++] = (char)nsyn[0].fvel;
			packbuf[j++] = (char)(nsyn[0].fvel>>8);
			packbuf[1] |= 1;
		}
		if (nsyn[0].svel != osyn[0].svel)
		{
			packbuf[j++] = (char)nsyn[0].svel;
			packbuf[j++] = (char)(nsyn[0].svel>>8);
			packbuf[1] |= 2;
		}
		if (nsyn[0].avel != osyn[0].avel)
		{
			packbuf[j++] = (signed char)nsyn[0].avel;
			packbuf[1] |= 4;
		}
		if ((nsyn[0].bits^osyn[0].bits)&0x000000ff) packbuf[j++] = (nsyn[0].bits&255), packbuf[1] |= 8;
		if ((nsyn[0].bits^osyn[0].bits)&0x0000ff00) packbuf[j++] = ((nsyn[0].bits>>8)&255), packbuf[1] |= 16;
		if ((nsyn[0].bits^osyn[0].bits)&0x00ff0000) packbuf[j++] = ((nsyn[0].bits>>16)&255), packbuf[1] |= 32;
		if ((nsyn[0].bits^osyn[0].bits)&0xff000000) packbuf[j++] = ((nsyn[0].bits>>24)&255), packbuf[1] |= 64;
		if (nsyn[0].horz != osyn[0].horz)
		{
			packbuf[j++] = (char)nsyn[0].horz;
			packbuf[1] |= 128;
		}

		while (syncvalhead[myconnectindex] != syncvaltail)
		{
			packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
			syncvaltail++;
		}

		sendpacket(connecthead,packbuf,j);
		return;
	}

	//This allows allow packet-resends
	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		if (movefifoend[i] <= movefifosendplc)
		{
			packbuf[0] = 127;
			for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
				sendpacket(i,packbuf,1);
			return;
		}
	}

	while (1)  //Master
	{
		for(i=connecthead;i>=0;i=connectpoint2[i])
			if (playerquitflag[i] && (movefifoend[i] <= movefifosendplc)) return;

		osyn = (input *)&inputfifo[(movefifosendplc-1)&(MOVEFIFOSIZ-1)][0];
		nsyn = (input *)&inputfifo[(movefifosendplc  )&(MOVEFIFOSIZ-1)][0];

		//MASTER -> SLAVE packet
		packbuf[0] = 0; j = 1;

		//Fix timers and buffer/jitter value
		if ((movefifosendplc&(TIMERUPDATESIZ-1)) == 0)
		{
			for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
				if (playerquitflag[i])
					packbuf[j++] = min(max(myminlag[i],-128),127);

			for(i=connecthead;i>=0;i=connectpoint2[i])
				myminlag[i] = 0x7fffffff;
		}

		k = j;
		for(i=connecthead;i>=0;i=connectpoint2[i])
			j += playerquitflag[i];
		for(i=connecthead;i>=0;i=connectpoint2[i])
		{
			if (playerquitflag[i] == 0) continue;

			packbuf[k] = 0;
			if (nsyn[i].fvel != osyn[i].fvel)
			{
				packbuf[j++] = (char)nsyn[i].fvel;
				packbuf[j++] = (char)(nsyn[i].fvel>>8);
				packbuf[k] |= 1;
			}
			if (nsyn[i].svel != osyn[i].svel)
			{
				packbuf[j++] = (char)nsyn[i].svel;
				packbuf[j++] = (char)(nsyn[i].svel>>8);
				packbuf[k] |= 2;
			}
			if (nsyn[i].avel != osyn[i].avel)
			{
				packbuf[j++] = (signed char)nsyn[i].avel;
				packbuf[k] |= 4;
			}
			if ((nsyn[i].bits^osyn[i].bits)&0x000000ff) packbuf[j++] = (nsyn[i].bits&255), packbuf[k] |= 8;
			if ((nsyn[i].bits^osyn[i].bits)&0x0000ff00) packbuf[j++] = ((nsyn[i].bits>>8)&255), packbuf[k] |= 16;
			if ((nsyn[i].bits^osyn[i].bits)&0x00ff0000) packbuf[j++] = ((nsyn[i].bits>>16)&255), packbuf[k] |= 32;
			if ((nsyn[i].bits^osyn[i].bits)&0xff000000) packbuf[j++] = ((nsyn[i].bits>>24)&255), packbuf[k] |= 64;
			if (nsyn[i].horz != osyn[i].horz)
			{
				packbuf[j++] = (char)nsyn[i].horz;
				packbuf[k] |= 128;
			}
			k++;
		}

		while (syncvalhead[myconnectindex] != syncvaltail)
		{
			packbuf[j++] = syncval[myconnectindex][syncvaltail&(MOVEFIFOSIZ-1)];
			syncvaltail++;
		}

		for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
		{
			if (playerquitflag[i])
			{
				sendpacket(i,packbuf,j);
				if (nsyn[i].bits&(1<<26))
					playerquitflag[i] = 0;
			}
		}

		movefifosendplc += movesperpacket;
	}
}

extern long cacnum;
typedef struct { long *hand, leng; char *lock; } cactype;
extern cactype cac[];

void caches(void)
{
	short i,k;

	k = 0;
	for(i=0;i<cacnum;i++)
		if ((*cac[i].lock) >= 200)
		{
			sprintf((char*)tempbuf,"Locked- %ld: Leng:%ld, Lock:%ld",i,cac[i].leng,*cac[i].lock);
			printext256(0L,k,31,-1,(char*)tempbuf,1); k += 6;
		}

	k += 6;
}



void checksync(void)
{
	long i;

	for(i=connecthead;i>=0;i=connectpoint2[i])
		if (syncvalhead[i] == syncvaltottail) break;
	if (i < 0)
	{
		syncstat = 0;
		do
		{
			for(i=connectpoint2[connecthead];i>=0;i=connectpoint2[i])
				if (syncval[i][syncvaltottail&(MOVEFIFOSIZ-1)] !=
					syncval[connecthead][syncvaltottail&(MOVEFIFOSIZ-1)])
					syncstat = 1;
			syncvaltottail++;
			for(i=connecthead;i>=0;i=connectpoint2[i])
				if (syncvalhead[i] == syncvaltottail) break;
		} while (i < 0);
	}
	if (connectpoint2[connecthead] < 0) syncstat = 0;

	if (syncstat)
	{
		printext256(4L,130L,31,0,"Out Of Sync - Please restart game",0);
		printext256(4L,138L,31,0,"RUN DN3DHELP.EXE for information.",0);
	}
	if (syncstate)
	{
		printext256(4L,160L,31,0,"Missed Network packet!",0);
		printext256(4L,138L,31,0,"RUN DN3DHELP.EXE for information.",0);
	}
}


void check_fta_sounds(short i)
{
	if(sprite[i].extra > 0) switch(sprite[i].picnum)
	{
		case LIZTROOPONTOILET:
		case LIZTROOPJUSTSIT:
		case LIZTROOPSHOOT:
		case LIZTROOPJETPACK:
		case LIZTROOPDUCKING:
		case LIZTROOPRUNNING:
		case LIZTROOP:
			spritesound(PRED_RECOG,i);
			break;
		case LIZMAN:
		case LIZMANSPITTING:
		case LIZMANFEEDING:
		case LIZMANJUMP:
			spritesound(CAPT_RECOG,i);
			break;
		case PIGCOP:
		case PIGCOPDIVE:
			spritesound(PIG_RECOG,i);
			break;
		case RECON:
			spritesound(RECO_RECOG,i);
			break;
		case DRONE:
			spritesound(DRON_RECOG,i);
			break;
		case COMMANDER:
		case COMMANDERSTAYPUT:
			spritesound(COMM_RECOG,i);
			break;
		case ORGANTIC:
			spritesound(TURR_RECOG,i);
			break;
		case OCTABRAIN:
		case OCTABRAINSTAYPUT:
			spritesound(OCTA_RECOG,i);
			break;
		case BOSS1:
			sound(BOS1_RECOG);
			break;
		case BOSS2:
			if(sprite[i].pal == 1)
				sound(BOS2_RECOG);
			else sound(WHIPYOURASS);
			break;
		case BOSS3:
			if(sprite[i].pal == 1)
				sound(BOS3_RECOG);
			else sound(RIPHEADNECK);
			break;
		case BOSS4:
		case BOSS4STAYPUT:
			if(sprite[i].pal == 1)
				sound(BOS4_RECOG);
			sound(BOSS4_FIRSTSEE);
			break;
		case GREENSLIME:
			spritesound(SLIM_RECOG,i);
			break;
	}
}

short inventory(spritetype *s)
{
	switch(s->picnum)
	{
	case FIRSTAID:
	case STEROIDS:
	case HEATSENSOR:
	case BOOTS:
	case JETPACK:
	case HOLODUKE:
	case AIRTANK:
		return 1;
	}
	return 0;
}


short badguy(spritetype *s)
{

	switch(s->picnum)
	{
	case SHARK:
	case RECON:
	case DRONE:
	case LIZTROOPONTOILET:
	case LIZTROOPJUSTSIT:
	case LIZTROOPSTAYPUT:
	case LIZTROOPSHOOT:
	case LIZTROOPJETPACK:
	case LIZTROOPDUCKING:
	case LIZTROOPRUNNING:
	case LIZTROOP:
	case OCTABRAIN:
	case COMMANDER:
	case COMMANDERSTAYPUT:
	case PIGCOP:
	case EGG:
	case PIGCOPSTAYPUT:
	case PIGCOPDIVE:
	case LIZMAN:
	case LIZMANSPITTING:
	case LIZMANFEEDING:
	case LIZMANJUMP:
	case ORGANTIC:
	case BOSS1:
	case BOSS2:
	case BOSS3:
	case BOSS4:
	case GREENSLIME:
	case GREENSLIME+1:
	case GREENSLIME+2:
	case GREENSLIME+3:
	case GREENSLIME+4:
	case GREENSLIME+5:
	case GREENSLIME+6:
	case GREENSLIME+7:
	case RAT:
	case ROTATEGUN:
		return 1;
	}
	if( actortype[s->picnum] ) return 1;

	return 0;
}


short badguypic(short pn)
{

	switch(pn)
	{
	case SHARK:
	case RECON:
	case DRONE:
	case LIZTROOPONTOILET:
	case LIZTROOPJUSTSIT:
	case LIZTROOPSTAYPUT:
	case LIZTROOPSHOOT:
	case LIZTROOPJETPACK:
	case LIZTROOPDUCKING:
	case LIZTROOPRUNNING:
	case LIZTROOP:
	case OCTABRAIN:
	case COMMANDER:
	case COMMANDERSTAYPUT:
	case PIGCOP:
	case EGG:
	case PIGCOPSTAYPUT:
	case PIGCOPDIVE:
	case LIZMAN:
	case LIZMANSPITTING:
	case LIZMANFEEDING:
	case LIZMANJUMP:
	case ORGANTIC:
	case BOSS1:
	case BOSS2:
	case BOSS3:
	case BOSS4:
	case GREENSLIME:
	case GREENSLIME+1:
	case GREENSLIME+2:
	case GREENSLIME+3:
	case GREENSLIME+4:
	case GREENSLIME+5:
	case GREENSLIME+6:
	case GREENSLIME+7:
	case RAT:
	case ROTATEGUN:
		return 1;
	}

	if( actortype[pn] ) return 1;

	return 0;
}



void myos(long x, long y, short tilenum, signed char shade, char orientation)
{
	char p;
	short a;

	if(orientation&4)
		a = 1024;
	else a = 0;

	p = sector[ps[screenpeek].cursectnum].floorpal;
	rotatesprite(x<<16,y<<16,65536L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);
}

void myospal(long x, long y, short tilenum, signed char shade, char orientation, char p)
{
	char fp;
	short a;

	if(orientation&4)
		a = 1024;
	else a = 0;

	fp = sector[ps[screenpeek].cursectnum].floorpal;

	rotatesprite(x<<16,y<<16,65536L,a,tilenum,shade,p,2|orientation,windowx1,windowy1,windowx2,windowy2);

}

void invennum(long x,long y,char num1,char ha,char sbits)
{
	char dabuf[80] = {0};
	sprintf(dabuf,"%ld",num1);
	if(num1 > 99)
	{
		rotatesprite((x-4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
		rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
		rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
	}
	else if(num1 > 9)
	{
		rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
		rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
	}
	else
		rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,sbits,0,0,xdim-1,ydim-1);
}

void orderweaponnum(short ind,long x,long y,long num1, long num2,char ha)
{
	rotatesprite((x-7)<<16,y<<16,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
	rotatesprite((x-3)<<16,y<<16,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);

	minitextshade(x+1,y-4,"ORDER",26,6,2+8+16+128);
}


void weaponnum(short ind,long x,long y,long num1, long num2,char ha)
{
	char dabuf[80] = {0};

	rotatesprite((x-7)<<16,y<<16,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
	rotatesprite((x-3)<<16,y<<16,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);
	rotatesprite((x+9)<<16,y<<16,65536L,0,THREEBYFIVE+11,ha,0,10+128,0,0,xdim-1,ydim-1);

	if(num1 > 99) num1 = 99;
	if(num2 > 99) num2 = 99;

	sprintf(dabuf,"%ld",num1);
	if(num1 > 9)
	{
		rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
		rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
	}
	else rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);

	sprintf(dabuf,"%ld",num2);
	if(num2 > 9)
	{
		rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
		rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
	}
	else rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
}

void weaponnum999(char ind,long x,long y,long num1, long num2,char ha)
{
	char dabuf[80] = {0};

	rotatesprite((x-7)<<16,y<<16,65536L,0,THREEBYFIVE+ind+1,ha-10,7,10+128,0,0,xdim-1,ydim-1);
	rotatesprite((x-4)<<16,y<<16,65536L,0,THREEBYFIVE+10,ha,0,10+128,0,0,xdim-1,ydim-1);
	rotatesprite((x+13)<<16,y<<16,65536L,0,THREEBYFIVE+11,ha,0,10+128,0,0,xdim-1,ydim-1);

	sprintf(dabuf,"%ld",num1);
	if(num1 > 99)
	{
		rotatesprite((x)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
		rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
		rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
	}
	else if(num1 > 9)
	{
		rotatesprite((x+4)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
		rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
	}
	else rotatesprite((x+8)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);

	sprintf(dabuf,"%ld",num2);
	if(num2 > 99)
	{
		rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
		rotatesprite((x+21)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
		rotatesprite((x+25)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[2]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
	}
	else if(num2 > 9)
	{
		rotatesprite((x+17)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
		rotatesprite((x+21)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[1]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
	}
	else rotatesprite((x+25)<<16,y<<16,65536L,0,THREEBYFIVE+dabuf[0]-'0',ha,0,10+128,0,0,xdim-1,ydim-1);
}


//REPLACE FULLY
void weapon_amounts(struct player_struct *p,long x,long y,long u)
{
	int cw;

	cw = p->curr_weapon;

	if (u&4)
	{
		if (u != 0xffffffff) patchstatusbar(96,178,96+12,178+6);
		weaponnum999(PISTOL_WEAPON,x,y,
			p->ammo_amount[PISTOL_WEAPON],max_ammo_amount[PISTOL_WEAPON],
			12-20*(cw == PISTOL_WEAPON) );
	}
	if (u&8)
	{
		if (u != 0xffffffff) patchstatusbar(96,184,96+12,184+6);
		weaponnum999(SHOTGUN_WEAPON,x,y+6,
			p->ammo_amount[SHOTGUN_WEAPON],max_ammo_amount[SHOTGUN_WEAPON],
			(!p->gotweapon[SHOTGUN_WEAPON]*9)+12-18*
			(cw == SHOTGUN_WEAPON) );
	}
	if (u&16)
	{
		if (u != 0xffffffff) patchstatusbar(96,190,96+12,190+6);
		weaponnum999(CHAINGUN_WEAPON,x,y+12,
			p->ammo_amount[CHAINGUN_WEAPON],max_ammo_amount[CHAINGUN_WEAPON],
			(!p->gotweapon[CHAINGUN_WEAPON]*9)+12-18*
			(cw == CHAINGUN_WEAPON) );
	}
	if (u&32)
	{
		if (u != 0xffffffff) patchstatusbar(135,178,135+8,178+6);
		weaponnum(RPG_WEAPON,x+39,y,
			p->ammo_amount[RPG_WEAPON],max_ammo_amount[RPG_WEAPON],
			(!p->gotweapon[RPG_WEAPON]*9)+12-19*
			(cw == RPG_WEAPON) );
	}
	if (u&64)
	{
		if (u != 0xffffffff) patchstatusbar(135,184,135+8,184+6);
		weaponnum(HANDBOMB_WEAPON,x+39,y+6,
			p->ammo_amount[HANDBOMB_WEAPON],max_ammo_amount[HANDBOMB_WEAPON],
			(((!p->ammo_amount[HANDBOMB_WEAPON])|(!p->gotweapon[HANDBOMB_WEAPON]))*9)+12-19*
			((cw == HANDBOMB_WEAPON) || (cw == HANDREMOTE_WEAPON)));
	}
	if (u&128)
	{
		if (u != 0xffffffff) patchstatusbar(135,190,135+8,190+6);


		if (VOLUMEONE)
		{
			orderweaponnum(SHRINKER_WEAPON,x+39,y+12,
				p->ammo_amount[SHRINKER_WEAPON],max_ammo_amount[SHRINKER_WEAPON],
				(!p->gotweapon[SHRINKER_WEAPON]*9)+12-18*
				(cw == SHRINKER_WEAPON) );
		}
		else
		{
			if(p->subweapon&(1<<GROW_WEAPON))
				weaponnum(SHRINKER_WEAPON,x+39,y+12,
				p->ammo_amount[GROW_WEAPON],max_ammo_amount[GROW_WEAPON],
				(!p->gotweapon[GROW_WEAPON]*9)+12-18*
				(cw == GROW_WEAPON) );
			else
				weaponnum(SHRINKER_WEAPON,x+39,y+12,
				p->ammo_amount[SHRINKER_WEAPON],max_ammo_amount[SHRINKER_WEAPON],
				(!p->gotweapon[SHRINKER_WEAPON]*9)+12-18*
				(cw == SHRINKER_WEAPON) );
		}
	}
	if (u&256)
	{
		if (u != 0xffffffff) patchstatusbar(166,178,166+8,178+6);

		if (VOLUMEONE)
		{
			orderweaponnum(DEVISTATOR_WEAPON,x+70,y,
				p->ammo_amount[DEVISTATOR_WEAPON],max_ammo_amount[DEVISTATOR_WEAPON],
				(!p->gotweapon[DEVISTATOR_WEAPON]*9)+12-18*
				(cw == DEVISTATOR_WEAPON) );
		}
		else
		{
			weaponnum(DEVISTATOR_WEAPON,x+70,y,
				p->ammo_amount[DEVISTATOR_WEAPON],max_ammo_amount[DEVISTATOR_WEAPON],
				(!p->gotweapon[DEVISTATOR_WEAPON]*9)+12-18*
				(cw == DEVISTATOR_WEAPON) );
		}
	}
	if (u&512)
	{
		if (u != 0xffffffff)
			patchstatusbar(166,184,166+8,184+6);
		if (VOLUMEONE)
		{
			orderweaponnum(TRIPBOMB_WEAPON,x+70,y+6,
				p->ammo_amount[TRIPBOMB_WEAPON],max_ammo_amount[TRIPBOMB_WEAPON],
				(!p->gotweapon[TRIPBOMB_WEAPON]*9)+12-18*
				(cw == TRIPBOMB_WEAPON) );
		}
		else
		{
			weaponnum(TRIPBOMB_WEAPON,x+70,y+6,
				p->ammo_amount[TRIPBOMB_WEAPON],max_ammo_amount[TRIPBOMB_WEAPON],
				(!p->gotweapon[TRIPBOMB_WEAPON]*9)+12-18*
				(cw == TRIPBOMB_WEAPON) );
		}
	}

	if (u&65536L)
	{
		if (u != 0xffffffff)
			patchstatusbar(166,190,166+8,190+6);
		if (VOLUMEONE)
		{
			orderweaponnum(-1,x+70,y+12,
				p->ammo_amount[FREEZE_WEAPON],max_ammo_amount[FREEZE_WEAPON],
				(!p->gotweapon[FREEZE_WEAPON]*9)+12-18*
				(cw == FREEZE_WEAPON) );
		}
		else
		{
			weaponnum(-1,x+70,y+12,
				p->ammo_amount[FREEZE_WEAPON],max_ammo_amount[FREEZE_WEAPON],
				(!p->gotweapon[FREEZE_WEAPON]*9)+12-18*
				(cw == FREEZE_WEAPON) );
		}
	}
}

void digitalnumber(long x,long y,long n,char s,char cs)
{
	short i, j, k, p, c;
	char b[10];

	ltoa(n,b,10);
	i = strlen(b);
	j = 0;

	for(k=0;k<i;k++)
	{
		p = DIGITALNUM+*(b+k)-'0';
		j += tilesizx[p]+1;
	}
	c = x-(j>>1);

	j = 0;
	for(k=0;k<i;k++)
	{
		p = DIGITALNUM+*(b+k)-'0';
		rotatesprite((c+j)<<16,y<<16,65536L,0,p,s,0,cs,0,0,xdim-1,ydim-1);
		j += tilesizx[p]+1;
	}
}

/*

void scratchmarks(long x,long y,long n,char s,char p)
{
long i, ni;

ni = n/5;
for(i=ni;i >= 0;i--)
{
overwritesprite(x-2,y,SCRATCH+4,s,0,0);
x += tilesizx[SCRATCH+4]-1;
}

ni = n%5;
if(ni) overwritesprite(x,y,SCRATCH+ni-1,s,p,0);
}
*/
void displayinventory(struct player_struct *p)
{
	short n, j, xoff, y;

	j = xoff = 0;

	n = (p->jetpack_amount > 0)<<3; if(n&8) j++;
	n |= ( p->scuba_amount > 0 )<<5; if(n&32) j++;
	n |= (p->steroids_amount > 0)<<1; if(n&2) j++;
	n |= ( p->holoduke_amount > 0)<<2; if(n&4) j++;
	n |= (p->firstaid_amount > 0); if(n&1) j++;
	n |= (p->heat_amount > 0)<<4; if(n&16) j++;
	n |= (p->boot_amount > 0)<<6; if(n&64) j++;

	xoff = 160-(j*11);

	j = 0;

	if(ud.screen_size > 4)
		y = 154;
	else y = 172;

	if(ud.screen_size == 4)
	{
		if(ud.multimode > 1)
			xoff += 56;
		else xoff += 65;
	}

	while( j <= 9 )
	{
		if( n&(1<<j) )
		{
			switch( n&(1<<j) )
			{
			case   1:
				rotatesprite(xoff<<16,y<<16,65536L,0,FIRSTAID_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
			case   2:
				rotatesprite((xoff+1)<<16,y<<16,65536L,0,STEROIDS_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
			case   4:
				rotatesprite((xoff+2)<<16,y<<16,65536L,0,HOLODUKE_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
			case   8:
				rotatesprite(xoff<<16,y<<16,65536L,0,JETPACK_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
			case  16:
				rotatesprite(xoff<<16,y<<16,65536L,0,HEAT_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
			case  32:
				rotatesprite(xoff<<16,y<<16,65536L,0,AIRTANK_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
			case 64:
				rotatesprite(xoff<<16,(y-1)<<16,65536L,0,BOOT_ICON,0,0,2+16,windowx1,windowy1,windowx2,windowy2);break;
			}

			xoff += 22;

			if(p->inven_icon == j+1)
				rotatesprite((xoff-2)<<16,(y+19)<<16,65536L,1024,ARROW,-32,0,2+16,windowx1,windowy1,windowx2,windowy2);
		}

		j++;
	}
}



void displayfragbar(void)
{
	short i, j;

	j = 0;

	for(i=connecthead;i>=0;i=connectpoint2[i])
		if(i > j) j = i;

	rotatesprite(0,0,65600L,0,FRAGBAR,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
	if(j >= 4) rotatesprite(319,(8)<<16,65600L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
	if(j >= 8) rotatesprite(319,(16)<<16,65600L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);
	if(j >= 12) rotatesprite(319,(24)<<16,65600L,0,FRAGBAR,0,0,10+16+64+128,0,0,xdim-1,ydim-1);

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		minitext(21+(73*(i&3)),2+((i&28)<<1),&ud.user_name[i][0],sprite[ps[i].i].pal,2+8+16+128);
		sprintf((char*)tempbuf,"%d",ps[i].frag-ps[i].fraggedself);
		minitext(17+50+(73*(i&3)),2+((i&28)<<1),(char*)tempbuf,sprite[ps[i].i].pal,2+8+16+128);
	}
}

void coolgaugetext(short snum)
{
	struct player_struct *p;
	long i, j, o, ss, u;
	char permbit;

	p = &ps[snum];

	if (p->invdisptime > 0) displayinventory(p);


	if(ps[snum].gm&MODE_MENU)
		if( (current_menu >= 400  && current_menu <= 405) )
			return;

	ss = ud.screen_size; if (ss < 4) return;

	if ( ud.multimode > 1 && ud.coop != 1 )
	{
		if (pus)
		{ displayfragbar(); }
		else
		{
			for(i=connecthead;i>=0;i=connectpoint2[i])
				if (ps[i].frag != sbar.frag[i]) { displayfragbar(); break; }
		}
		for(i=connecthead;i>=0;i=connectpoint2[i])
			if (i != myconnectindex)
				sbar.frag[i] = ps[i].frag;
	}

	if (ss == 4)   //DRAW MINI STATUS BAR:
	{
		rotatesprite(5<<16,(200-28)<<16,65536L,0,HEALTHBOX,0,21,10+16,0,0,xdim-1,ydim-1);
		if (p->inven_icon)
			rotatesprite(69<<16,(200-30)<<16,65536L,0,INVENTORYBOX,0,21,10+16,0,0,xdim-1,ydim-1);

		if(sprite[p->i].pal == 1 && p->last_extra < 2)
			digitalnumber(20,200-17,1,-16,10+16);
		else digitalnumber(20,200-17,p->last_extra,-16,10+16);

		rotatesprite(37<<16,(200-28)<<16,65536L,0,AMMOBOX,0,21,10+16,0,0,xdim-1,ydim-1);

		if (p->curr_weapon == HANDREMOTE_WEAPON) i = HANDBOMB_WEAPON; else i = p->curr_weapon;
		digitalnumber(53,200-17,p->ammo_amount[i],-16,10+16);

		o = 158; permbit = 0;
		if (p->inven_icon)
		{
			switch(p->inven_icon)
			{
			case 1: i = FIRSTAID_ICON; break;
			case 2: i = STEROIDS_ICON; break;
			case 3: i = HOLODUKE_ICON; break;
			case 4: i = JETPACK_ICON; break;
			case 5: i = HEAT_ICON; break;
			case 6: i = AIRTANK_ICON; break;
			case 7: i = BOOT_ICON; break;
			default: i = -1;
			}
			if (i >= 0) rotatesprite((231-o)<<16,(200-21)<<16,65536L,0,i,0,0,10+16+permbit,0,0,xdim-1,ydim-1);

			minitext(292-30-o,190,"%",6,10+16+permbit);

			j = 0x80000000;
			switch(p->inven_icon)
			{
			case 1: i = p->firstaid_amount; break;
			case 2: i = ((p->steroids_amount+3)>>2); break;
			case 3: i = ((p->holoduke_amount+15)/24); j = p->holoduke_on; break;
			case 4: i = ((p->jetpack_amount+15)>>4); j = p->jetpack_on; break;
			case 5: i = p->heat_amount/12; j = p->heat_on; break;
			case 6: i = ((p->scuba_amount+63)>>6); break;
			case 7: i = (p->boot_amount>>1); break;
			}
			invennum(284-30-o,200-6,(char)i,0,10+permbit);
			if (j > 0) minitext(288-30-o,180,"ON",0,10+16+permbit);
			else if (j != 0x80000000) minitext(284-30-o,180,"OFF",2,10+16+permbit);
			if (p->inven_icon >= 6) minitext(284-35-o,180,"AUTO",2,10+16+permbit);
		}
		return;
	}

	//DRAW/UPDATE FULL STATUS BAR:

	if (pus) { pus = 0; u = 0xffffffff; } else u = 0;

	if (sbar.frag[myconnectindex] != p->frag) { sbar.frag[myconnectindex] = p->frag; u |= 32768; }
	if (sbar.got_access != p->got_access) { sbar.got_access = p->got_access; u |= 16384; }
	if (sbar.last_extra != p->last_extra) { sbar.last_extra = p->last_extra; u |= 1; }
	if (sbar.shield_amount != p->shield_amount) { sbar.shield_amount = p->shield_amount; u |= 2; }
	if (sbar.curr_weapon != p->curr_weapon) { sbar.curr_weapon = p->curr_weapon; u |= (4+8+16+32+64+128+256+512+1024+65536L); }
	for(i=1;i < 10;i++)
	{
		if (sbar.ammo_amount[i] != p->ammo_amount[i]) {
			sbar.ammo_amount[i] = p->ammo_amount[i]; if(i < 9) u |= ((2<<i)+1024); else u |= 65536L+1024; }
		if (sbar.gotweapon[i] != p->gotweapon[i]) { sbar.gotweapon[i] =
			p->gotweapon[i]; if(i < 9 ) u |= ((2<<i)+1024); else u |= 65536L+1024; }
	}
	if (sbar.inven_icon != p->inven_icon) { sbar.inven_icon = p->inven_icon; u |= (2048+4096+8192); }
	if (sbar.holoduke_on != p->holoduke_on) { sbar.holoduke_on = p->holoduke_on; u |= (4096+8192); }
	if (sbar.jetpack_on != p->jetpack_on) { sbar.jetpack_on = p->jetpack_on; u |= (4096+8192); }
	if (sbar.heat_on != p->heat_on) { sbar.heat_on = p->heat_on; u |= (4096+8192); }
	if (sbar.firstaid_amount != p->firstaid_amount) { sbar.firstaid_amount = p->firstaid_amount; u |= 8192; }
	if (sbar.steroids_amount != p->steroids_amount) { sbar.steroids_amount = p->steroids_amount; u |= 8192; }
	if (sbar.holoduke_amount != p->holoduke_amount) { sbar.holoduke_amount = p->holoduke_amount; u |= 8192; }
	if (sbar.jetpack_amount != p->jetpack_amount) { sbar.jetpack_amount = p->jetpack_amount; u |= 8192; }
	if (sbar.heat_amount != p->heat_amount) { sbar.heat_amount = p->heat_amount; u |= 8192; }
	if (sbar.scuba_amount != p->scuba_amount) { sbar.scuba_amount = p->scuba_amount; u |= 8192; }
	if (sbar.boot_amount != p->boot_amount) { sbar.boot_amount = p->boot_amount; u |= 8192; }
	if (u == 0) return;

	//0 - update health
	//1 - update armor
	//2 - update PISTOL_WEAPON ammo
	//3 - update SHOTGUN_WEAPON ammo
	//4 - update CHAINGUN_WEAPON ammo
	//5 - update RPG_WEAPON ammo
	//6 - update HANDBOMB_WEAPON ammo
	//7 - update SHRINKER_WEAPON ammo
	//8 - update DEVISTATOR_WEAPON ammo
	//9 - update TRIPBOMB_WEAPON ammo
	//10 - update ammo display
	//11 - update inventory icon
	//12 - update inventory on/off
	//13 - update inventory %
	//14 - update keys
	//15 - update kills
	//16 - update FREEZE_WEAPON ammo

	if (u == 0xffffffff)
	{
		patchstatusbar(0,0,320,200);
		if (ud.multimode > 1 && ud.coop != 1)
			rotatesprite(277<<16,(200-27)<<16,65536L,0,KILLSICON,0,0,10+16+128,0,0,xdim-1,ydim-1);
	}
	if (ud.multimode > 1 && ud.coop != 1)
	{
		if (u&32768)
		{
			if (u != 0xffffffff) patchstatusbar(276,183,299,193);
			digitalnumber(287,200-17,max(p->frag-p->fraggedself,0),-16,10+16+128);
		}
	}
	else
	{
		if (u&16384)
		{
			if (u != 0xffffffff) patchstatusbar(275,182,299,194);
			if (p->got_access&4) rotatesprite(275<<16,182<<16,65536L,0,ACCESS_ICON,0,23,10+16+128,0,0,xdim-1,ydim-1);
			if (p->got_access&2) rotatesprite(288<<16,182<<16,65536L,0,ACCESS_ICON,0,21,10+16+128,0,0,xdim-1,ydim-1);
			if (p->got_access&1) rotatesprite(281<<16,189<<16,65536L,0,ACCESS_ICON,0,0,10+16+128,0,0,xdim-1,ydim-1);
		}
	}
	if (u&(4+8+16+32+64+128+256+512+65536L)) weapon_amounts(p,96,182,u);

	if (u&1)
	{
		if (u != 0xffffffff) patchstatusbar(20,183,43,193);
		if(sprite[p->i].pal == 1 && p->last_extra < 2)
			digitalnumber(32,200-17,1,-16,10+16+128);
		else digitalnumber(32,200-17,p->last_extra,-16,10+16+128);
	}
	if (u&2)
	{
		if (u != 0xffffffff) patchstatusbar(52,183,75,193);
		digitalnumber(64,200-17,p->shield_amount,-16,10+16+128);
	}

	if (u&1024)
	{
		if (u != 0xffffffff) patchstatusbar(196,183,219,193);
		if (p->curr_weapon != KNEE_WEAPON)
		{
			if (p->curr_weapon == HANDREMOTE_WEAPON) i = HANDBOMB_WEAPON; else i = p->curr_weapon;
			digitalnumber(230-22,200-17,p->ammo_amount[i],-16,10+16+128);
		}
	}

	if (u&(2048+4096+8192))
	{
		if (u != 0xffffffff)
		{
			if (u&(2048+4096)) { patchstatusbar(231,179,265,197); }
			else { patchstatusbar(250,190,261,195); }
		}
		if (p->inven_icon)
		{
			o = 0; permbit = 128;

			if (u&(2048+4096))
			{
				switch(p->inven_icon)
				{
				case 1: i = FIRSTAID_ICON; break;
				case 2: i = STEROIDS_ICON; break;
				case 3: i = HOLODUKE_ICON; break;
				case 4: i = JETPACK_ICON; break;
				case 5: i = HEAT_ICON; break;
				case 6: i = AIRTANK_ICON; break;
				case 7: i = BOOT_ICON; break;
				}
				rotatesprite((231-o)<<16,(200-21)<<16,65536L,0,i,0,0,10+16+permbit,0,0,xdim-1,ydim-1);
				minitext(292-30-o,190,"%",6,10+16+permbit);
				if (p->inven_icon >= 6) minitext(284-35-o,180,"AUTO",2,10+16+permbit);
			}
			if (u&(2048+4096))
			{
				switch(p->inven_icon)
				{
				case 3: j = p->holoduke_on; break;
				case 4: j = p->jetpack_on; break;
				case 5: j = p->heat_on; break;
				default: j = 0x80000000;
				}
				if (j > 0) minitext(288-30-o,180,"ON",0,10+16+permbit);
				else if (j != 0x80000000) minitext(284-30-o,180,"OFF",2,10+16+permbit);
			}
			if (u&8192)
			{
				switch(p->inven_icon)
				{
				case 1: i = p->firstaid_amount; break;
				case 2: i = ((p->steroids_amount+3)>>2); break;
				case 3: i = ((p->holoduke_amount+15)/24); break;
				case 4: i = ((p->jetpack_amount+15)>>4); break;
				case 5: i = p->heat_amount/12; break;
				case 6: i = ((p->scuba_amount+63)>>6); break;
				case 7: i = (p->boot_amount>>1); break;
				}
				invennum(284-30-o,200-6,(char)i,0,10+permbit);
			}
		}
	}
}


#define AVERAGEFRAMES 16
static long frameval[AVERAGEFRAMES], framecnt = 0;

void tics(void)
{
	long i;
	char b[10];

	i = totalclock;
	if (i != frameval[framecnt])
	{
		sprintf(b,"%ld",(TICRATE*AVERAGEFRAMES)/(i-frameval[framecnt]));
		printext256(windowx1,windowy1,31,-21,b,1);
		frameval[framecnt] = i;
	}

	framecnt = ((framecnt+1)&(AVERAGEFRAMES-1));
}

void coords(short snum)
{
	short y = 0;

	if(ud.coop != 1)
	{
		if(ud.multimode > 1 && ud.multimode < 5)
			y = 8;
		else if(ud.multimode > 4)
			y = 16;
	}

	sprintf((char*)tempbuf,"X= %ld",ps[snum].posx);
	printext256(250L,y,31,-1,(char*)tempbuf,1);
	sprintf((char*)tempbuf,"Y= %ld",ps[snum].posy);
	printext256(250L,y+7L,31,-1,(char*)tempbuf,1);
	sprintf((char*)tempbuf,"Z= %ld",ps[snum].posz);
	printext256(250L,y+14L,31,-1,(char*)tempbuf,1);
	sprintf((char*)tempbuf,"A= %ld",ps[snum].ang);
	printext256(250L,y+21L,31,-1,(char*)tempbuf,1);
	sprintf((char*)tempbuf,"ZV= %ld",ps[snum].poszv);
	printext256(250L,y+28L,31,-1,(char*)tempbuf,1);
	sprintf((char*)tempbuf,"OG= %ld",ps[snum].on_ground);
	printext256(250L,y+35L,31,-1,(char*)tempbuf,1);
	sprintf((char*)tempbuf,"AM= %ld",ps[snum].ammo_amount[GROW_WEAPON]);
	printext256(250L,y+43L,31,-1,(char*)tempbuf,1);
	sprintf((char*)tempbuf,"LFW= %ld",ps[snum].last_full_weapon);
	printext256(250L,y+50L,31,-1,(char*)tempbuf,1);
	sprintf((char*)tempbuf,"SECTL= %ld",sector[ps[snum].cursectnum].lotag);
	printext256(250L,y+57L,31,-1,(char*)tempbuf,1);
	sprintf((char*)tempbuf,"SEED= %ld",randomseed);
	printext256(250L,y+64L,31,-1,(char*)tempbuf,1);
	sprintf((char*)tempbuf,"THOLD= %ld",ps[snum].transporter_hold);
	printext256(250L,y+64L+7,31,-1,(char*)tempbuf,1);
}

void operatefta(void)
{
	long i, j, k;

	// Draw user quotes (i.e. chat messages) at the bottom of the screen
	if(ud.screen_size > 0) j = 200-45; else j = 200-8;
	quotebot = min(quotebot,j);
	quotebotgoal = min(quotebotgoal,j);
	if(ps[myconnectindex].gm&MODE_TYPE) j -= 8;
	quotebotgoal = j; j = quotebot;
	for(i=0;i<MAXUSERQUOTES;i++)
	{
		k = user_quote_time[i]; if (k <= 0) break;
		short bits = 2+8+16;

		if (k <= 2)
		{
			bits += 1+32;
		}
		else if (k <= 4)
		{
			bits += 1;
		}

		gametext(320>>1,j,user_quote[i],0,bits);
		j -= 8;
	}

	if (ps[screenpeek].fta <= 1) return;

	if (ps[screenpeek].ftq == 115 || ps[screenpeek].ftq == 116)
	{ // Killed and Killed By messages go at the bottom
		k = j - 4;

		j = ps[screenpeek].fta;
		if (j > 4)
			gametext(320>>1,k,fta_quotes[ps[screenpeek].ftq],0,2+8+16);
		else if (j > 2)
			gametext(320>>1,k,fta_quotes[ps[screenpeek].ftq],0,2+8+16+1);
		else
			gametext(320>>1,k,fta_quotes[ps[screenpeek].ftq],0,2+8+16+1+32);
	}
}

void FTA(short q,struct player_struct *p,bool noDups)
{
	if (q != 115 && q != 116)
	{
		if (screenpeek == p - ps)
		{
			C_AddQuote (q, noDups);
		}
	}

	if( ud.fta_on == 1)
	{
		if( p->fta > 0 && q != 115 && q != 116 )
			if( p->ftq == 115 || p->ftq == 116 ) return;

		p->fta = 100;

		if( p->ftq != q || q == 26 )
			// || q == 26 || q == 115 || q ==116 || q == 117 || q == 122 )
		{
			p->ftq = q;
			pub = NUMPAGES;
			pus = NUMPAGES;
		}
	}
}

void showtwoscreens(void)
{
	// [RH] No reason why we can't have this fade out
	setview(0,0,xdim-1,ydim-1);
	flushperms();
	ps[myconnectindex].palette = palette;
	FadeOut (7);

	if (!VOLUMEALL)
	{
		D_AddResponder (Logo_Responder, 50);
		ClearSkipLogoTime ();
		rotatesprite(0,0,65536L,0,3291,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
		FadeIn (7);
		while( !CheckSkipLogoTime() ) getpackets();

		FadeOut (7);
		ClearSkipLogoTime ();
		rotatesprite(0,0,65536L,0,3290,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
		FadeIn (7);
		while( !CheckSkipLogoTime() ) getpackets();
		FadeOut (7);
		D_RemoveResponder (Logo_Responder);
	}
	// CTW - REMOVED
	/*
	else
	{
		KB_FlushKeyboardQueue();
		clearview(0L);
		rotatesprite(0,0,65536L,0,TENSCREEN,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
		FadeIn (7);
		totalclock = 0;
		while( !KB_KeyWaiting() && totalclock < 2400) getpackets();
	}*/
	// CTW END - REMOVED
}

void binscreen(void)
{
	long fil;

	fil = kopen4load(VOLUMEONE ? "dukesw.bin" : "duke3d.bin",1);
	if(fil == -1) return;
	//kread(fil,(char *)0xb8000,4000);
	kclose(fil);
}


void gameexit(char *t)
{
	if(*t != 0) ps[myconnectindex].palette = (char *) &palette[0];

	if(numplayers > 1)
	{
		allowtimetocorrecterrorswhenquitting();
	}

	if(ud.recstat == 1) closedemowrite();
	else if(ud.recstat == 2) { kclose(recfilep); }

	if(qe)
		goto GOTOHERE;

	if(playerswhenstarted > 1 && ud.coop != 1 && *t == ' ')
	{
		dobonus(1);
		// CTW - MODIFICATION
		//      setgamemode();
		//[RH] Why?
		//setgamemode(ScreenMode,ScreenWidth,ScreenHeight);
		// CTW END - MODIFICATION
	}

	if (ONELEVELDEMO)
	{
		doorders();
	}

	if (playerswhenstarted > 1)
		uninitmultiplayers();

	// CTW - MODIFICATION
	/*  if( *t != 0 && *(t+1) != 'V' && *(t+1) != 'Y' && playonten == 0 )
	showtwoscreens();*/
	if( *t != 0 && *(t+1) != 'V' && *(t+1) != 'Y' && xdim != 0)
	{
		showtwoscreens();
	}
	// CTW END - MODIFICATION

GOTOHERE:

	ShutDown();

	uninitgroupfile();

	unlink("duke3d.tmp");

	if (ONELEVELDEMO)
	{
		I_OkayMessage ("You have been playing a ONE LEVEL demo of Duke Nukem 3D.");
	}
	else if(*t != 0)
	{
		//        setvmode(0x3);
		binscreen();
		// CTW - MODIFICATION
		/*      if(playonten == 0)
		{
		if(*t == ' ' && *(t+1) == 0) *t = 0;
		printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
		printf("%s%s","\n",t);
		}*/
		if(*t == ' ' && *(t+1) == 0)
			t = "";
		I_FatalError ("%s", t);
		// CTW END - MODIFICATION        
	}

	exit(0);
}




short inputloc = 0;

#define MAX_TYPE 128
char TypeQueue[MAX_TYPE];
int TypePos = 0;

bool Type_Responder (const event_t *ev)
{
	if (!(ps[myconnectindex].gm & MODE_TYPE) &&
		(!(ps[myconnectindex].gm & MODE_MENU) ||
		 (current_menu != 10001 &&	// enter password menu
		 (current_menu < 360 || current_menu > 369))))	// save game menus
	{
		TypePos = 0;
		return false;
	}

	if (ev->type != EV_GUI_Event ||
		(ev->subtype != EV_GUI_KeyDown &&
		 ev->subtype != EV_GUI_KeyRepeat))
	{
		return false;
	}

	if (TypePos < MAX_TYPE)
	{
		TypeQueue[TypePos++] = ev->data1;
	}

	return true;
}

short strget(short x,short y,char *t,short dalen,short c)
{
	short ch;
	int i;

	for (i = 0; i < TypePos; ++i)
	{
		ch = TypeQueue[i];

		if(ch == GK_BACKSPACE)
		{
			if( inputloc > 0 )
			{
				inputloc--;
				*(t+inputloc) = 0;
			}
		}
		else
		{
			if(ch == GK_RETURN)
			{
				inputloc = 0;
				return (1);
			}
			else if(ch == GK_ESCAPE)
			{
				inputloc = 0;
				return (-1);
			}
			else if ( ch >= 32 && inputloc < dalen && ch < 127)
			{
				ch = toupper(ch);
				*(t+inputloc) = ch;
				*(t+inputloc+1) = 0;
				inputloc++;
			}
		}
	}

	TypePos = 0;

	if( c == 999 ) return(0);
	if( c == 998 )
	{
		char b[41],ii;
		for(ii=0;ii<inputloc;ii++)
			b[ii] = '*';
		b[ii] = 0;
		x = gametext(x,y,b,c,2+8+16);
	}
	else x = gametext(x,y,t,c,2+8+16);
	c = 4-(sintable[(totalclock<<4)&2047]>>11);
	rotatesprite((x+8)<<16,(y+4)<<16,32768L,0,SPINNINGNUKEICON+((totalclock>>3)%7),c,0,2+8,0,0,xdim-1,ydim-1);

	return (0);
}

void typemode(void)
{
	short ch, hitstate, i, j;

	if( ps[myconnectindex].gm&MODE_SENDTOWHOM )
	{
		if(sendmessagecommand != -1 || ud.multimode < 3 || movesperpacket == 4)
		{
			tempbuf[0] = 4;
			tempbuf[1] = 0;
			recbuf[0]  = 0;

			if(ud.multimode < 3)
				sendmessagecommand = 2;

			strcat(recbuf,ud.user_name[myconnectindex]);
			strcat(recbuf,": ");
			strcat(recbuf,typebuf);
			j = strlen(recbuf);
			recbuf[j] = 0;
			strcat((char*)tempbuf+1,recbuf);

			if(sendmessagecommand >= ud.multimode || movesperpacket == 4)
			{
				for(ch=connecthead;ch >= 0;ch=connectpoint2[ch])
					if (ch != myconnectindex)
						sendpacket(ch,tempbuf,j+1);

				adduserquote(recbuf);
				quotebot += 8;
				quotebotgoal = quotebot;
			}
			else if(sendmessagecommand >= 0)
				sendpacket(sendmessagecommand,tempbuf,j+1);

			sendmessagecommand = -1;
			ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
		}
		else if(sendmessagecommand == -1)
		{
			j = 50;
			gametext(320>>1,j,"SEND MESSAGE TO...",0,2+8+16); j += 8;
			for(i=connecthead;i>=0;i=connectpoint2[i])
				//                for(i=0;i<ud.multimode;i++)
			{
				if (i == myconnectindex)
				{
					minitextshade((320>>1)-40+1,j+1,"A/ENTER - ALL",26,0,2+8+16);
					minitext((320>>1)-40,j,"A/ENTER - ALL",0,2+8+16); j += 7;
				}
				else
				{
					sprintf(buf,"      %ld - %s",i+1,ud.user_name[i]);
					minitextshade((320>>1)-40-6+1,j+1,buf,26,0,2+8+16);
					minitext((320>>1)-40-6,j,buf,0,2+8+16); j += 7;
				}
			}
			minitextshade((320>>1)-40-4+1,j+1,"    ESC - Abort",26,0,2+8+16);
			minitext((320>>1)-40-4,j,"    ESC - Abort",0,2+8+16); j += 7;

			//sprintf(buf,"PRESS 1-%ld FOR INDIVIDUAL PLAYER.",ud.multimode);
			//gametext(320>>1,j,buf,0,2+8+16); j += 8;
			//gametext(320>>1,j,"'A' OR 'ENTER' FOR ALL PLAYERS",0,2+8+16); j += 8;
			//gametext(320>>1,j,"ESC ABORTS",0,2+8+16); j += 8;

			if (ud.screen_size > 0) j = 200-45; else j = 200-8;
			gametext(320>>1,j,typebuf,0,2+8+16);

			if( TypePos > 0 )
			{
				i = TypeQueue[0];

				if(i == 'A' || i == 'a' || i == GK_RETURN)
					sendmessagecommand = ud.multimode;
				else if(i >= '1' || i <= (ud.multimode + '1') )
					sendmessagecommand = i - '1';
				else
				{
					sendmessagecommand = ud.multimode;
					if(i == GK_ESCAPE)
					{
						ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
						sendmessagecommand = -1;
					}
					else
						typebuf[0] = 0;
				}
				TypePos = 0;
			}
		}
	}
	else
	{
		if(ud.screen_size > 0) j = 200-45; else j = 200-8;
		hitstate = strget(320>>1,j,typebuf,30,1);

		if(hitstate == 1)
		{
			TypePos = 0;
			ps[myconnectindex].gm |= MODE_SENDTOWHOM;
		}
		else if(hitstate == -1)
		{
			ps[myconnectindex].gm &= ~(MODE_TYPE|MODE_SENDTOWHOM);
		}
		else pub = NUMPAGES;
	}
}

void moveclouds(void)
{
	if( totalclock > cloudtotalclock || totalclock < (cloudtotalclock-7))
	{
		short i;

		cloudtotalclock = totalclock+6;

		for(i=0;i<numclouds;i++)
		{
			cloudx[i] += (sintable[(ps[screenpeek].ang+512)&2047]>>9);
			cloudy[i] += (sintable[ps[screenpeek].ang&2047]>>9);

			sector[clouds[i]].ceilingxpanning = cloudx[i]>>6;
			sector[clouds[i]].ceilingypanning = cloudy[i]>>6;
		}
	}
}


void displayrest(long smoothratio)
{
	long a, i, j;

	struct player_struct *pp;
	walltype *wal;
	long cposx,cposy,cang;

	pp = &ps[screenpeek];

	if( pp->pals_time > 0 && pp->loogcnt == 0)
	{
		palto( pp->pals[0],
			pp->pals[1],
			pp->pals[2],
			pp->pals_time|128);

		restorepalette = 1;
	}
	else if( restorepalette )
	{
		setbrightness(ud.brightness>>2,&pp->palette[0]);
		restorepalette = 0;
	}
	else if(pp->loogcnt > 0) palto(0,64,0,(pp->loogcnt>>1)+128);

	if(ud.show_help)
	{
		switch(ud.show_help)
		{
		case 1:
			rotatesprite(0,0,65536L,0,TEXTSTORY,0,0,10+16+64, 0,0,xdim-1,ydim-1);
			break;
		case 2:
			rotatesprite(0,0,65536L,0,F1HELP,0,0,10+16+64, 0,0,xdim-1,ydim-1);
			break;
		}

		// [RH] Draw the console
		if (!(ps[myconnectindex].gm & MODE_MENU))
		{
			C_DrawConsole ();
		}

		return;
	}

	i = pp->cursectnum;

	show2dsector[i>>3] |= (1<<(i&7));
	wal = &wall[sector[i].wallptr];
	for(j=sector[i].wallnum;j>0;j--,wal++)
	{
		i = wal->nextsector;
		if (i < 0) continue;
		if (wal->cstat&(WSTAT_ALLBLOCKING|WSTAT_MASKED|WSTAT_ONEWAY)) continue;
		if (wall[wal->nextwall].cstat&(WSTAT_ALLBLOCKING|WSTAT_MASKED|WSTAT_ONEWAY)) continue;
		if (sector[i].lotag == 32767) continue;
		if (sector[i].ceilingz >= sector[i].floorz) continue;
		show2dsector[i>>3] |= (1<<(i&7));
	}

	if(ud.camerasprite == -1)
	{
		if( ud.overhead_on != 2 )
		{
			if(pp->newowner >= 0)
				cameratext(pp->newowner);
			else
			{
				displayweapon(screenpeek);
				if(pp->over_shoulder_on == 0 )
					displaymasks(screenpeek);
			}
			moveclouds();
		}

		if( ud.overhead_on > 0 )
		{
			smoothratio = min(max(smoothratio,0),65536);
			dointerpolations(smoothratio);
			if( ud.scrollmode == 0 )
			{
				if(pp->newowner == -1)
				{
					if (screenpeek == myconnectindex && numplayers > 1)
					{
						cposx = omyx+mulscale16((long)(myx-omyx),smoothratio);
						cposy = omyy+mulscale16((long)(myy-omyy),smoothratio);
						cang = omyang+mulscale16((long)(((myang+1024-omyang)&2047)-1024),smoothratio);
					}
					else
					{
						cposx = pp->oposx+mulscale16((long)(pp->posx-pp->oposx),smoothratio);
						cposy = pp->oposy+mulscale16((long)(pp->posy-pp->oposy),smoothratio);
						cang = pp->oang+mulscale16((long)(((pp->ang+1024-pp->oang)&2047)-1024),smoothratio);
					}
				}
				else
				{
					cposx = pp->oposx;
					cposy = pp->oposy;
					cang = pp->oang;
				}
			}
			else
			{

				ud.fola += ud.folavel>>3;
				ud.folx += (ud.folfvel*sintable[(512+2048-ud.fola)&2047])>>14;
				ud.foly += (ud.folfvel*sintable[(512+1024-512-ud.fola)&2047])>>14;

				cposx = ud.folx;
				cposy = ud.foly;
				cang = ud.fola;
			}

			if(ud.overhead_on == 2)
			{
				clearview(0L);
				drawmapview(cposx,cposy,pp->zoom,cang);
			}
			drawoverheadmap( cposx,cposy,pp->zoom,cang);

			restoreinterpolations();

			if(ud.overhead_on == 2)
			{
				if(ud.screen_size > 0) a = 147;
				else a = 182;

				minitext(1,a+6,volume_names[ud.volume_number],0,2+8+16);
				minitext(1,a+12,level_names[ud.volume_number*11 + ud.level_number],0,2+8+16);
			}
		}
	}

	coolgaugetext(screenpeek);
	S_NoiseDebug();
	operatefta();

	if(ps[myconnectindex].newowner == -1 && ud.overhead_on == 0 && ud.crosshair && ud.camerasprite == -1)
		rotatesprite((160L-(ps[myconnectindex].look_ang>>1))<<16,100L<<16,65536L,0,CROSSHAIR,0,0,2+1,windowx1,windowy1,windowx2,windowy2);

	if(ps[myconnectindex].gm&MODE_TYPE)
		typemode();
	else
		menus();

	if( ud.pause_on==1 && (ps[myconnectindex].gm&MODE_MENU) == 0 )
		menutext(160,100,0,0,"GAME PAUSED");

	if(ud.coords)
		coords(screenpeek);

	// [RH] Draw the console
	if (!(ps[myconnectindex].gm & MODE_MENU))
	{
		C_DrawConsole ();
	}

	if(ud.tickrate)
		tics();
}


void updatesectorz(long x, long y, long z, short *sectnum)
{
	walltype *wal;
	long i, j, cz, fz;

	getzsofslope(*sectnum,x,y,&cz,&fz);
	if ((z >= cz) && (z <= fz))
		if (inside(x,y,*sectnum) != 0) return;

	if ((*sectnum >= 0) && (*sectnum < numsectors))
	{
		wal = &wall[sector[*sectnum].wallptr];
		j = sector[*sectnum].wallnum;
		do
		{
			i = wal->nextsector;
			if (i >= 0)
			{
				getzsofslope(i,x,y,&cz,&fz);
				if ((z >= cz) && (z <= fz))
					if (inside(x,y,(short)i) == 1)
					{ *sectnum = i; return; }
			}
			wal++; j--;
		} while (j != 0);
	}

	for(i=numsectors-1;i>=0;i--)
	{
		getzsofslope(i,x,y,&cz,&fz);
		if ((z >= cz) && (z <= fz))
			if (inside(x,y,(short)i) == 1)
			{ *sectnum = i; return; }
	}

	*sectnum = -1;
}

void view(struct player_struct *pp, long *vx, long *vy,long *vz,short *vsectnum, short ang, short horiz)
{
	spritetype *sp;
	long i, nx, ny, nz, hx, hy, hitx, hity, hitz;
	short bakcstat, hitsect, hitwall, hitsprite, daang;

	nx = (sintable[(ang+1536)&2047]>>4);
	ny = (sintable[(ang+1024)&2047]>>4);
	nz = (horiz-100)*128;

	sp = &sprite[pp->i];

	bakcstat = sp->cstat;
	sp->cstat &= ~CSTAT_ALLBLOCKING;

	updatesectorz(*vx,*vy,*vz,vsectnum);
	hitscan(*vx,*vy,*vz,*vsectnum,nx,ny,nz,&hitsect,&hitwall,&hitsprite,&hitx,&hity,&hitz,CLIPMASK1);

	if(*vsectnum < 0)
	{
		sp->cstat = bakcstat;
		return;
	}

	hx = hitx-(*vx); hy = hity-(*vy);
	if (klabs(nx)+klabs(ny) > klabs(hx)+klabs(hy))
	{
		*vsectnum = hitsect;
		if (hitwall >= 0)
		{
			daang = getangle(wall[wall[hitwall].point2].x-wall[hitwall].x,
				wall[wall[hitwall].point2].y-wall[hitwall].y);

			i = nx*sintable[daang]+ny*sintable[(daang+1536)&2047];
			if (klabs(nx) > klabs(ny)) hx -= mulscale28(nx,i);
			else hy -= mulscale28(ny,i);
		}
		else if (hitsprite < 0)
		{
			if (klabs(nx) > klabs(ny)) hx -= (nx>>5);
			else hy -= (ny>>5);
		}
		if (klabs(nx) > klabs(ny)) i = divscale16(hx,nx);
		else i = divscale16(hy,ny);
		if (i < cameradist) cameradist = i;
	}
	*vx = (*vx)+mulscale16(nx,cameradist);
	*vy = (*vy)+mulscale16(ny,cameradist);
	*vz = (*vz)+mulscale16(nz,cameradist);

	cameradist = min(cameradist+((totalclock-cameraclock)<<10),65536);
	cameraclock = totalclock;

	updatesectorz(*vx,*vy,*vz,vsectnum);

	sp->cstat = bakcstat;
}

//REPLACE FULLY
void drawbackground(void)
{
	short dapicnum;
	long x,y,x1,y1,x2,y2;

	flushperms();

	switch(ud.m_volume_number)
	{
	default:dapicnum = BIGHOLE;break;
	case 1:dapicnum = BIGHOLE;break;
	case 2:dapicnum = BIGHOLE;break;
	}

	y1 = 0; y2 = ydim;
	if( ready2send || ud.recstat == 2 )
	{
		if(ud.coop != 1)
		{
			if (ud.multimode > 1) y1 += scale(ydim,8,200);
			if (ud.multimode > 4) y1 += scale(ydim,8,200);
		}
		if (ud.screen_size >= 8) y2 = scale(ydim,200-34,200);
	}

	for(y=y1;y<y2;y+=128)
		for(x=0;x<xdim;x+=128)
			rotatesprite(x<<16,y<<16,65536L,0,dapicnum,8,0,8+16+64+128,0,y1,xdim-1,y2-1);

	if(ud.screen_size > 8)
	{
		y = 0;
		if(ud.coop != 1)
		{
			if (ud.multimode > 1) y += 8;
			if (ud.multimode > 4) y += 8;
		}

		x1 = max(windowx1-4,0);
		y1 = max(windowy1-4,y);
		x2 = min(windowx2+4,xdim-1);
		y2 = min(windowy2+4,scale(ydim,200-34,200)-1);

		for(y=y1+4;y<y2-4;y+=64)
		{
			rotatesprite(x1<<16,y<<16,65536L,0,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
			rotatesprite((x2+1)<<16,(y+64)<<16,65536L,1024,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
		}

		for(x=x1+4;x<x2-4;x+=64)
		{
			rotatesprite((x+64)<<16,y1<<16,65536L,512,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
			rotatesprite(x<<16,(y2+1)<<16,65536L,1536,VIEWBORDER,0,0,8+16+64+128,x1,y1,x2,y2);
		}

		rotatesprite(x1<<16,y1<<16,65536L,0,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
		rotatesprite((x2+1)<<16,y1<<16,65536L,512,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
		rotatesprite((x2+1)<<16,(y2+1)<<16,65536L,1024,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
		rotatesprite(x1<<16,(y2+1)<<16,65536L,1536,VIEWBORDER+1,0,0,8+16+64+128,x1,y1,x2,y2);
	}
}


// Floor Over Floor

// If standing in sector with SE42
// then draw viewing to SE41 and raise all =hi SE43 cielings.

// If standing in sector with SE43
// then draw viewing to SE40 and lower all =hi SE42 floors.

// If standing in sector with SE44
// then draw viewing to SE40.

// If standing in sector with SE45
// then draw viewing to SE41.

#define FOFTILE 13
#define FOFTILEX 32
#define FOFTILEY 32
long tempsectorz[MAXSECTORS];
long tempsectorpicnum[MAXSECTORS];
//short tempcursectnum;

void SE40_Draw(int spnum,long x,long y,long z,short a,short h,long smoothratio)
{
	int i=0,j=0,k=0;
	int floor1=0,floor2=0,ok=0,fofmode=0;
	long offx,offy;

	if(sprite[spnum].ang!=512) return;

	i = FOFTILE;    //Effect TILE
	if (!(gotpic[i>>3]&(1<<(i&7)))) return;
	gotpic[i>>3] &= ~(1<<(i&7));

	floor1=spnum;

	if(sprite[spnum].lotag==42) fofmode=40;
	if(sprite[spnum].lotag==43) fofmode=41;
	if(sprite[spnum].lotag==44) fofmode=40;
	if(sprite[spnum].lotag==45) fofmode=41;

	// fofmode=sprite[spnum].lotag-2;

	// sectnum=sprite[j].sectnum;
	// sectnum=cursectnum;
	ok++;

	/*  recursive?
	for(j=0;j<MAXSPRITES;j++)
	{
	if(
	sprite[j].sectnum==sectnum &&
	sprite[j].picnum==1 &&
	sprite[j].lotag==110
	) { DrawFloorOverFloor(j); break;}
	}
	*/

	// if(ok==0) { Message("no fof",RED); return; }

	for(j=0;j<MAXSPRITES;j++)
	{
		if(
			sprite[j].picnum==1 &&
			sprite[j].lotag==fofmode &&
			sprite[j].hitag==sprite[floor1].hitag
			) { floor1=j; fofmode=sprite[j].lotag; ok++; break;}
	}
	// if(ok==1) { Message("no floor1",RED); return; }

	if(fofmode==40) k=41; else k=40;

	for(j=0;j<MAXSPRITES;j++)
	{
		if(
			sprite[j].picnum==1 &&
			sprite[j].lotag==k &&
			sprite[j].hitag==sprite[floor1].hitag
			) {floor2=j; ok++; break;}
	}

	// if(ok==2) { Message("no floor2",RED); return; }

	for(j=0;j<MAXSPRITES;j++)  // raise ceiling or floor
	{
		if(sprite[j].picnum==1 &&
			sprite[j].lotag==k+2 &&
			sprite[j].hitag==sprite[floor1].hitag
			)
		{
			if(k==40)
			{tempsectorz[sprite[j].sectnum]=sector[sprite[j].sectnum].floorz;
			sector[sprite[j].sectnum].floorz+=(((z-sector[sprite[j].sectnum].floorz)/32768)+1)*32768;
			tempsectorpicnum[sprite[j].sectnum]=sector[sprite[j].sectnum].floorpicnum;
			sector[sprite[j].sectnum].floorpicnum=13;
			}
			if(k==41)
			{tempsectorz[sprite[j].sectnum]=sector[sprite[j].sectnum].ceilingz;
			sector[sprite[j].sectnum].ceilingz+=(((z-sector[sprite[j].sectnum].ceilingz)/32768)-1)*32768;
			tempsectorpicnum[sprite[j].sectnum]=sector[sprite[j].sectnum].ceilingpicnum;
			sector[sprite[j].sectnum].ceilingpicnum=13;
			}
		}
	}

	i=floor1;
	offx=x-sprite[i].x;
	offy=y-sprite[i].y;
	i=floor2;
	drawrooms(offx+sprite[i].x,offy+sprite[i].y,z,a,h,sprite[i].sectnum);
	animatesprites(x,y,a,smoothratio);
	drawmasks();

	for(j=0;j<MAXSPRITES;j++)  // restore ceiling or floor
	{
		if(sprite[j].picnum==1 &&
			sprite[j].lotag==k+2 &&
			sprite[j].hitag==sprite[floor1].hitag
			)
		{
			if(k==40)
			{sector[sprite[j].sectnum].floorz=tempsectorz[sprite[j].sectnum];
			sector[sprite[j].sectnum].floorpicnum=tempsectorpicnum[sprite[j].sectnum];
			}
			if(k==41)
			{sector[sprite[j].sectnum].ceilingz=tempsectorz[sprite[j].sectnum];
			sector[sprite[j].sectnum].ceilingpicnum=tempsectorpicnum[sprite[j].sectnum];
			}
		}// end if
	}// end for

} // end SE40




void se40code(long x,long y,long z,long a,long h, long smoothratio)
{
	int i;

	i = headspritestat[15];
	while(i >= 0)
	{
		switch(sprite[i].lotag)
		{
			//            case 40:
			//            case 41:
			//                SE40_Draw(i,x,y,a,smoothratio);
			//                break;
		case 42:
		case 43:
		case 44:
		case 45:
			if(ps[screenpeek].cursectnum == sprite[i].sectnum)
				SE40_Draw(i,x,y,z,a,h,smoothratio);
			break;
		}
		i = nextspritestat[i];
	}
}

static long oyrepeat=-1;
extern bool setmodeneeded;
extern int NewWidth, NewHeight, NewBits, DisplayBits;

void displayrooms(short snum,long smoothratio)
{
	long cposx,cposy,cposz,dst,j,fz,cz;
	short sect, cang, k, choriz;
	struct player_struct *p;
	long tposx,tposy,i;
	short tang;

	// [RH] change the screen mode if needed
	if (setmodeneeded)
	{
		setmodeneeded = false;
		// Change screen mode.
		if (V_SetResolution (NewWidth, NewHeight, NewBits))
		{
			// Recalculate various view parameters.
			screen->Lock (false);
			ScreenWidth = screen->GetWidth ();
			ScreenHeight = screen->GetHeight ();
			ScreenMode = 1;
			setgamemode ((char*)screen->GetBuffer(), ScreenWidth, ScreenHeight, screen->GetPitch());
			screen->Unlock ();

			pub = pus = NUMPAGES;
			// Refresh the console.
			C_NewModeAdjust ();
		}
	}

	p = &ps[snum];

	//    if(screencapt == 0 && (p->gm&MODE_MENU) && ( (current_menu/100) == 3 ) || (current_menu >= 1000 && current_menu < 2999 ) )
	//      return;

	if(pub > 0)
	{
		if(ud.screen_size > 8) drawbackground();
		pub = 0;
	}

	if( ud.overhead_on == 2 || ud.show_help || p->cursectnum == -1)
		return;

	smoothratio = min(max(smoothratio,0),65536);

	visibility = p->visibility;

	if(ud.pause_on || ps[snum].on_crane > -1) smoothratio = 65536;

	sect = p->cursectnum;
	if(sect < 0 || sect >= MAXSECTORS) return;

	dointerpolations(smoothratio);

	animatecamsprite();

	if(ud.camerasprite >= 0)
	{
		spritetype *s;

		s = &sprite[ud.camerasprite];

		if(s->yvel < 0) s->yvel = -100;
		else if(s->yvel > 199) s->yvel = 300;

		cang = hittype[ud.camerasprite].tempang+mulscale16((long)(((s->ang+1024-hittype[ud.camerasprite].tempang)&2047)-1024),smoothratio);

		se40code(s->x,s->y,s->z,cang,s->yvel,smoothratio);

		drawrooms(s->x,s->y,s->z-(4<<8),cang,s->yvel,s->sectnum);
		animatesprites(s->x,s->y,cang,smoothratio);
		drawmasks();
	}
	else
	{
		i = divscale22(1,sprite[p->i].yrepeat+28);
		if (i != oyrepeat)
		{
			oyrepeat = i;
			setaspect(oyrepeat,yxaspect);
		}

		if(screencapt)
		{
			walock[MAXTILES-1] = 254;
			if (waloff[MAXTILES-1] == 0)
				allocache((void**)&waloff[MAXTILES-1],100*160,&walock[MAXTILES-1]);
			setviewtotile(MAXTILES-1,100L,160L);
		}
		else if( ( ud.screen_tilting && p->rotscrnang ) || ud.detail==0 )
		{
			if (ud.screen_tilting) tang = p->rotscrnang; else tang = 0;

			walock[MAXTILES-2] = 255;
			if (waloff[MAXTILES-2] == 0)
				allocache((void**)&waloff[MAXTILES-2],320L*320L,&walock[MAXTILES-2]);
			if ((tang&1023) == 0)
				setviewtotile(MAXTILES-2,200L>>(1-ud.detail),320L>>(1-ud.detail));
			else
				setviewtotile(MAXTILES-2,320L>>(1-ud.detail),320L>>(1-ud.detail));
			if ((tang&1023) == 512)
			{     //Block off unscreen section of 90ø tilted screen
				j = ((320-60)>>(1-ud.detail));
				for(i=(60>>(1-ud.detail))-1;i>=0;i--)
				{
					startumost[i] = 1; startumost[i+j] = 1;
					startdmost[i] = 0; startdmost[i+j] = 0;
				}
			}

			i = (tang&511); if (i > 256) i = 512-i;
			i = sintable[i+512]*8 + sintable[i]*5L;
			setaspect(i>>1,yxaspect);
		}

		if ( (snum == myconnectindex) && (numplayers > 1) )
		{
			cposx = omyx+mulscale16((long)(myx-omyx),smoothratio);
			cposy = omyy+mulscale16((long)(myy-omyy),smoothratio);
			cposz = omyz+mulscale16((long)(myz-omyz),smoothratio);
			cang = omyang+mulscale16((long)(((myang+1024-omyang)&2047)-1024),smoothratio);
			choriz = omyhoriz+omyhorizoff+mulscale16((long)(myhoriz+myhorizoff-omyhoriz-omyhorizoff),smoothratio);
			sect = mycursectnum;
		}
		else
		{
			cposx = p->oposx+mulscale16((long)(p->posx-p->oposx),smoothratio);
			cposy = p->oposy+mulscale16((long)(p->posy-p->oposy),smoothratio);
			cposz = p->oposz+mulscale16((long)(p->posz-p->oposz),smoothratio);
			cang = p->oang+mulscale16((long)(((p->ang+1024-p->oang)&2047)-1024),smoothratio);
			choriz = p->ohoriz+p->ohorizoff+mulscale16((long)(p->horiz+p->horizoff-p->ohoriz-p->ohorizoff),smoothratio);
		}
		cang += p->look_ang;

		if (p->newowner >= 0)
		{
			cang = p->ang+p->look_ang;
			choriz = p->horiz+p->horizoff;
			cposx = p->posx;
			cposy = p->posy;
			cposz = p->posz;
			sect = sprite[p->newowner].sectnum;
			smoothratio = 65536L;
		}

		else if( p->over_shoulder_on == 0 )
			cposz += p->opyoff+mulscale16((long)(p->pyoff-p->opyoff),smoothratio);
		else view(p,&cposx,&cposy,&cposz,&sect,cang,choriz);

		cz = hittype[p->i].ceilingz;
		fz = hittype[p->i].floorz;

		if(earthquaketime > 0 && p->on_ground == 1)
		{
			cposz += 256-(((earthquaketime)&1)<<9);
			cang += (2-((earthquaketime)&2))<<2;
		}

		if(sprite[p->i].pal == 1) cposz -= (18<<8);

		if(p->newowner >= 0)
			choriz = 100+sprite[p->newowner].shade;
		else if(p->spritebridge == 0)
		{
			if( cposz < ( p->truecz + (4<<8) ) ) cposz = cz + (4<<8);
			else if( cposz > ( p->truefz - (4<<8) ) ) cposz = fz - (4<<8);
		}

		if (sect >= 0)
		{
			getzsofslope(sect,cposx,cposy,&cz,&fz);
			if (cposz < cz+(4<<8)) cposz = cz+(4<<8);
			if (cposz > fz-(4<<8)) cposz = fz-(4<<8);
		}

		if(choriz > 299) choriz = 299;
		else if(choriz < -99) choriz = -99;

		se40code(cposx,cposy,cposz,cang,choriz,smoothratio);

		if ((gotpic[MIRROR>>3]&(1<<(MIRROR&7))) > 0)
		{
			dst = 0x7fffffff; i = 0;
			for(k=0;k<mirrorcnt;k++)
			{
				j = klabs(wall[mirrorwall[k]].x-cposx);
				j += klabs(wall[mirrorwall[k]].y-cposy);
				if (j < dst) dst = j, i = k;
			}

			if( wall[mirrorwall[i]].overpicnum == MIRROR )
			{
				preparemirror(cposx,cposy,cposz,cang,choriz,mirrorwall[i],mirrorsector[i],&tposx,&tposy,&tang);

				j = visibility;
				visibility = (j>>1) + (j>>2);

				drawrooms(tposx,tposy,cposz,tang,choriz,mirrorsector[i]+MAXSECTORS);

				display_mirror = 1;
				animatesprites(tposx,tposy,tang,smoothratio);
				display_mirror = 0;

				drawmasks();
				completemirror();   //Reverse screen x-wise in this function
				visibility = j;
			}
			gotpic[MIRROR>>3] &= ~(1<<(MIRROR&7));
		}

		drawrooms(cposx,cposy,cposz,cang,choriz,sect);
		animatesprites(cposx,cposy,cang,smoothratio);
		drawmasks();

		if(screencapt == 1)
		{
			setviewback();
			walock[MAXTILES-1] = 1;
			screencapt = 0;
		}
		else if( ( ud.screen_tilting && p->rotscrnang) || ud.detail==0 )
		{
			if (ud.screen_tilting) tang = p->rotscrnang; else tang = 0;
			setviewback();
			picanm[MAXTILES-2] &= 0xff0000ff;
			i = (tang&511); if (i > 256) i = 512-i;
			i = sintable[i+512]*8 + sintable[i]*5L;
			if ((1-ud.detail) == 0) i >>= 1;
			rotatesprite(160<<16,100<<16,i,tang+512,MAXTILES-2,0,0,4+2+64,windowx1,windowy1,windowx2,windowy2);
			walock[MAXTILES-2] = 199;
		}
	}

	restoreinterpolations();

	if (totalclock < lastvisinc)
	{
		if (klabs(p->visibility-ud.const_visibility) > 8)
			p->visibility += (ud.const_visibility-p->visibility)>>2;
	}
	else p->visibility = ud.const_visibility;
}





short LocateTheLocator(short n,short sn)
{
	short i;

	i = headspritestat[7];
	while(i >= 0)
	{
		if( (sn == -1 || sn == sprite[i].sectnum) && n == sprite[i].lotag )
			return i;
		i = nextspritestat[i];
	}
	return -1;
}

short EGS(short whatsect,long s_x,long s_y,long s_z,short s_pn,signed char s_s,signed char s_xr,signed char s_yr,short s_a,short s_ve,long s_zv,short s_ow,signed char s_ss)
{
	short i;
	spritetype *s;

	i = insertsprite(whatsect,s_ss);

	if( i < 0 )
		gameexit(" Too many sprites spawned.");

	hittype[i].bposx = s_x;
	hittype[i].bposy = s_y;
	hittype[i].bposz = s_z;

	s = &sprite[i];

	s->x = s_x;
	s->y = s_y;
	s->z = s_z;
	s->cstat = 0;
	s->picnum = s_pn;
	s->shade = s_s;
	s->xrepeat = s_xr;
	s->yrepeat = s_yr;
	s->pal = 0;

	s->ang = s_a;
	s->xvel = s_ve;
	s->zvel = s_zv;
	s->owner = s_ow;
	s->xoffset = 0;
	s->yoffset = 0;
	s->yvel = 0;
	s->clipdist = 0;
	s->pal = 0;
	s->lotag = 0;

	hittype[i].picnum = sprite[s_ow].picnum;

	hittype[i].lastvx = 0;
	hittype[i].lastvy = 0;

	hittype[i].timetosleep = 0;
	hittype[i].actorstayput = -1;
	hittype[i].extra = -1;
	hittype[i].owner = s_ow;
	hittype[i].cgg = 0;
	hittype[i].movflag = 0;
	hittype[i].tempang = 0;
	hittype[i].dispicnum = 0;
	hittype[i].floorz = hittype[s_ow].floorz;
	hittype[i].ceilingz = hittype[s_ow].ceilingz;

	hittype[i].temp_data[0]=hittype[i].temp_data[2]=hittype[i].temp_data[3]=hittype[i].temp_data[5]=0;
	if( actorscrptr[s_pn] )
	{
		s->extra = *actorscrptr[s_pn];
		hittype[i].temp_data[4] = *(actorscrptr[s_pn]+1);
		hittype[i].temp_data[1] = *(actorscrptr[s_pn]+2);
		s->hitag = *(actorscrptr[s_pn]+3);
	}
	else
	{
		hittype[i].temp_data[1]=hittype[i].temp_data[4]=0;
		s->extra = 0;
		s->hitag = 0;
	}

	if (show2dsector[sprite[i].sectnum>>3]&(1<<(sprite[i].sectnum&7))) show2dsprite[i>>3] |= (1<<(i&7));
	else show2dsprite[i>>3] &= ~(1<<(i&7));
	/*
	if(s->sectnum < 0)
	{
	s->xrepeat = s->yrepeat = 0;
	changespritestat(i,5);
	}
	*/
	return(i);
}

char wallswitchcheck(short i)
{
	switch(sprite[i].picnum)
	{
	case HANDPRINTSWITCH:
	case HANDPRINTSWITCH+1:
	case ALIENSWITCH:
	case ALIENSWITCH+1:
	case MULTISWITCH:
	case MULTISWITCH+1:
	case MULTISWITCH+2:
	case MULTISWITCH+3:
	case ACCESSSWITCH:
	case ACCESSSWITCH2:
	case PULLSWITCH:
	case PULLSWITCH+1:
	case HANDSWITCH:
	case HANDSWITCH+1:
	case SLOTDOOR:
	case SLOTDOOR+1:
	case LIGHTSWITCH:
	case LIGHTSWITCH+1:
	case SPACELIGHTSWITCH:
	case SPACELIGHTSWITCH+1:
	case SPACEDOORSWITCH:
	case SPACEDOORSWITCH+1:
	case FRANKENSTINESWITCH:
	case FRANKENSTINESWITCH+1:
	case LIGHTSWITCH2:
	case LIGHTSWITCH2+1:
	case POWERSWITCH1:
	case POWERSWITCH1+1:
	case LOCKSWITCH1:
	case LOCKSWITCH1+1:
	case POWERSWITCH2:
	case POWERSWITCH2+1:
	case DIPSWITCH:
	case DIPSWITCH+1:
	case DIPSWITCH2:
	case DIPSWITCH2+1:
	case TECHSWITCH:
	case TECHSWITCH+1:
	case DIPSWITCH3:
	case DIPSWITCH3+1:
		return 1;
	}
	return 0;
}


long tempwallptr;
short spawn( short j, short pn )
{
	short i, s, startwall, endwall, sect, clostest;
	long x, y, d;
	spritetype *sp;

	if(j >= 0)
	{
		i = EGS(sprite[j].sectnum,sprite[j].x,sprite[j].y,sprite[j].z
			,pn,0,0,0,0,0,0,j,0);
		hittype[i].picnum = sprite[j].picnum;
	}
	else
	{
		i = pn;

		hittype[i].picnum = sprite[i].picnum;
		hittype[i].timetosleep = 0;
		hittype[i].extra = -1;

		hittype[i].bposx = sprite[i].x;
		hittype[i].bposy = sprite[i].y;
		hittype[i].bposz = sprite[i].z;

		sprite[i].owner = hittype[i].owner = i;
		hittype[i].cgg = 0;
		hittype[i].movflag = 0;
		hittype[i].tempang = 0;
		hittype[i].dispicnum = 0;
		hittype[i].floorz = sector[sprite[i].sectnum].floorz;
		hittype[i].ceilingz = sector[sprite[i].sectnum].ceilingz;

		hittype[i].lastvx = 0;
		hittype[i].lastvy = 0;
		hittype[i].actorstayput = -1;

		hittype[i].temp_data[0] = hittype[i].temp_data[1] = hittype[i].temp_data[2] = hittype[i].temp_data[3] = hittype[i].temp_data[4] = hittype[i].temp_data[5] = 0;

		if( sprite[i].picnum != SPEAKER &&
			sprite[i].picnum != LETTER &&
			sprite[i].picnum != DUCK &&
			sprite[i].picnum != TARGET &&
			sprite[i].picnum != TRIPBOMB &&
			sprite[i].picnum != VIEWSCREEN &&
			sprite[i].picnum != VIEWSCREEN2 &&
			(sprite[i].cstat & CSTAT_TYPEMASK) != CSTAT_FACE)
			if( !(sprite[i].picnum >= CRACK1 && sprite[i].picnum <= CRACK4) )
			{
				if(sprite[i].shade == 127) return i;
				if( wallswitchcheck(i) == 1 && (sprite[i].cstat&CSTAT_TYPEMASK) == CSTAT_WALL)
				{
					if( sprite[i].picnum != ACCESSSWITCH && sprite[i].picnum != ACCESSSWITCH2 && sprite[i].pal)
					{
						if( (ud.multimode < 2) || (ud.multimode > 1 && ud.coop==1) )
						{
							sprite[i].xrepeat = sprite[i].yrepeat = 0;
							sprite[i].cstat = sprite[i].lotag = sprite[i].hitag = 0;
							return i;
						}
					}
					sprite[i].cstat |= CSTAT_ALLBLOCKING;
					if( sprite[i].pal && sprite[i].picnum != ACCESSSWITCH && sprite[i].picnum != ACCESSSWITCH2)
						sprite[i].pal = 0;
					return i;
				}

				if( sprite[i].hitag )
				{
					changespritestat(i,12);
					sprite[i].cstat |= CSTAT_ALLBLOCKING;
					sprite[i].extra = impact_damage;
					return i;
				}
			}

			s = sprite[i].picnum;

			if( sprite[i].cstat&CSTAT_CLIPBLOCKING ) sprite[i].cstat |= CSTAT_HITBLOCKING;

			if( actorscrptr[s] )
			{
				sprite[i].extra = *(actorscrptr[s]);
				hittype[i].temp_data[4] = *(actorscrptr[s]+1);
				hittype[i].temp_data[1] = *(actorscrptr[s]+2);
				if( *(actorscrptr[s]+3) && sprite[i].hitag == 0 )
					sprite[i].hitag = *(actorscrptr[s]+3);
			}
			else hittype[i].temp_data[1] = hittype[i].temp_data[4] = 0;
	}

	sp = &sprite[i];
	sect = sp->sectnum;

	switch(sp->picnum)
	{
	default:

		if( actorscrptr[sp->picnum] )
		{
			if( j == -1 && sp->lotag > ud.player_skill )
			{
				sp->xrepeat=sp->yrepeat=0;
				changespritestat(i,5);
				break;
			}

			//  Init the size
			if(sp->xrepeat == 0 || sp->yrepeat == 0)
				sp->xrepeat = sp->yrepeat = 1;

			if( actortype[sp->picnum] & 3)
			{
				if( ud.monsters_off == 1 )
				{
					sp->xrepeat=sp->yrepeat=0;
					changespritestat(i,5);
					break;
				}

				makeitfall(i);

				if( actortype[sp->picnum] & 2)
					hittype[i].actorstayput = sp->sectnum;

				ps[myconnectindex].max_actors_killed++;
				sp->clipdist = 80;
				if(j >= 0)
				{
					if(sprite[j].picnum == RESPAWN)
						hittype[i].tempang = sprite[i].pal = sprite[j].pal;
					changespritestat(i,1);
				}
				else changespritestat(i,2);
			}
			else
			{
				sp->clipdist = 40;
				sp->owner = i;
				changespritestat(i,1);
			}

			hittype[i].timetosleep = 0;

			if(j >= 0)
				sp->ang = sprite[j].ang;
		}
		break;
	case FOF:
		sp->xrepeat = sp->yrepeat = 0;
		changespritestat(i,5);
		break;
	case WATERSPLASH2:
		if(j >= 0)
		{
			setsprite(i,sprite[j].x,sprite[j].y,sprite[j].z);
			sp->xrepeat = sp->yrepeat = 8+(TRAND&7);
		}
		else sp->xrepeat = sp->yrepeat = 16+(TRAND&15);

		sp->shade = -16;
		sp->cstat |= CSTAT_CENTERED;
		if(j >= 0)
		{
			if(sector[sprite[j].sectnum].lotag == 2)
			{
				sp->z = getceilzofslope(sprite[i].sectnum,sprite[i].x,sprite[i].y)+(16<<8);
				sp->cstat |= CSTAT_YFLIP;
			}
			else if( sector[sprite[j].sectnum].lotag == 1)
				sp->z = getflorzofslope(sprite[i].sectnum,sprite[i].x,sprite[i].y);
		}

		if(sector[sect].floorpicnum == FLOORSLIME ||
			sector[sect].ceilingpicnum == FLOORSLIME)
			sp->pal = 7;
	case NEON1:
	case NEON2:
	case NEON3:
	case NEON4:
	case NEON5:
	case NEON6:
	case DOMELITE:
		if(sp->picnum != WATERSPLASH2)
			sp->cstat |= CSTAT_ALLBLOCKING;
	case NUKEBUTTON:
		if(sp->picnum == DOMELITE)	// [RH] Methinks somebody made a mistake here
			sp->cstat |= CSTAT_ALLBLOCKING;
	case JIBS1:
	case JIBS2:
	case JIBS3:
	case JIBS4:
	case JIBS5:
	case JIBS6:
	case HEADJIB1:
	case ARMJIB1:
	case LEGJIB1:
	case LIZMANHEAD1:
	case LIZMANARM1:
	case LIZMANLEG1:
	case DUKETORSO:
	case DUKEGUN:
	case DUKELEG:
		changespritestat(i,5);
		break;
	case TONGUE:
		if(j >= 0)
			sp->ang = sprite[j].ang;
		sp->z -= 38<<8;
		sp->zvel = 256-(TRAND&511);
		sp->xvel = 64-(TRAND&127);
		changespritestat(i,4);
		break;
	case NATURALLIGHTNING:
		sp->cstat &= ~CSTAT_ALLBLOCKING;
		sp->cstat |= CSTAT_INVISIBLE;
		break;
	case TRANSPORTERSTAR:
	case TRANSPORTERBEAM:
		if(j == -1) break;
		if(sp->picnum == TRANSPORTERBEAM)
		{
			sp->xrepeat = 31;
			sp->yrepeat = 1;
			sp->z = sector[sprite[j].sectnum].floorz-(40<<8);
		}
		else
		{
			if(sprite[j].statnum == 4)
			{
				sp->xrepeat = 8;
				sp->yrepeat = 8;
			}
			else
			{
				sp->xrepeat = 48;
				sp->yrepeat = 64;
				if(sprite[j].statnum == 10 || badguy(&sprite[j]) )
					sp->z -= (32<<8);
			}
		}

		sp->shade = -127;
		sp->cstat = CSTAT_CENTERED|CSTAT_TRANS;
		sp->ang = sprite[j].ang;

		sp->xvel = 128;
		changespritestat(i,5);
		ssp(i,CLIPMASK0);
		setsprite(i,sp->x,sp->y,sp->z);
		break;

	case FRAMEEFFECT1:
		if(j >= 0)
		{
			sp->xrepeat = sprite[j].xrepeat;
			sp->yrepeat = sprite[j].yrepeat;
			hittype[i].temp_data[1] = sprite[j].picnum;
		}
		else sp->xrepeat = sp->yrepeat = 0;

		changespritestat(i,5);

		break;

	case LASERLINE:
		sp->yrepeat = 6;
		sp->xrepeat = 32;

		if(lasermode == 1)
			sp->cstat = CSTAT_WALL + CSTAT_TRANS;
		else if(lasermode == 0 || lasermode == 2)
			sp->cstat = CSTAT_WALL;
		else
		{
			sp->xrepeat = 0;
			sp->yrepeat = 0;
		}

		if(j >= 0) sp->ang = hittype[j].temp_data[5]+512;
		changespritestat(i,5);
		break;

	case FORCESPHERE:
		if(j == -1 )
		{
			sp->cstat = CSTAT_INVISIBLE;
			changespritestat(i,2);
		}
		else
		{
			sp->xrepeat = sp->yrepeat = 1;
			changespritestat(i,5);
		}
		break;

	case BLOOD:
		sp->xrepeat = sp->yrepeat = 16;
		sp->z -= (26<<8);
		if( j >= 0 && sprite[j].pal == 6 )
			sp->pal = 6;
		changespritestat(i,5);
		break;
	case BLOODPOOL:
	case PUKE:
		{
			short s1;
			s1 = sp->sectnum;

			updatesector(sp->x+108,sp->y+108,&s1);
			if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
			{
				updatesector(sp->x-108,sp->y-108,&s1);
				if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
				{
					updatesector(sp->x+108,sp->y-108,&s1);
					if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
					{
						updatesector(sp->x-108,sp->y+108,&s1);
						if(s1 >= 0 && sector[s1].floorz != sector[sp->sectnum].floorz)
						{ sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
					}
					else { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
				}
				else { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
			}
			else { sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
		}

		if( sector[sprite[i].sectnum].lotag == 1 )
		{
			changespritestat(i,5);
			break;
		}

		if(j >= 0 && sp->picnum != PUKE)
		{
			if( sprite[j].pal == 1)
				sp->pal = 1;
			else if( sprite[j].pal != 6 && sprite[j].picnum != NUKEBARREL && sprite[j].picnum != TIRE )
			{
				if(sprite[j].picnum == FECES)
					sp->pal = 7; // Brown
				else sp->pal = 2; // Red
			}
			else sp->pal = 0;  // green

			if(sprite[j].picnum == TIRE)
				sp->shade = 127;
		}
		if (!(sp->cstat & CSTAT_WALL))
		{
			sp->cstat |= CSTAT_FLOOR;
		}
	case FECES:
		if( j >= 0)
			sp->xrepeat = sp->yrepeat = 1;
		changespritestat(i,5);
		break;

	case BLOODSPLAT1:
	case BLOODSPLAT2:
	case BLOODSPLAT3:
	case BLOODSPLAT4:
		if (!(sp->cstat & CSTAT_FLOOR))
		{
			sp->cstat |= CSTAT_WALL;
		}
		sp->xrepeat = 7+(TRAND&7);
		sp->yrepeat = 7+(TRAND&7);
		sp->z -= (16<<8);
		if(j >= 0 && sprite[j].pal == 6)
			sp->pal = 6;
		insertspriteq(i);
		changespritestat(i,5);
		break;

	case TRIPBOMB:
		if( sp->lotag > ud.player_skill )
		{
			sp->xrepeat=sp->yrepeat=0;
			changespritestat(i,5);
			break;
		}

		sp->xrepeat=4;
		sp->yrepeat=5;

		sp->owner = i;
		sp->hitag = i;

		sp->xvel = 16;
		ssp(i,CLIPMASK0);
		hittype[i].temp_data[0] = 17;
		hittype[i].temp_data[2] = 0;
		hittype[i].temp_data[5] = sp->ang;

	case SPACEMARINE:
		if(sp->picnum == SPACEMARINE)
		{
			sp->extra = 20;
			sp->cstat |= CSTAT_ALLBLOCKING;
		}
		changespritestat(i,2);
		break;

	case HYDRENT:
	case PANNEL1:
	case PANNEL2:
	case SATELITE:
	case FUELPOD:
	case SOLARPANNEL:
	case ANTENNA:
	case GRATE1:
	case CHAIR1:
	case CHAIR2:
	case CHAIR3:
	case BOTTLE1:
	case BOTTLE2:
	case BOTTLE3:
	case BOTTLE4:
	case BOTTLE5:
	case BOTTLE6:
	case BOTTLE7:
	case BOTTLE8:
	case BOTTLE10:
	case BOTTLE11:
	case BOTTLE12:
	case BOTTLE13:
	case BOTTLE14:
	case BOTTLE15:
	case BOTTLE16:
	case BOTTLE17:
	case BOTTLE18:
	case BOTTLE19:
	case OCEANSPRITE1:
	case OCEANSPRITE2:
	case OCEANSPRITE3:
	case OCEANSPRITE5:
	case MONK:
	case INDY:
	case LUKE:
	case JURYGUY:
	case SCALE:
	case VACUUM:
	case FANSPRITE:
	case CACTUS:
	case CACTUSBROKE:
	case HANGLIGHT:
	case FETUS:
	case FETUSBROKE:
	case CAMERALIGHT:
	case MOVIECAMERA:
	case IVUNIT:
	case POT1:
	case POT2:
	case POT3:
	case TRIPODCAMERA:
	case SUSHIPLATE1:
	case SUSHIPLATE2:
	case SUSHIPLATE3:
	case SUSHIPLATE4:
	case SUSHIPLATE5:
	case WAITTOBESEATED:
	case VASE:
	case PIPE1:
	case PIPE2:
	case PIPE3:
	case PIPE4:
	case PIPE5:
	case PIPE6:
		sp->clipdist = 32;
		sp->cstat |= CSTAT_ALLBLOCKING;
	case OCEANSPRITE4:
		changespritestat(i,0);
		break;
	case FEMMAG1:
	case FEMMAG2:
		sp->cstat &= ~CSTAT_ALLBLOCKING;
		changespritestat(i,0);
		break;
	case DUKETAG:
	case SIGN1:
	case SIGN2:
		if(ud.multimode < 2 && sp->pal)
		{
			sp->xrepeat = sp->yrepeat = 0;
			changespritestat(i,5);
		}
		else sp->pal = 0;
		break;
	case MASKWALL1:
	case MASKWALL2:
	case MASKWALL3:
	case MASKWALL4:
	case MASKWALL5:
	case MASKWALL6:
	case MASKWALL7:
	case MASKWALL8:
	case MASKWALL9:
	case MASKWALL10:
	case MASKWALL11:
	case MASKWALL12:
	case MASKWALL13:
	case MASKWALL14:
	case MASKWALL15:
		j = sp->cstat&(CSTAT_TYPEMASK|CSTAT_ONESIDED|CSTAT_CENTERED);
		sp->cstat = j|CSTAT_CLIPBLOCKING;
		changespritestat(i,0);
		break;
	case FOOTPRINTS:
	case FOOTPRINTS2:
	case FOOTPRINTS3:
	case FOOTPRINTS4:
		if(j >= 0)
		{
			short s1;
			s1 = sp->sectnum;

			updatesector(sp->x+84,sp->y+84,&s1);
			if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
			{
				updatesector(sp->x-84,sp->y-84,&s1);
				if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
				{
					updatesector(sp->x+84,sp->y-84,&s1);
					if(s1 >= 0 && sector[s1].floorz == sector[sp->sectnum].floorz)
					{
						updatesector(sp->x-84,sp->y+84,&s1);
						if(s1 >= 0 && sector[s1].floorz != sector[sp->sectnum].floorz)
						{ sp->xrepeat = sp->yrepeat = 0;changespritestat(i,5);break;}
					}
					else { sp->xrepeat = sp->yrepeat = 0;break;}
				}
				else { sp->xrepeat = sp->yrepeat = 0;break;}
			}
			else { sp->xrepeat = sp->yrepeat = 0;break;}

			sp->cstat = CSTAT_FLOOR+((ps[sprite[j].yvel].footprintcount&1)<<CSTATB_XFLIP);
			sp->ang = sprite[j].ang;
		}

		sp->z = sector[sect].floorz;
		if(sector[sect].lotag != 1 && sector[sect].lotag != 2)
			sp->xrepeat = sp->yrepeat = 32;

		insertspriteq(i);
		changespritestat(i,5);
		break;

	case FEM1:
	case FEM2:
	case FEM3:
	case FEM4:
	case FEM5:
	case FEM6:
	case FEM7:
	case FEM8:
	case FEM9:
	case FEM10:
	case PODFEM1:
	case NAKED1:
	case STATUE:
	case TOUGHGAL:
		sp->yvel = sp->hitag;
		sp->hitag = -1;
		if(sp->picnum == PODFEM1) sp->extra <<= 1;
	case BLOODYPOLE:

	case QUEBALL:
	case STRIPEBALL:

		if(sp->picnum == QUEBALL || sp->picnum == STRIPEBALL)
		{
			sp->cstat = CSTAT_HITBLOCKING;
			sp->clipdist = 8;
		}
		else
		{
			sp->cstat |= CSTAT_ALLBLOCKING;
			sp->clipdist = 32;
		}

		changespritestat(i,2);
		break;

	case DUKELYINGDEAD:
		if(j >= 0 && sprite[j].picnum == APLAYER)
		{
			sp->xrepeat = sprite[j].xrepeat;
			sp->yrepeat = sprite[j].yrepeat;
			sp->shade = sprite[j].shade;
			sp->pal = ps[sprite[j].yvel].palookup;
		}
	case DUKECAR:
	case HELECOPT:
		//                if(sp->picnum == HELECOPT || sp->picnum == DUKECAR) sp->xvel = 1024;
		sp->cstat = 0;
		sp->extra = 1;
		sp->xvel = 292;
		sp->zvel = 360;
	case RESPAWNMARKERRED:
	case BLIMP:

		if(sp->picnum == RESPAWNMARKERRED)
		{
			sp->xrepeat = sp->yrepeat = 24;
			if(j >= 0) sp->z = hittype[j].floorz; // -(1<<4);
		}
		else
		{
			sp->cstat |= CSTAT_ALLBLOCKING;
			sp->clipdist = 128;
		}
	case MIKE:
		if(sp->picnum == MIKE)
			sp->yvel = sp->hitag;
	case WEATHERWARN:
		changespritestat(i,1);
		break;

	case SPOTLITE:
		hittype[i].temp_data[0] = sp->x;
		hittype[i].temp_data[1] = sp->y;
		break;
	case BULLETHOLE:
		sp->xrepeat = sp->yrepeat = 3;
		sp->cstat = CSTAT_WALL+(krand()&(CSTAT_XFLIP|CSTAT_YFLIP));
		insertspriteq(i);
	case MONEY:
	case MAIL:
	case PAPER:
		if( sp->picnum == MONEY || sp->picnum == MAIL || sp->picnum == PAPER )
		{
			hittype[i].temp_data[0] = TRAND&2047;
			sp->cstat = TRAND&(CSTAT_XFLIP|CSTAT_YFLIP);
			sp->xrepeat = sp->yrepeat = 8;
			sp->ang = TRAND&2047;
		}
		changespritestat(i,5);
		break;

	case VIEWSCREEN:
	case VIEWSCREEN2:
		sp->owner = i;
		sp->lotag = 1;
		sp->extra = 1;
		changespritestat(i,6);
		break;

	case SHELL: //From the player
	case SHOTGUNSHELL:
		if( j >= 0 )
		{
			short snum,a;

			if(sprite[j].picnum == APLAYER)
			{
				snum = sprite[j].yvel;
				a = ps[snum].ang-(TRAND&63)+8;  //Fine tune

				hittype[i].temp_data[0] = TRAND&1;
				if(sp->picnum == SHOTGUNSHELL)
					sp->z = (6<<8)+ps[snum].pyoff+ps[snum].posz-((ps[snum].horizoff+ps[snum].horiz-100)<<4);
				else sp->z = (3<<8)+ps[snum].pyoff+ps[snum].posz-((ps[snum].horizoff+ps[snum].horiz-100)<<4);
				sp->zvel = -(TRAND&255);
			}
			else
			{
				a = sp->ang;
				sp->z = sprite[j].z-PHEIGHT+(3<<8);
			}

			sp->x = sprite[j].x+(sintable[(a+512)&2047]>>7);
			sp->y = sprite[j].y+(sintable[a&2047]>>7);

			sp->shade = -8;

			sp->ang = a-512;
			sp->xvel = 20;

			sp->xrepeat=sp->yrepeat=4;

			changespritestat(i,5);
		}
		break;

	case RESPAWN:
		sp->extra = 66-13;
	case MUSICANDSFX:
		if( ud.multimode < 2 && sp->pal == 1)
		{
			sp->xrepeat = sp->yrepeat = 0;
			changespritestat(i,5);
			break;
		}
		sp->cstat = CSTAT_INVISIBLE;
		changespritestat(i,11);
		break;

	case EXPLOSION2:
	case EXPLOSION2BOT:
	case BURNING:
	case BURNING2:
	case SMALLSMOKE:
	case SHRINKEREXPLOSION:
	case COOLEXPLOSION1:

		if(j >= 0)
		{
			sp->ang = sprite[j].ang;
			sp->shade = -64;
			sp->cstat = CSTAT_CENTERED | (TRAND & CSTAT_XFLIP);
		}

		if(sp->picnum == EXPLOSION2 || sp->picnum == EXPLOSION2BOT)
		{
			sp->xrepeat = 48;
			sp->yrepeat = 48;
			sp->shade = -127;
			sp->cstat |= CSTAT_CENTERED;
		}
		else if(sp->picnum == SHRINKEREXPLOSION )
		{
			sp->xrepeat = 32;
			sp->yrepeat = 32;
		}
		else if( sp->picnum == SMALLSMOKE )
		{
			// 64 "money"
			sp->xrepeat = 24;
			sp->yrepeat = 24;
		}
		else if(sp->picnum == BURNING || sp->picnum == BURNING2)
		{
			sp->xrepeat = 4;
			sp->yrepeat = 4;
		}

		if(j >= 0)
		{
			x = getflorzofslope(sp->sectnum,sp->x,sp->y);
			if(sp->z > x-(12<<8) )
				sp->z = x-(12<<8);
		}

		changespritestat(i,5);

		break;

	case PLAYERONWATER:
		if(j >= 0)
		{
			sp->xrepeat = sprite[j].xrepeat;
			sp->yrepeat = sprite[j].yrepeat;
			sp->zvel = 128;
			if(sector[sp->sectnum].lotag != 2)
				sp->cstat |= CSTAT_INVISIBLE;
		}
		changespritestat(i,13);
		break;

	case APLAYER:
		sp->xrepeat = sp->yrepeat = 0;
		j = ud.coop;
		if(j == 2) j = 0;

		if( ud.multimode < 2 || (ud.multimode > 1 && j != sp->lotag) )
			changespritestat(i,5);
		else
			changespritestat(i,10);
		break;
	case WATERBUBBLE:
		if(j >= 0 && sprite[j].picnum == APLAYER)
			sp->z -= (16<<8);
		if( sp->picnum == WATERBUBBLE)
		{
			if( j >= 0 )
				sp->ang = sprite[j].ang;
			sp->xrepeat = sp->yrepeat = 4;
		}
		else sp->xrepeat = sp->yrepeat = 32;

		changespritestat(i,5);
		break;

	case CRANE:

		sp->cstat |= CSTAT_ONESIDED|CSTAT_ALLBLOCKING;

		sp->picnum += 2;
		sp->z = sector[sect].ceilingz+(48<<8);
		hittype[i].temp_data[4] = tempwallptr;

		msx[tempwallptr] = sp->x;
		msy[tempwallptr] = sp->y;
		msx[tempwallptr+2] = sp->z;

		s = headspritestat[0];
		while(s >= 0)
		{
			if( sprite[s].picnum == CRANEPOLE && sprite[i].hitag == (sprite[s].hitag) )
			{
				msy[tempwallptr+2] = s;

				hittype[i].temp_data[1] = sprite[s].sectnum;

				sprite[s].xrepeat = 48;
				sprite[s].yrepeat = 128;

				msx[tempwallptr+1] = sprite[s].x;
				msy[tempwallptr+1] = sprite[s].y;

				sprite[s].x = sp->x;
				sprite[s].y = sp->y;
				sprite[s].z = sp->z;
				sprite[s].shade = sp->shade;

				setsprite(s,sprite[s].x,sprite[s].y,sprite[s].z);
				break;
			}
			s = nextspritestat[s];
		}

		tempwallptr += 3;
		sp->owner = -1;
		sp->extra = 8;
		changespritestat(i,6);
		break;

	case WATERDRIP:
		if(j >= 0 && sprite[j].statnum == 10 || sprite[j].statnum == 1)
		{
			sp->shade = 32;
			if(sprite[j].pal != 1)
			{
				sp->pal = 2;
				sp->z -= (18<<8);
			}
			else sp->z -= (13<<8);
			sp->ang = getangle(ps[connecthead].posx-sp->x,ps[connecthead].posy-sp->y);
			sp->xvel = 48-(TRAND&31);
			ssp(i,CLIPMASK0);
		}
		else if(j == -1)
		{
			sp->z += (4<<8);
			hittype[i].temp_data[0] = sp->z;
			hittype[i].temp_data[1] = TRAND&127;
		}
	case TRASH:

		if(sp->picnum != WATERDRIP)
			sp->ang = TRAND&2047;

	case WATERDRIPSPLASH:

		sp->xrepeat = 24;
		sp->yrepeat = 24;


		changespritestat(i,6);
		break;

	case PLUG:
		sp->lotag = 9999;
		changespritestat(i,6);
		break;
	case TOUCHPLATE:
		hittype[i].temp_data[2] = sector[sect].floorz;
		if(sector[sect].lotag != 1 && sector[sect].lotag != 2)
			sector[sect].floorz = sp->z;
		if(sp->pal && ud.multimode > 1)
		{
			sp->xrepeat=sp->yrepeat=0;
			changespritestat(i,5);
			break;
		}
	case WATERBUBBLEMAKER:
		sp->cstat |= CSTAT_INVISIBLE;
		changespritestat(i,6);
		break;
	case BOLT1:
	case BOLT1+1:
	case BOLT1+2:
	case BOLT1+3:
	case SIDEBOLT1:
	case SIDEBOLT1+1:
	case SIDEBOLT1+2:
	case SIDEBOLT1+3:
		hittype[i].temp_data[0] = sp->xrepeat;
		hittype[i].temp_data[1] = sp->yrepeat;
	case MASTERSWITCH:
		if(sp->picnum == MASTERSWITCH)
			sp->cstat |= CSTAT_INVISIBLE;
		sp->yvel = 0;
		changespritestat(i,6);
		break;
	case TARGET:
	case DUCK:
	case LETTER:
		sp->extra = 1;
		sp->cstat |= CSTAT_ALLBLOCKING;
		changespritestat(i,1);
		break;
	case OCTABRAINSTAYPUT:
	case LIZTROOPSTAYPUT:
	case PIGCOPSTAYPUT:
	case LIZMANSTAYPUT:
	case BOSS1STAYPUT:
	case PIGCOPDIVE:
	case COMMANDERSTAYPUT:
	case BOSS4STAYPUT:
		hittype[i].actorstayput = sp->sectnum;
	case BOSS1:
	case BOSS2:
	case BOSS3:
	case BOSS4:
	case ROTATEGUN:
	case GREENSLIME:
		if(sp->picnum == GREENSLIME)
			sp->extra = 1;
	case DRONE:
	case LIZTROOPONTOILET:
	case LIZTROOPJUSTSIT:
	case LIZTROOPSHOOT:
	case LIZTROOPJETPACK:
	case LIZTROOPDUCKING:
	case LIZTROOPRUNNING:
	case LIZTROOP:
	case OCTABRAIN:
	case COMMANDER:
	case PIGCOP:
	case LIZMAN:
	case LIZMANSPITTING:
	case LIZMANFEEDING:
	case LIZMANJUMP:
	case ORGANTIC:
	case RAT:
	case SHARK:

		if(sp->pal == 0)
		{
			switch(sp->picnum)
			{
			case LIZTROOPONTOILET:
			case LIZTROOPSHOOT:
			case LIZTROOPJETPACK:
			case LIZTROOPDUCKING:
			case LIZTROOPRUNNING:
			case LIZTROOPSTAYPUT:
			case LIZTROOPJUSTSIT:
			case LIZTROOP:
				sp->pal = 22;
				break;
			}
		}

		if( sp->picnum == BOSS4STAYPUT || sp->picnum == BOSS1 || sp->picnum == BOSS2 || sp->picnum == BOSS1STAYPUT || sp->picnum == BOSS3 || sp->picnum == BOSS4 )
		{
			if(j >= 0 && sprite[j].picnum == RESPAWN)
				sp->pal = sprite[j].pal;
			if(sp->pal)
			{
				sp->clipdist = 80;
				sp->xrepeat = 40;
				sp->yrepeat = 40;
			}
			else
			{
				sp->xrepeat = 80;
				sp->yrepeat = 80;
				sp->clipdist = 164;
			}
		}
		else
		{
			if(sp->picnum != SHARK)
			{
				sp->xrepeat = 40;
				sp->yrepeat = 40;
				sp->clipdist = 80;
			}
			else
			{
				sp->xrepeat = 60;
				sp->yrepeat = 60;
				sp->clipdist = 40;
			}
		}

		if(j >= 0) sp->lotag = 0;

		if( ( sp->lotag > ud.player_skill ) || ud.monsters_off == 1 )
		{
			sp->xrepeat=sp->yrepeat=0;
			changespritestat(i,5);
			break;
		}
		else
		{
			makeitfall(i);

			if(sp->picnum == RAT)
			{
				sp->ang = TRAND&2047;
				sp->xrepeat = sp->yrepeat = 48;
				sp->cstat = 0;
			}
			else
			{
				sp->cstat |= CSTAT_ALLBLOCKING;

				if(sp->picnum != SHARK)
					ps[myconnectindex].max_actors_killed++;
			}

			if(sp->picnum == ORGANTIC) sp->cstat |= CSTAT_CENTERED;

			if(j >= 0)
			{
				hittype[i].timetosleep = 0;
				check_fta_sounds(i);
				changespritestat(i,1);
			}
			else changespritestat(i,2);
		}

		if(sp->picnum == ROTATEGUN)
			sp->zvel = 0;

		break;

	case LOCATORS:
		sp->cstat |= CSTAT_INVISIBLE;
		changespritestat(i,7);
		break;

	case ACTIVATORLOCKED:
	case ACTIVATOR:
		sp->cstat = CSTAT_INVISIBLE;
		if(sp->picnum == ACTIVATORLOCKED)
			sector[sp->sectnum].lotag |= 16384;
		changespritestat(i,8);
		break;

	case DOORSHOCK:
		sp->cstat |= CSTAT_ALLBLOCKING;
		sp->shade = -12;
		changespritestat(i,6);
		break;

	case OOZ:
	case OOZ2:
		sp->shade = -12;

		if(j >= 0)
		{
			if( sprite[j].picnum == NUKEBARREL )
				sp->pal = 8;
			insertspriteq(i);
		}

		changespritestat(i,1);

		getglobalz(i);

		j = (hittype[i].floorz-hittype[i].ceilingz)>>9;

		sp->yrepeat = j;
		sp->xrepeat = 25-(j>>1);
		sp->cstat |= (TRAND & CSTAT_XFLIP);

		break;

	case HEAVYHBOMB:
		if(j >= 0)
			sp->owner = j;
		else sp->owner = i;
		sp->xrepeat = sp->yrepeat = 9;
		sp->yvel = 4;
	case REACTOR2:
	case REACTOR:
	case RECON:

		if(sp->picnum == RECON)
		{
			if( sp->lotag > ud.player_skill )
			{
				sp->xrepeat = sp->yrepeat = 0;
				changespritestat(i,5);
				return i;
			}
			ps[myconnectindex].max_actors_killed++;
			hittype[i].temp_data[5] = 0;
			if(ud.monsters_off == 1)
			{
				sp->xrepeat = sp->yrepeat = 0;
				changespritestat(i,5);
				break;
			}
			sp->extra = 130;
		}

		if(sp->picnum == REACTOR || sp->picnum == REACTOR2)
			sp->extra = impact_damage;

		sprite[i].cstat |= CSTAT_ALLBLOCKING; // Make it hitable

		if( ud.multimode < 2 && sp->pal != 0)
		{
			sp->xrepeat = sp->yrepeat = 0;
			changespritestat(i,5);
			break;
		}
		sp->pal = 0;
		sprite[i].shade = -17;

		changespritestat(i,2);
		break;

	case ATOMICHEALTH:
	case STEROIDS:
	case HEATSENSOR:
	case SHIELD:
	case AIRTANK:
	case TRIPBOMBSPRITE:
	case JETPACK:
	case HOLODUKE:

	case FIRSTGUNSPRITE:
	case CHAINGUNSPRITE:
	case SHOTGUNSPRITE:
	case RPGSPRITE:
	case SHRINKERSPRITE:
	case FREEZESPRITE:
	case DEVISTATORSPRITE:

	case SHOTGUNAMMO:
	case FREEZEAMMO:
	case HBOMBAMMO:
	case CRYSTALAMMO:
	case GROWAMMO:
	case BATTERYAMMO:
	case DEVISTATORAMMO:
	case RPGAMMO:
	case BOOTS:
	case AMMO:
	case AMMOLOTS:
	case COLA:
	case FIRSTAID:
	case SIXPAK:
		if(j >= 0)
		{
			sp->lotag = 0;
			sp->z -= (32<<8);
			sp->zvel = -1024;
			ssp(i,CLIPMASK0);
			sp->cstat = TRAND & CSTAT_XFLIP;
		}
		else
		{
			sp->owner = i;
			sp->cstat = 0;
		}

		if( ( ud.multimode < 2 && sp->pal != 0) || (sp->lotag > ud.player_skill) )
		{
			sp->xrepeat = sp->yrepeat = 0;
			changespritestat(i,5);
			break;
		}

		sp->pal = 0;

	case ACCESSCARD:

		if(sp->picnum == ATOMICHEALTH)
			sp->cstat |= CSTAT_CENTERED;

		if(ud.multimode > 1 && ud.coop != 1 && sp->picnum == ACCESSCARD)
		{
			sp->xrepeat = sp->yrepeat = 0;
			changespritestat(i,5);
			break;
		}
		else
		{
			if(sp->picnum == AMMO)
				sp->xrepeat = sp->yrepeat = 16;
			else sp->xrepeat = sp->yrepeat = 32;
		}

		sp->shade = -17;

		if(j >= 0) changespritestat(i,1);
		else
		{
			changespritestat(i,2);
			makeitfall(i);
		}
		break;

	case WATERFOUNTAIN:
		sprite[i].lotag = 1;

	case TREE1:
	case TREE2:
	case TIRE:
	case CONE:
	case BOX:
		sprite[i].cstat = CSTAT_ALLBLOCKING; // Make it hitable
		sprite[i].extra = 1;
		changespritestat(i,6);
		break;

	case FLOORFLAME:
		sp->shade = -127;
		changespritestat(i,6);
		break;

	case BOUNCEMINE:
		sp->owner = i;
		sp->cstat |= CSTAT_ALLBLOCKING; //Make it hitable
		sp->xrepeat = sp->yrepeat = 24;
		sp->shade = -127;
		sp->extra = impact_damage<<2;
		changespritestat(i,2);
		break;

	case CAMERA1:
	case CAMERA1+1:
	case CAMERA1+2:
	case CAMERA1+3:
	case CAMERA1+4:
	case CAMERAPOLE:
		sp->extra = 1;

		if(camerashitable) sp->cstat = CSTAT_ALLBLOCKING;
		else sp->cstat = 0;

	case GENERICPOLE:

		if( ud.multimode < 2 && sp->pal != 0 )
		{
			sp->xrepeat = sp->yrepeat = 0;
			changespritestat(i,5);
			break;
		}
		else sp->pal = 0;
		if(sp->picnum == CAMERAPOLE || sp->picnum == GENERICPOLE) break;
		sp->picnum = CAMERA1;
		changespritestat(i,1);
		break;
	case STEAM:
		if(j >= 0)
		{
			sp->ang = sprite[j].ang;
			sp->cstat = CSTAT_WALL + CSTAT_HITBLOCKING + CSTAT_TRANS;
			sp->xrepeat=sp->yrepeat=1;
			sp->xvel = -8;
			ssp(i,CLIPMASK0);
		}
	case CEILINGSTEAM:
		changespritestat(i,6);
		break;

	case SECTOREFFECTOR:
		sp->yvel = sector[sect].extra;
		sp->cstat |= CSTAT_INVISIBLE;
		sp->xrepeat = sp->yrepeat = 0;

		switch(sp->lotag)
		{
		case 28:
			hittype[i].temp_data[5] = 65;// Delay for lightning
			break;
		case 7: // Transporters!!!!
		case 23:// XPTR END
			if(sp->lotag != 23)
			{
				for(j=0;j<MAXSPRITES;j++)
					if(sprite[j].statnum < MAXSTATUS && sprite[j].picnum == SECTOREFFECTOR && ( sprite[j].lotag == 7 || sprite[j].lotag == 23 ) && i != j && sprite[j].hitag == sprite[i].hitag)
					{
						sprite[i].owner = j;
						break;
					}
			}
			else sprite[i].owner = i;

			hittype[i].temp_data[4] = sector[sect].floorz == sprite[i].z;
			sp->cstat = 0;
			changespritestat(i,9);
			return i;
		case 1:
			sp->owner = -1;
			hittype[i].temp_data[0] = 1;
			break;
		case 18:

			if(sp->ang == 512)
			{
				hittype[i].temp_data[1] = sector[sect].ceilingz;
				if(sp->pal)
					sector[sect].ceilingz = sp->z;
			}
			else
			{
				hittype[i].temp_data[1] = sector[sect].floorz;
				if(sp->pal)
					sector[sect].floorz = sp->z;
			}

			sp->hitag <<= 2;
			break;

		case 19:
			sp->owner = -1;
			break;
		case 25: // Pistons
			hittype[i].temp_data[3] = sector[sect].ceilingz;
			hittype[i].temp_data[4] = 1;
			sector[sect].ceilingz = sp->z;
			setinterpolation(&sector[sect].ceilingz);
			break;
		case 35:
			sector[sect].ceilingz = sp->z;
			break;
		case 27:
			if(ud.recstat == 1)
			{
				sp->xrepeat=sp->yrepeat=64;
				sp->cstat &= ~(CSTAT_INVISIBLE);
			}
			break;
		case 12:

			hittype[i].temp_data[1] = sector[sect].floorshade;
			hittype[i].temp_data[2] = sector[sect].ceilingshade;
			break;

		case 13:

			hittype[i].temp_data[0] = sector[sect].ceilingz;
			hittype[i].temp_data[1] = sector[sect].floorz;

			if( klabs(hittype[i].temp_data[0]-sp->z) < klabs(hittype[i].temp_data[1]-sp->z) )
				sp->owner = 1;
			else sp->owner = 0;

			if(sp->ang == 512)
			{
				if(sp->owner)
					sector[sect].ceilingz = sp->z;
				else
					sector[sect].floorz = sp->z;
			}
			else
				sector[sect].ceilingz = sector[sect].floorz = sp->z;

			if( sector[sect].ceilingstat&SSTAT_PARALLAX )
			{
				sector[sect].ceilingstat ^= SSTAT_PARALLAX;
				hittype[i].temp_data[3] = 1;

				if(!sp->owner && sp->ang==512)
				{
					sector[sect].ceilingstat ^= SSTAT_PARALLAX;
					hittype[i].temp_data[3] = 0;
				}

				sector[sect].ceilingshade =
					sector[sect].floorshade;

				if(sp->ang==512)
				{
					startwall = sector[sect].wallptr;
					endwall = startwall+sector[sect].wallnum;
					for(j=startwall;j<endwall;j++)
					{
						x = wall[j].nextsector;
						if(x >= 0)
							if( !(sector[x].ceilingstat&SSTAT_PARALLAX) )
							{
								sector[sect].ceilingpicnum = sector[x].ceilingpicnum;
								sector[sect].ceilingshade = sector[x].ceilingshade;
								break; //Leave earily
							}
					}
				}
			}

			break;

		case 17:

			hittype[i].temp_data[2] = sector[sect].floorz; //Stopping loc

			j = nextsectorneighborz(sect,sector[sect].floorz,-1,-1);
			hittype[i].temp_data[3] = sector[j].ceilingz;

			j = nextsectorneighborz(sect,sector[sect].ceilingz,1,1);
			hittype[i].temp_data[4] = sector[j].floorz;

			if(numplayers < 2)
			{
				setinterpolation(&sector[sect].floorz);
				setinterpolation(&sector[sect].ceilingz);
			}

			break;

		case 24:
			sp->yvel <<= 1;
		case 36:
			break;

		case 20:
			{
				long q;

				startwall = sector[sect].wallptr;
				endwall = startwall+sector[sect].wallnum;

				//find the two most clostest wall x's and y's
				q = 0x7fffffff;

				for(s=startwall;s<endwall;s++)
				{
					x = wall[s].x;
					y = wall[s].y;

					d = FindDistance2D(sp->x-x,sp->y-y);
					if( d < q )
					{
						q = d;
						clostest = s;
					}
				}

				hittype[i].temp_data[1] = clostest;

				q = 0x7fffffff;

				for(s=startwall;s<endwall;s++)
				{
					x = wall[s].x;
					y = wall[s].y;

					d = FindDistance2D(sp->x-x,sp->y-y);
					if(d < q && s != hittype[i].temp_data[1])
					{
						q = d;
						clostest = s;
					}
				}

				hittype[i].temp_data[2] = clostest;
			}

			break;

		case 3:

			hittype[i].temp_data[3]=sector[sect].floorshade;

			sector[sect].floorshade = sp->shade;
			sector[sect].ceilingshade = sp->shade;

			sp->owner = sector[sect].ceilingpal<<8;
			sp->owner |= sector[sect].floorpal;

			//fix all the walls;

			startwall = sector[sect].wallptr;
			endwall = startwall+sector[sect].wallnum;

			for(s=startwall;s<endwall;s++)
			{
				if(!(wall[s].hitag&1))
					wall[s].shade=sp->shade;
				if( (wall[s].cstat & WSTAT_SWAPBOTTOMS) && wall[s].nextwall >= 0)
					wall[wall[s].nextwall].shade = sp->shade;
			}
			break;

		case 31:
			hittype[i].temp_data[1] = sector[sect].floorz;
			//    hittype[i].temp_data[2] = sp->hitag;
			if(sp->ang != 1536) sector[sect].floorz = sp->z;

			startwall = sector[sect].wallptr;
			endwall = startwall+sector[sect].wallnum;

			for(s=startwall;s<endwall;s++)
				if(wall[s].hitag == 0) wall[s].hitag = 9999;

			setinterpolation(&sector[sect].floorz);

			break;
		case 32:
			hittype[i].temp_data[1] = sector[sect].ceilingz;
			hittype[i].temp_data[2] = sp->hitag;
			if(sp->ang != 1536) sector[sect].ceilingz = sp->z;

			startwall = sector[sect].wallptr;
			endwall = startwall+sector[sect].wallnum;

			for(s=startwall;s<endwall;s++)
				if(wall[s].hitag == 0) wall[s].hitag = 9999;

			setinterpolation(&sector[sect].ceilingz);

			break;

		case 4: //Flashing lights

			hittype[i].temp_data[2] = sector[sect].floorshade;

			startwall = sector[sect].wallptr;
			endwall = startwall+sector[sect].wallnum;

			sp->owner = sector[sect].ceilingpal<<8;
			sp->owner |= sector[sect].floorpal;

			for(s=startwall;s<endwall;s++)
				if(wall[s].shade > hittype[i].temp_data[3])
					hittype[i].temp_data[3] = wall[s].shade;

			break;

		case 9:
			if( sector[sect].lotag &&
				labs(sector[sect].ceilingz-sp->z) > 1024)
				sector[sect].lotag |= 32768; //If its open
		case 8:
			//First, get the ceiling-floor shade

			hittype[i].temp_data[0] = sector[sect].floorshade;
			hittype[i].temp_data[1] = sector[sect].ceilingshade;

			startwall = sector[sect].wallptr;
			endwall = startwall+sector[sect].wallnum;

			for(s=startwall;s<endwall;s++)
				if(wall[s].shade > hittype[i].temp_data[2])
					hittype[i].temp_data[2] = wall[s].shade;

			hittype[i].temp_data[3] = 1; //Take Out;

			break;

		case 11://Pivitor rotater
			if(sp->ang>1024) hittype[i].temp_data[3] = 2;
			else hittype[i].temp_data[3] = -2;
		case 0:
		case 2://Earthquakemakers
		case 5://Boss Creature
		case 6://Subway
		case 14://Caboos
		case 15://Subwaytype sliding door
		case 16://That rotating blocker reactor thing
		case 26://ESCELATOR
		case 30://No rotational subways

			if(sp->lotag == 0)
			{
				if( sector[sect].lotag == 30 )
				{
					if(sp->pal) sprite[i].clipdist = 1;
					else sprite[i].clipdist = 0;
					hittype[i].temp_data[3] = sector[sect].floorz;
					sector[sect].hitag = i;
				}

				for(j = 0;j < MAXSPRITES;j++)
				{
					if( sprite[j].statnum < MAXSTATUS )
						if( sprite[j].picnum == SECTOREFFECTOR &&
							sprite[j].lotag == 1 &&
							sprite[j].hitag == sp->hitag)
						{
							if( sp->ang == 512 )
							{
								sp->x = sprite[j].x;
								sp->y = sprite[j].y;
							}
							break;
						}
				}
				if(j == MAXSPRITES)
				{
					sprintf((char*)tempbuf,"Found lonely Sector Effector (lotag 0) at (%ld,%ld)\n",sp->x,sp->y);
					gameexit((char*)tempbuf);
				}
				sp->owner = j;
			}

			startwall = sector[sect].wallptr;
			endwall = startwall+sector[sect].wallnum;

			hittype[i].temp_data[1] = tempwallptr;
			for(s=startwall;s<endwall;s++)
			{
				msx[tempwallptr] = wall[s].x-sp->x;
				msy[tempwallptr] = wall[s].y-sp->y;
				tempwallptr++;
				if(tempwallptr > 2047)
				{
					sprintf((char*)tempbuf,"Too many moving sectors at (%ld,%ld).\n",wall[s].x,wall[s].y);
					gameexit((char*)tempbuf);
				}
			}
			if( sp->lotag == 30 || sp->lotag == 6 || sp->lotag == 14 || sp->lotag == 5 )
			{

				startwall = sector[sect].wallptr;
				endwall = startwall+sector[sect].wallnum;

				if(sector[sect].hitag == -1)
					sp->extra = 0;
				else sp->extra = 1;

				sector[sect].hitag = i;

				j = 0;

				for(s=startwall;s<endwall;s++)
				{
					if( wall[ s ].nextsector >= 0 &&
						sector[ wall[ s ].nextsector].hitag == 0 &&
						sector[ wall[ s ].nextsector].lotag < 3 )
					{
						s = wall[s].nextsector;
						j = 1;
						break;
					}
				}

				if(j == 0)
				{
					sprintf((char*)tempbuf,"Subway found no zero'd sectors with locators\nat (%ld,%ld).\n",sp->x,sp->y);
					gameexit((char*)tempbuf);
				}

				sp->owner = -1;
				hittype[i].temp_data[0] = s;

				if(sp->lotag != 30)
					hittype[i].temp_data[3] = sp->hitag;
			}

			else if(sp->lotag == 16)
				hittype[i].temp_data[3] = sector[sect].ceilingz;

			else if( sp->lotag == 26 )
			{
				hittype[i].temp_data[3] = sp->x;
				hittype[i].temp_data[4] = sp->y;
				if(sp->shade==sector[sect].floorshade) //UP
					sp->zvel = -256;
				else
					sp->zvel = 256;

				sp->shade = 0;
			}
			else if( sp->lotag == 2)
			{
				hittype[i].temp_data[5] = sector[sp->sectnum].floorheinum;
				sector[sp->sectnum].floorheinum = 0;
			}
		}

		switch(sp->lotag)
		{
		case 6:
		case 14:
			j = callsound(sect,i);
			if(j == -1) j = SUBWAY;
			hittype[i].lastvx = j;
		case 30:
			if(numplayers > 1) break;
		case 0:
		case 1:
		case 5:
		case 11:
		case 15:
		case 16:
		case 26:
			setsectinterpolate(i);
			break;
		}

		switch(sprite[i].lotag)
		{
		case 40:
		case 41:
		case 43:
		case 44:
		case 45:
			changespritestat(i,15);
			break;
		default:
			changespritestat(i,3);
			break;
		}

		break;


	case SEENINE:
	case OOZFILTER:

		sp->shade = -16;
		if(sp->xrepeat <= 8)
		{
			sp->cstat = CSTAT_INVISIBLE;
			sp->xrepeat=sp->yrepeat=0;
		}
		else sp->cstat = CSTAT_ALLBLOCKING;
		sp->extra = impact_damage<<2;
		sp->owner = i;

		changespritestat(i,6);
		break;

	case CRACK1:
	case CRACK2:
	case CRACK3:
	case CRACK4:
	case FIREEXT:
		if(sp->picnum == FIREEXT)
		{
			sp->cstat = CSTAT_ALLBLOCKING;
			sp->extra = impact_damage<<2;
		}
		else
		{
			sp->cstat |= (sp->cstat & CSTAT_TYPEMASK) ? CSTAT_CLIPBLOCKING
				: (CSTAT_CLIPBLOCKING | CSTAT_WALL);
			sp->extra = 1;
		}

		if( ud.multimode < 2 && sp->pal != 0)
		{
			sp->xrepeat = sp->yrepeat = 0;
			changespritestat(i,5);
			break;
		}

		sp->pal = 0;
		sp->owner = i;
		changespritestat(i,6);
		sp->xvel = 8;
		ssp(i,CLIPMASK0);
		break;

	case TOILET:
	case STALL:
		sp->lotag = 1;
		sp->cstat |= CSTAT_ALLBLOCKING;
		sp->clipdist = 8;
		sp->owner = i;
		break;
	case CANWITHSOMETHING:
	case CANWITHSOMETHING2:
	case CANWITHSOMETHING3:
	case CANWITHSOMETHING4:
	case RUBBERCAN:
		sp->extra = 0;
	case EXPLODINGBARREL:
	case HORSEONSIDE:
	case FIREBARREL:
	case NUKEBARREL:
	case FIREVASE:
	case NUKEBARRELDENTED:
	case NUKEBARRELLEAKED:
	case WOODENHORSE:

		if(j >= 0)
			sp->xrepeat = sp->yrepeat = 32;
		sp->clipdist = 72;
		makeitfall(i);
		if(j >= 0)
			sp->owner = j;
		else sp->owner = i;
	case EGG:
		if( ud.monsters_off == 1 && sp->picnum == EGG )
		{
			sp->xrepeat = sp->yrepeat = 0;
			changespritestat(i,5);
		}
		else
		{
			if(sp->picnum == EGG)
				sp->clipdist = 24;
			sp->cstat = CSTAT_ALLBLOCKING|(TRAND & CSTAT_XFLIP);
			changespritestat(i,2);
		}
		break;
	case TOILETWATER:
		sp->shade = -16;
		changespritestat(i,6);
		break;
	}
	return i;
}


void animatesprites(long x,long y,short a,long smoothratio)
{
	short i, j, k, p, sect;
	long l, t1,t3,t4;
	spritetype *s,*t;

	for(j=0;j < spritesortcnt; j++)
	{
		t = &tsprite[j];
		i = t->owner;
		s = &sprite[t->owner];

		switch(t->picnum)
		{
		case BLOODPOOL:
		case PUKE:
		case FOOTPRINTS:
		case FOOTPRINTS2:
		case FOOTPRINTS3:
		case FOOTPRINTS4:
			if(t->shade == 127) continue;
			break;
		case RESPAWNMARKERRED:
		case RESPAWNMARKERYELLOW:
		case RESPAWNMARKERGREEN:
			if(ud.marker == 0)
				t->xrepeat = t->yrepeat = 0;
			continue;
		case CHAIR3:

			k = (((t->ang+3072+128-a)&2047)>>8)&7;
			if(k>4)
			{
				k = 8-k;
				t->cstat |= CSTAT_XFLIP;
			}
			else t->cstat &= ~CSTAT_XFLIP;
			t->picnum = s->picnum+k;
			break;
		case BLOODSPLAT1:
		case BLOODSPLAT2:
		case BLOODSPLAT3:
		case BLOODSPLAT4:
			if(ud.lockout) t->xrepeat = t->yrepeat = 0;
			else if(t->pal == 6)
			{
				t->shade = -127;
				continue;
			}
		case BULLETHOLE:
		case CRACK1:
		case CRACK2:
		case CRACK3:
		case CRACK4:
			t->shade = 16;
			continue;
		case NEON1:
		case NEON2:
		case NEON3:
		case NEON4:
		case NEON5:
		case NEON6:
			continue;
		case GREENSLIME:
		case GREENSLIME+1:
		case GREENSLIME+2:
		case GREENSLIME+3:
		case GREENSLIME+4:
		case GREENSLIME+5:
		case GREENSLIME+6:
		case GREENSLIME+7:
			break;
		default:
			if( ( (t->cstat & CSTAT_TYPEMASK) == CSTAT_WALL ) ||
				( badguy(t) && t->extra > 0) ||
				t->statnum == 10)
			{
				continue;
			}
		}

		if (sector[t->sectnum].ceilingstat&SSTAT_PARALLAX)
			l = sector[t->sectnum].ceilingshade;
		else
			l = sector[t->sectnum].floorshade;

		if(l < -127) l = -127;
		if(l > 128) l =  127;
		t->shade = l;
	}


	for(j=0;j < spritesortcnt; j++ )  //Between drawrooms() and drawmasks()
	{                             //is the perfect time to animate sprites
		t = &tsprite[j];
		i = t->owner;
		s = &sprite[i];

		switch(s->picnum)
		{
		case SECTOREFFECTOR:
			if(t->lotag == 27 && ud.recstat == 1)
			{
				t->picnum = 11+((totalclock>>3)&1);
				t->cstat |= CSTAT_CENTERED;
			}
			else
				t->xrepeat = t->yrepeat = 0;
			break;
		case NATURALLIGHTNING:
			t->shade = -127;
			break;
		case FEM1:
		case FEM2:
		case FEM3:
		case FEM4:
		case FEM5:
		case FEM6:
		case FEM7:
		case FEM8:
		case FEM9:
		case FEM10:
		case MAN:
		case MAN2:
		case WOMAN:
		case NAKED1:
		case PODFEM1:
		case FEMMAG1:
		case FEMMAG2:
		case FEMPIC1:
		case FEMPIC2:
		case FEMPIC3:
		case FEMPIC4:
		case FEMPIC5:
		case FEMPIC6:
		case FEMPIC7:
		case BLOODYPOLE:
		case FEM6PAD:
		case STATUE:
		case STATUEFLASH:
		case OOZ:
		case OOZ2:
		case WALLBLOOD1:
		case WALLBLOOD2:
		case WALLBLOOD3:
		case WALLBLOOD4:
		case WALLBLOOD5:
		case WALLBLOOD7:
		case WALLBLOOD8:
		case SUSHIPLATE1:
		case SUSHIPLATE2:
		case SUSHIPLATE3:
		case SUSHIPLATE4:
		case FETUS:
		case FETUSJIB:
		case FETUSBROKE:
		case HOTMEAT:
		case FOODOBJECT16:
		case DOLPHIN1:
		case DOLPHIN2:
		case TOUGHGAL:
		case TAMPON:
		case XXXSTACY:
		case 4946:
		case 4947:
		case 693:
		case 2254:
		case 4560:
		case 4561:
		case 4562:
		case 4498:
		case 4957:
			if(ud.lockout)
			{
				t->xrepeat = t->yrepeat = 0;
				continue;
			}
		}

		if( t->statnum == 99 ) continue;
		if( s->statnum != 1 && s->picnum == APLAYER && ps[s->yvel].newowner == -1 && s->owner >= 0 )
		{
			t->x -= mulscale16(65536-smoothratio,ps[s->yvel].posx-ps[s->yvel].oposx);
			t->y -= mulscale16(65536-smoothratio,ps[s->yvel].posy-ps[s->yvel].oposy);
			t->z = ps[s->yvel].oposz + mulscale16(smoothratio,ps[s->yvel].posz-ps[s->yvel].oposz);
			t->z += (40<<8);
		}
		else if( ( s->statnum == 0 && s->picnum != CRANEPOLE) || s->statnum == 10 || s->statnum == 6 || s->statnum == 4 || s->statnum == 5 || s->statnum == 1 )
		{
			t->x -= mulscale16(65536-smoothratio,s->x-hittype[i].bposx);
			t->y -= mulscale16(65536-smoothratio,s->y-hittype[i].bposy);
			t->z -= mulscale16(65536-smoothratio,s->z-hittype[i].bposz);
		}

		sect = s->sectnum;
		t1 = hittype[i].temp_data[1];t3 = hittype[i].temp_data[3];t4 = hittype[i].temp_data[4];

		switch(s->picnum)
		{
		case DUKELYINGDEAD:
			t->z += (24<<8);
			break;
		case BLOODPOOL:
		case FOOTPRINTS:
		case FOOTPRINTS2:
		case FOOTPRINTS3:
		case FOOTPRINTS4:
			if(t->pal == 6)
				t->shade = -127;
		case PUKE:
		case MONEY:
		case MONEY+1:
		case MAIL:
		case MAIL+1:
		case PAPER:
		case PAPER+1:
			if(ud.lockout && s->pal == 2)
			{
				t->xrepeat = t->yrepeat = 0;
				continue;
			}
			break;
		case TRIPBOMB:
			continue;
		case FORCESPHERE:
			if(t->statnum == 5)
			{
				short sqa,sqb;

				sqa =
					getangle(
					sprite[s->owner].x-ps[screenpeek].posx,
					sprite[s->owner].y-ps[screenpeek].posy);
				sqb =
					getangle(
					sprite[s->owner].x-t->x,
					sprite[s->owner].y-t->y);

				if( klabs(getincangle(sqa,sqb)) > 512 )
					if( ldist(&sprite[s->owner],t) < ldist(&sprite[ps[screenpeek].i],&sprite[s->owner]) )
						t->xrepeat = t->yrepeat = 0;
			}
			continue;
		case BURNING:
		case BURNING2:
			if( sprite[s->owner].statnum == 10 )
			{
				if( display_mirror == 0 && sprite[s->owner].yvel == screenpeek && ps[sprite[s->owner].yvel].over_shoulder_on == 0 )
					t->xrepeat = 0;
				else
				{
					t->ang = getangle(x-t->x,y-t->y);
					t->x = sprite[s->owner].x;
					t->y = sprite[s->owner].y;
					t->x += sintable[(t->ang+512)&2047]>>10;
					t->y += sintable[t->ang&2047]>>10;
				}
			}
			break;

		case ATOMICHEALTH:
			t->z -= (4<<8);
			break;
		case CRYSTALAMMO:
			t->shade = (sintable[(totalclock<<4)&2047]>>10);
			continue;
		case VIEWSCREEN:
		case VIEWSCREEN2:
			if(camsprite >= 0 && hittype[sprite[i].owner].temp_data[0] == 1)
			{
				t->picnum = STATIC;
				t->cstat |= (rand() & (CSTAT_XFLIP|CSTAT_YFLIP));
				t->xrepeat += 8;
				t->yrepeat += 8;
			}
			break;

		case SHRINKSPARK:
			t->picnum = SHRINKSPARK+( (totalclock>>4)&3 );
			break;
		case GROWSPARK:
			t->picnum = GROWSPARK+( (totalclock>>4)&3 );
			break;
		case RPG:
			k = getangle(s->x-x,s->y-y);
			k = (((s->ang+3072+128-k)&2047)/170);
			if(k > 6)
			{
				k = 12-k;
				t->cstat |= CSTAT_XFLIP;
			}
			else t->cstat &= ~CSTAT_XFLIP;
			t->picnum = RPG+k;
			break;

		case RECON:

			k = getangle(s->x-x,s->y-y);
			if( hittype[i].temp_data[0] < 4 )
				k = (((s->ang+3072+128-k)&2047)/170);
			else k = (((s->ang+3072+128-k)&2047)/170);

			if(k>6)
			{
				k = 12-k;
				t->cstat |= CSTAT_XFLIP;
			}
			else t->cstat &= ~CSTAT_XFLIP;

			if( klabs(t3) > 64 ) k += 7;
			t->picnum = RECON+k;

			break;

		case APLAYER:

			p = s->yvel;

			if(t->pal == 1) t->z -= (18<<8);

			if(ps[p].over_shoulder_on > 0 && ps[p].newowner < 0 )
			{
				t->cstat |= CSTAT_TRANS;
				if ( screenpeek == myconnectindex && numplayers >= 2 )
				{
					t->x = omyx+mulscale16((long)(myx-omyx),smoothratio);
					t->y = omyy+mulscale16((long)(myy-omyy),smoothratio);
					t->z = omyz+mulscale16((long)(myz-omyz),smoothratio)+(40<<8);
					t->ang = omyang+mulscale16((long)(((myang+1024-omyang)&2047)-1024),smoothratio);
					t->sectnum = mycursectnum;
				}
			}

			if( ( display_mirror == 1 || screenpeek != p || s->owner == -1 ) && ud.multimode > 1 && ud.showweapons && sprite[ps[p].i].extra > 0 && ps[p].curr_weapon > 0 )
			{
				memcpy((spritetype *)&tsprite[spritesortcnt],(spritetype *)t,sizeof(spritetype));

				tsprite[spritesortcnt].statnum = 99;

				tsprite[spritesortcnt].yrepeat = ( t->yrepeat>>3 );
				if(t->yrepeat < 4) t->yrepeat = 4;

				tsprite[spritesortcnt].shade = t->shade;
				tsprite[spritesortcnt].cstat = 0;

				switch(ps[p].curr_weapon)
				{
				case PISTOL_WEAPON:      tsprite[spritesortcnt].picnum = FIRSTGUNSPRITE;       break;
				case SHOTGUN_WEAPON:     tsprite[spritesortcnt].picnum = SHOTGUNSPRITE;        break;
				case CHAINGUN_WEAPON:    tsprite[spritesortcnt].picnum = CHAINGUNSPRITE;       break;
				case RPG_WEAPON:         tsprite[spritesortcnt].picnum = RPGSPRITE;            break;
				case HANDREMOTE_WEAPON:
				case HANDBOMB_WEAPON:    tsprite[spritesortcnt].picnum = HEAVYHBOMB;           break;
				case TRIPBOMB_WEAPON:    tsprite[spritesortcnt].picnum = TRIPBOMBSPRITE;       break;
				case GROW_WEAPON:        tsprite[spritesortcnt].picnum = GROWSPRITEICON;       break;
				case SHRINKER_WEAPON:    tsprite[spritesortcnt].picnum = SHRINKERSPRITE;       break;
				case FREEZE_WEAPON:      tsprite[spritesortcnt].picnum = FREEZESPRITE;         break;
				case DEVISTATOR_WEAPON:  tsprite[spritesortcnt].picnum = DEVISTATORSPRITE;     break;
				}

				if(s->owner >= 0)
					tsprite[spritesortcnt].z = ps[p].posz-(12<<8);
				else tsprite[spritesortcnt].z = s->z-(51<<8);
				if(ps[p].curr_weapon == HANDBOMB_WEAPON)
				{
					tsprite[spritesortcnt].xrepeat = 10;
					tsprite[spritesortcnt].yrepeat = 10;
				}
				else
				{
					tsprite[spritesortcnt].xrepeat = 16;
					tsprite[spritesortcnt].yrepeat = 16;
				}
				tsprite[spritesortcnt].pal = 0;
				spritesortcnt++;
			}

			if(s->owner == -1)
			{
				k = (((s->ang+3072+128-a)&2047)>>8)&7;
				if(k>4)
				{
					k = 8-k;
					t->cstat |= CSTAT_XFLIP;
				}
				else t->cstat &= ~CSTAT_XFLIP;

				if(sector[t->sectnum].lotag == 2) k += 1795-1405;
				else if( (hittype[i].floorz-s->z) > (64<<8) ) k += 60;

				t->picnum += k;
				t->pal = ps[p].palookup;

				goto PALONLY;
			}

			if( ps[p].on_crane == -1 && (sector[s->sectnum].lotag&0x7ff) != 1 )
			{
				l = s->z-hittype[ps[p].i].floorz+(3<<8);
				if( l > 1024 && s->yrepeat > 32 && s->extra > 0 )
					s->yoffset = (signed char)(l/(s->yrepeat<<2));
				else s->yoffset=0;
			}

			if(ps[p].newowner > -1)
			{
				t4 = *(actorscrptr[APLAYER]+1);
				t3 = 0;
				t1 = *(actorscrptr[APLAYER]+2);
			}

			if(ud.camerasprite == -1 && ps[p].newowner == -1)
				if(s->owner >= 0 && display_mirror == 0 && ps[p].over_shoulder_on == 0 )
					if( ud.multimode < 2 || ( ud.multimode > 1 && p == screenpeek ) )
					{
						t->owner = -1;
						t->xrepeat = t->yrepeat = 0;
						continue;
					}

PALONLY:

					if( sector[sect].floorpal )
						t->pal = sector[sect].floorpal;

					if(s->owner == -1) continue;

					if( t->z > hittype[i].floorz && t->xrepeat < 32 )
						t->z = hittype[i].floorz;

					break;

		case JIBS1:
		case JIBS2:
		case JIBS3:
		case JIBS4:
		case JIBS5:
		case JIBS6:
		case HEADJIB1:
		case LEGJIB1:
		case ARMJIB1:
		case LIZMANHEAD1:
		case LIZMANARM1:
		case LIZMANLEG1:
		case DUKELEG:
		case DUKEGUN:
		case DUKETORSO:
			if(ud.lockout)
			{
				t->xrepeat = t->yrepeat = 0;
				continue;
			}
			if(t->pal == 6) t->shade = -120;

		case SCRAP1:
		case SCRAP2:
		case SCRAP3:
		case SCRAP4:
		case SCRAP5:
		case SCRAP6:
		case SCRAP6+1:
		case SCRAP6+2:
		case SCRAP6+3:
		case SCRAP6+4:
		case SCRAP6+5:
		case SCRAP6+6:
		case SCRAP6+7:

			if(hittype[i].picnum == BLIMP && t->picnum == SCRAP1 && s->yvel >= 0)
				t->picnum = s->yvel;
			else t->picnum += hittype[i].temp_data[0];
			t->shade -= 6;

			if( sector[sect].floorpal )
				t->pal = sector[sect].floorpal;
			break;

		case WATERBUBBLE:
			if(sector[t->sectnum].floorpicnum == FLOORSLIME)
			{
				t->pal = 7;
				break;
			}
		default:

			if( sector[sect].floorpal )
				t->pal = sector[sect].floorpal;
			break;
		}

		if( actorscrptr[s->picnum] )
		{
			if(t4)
			{
				l = *(long *)(t4+8);

				switch( l )
				{
				case 2:
					k = (((s->ang+3072+128-a)&2047)>>8)&1;
					break;

				case 3:
				case 4:
					k = (((s->ang+3072+128-a)&2047)>>7)&7;
					if(k > 3)
					{
						t->cstat |= CSTAT_XFLIP;
						k = 7-k;
					}
					else t->cstat &= ~CSTAT_XFLIP;
					break;

				case 5:
					k = getangle(s->x-x,s->y-y);
					k = (((s->ang+3072+128-k)&2047)>>8)&7;
					if(k>4)
					{
						k = 8-k;
						t->cstat |= CSTAT_XFLIP;
					}
					else t->cstat &= ~CSTAT_XFLIP;
					break;
				case 7:
					k = getangle(s->x-x,s->y-y);
					k = (((s->ang+3072+128-k)&2047)/170);
					if(k>6)
					{
						k = 12-k;
						t->cstat |= CSTAT_XFLIP;
					}
					else t->cstat &= ~CSTAT_XFLIP;
					break;
				case 8:
					k = (((s->ang+3072+128-a)&2047)>>8)&7;
					t->cstat &= ~CSTAT_XFLIP;
					break;
				default:
					k = 0;
					break;
				}

				t->picnum += k + ( *(long *)t4 ) + l * t3;

				if(l > 0) while(tilesizx[t->picnum] == 0 && t->picnum > 0 )
					t->picnum -= l;       //Hack, for actors

				if( hittype[i].dispicnum >= 0)
					hittype[i].dispicnum = t->picnum;
			}
			else if(display_mirror == 1)
				t->cstat |= CSTAT_XFLIP;
		}

		if( s->statnum == 13 || badguy(s) || (s->picnum == APLAYER && s->owner >= 0) )
			if(t->statnum != 99 && s->picnum != EXPLOSION2 && s->picnum != HANGLIGHT && s->picnum != DOMELITE)
				if(s->picnum != HOTMEAT)
				{
					if( hittype[i].dispicnum < 0 )
					{
						hittype[i].dispicnum++;
						continue;
					}
					else if( ud.shadows && spritesortcnt < (MAXSPRITESONSCREEN-2))
					{
						long daz,xrep,yrep;

						if( (sector[sect].lotag&0xff) > 2 || s->statnum == 4 || s->statnum == 5 || s->picnum == DRONE || s->picnum == COMMANDER )
							daz = sector[sect].floorz;
						else
							daz = hittype[i].floorz;

						if( (s->z-daz) < (8<<8) )
							if( ps[screenpeek].posz < daz )
							{
								memcpy((spritetype *)&tsprite[spritesortcnt],(spritetype *)t,sizeof(spritetype));

								tsprite[spritesortcnt].statnum = 99;

								tsprite[spritesortcnt].yrepeat = ( t->yrepeat>>3 );
								if(t->yrepeat < 4) t->yrepeat = 4;

								tsprite[spritesortcnt].shade = 127;
								tsprite[spritesortcnt].cstat |= CSTAT_TRANS;

								tsprite[spritesortcnt].z = daz;
								xrep = tsprite[spritesortcnt].xrepeat;// - (klabs(daz-t->z)>>11);
								tsprite[spritesortcnt].xrepeat = xrep;
								tsprite[spritesortcnt].pal = 4;

								yrep = tsprite[spritesortcnt].yrepeat;// - (klabs(daz-t->z)>>11);
								tsprite[spritesortcnt].yrepeat = yrep;
								spritesortcnt++;
							}
					}

					if( ps[screenpeek].heat_amount > 0 && ps[screenpeek].heat_on )
					{
						t->pal = 6;
						t->shade = 0;
					}
				}


				switch(s->picnum)
				{
				case LASERLINE:
					if(sector[t->sectnum].lotag == 2) t->pal = 8;
					t->z = sprite[s->owner].z-(3<<8);
					if(lasermode == 2 && ps[screenpeek].heat_on == 0 )
						t->yrepeat = 0;
				case EXPLOSION2:
				case EXPLOSION2BOT:
				case FREEZEBLAST:
				case ATOMICHEALTH:
				case FIRELASER:
				case SHRINKSPARK:
				case GROWSPARK:
				case CHAINGUN:
				case SHRINKEREXPLOSION:
				case RPG:
				case FLOORFLAME:
					if(t->picnum == EXPLOSION2)
					{
						ps[screenpeek].visibility = -127;
						lastvisinc = totalclock+32;
						restorepalette = 1;
					}
					t->shade = -127;
					break;
				case FIRE:
				case FIRE2:
				case BURNING:
				case BURNING2:
					if( sprite[s->owner].picnum != TREE1 && sprite[s->owner].picnum != TREE2 )
						t->z = sector[t->sectnum].floorz;
					t->shade = -127;
					break;
				case COOLEXPLOSION1:
					t->shade = -127;
					t->picnum += (s->shade>>1);
					break;
				case PLAYERONWATER:

					k = (((t->ang+3072+128-a)&2047)>>8)&7;
					if(k>4)
					{
						k = 8-k;
						t->cstat |= CSTAT_XFLIP;
					}
					else t->cstat &= ~CSTAT_XFLIP;

					t->picnum = s->picnum+k+((hittype[i].temp_data[0]<4)*5);
					t->shade = sprite[s->owner].shade;

					break;

				case WATERSPLASH2:
					t->picnum = WATERSPLASH2+t1;
					break;
				case REACTOR2:
					t->picnum = s->picnum + hittype[i].temp_data[2];
					break;
				case SHELL:
					t->picnum = s->picnum+(hittype[i].temp_data[0]&1);
				case SHOTGUNSHELL:
					t->cstat |= CSTAT_XFLIP | CSTAT_YFLIP;
					if(hittype[i].temp_data[0] > 1) t->cstat &= ~CSTAT_XFLIP;
					if(hittype[i].temp_data[0] > 2) t->cstat &= ~(CSTAT_XFLIP|CSTAT_YFLIP);
					break;
				case FRAMEEFFECT1:
					if(s->owner >= 0 && sprite[s->owner].statnum < MAXSTATUS)
					{
						if(sprite[s->owner].picnum == APLAYER)
							if(ud.camerasprite == -1)
								if(screenpeek == sprite[s->owner].yvel && display_mirror == 0)
								{
									t->owner = -1;
									break;
								}
						if( (sprite[s->owner].cstat&CSTAT_INVISIBLE) == 0 )
						{
							t->picnum = hittype[s->owner].dispicnum;
							t->pal = sprite[s->owner].pal;
							t->shade = sprite[s->owner].shade;
							t->ang = sprite[s->owner].ang;
							t->cstat = CSTAT_TRANS|sprite[s->owner].cstat;
						}
					}
					break;

				case CAMERA1:
				case RAT:
					k = (((t->ang+3072+128-a)&2047)>>8)&7;
					if(k>4)
					{
						k = 8-k;
						t->cstat |= CSTAT_XFLIP;
					}
					else t->cstat &= ~CSTAT_XFLIP;
					t->picnum = s->picnum+k;
					break;
				}

				hittype[i].dispicnum = t->picnum;
				if(sector[t->sectnum].floorpicnum == MIRROR)
					t->xrepeat = t->yrepeat = 0;
	}
}



#define NUMCHEATCODES 26
char cheatquotes_volA[NUMCHEATCODES][14] = {
	{"cornholio"},		// 0
	{"stuff"},			// 1
	{"scotty###"},		// 2
	{"coords"},			// 3
	{"view"},			// 4
	{"time"},			// 5
	{"unlock"},			// 6
	{"cashman"},		// 7
	{"items"},			// 8
	{"rate"},			// 9
	{"skill#"},			// 10
	{"beta"},			// 11
	{"hyper"},			// 12
	{"monsters"},		// 13
	// #ifndef VOLUMEONE
	//  {"bonus"},
	// #else
	{"<RESERVED>"},		// 14
	{"<RESERVED>"},		// 15
	// #endif
	{"todd"},			// 16
	{"showmap"},		// 17
	{"kroz"},			// 18
	{"allen"},			// 19
	{"clip"},			// 20
	{"weapons"},		// 21
	{"inventory"},		// 22
	{"keys"},			// 23
	{"debug"},			// 24
	{"ending"}			// [RH] uncomment this

};

char cheatquotes_vol1[NUMCHEATCODES][14] = {
	{"cornholio"},		// 0
	{"stuff"},			// 1
	{"scotty##"},		// 2
	{"coords"},			// 3
	{"view"},			// 4
	{"time"},			// 5
	{"<RESERVED>"},		// 6
	{"cashman"},		// 7
	{"items"},			// 8
	{"rate"},			// 9
	{"skill#"},			// 10
	{"beta"},			// 11
	{"hyper"},			// 12
	{"monsters"},		// 13
	{"<RESERVED>"},		// 14
	{"<RESERVED>"},		// 15
	{"todd"},			// 16
	{"showmap"},		// 17
	{"kroz"},			// 18
	{"allen"},			// 19
	{"clip"},			// 20
	{"weapons"},		// 21
	{"inventory"},		// 22
	{"keys"},			// 23
	{"debug"},			// 24
	//{"ending"}

};


char cheatbuf[10],cheatbuflen;
bool Cheat_Responder (const event_t *ev)
{
	short ch, i, j, k, weapon;

	if( (ps[myconnectindex].gm&MODE_TYPE) || (ps[myconnectindex].gm&MODE_MENU))
		return false;

	if (ev->type != EV_KeyDown)
		return false;

	if ( ps[myconnectindex].cheat_phase == 1)
	{
		ch = tolower (ev->data2);

		if( !( (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ) )
		{
			ps[myconnectindex].cheat_phase = 0;
			//             FTA(46,&ps[myconnectindex]);
			return false;
		}

		cheatbuf[cheatbuflen++] = ch;
		cheatbuf[cheatbuflen] = 0;

		if(cheatbuflen > 11)
		{
			ps[myconnectindex].cheat_phase = 0;
			return false;
		}

		char (*cheatquotes)[14];

		cheatquotes = VOLUMEALL ? cheatquotes_volA : cheatquotes_vol1;

		for(k = 0;k < NUMCHEATCODES;k++)
		{
			for(j = 0;j<cheatbuflen;j++)
			{
				if( cheatbuf[j] == cheatquotes[k][j] || (cheatquotes[k][j] == '#' && ch >= '0' && ch <= '9') )
				{
					if( cheatquotes[k][j+1] == 0 ) goto FOUNDCHEAT;
					if(j == cheatbuflen-1) return true;
				}
				else break;
			}
		}

		ps[myconnectindex].cheat_phase = 0;
		return false;

		FOUNDCHEAT:
		{
			switch(k)
			{
			case 21:		// weapons
				j = VOLUMEONE ? 6 : 0;

				for ( weapon = PISTOL_WEAPON;weapon < MAX_WEAPONS-j;weapon++ )
				{
					addammo( weapon, &ps[myconnectindex], max_ammo_amount[weapon] );
					ps[myconnectindex].gotweapon[weapon]  = 1;
				}

				ps[myconnectindex].cheat_phase = 0;
				FTA(119,&ps[myconnectindex]);
				return true;
			case 22:		// inventory
				ps[myconnectindex].cheat_phase = 0;
				ps[myconnectindex].steroids_amount =         400;
				ps[myconnectindex].heat_amount     =        1200;
				ps[myconnectindex].boot_amount          =    200;
				ps[myconnectindex].shield_amount =           100;
				ps[myconnectindex].scuba_amount =            6400;
				ps[myconnectindex].holoduke_amount =         2400;
				ps[myconnectindex].jetpack_amount =          1600;
				ps[myconnectindex].firstaid_amount =         max_player_health;
				FTA(120,&ps[myconnectindex]);
				ps[myconnectindex].cheat_phase = 0;
				return true;
			case 23:		// keys
				ps[myconnectindex].got_access =              7;
				ps[myconnectindex].cheat_phase = 0;
				FTA(121,&ps[myconnectindex]);
				return true;
			case 24:		// debug
				debug_on = 1-debug_on;
				ps[myconnectindex].cheat_phase = 0;
				return true;
			case 20:		// clip
				ud.clipping = 1-ud.clipping;
				ps[myconnectindex].cheat_phase = 0;
				FTA(112+ud.clipping,&ps[myconnectindex]);
				return true;

			case 15:		// <RESERVED>
				ps[myconnectindex].gm = MODE_EOL;
				ps[myconnectindex].cheat_phase = 0;
				return true;

			case 19:		// allen
				FTA(79,&ps[myconnectindex]);
				ps[myconnectindex].cheat_phase = 0;
				return true;
			case 0:			// cornholio
			case 18:		// kroz

				ud.god = 1-ud.god;

				if(ud.god)
				{
					pus = 1;
					pub = 1;
					sprite[ps[myconnectindex].i].cstat = CSTAT_ALLBLOCKING;

					hittype[ps[myconnectindex].i].temp_data[0] = 0;
					hittype[ps[myconnectindex].i].temp_data[1] = 0;
					hittype[ps[myconnectindex].i].temp_data[2] = 0;
					hittype[ps[myconnectindex].i].temp_data[3] = 0;
					hittype[ps[myconnectindex].i].temp_data[4] = 0;
					hittype[ps[myconnectindex].i].temp_data[5] = 0;

					sprite[ps[myconnectindex].i].hitag = 0;
					sprite[ps[myconnectindex].i].lotag = 0;
					sprite[ps[myconnectindex].i].pal =
						ps[myconnectindex].palookup;

					FTA(17,&ps[myconnectindex]);
				}
				else
				{
					ud.god = 0;
					sprite[ps[myconnectindex].i].extra = max_player_health;
					hittype[ps[myconnectindex].i].extra = -1;
					ps[myconnectindex].last_extra = max_player_health;
					FTA(18,&ps[myconnectindex]);
				}

				sprite[ps[myconnectindex].i].extra = max_player_health;
				hittype[ps[myconnectindex].i].extra = 0;
				ps[myconnectindex].cheat_phase = 0;

				return true;

			case 1:		// stuff

				j = VOLUMEONE ? 6 : 0;

				for ( weapon = PISTOL_WEAPON;weapon < MAX_WEAPONS-j;weapon++ )
					ps[myconnectindex].gotweapon[weapon]  = 1;

				for ( weapon = PISTOL_WEAPON;
					weapon < (MAX_WEAPONS-j);
					weapon++ )
					addammo( weapon, &ps[myconnectindex], max_ammo_amount[weapon] );

				ps[myconnectindex].ammo_amount[GROW_WEAPON] = 50;

				ps[myconnectindex].steroids_amount =         400;
				ps[myconnectindex].heat_amount     =        1200;
				ps[myconnectindex].boot_amount          =    200;
				ps[myconnectindex].shield_amount =           100;
				ps[myconnectindex].scuba_amount =            6400;
				ps[myconnectindex].holoduke_amount =         2400;
				ps[myconnectindex].jetpack_amount =          1600;
				ps[myconnectindex].firstaid_amount =         max_player_health;

				ps[myconnectindex].got_access =              7;
				FTA(5,&ps[myconnectindex]);
				ps[myconnectindex].cheat_phase = 0;


				//                        FTA(21,&ps[myconnectindex]);
				ps[myconnectindex].cheat_phase = 0;
				ps[myconnectindex].inven_icon = 1;
				return true;

			case 2:			// scotty
			case 10:		// skill
				if (ONELEVELDEMO)
				{
					ps[myconnectindex].cheat_phase = 0;
					break;
				}

				if(k == 2)	// scotty
				{
					short volnume,levnume;

					volnume = cheatbuf[6] - '0';
					if (VOLUMEALL)
					{
						levnume = (cheatbuf[7] - '0')*10+(cheatbuf[8]-'0');
					}
					else
					{
						levnume =  cheatbuf[7] - '0';
					}

					volnume--;
					levnume--;
					if((VOLUMEONE && volnume > 0) ||
						(PLUTOPAK && volnume > 4) ||
						(!PLUTOPAK && volnume > 3))
					{
						ps[myconnectindex].cheat_phase = 0;
						return true;
					}
					else

						if(volnume == 0)
						{
							if(levnume > 7-VOLUMEONE)
							{
								ps[myconnectindex].cheat_phase = 0;
								return true;
							}
						}
						else
						{
							if(levnume >= 11)
							{
								ps[myconnectindex].cheat_phase = 0;
								return true;
							}
						}

						ud.m_volume_number = ud.volume_number = volnume;
						ud.m_level_number = ud.level_number = levnume;

				}
				else ud.m_player_skill = ud.player_skill =
					cheatbuf[5] - '1';

				if(numplayers > 1 && myconnectindex == connecthead)
				{
					tempbuf[0] = 5;
					tempbuf[1] = ud.m_level_number;
					tempbuf[2] = ud.m_volume_number;
					tempbuf[3] = ud.m_player_skill;
					tempbuf[4] = ud.m_monsters_off;
					tempbuf[5] = ud.m_respawn_monsters;
					tempbuf[6] = ud.m_respawn_items;
					tempbuf[7] = ud.m_respawn_inventory;
					tempbuf[8] = ud.m_coop;
					tempbuf[9] = ud.m_marker;
					tempbuf[10] = ud.m_ffire;

					for(i=connecthead;i>=0;i=connectpoint2[i])
						sendpacket(i,tempbuf,11);
				}
				else ps[myconnectindex].gm |= MODE_RESTART;

				ps[myconnectindex].cheat_phase = 0;
				return true;

			case 3:			// coords
				ps[myconnectindex].cheat_phase = 0;
				ud.coords = 1-ud.coords;
				return true;

			case 4:			// view
				if( ps[myconnectindex].over_shoulder_on )
					ps[myconnectindex].over_shoulder_on = 0;
				else
				{
					ps[myconnectindex].over_shoulder_on = 1;
					cameradist = 0;
					cameraclock = totalclock;
				}
				FTA(22,&ps[myconnectindex]);
				ps[myconnectindex].cheat_phase = 0;
				return true;

			case 5:			// time
				FTA(21,&ps[myconnectindex]);
				ps[myconnectindex].cheat_phase = 0;
				return true;

			case 6:			// unlock
				for(i=numsectors-1;i>=0;i--) //Unlock
				{
					j = sector[i].lotag;
					if(j == -1 || j == 32767) continue;
					if( (j & 0x7fff) > 2 )
					{
						if( j&(0xffff-16384) )
							sector[i].lotag &= (0xffff-16384);
						operatesectors(i,ps[myconnectindex].i);
					}
				}
				operateforcefields(ps[myconnectindex].i,-1);

				FTA(100,&ps[myconnectindex]);
				ps[myconnectindex].cheat_phase = 0;
				return true;

			case 7:			// cashman
				ud.cashman = 1-ud.cashman;
				ps[myconnectindex].cheat_phase = 0;
				return true;

			case 8:			// items
				ps[myconnectindex].steroids_amount =         400;
				ps[myconnectindex].heat_amount     =        1200;
				ps[myconnectindex].boot_amount          =    200;
				ps[myconnectindex].shield_amount =           100;
				ps[myconnectindex].scuba_amount =            6400;
				ps[myconnectindex].holoduke_amount =         2400;
				ps[myconnectindex].jetpack_amount =          1600;

				ps[myconnectindex].firstaid_amount =         max_player_health;
				ps[myconnectindex].got_access =              7;
				FTA(5,&ps[myconnectindex]);
				ps[myconnectindex].cheat_phase = 0;
				return true;

			case 17: // SHOW ALL OF THE MAP TOGGLE;	// showmap
				ud.showallmap = 1-ud.showallmap;
				if(ud.showallmap)
				{
					for(i=0;i<(MAXSECTORS>>3);i++)
						show2dsector[i] = 255;
					for(i=0;i<(MAXWALLS>>3);i++)
						show2dwall[i] = 255;
					FTA(111,&ps[myconnectindex]);
				}
				else
				{
					for(i=0;i<(MAXSECTORS>>3);i++)
						show2dsector[i] = 0;
					for(i=0;i<(MAXWALLS>>3);i++)
						show2dwall[i] = 0;
					FTA(1,&ps[myconnectindex]);
				}
				ps[myconnectindex].cheat_phase = 0;
				return true;

			case 16:		// todd
				FTA(99,&ps[myconnectindex]);
				ps[myconnectindex].cheat_phase = 0;
				return true;
			case 9:			// rate
				ud.tickrate = !ud.tickrate;
				ps[myconnectindex].cheat_phase = 0;
				return true;
			case 11:		// beta
				FTA(105,&ps[myconnectindex]);
				ps[myconnectindex].cheat_phase = 0;
				return true;
			case 12:		// hyper
				ps[myconnectindex].steroids_amount = 399;
				ps[myconnectindex].heat_amount = 1200;
				ps[myconnectindex].cheat_phase = 0;
				FTA(37,&ps[myconnectindex]);
				return true;
			case 13:		// monsters
				if(actor_tog == 3) actor_tog = 0;
				actor_tog++;
				ps[screenpeek].cheat_phase = 0;
				return true;
			case 14:		// bonus
			case 25:		// ending, but it's commented out
				ud.eog = 1;
				ps[myconnectindex].gm |= MODE_EOL;
				return true;
			}
		}
	}
	else
	{
		if (ev->data2 == 'D' || ev->data2 == 'd')
		{
			if( ps[myconnectindex].cheat_phase >= 0 && numplayers < 2 && ud.recstat == 0)
				ps[myconnectindex].cheat_phase = -1;
		}

		if (ev->data2 == 'N' || ev->data2 == 'n')
		{
			if( ps[myconnectindex].cheat_phase == -1 )
			{
				if(ud.player_skill == 4)
				{
					FTA(22,&ps[myconnectindex]);
					ps[myconnectindex].cheat_phase = 0;
				}
				else
				{
					ps[myconnectindex].cheat_phase = 1;
					//                    FTA(25,&ps[myconnectindex]);
					cheatbuflen = 0;
				}
			}
			else if(ps[myconnectindex].cheat_phase != 0)
			{
				ps[myconnectindex].cheat_phase = 0;
			}
		}
	}
	return false;
}


long nonsharedtimer;
bool ToggleFullscreen;
CVAR (Bool, musicselectoron, false, 0)
EXTERN_CVAR (Bool, fullscreen)

void nonsharedkeys(void)
{
	long j;

	musicselectoron = QuoteVisible (26);

	if (ToggleFullscreen)
	{
		ToggleFullscreen = false;
		fullscreen = !fullscreen;
	}

	C_DispatchCommands (totalclock);

	if(ud.recstat == 2)
	{
		ControlInfo noshareinfo;
		CONTROL_GetInput( &noshareinfo );
	}

	if (!ALT_IS_PRESSED && ud.overhead_on == 0)
	{
        if( BUTTON( gamefunc_Enlarge_Screen ) )
        {
            CONTROL_ClearButton( gamefunc_Enlarge_Screen );
            if(ud.screen_size > 0)
                sound(THUD);
            ud.screen_size -= 4;
            vscrn();
        }
        if( BUTTON( gamefunc_Shrink_Screen ) )
        {
            CONTROL_ClearButton( gamefunc_Shrink_Screen );
            if(ud.screen_size < 64) sound(THUD);
            ud.screen_size += 4;
            vscrn();
        }
	}

	if( ps[myconnectindex].cheat_phase == 1 || ps[myconnectindex].gm&(MODE_MENU|MODE_TYPE)) return;

	if(!ALT_IS_PRESSED && !SHIFTS_IS_PRESSED)
	{
        if( ud.overhead_on != 0)
        {
            j = totalclock-nonsharedtimer; nonsharedtimer += j;
            if ( BUTTON( gamefunc_Enlarge_Screen ) )
                ps[myconnectindex].zoom += mulscale6(j,max(ps[myconnectindex].zoom,256));
            if ( BUTTON( gamefunc_Shrink_Screen ) )
                ps[myconnectindex].zoom -= mulscale6(j,max(ps[myconnectindex].zoom,256));

            if( (ps[myconnectindex].zoom > 2048) )
                ps[myconnectindex].zoom = 2048;
            if( (ps[myconnectindex].zoom < 48) )
                ps[myconnectindex].zoom = 48;

        }
	}
}

// The following are all moved from nonsharedkeys

bool NonShared_Responder (const event_t *ev)
{
	if (ev->type != EV_KeyDown)
	{
		return false;
	}
	if (ud.show_help)
	{
		if (ev->data1 == KEY_SPACE || ev->data1 == KEY_ENTER || ev->data1 == KEY_KPENTER)
		{
			AddCommandString ("helpscreen");
			return true;
		}
		else if (ev->data1 == KEY_ESCAPE)
		{
			AddCommandString ("helpscreen 0");
			return true;
		}
	}
	if (ev->data1 == KEY_ESCAPE && ud.overhead_on && ps[myconnectindex].newowner == -1)
	{
		ud.last_overhead = ud.overhead_on;
		ud.overhead_on = 0;
		ud.scrollmode = 0;
		vscrn();
		return true;
	}
	return false;
}

CCMD (taunttext)
{
	if ((ps[myconnectindex].gm & (MODE_GAME|MODE_MENU|MODE_TYPE)) != MODE_GAME)
		return;

	if (ud.recstat == 2 || argv.argc() < 2)
		return;

	int i = atoi (argv[1]) - 1;
	if (i < 0 || i > 9)
		return;

	adduserquote(ud.ridecule[i]);

	int ch = 0;

	tempbuf[ch] = 4;
	tempbuf[ch+1] = 0;
	strcat((char*)tempbuf+1,ud.ridecule[i]);

	i = 1+strlen(ud.ridecule[i]);

	if(ud.multimode > 1)
		for(ch=connecthead;ch>=0;ch=connectpoint2[ch])
			if (ch != myconnectindex)
				sendpacket(ch,tempbuf,i);

	pus = NUMPAGES;
	pub = NUMPAGES;
}

CCMD (tauntvoice)
{
	if ((ps[myconnectindex].gm & (MODE_GAME|MODE_MENU|MODE_TYPE)) != MODE_GAME)
		return;

	if(ud.recstat != 2 || ud.lockout == 0 && argv.argc() >= 2)
	{
		int i = atoi (argv[1]) - 1;
		if (i < 0 || i > 9)
		{
			return;
		}
		if(SoundToggle && ( RTS_NumSounds() > 0 ) && rtsplaying == 0 && VoiceToggle )
		{
			sound (RTS_FirstSound + i);

			rtsplaying = 7;

			if(ud.multimode > 1)
			{
				tempbuf[0] = 7;
				tempbuf[1] = i;

				for(int ch=connecthead;ch>=0;ch=connectpoint2[ch])
					if(ch != myconnectindex)
						sendpacket(ch,tempbuf,2);
			}

			pus = NUMPAGES;
			pub = NUMPAGES;
		}
	}
}

CCMD (helpscreen)
{
	if (argv.argc() == 1)
	{
		ud.show_help++;
	}
	else
	{
		ud.show_help = max (0, atoi (argv[1]));
	}
	if( ud.show_help > 2 )
	{
		ud.show_help = 0;
		vscrn();
	}
	else
	{
		setview(0,0,xdim-1,ydim-1);
	}
	if(ud.multimode < 2 && ud.recstat != 2)
	{
		ready2send = (ud.show_help == 0);
		totalclock = ototalclock;
	}
}

CCMD (menu_save)
{
	if(movesperpacket == 4 && connecthead != myconnectindex || ud.recstat == 2)
		return;

	if(sprite[ps[myconnectindex].i].extra <= 0)
	{
		FTA(118,&ps[myconnectindex]);
		return;
	}
	cmenu(350);
	screencapt = 1;
	displayrooms(myconnectindex,65536);
	savetemp("duke3d.tmp",(long)waloff[MAXTILES-1],160*100);
	screencapt = 0;
	FX_StopAllSounds();
	clearsoundlocks();

	//                setview(0,0,xdim-1,ydim-1);
	ps[myconnectindex].gm |= MODE_MENU;

	if(ud.multimode < 2)
	{
		ready2send = 0;
		totalclock = ototalclock;
		screenpeek = myconnectindex;
	}
}

CCMD (menu_load)
{
	if(movesperpacket == 4 && connecthead != myconnectindex)
		return;

	cmenu(300);
	FX_StopAllSounds();
	clearsoundlocks();

	//                setview(0,0,xdim-1,ydim-1);
	ps[myconnectindex].gm |= MODE_MENU;
	if(ud.multimode < 2 && ud.recstat != 2)
	{
		ready2send = 0;
		totalclock = ototalclock;
	}
	screenpeek = myconnectindex;
}

CCMD (menu_sound)
{
	if (FXDevice != NumSoundCards)
	{
		FX_StopAllSounds();
		clearsoundlocks();

		ps[myconnectindex].gm |= MODE_MENU;
		if(ud.multimode < 2 && ud.recstat != 2)
		{
			ready2send = 0;
			totalclock = ototalclock;
		}
		cmenu(700);
	}
}

CCMD (quicksave)
{
	if(ps[myconnectindex].gm&MODE_GAME)
	{
		if(movesperpacket == 4 && connecthead != myconnectindex)
			return;

		if(lastsavedpos == -1)
		{
			AddCommandString ("menu_save");
			return;
		}

		if(sprite[ps[myconnectindex].i].extra <= 0)
		{
			FTA(118,&ps[myconnectindex]);
			return;
		}
		screencapt = 1;
		displayrooms(myconnectindex,65536);
		savetemp("duke3d.tmp",(long)waloff[MAXTILES-1],160*100);
		screencapt = 0;
		if( lastsavedpos >= 0 )
		{
			inputloc = strlen(&ud.savegame[lastsavedpos][0]);
			current_menu = 360+lastsavedpos;
			probey = lastsavedpos;
		}
		FX_StopAllSounds();
		clearsoundlocks();

		setview(0,0,xdim-1,ydim-1);
		ps[myconnectindex].gm |= MODE_MENU;
		if(ud.multimode < 2 && ud.recstat != 2)
		{
			ready2send = 0;
			totalclock = ototalclock;
		}
	}
}

CCMD (quickload)
{
	if( (ps[myconnectindex].gm&MODE_GAME) )
	{
		if(movesperpacket == 4 && myconnectindex != connecthead)
			return;

		if( lastsavedpos >= 0 ) cmenu(15001);
		else cmenu(25000);
		FX_StopAllSounds();
		clearsoundlocks();
		ps[myconnectindex].gm |= MODE_MENU;
		if(ud.multimode < 2 && ud.recstat != 2)
		{
			ready2send = 0;
			totalclock = ototalclock;
		}
	}
}

CCMD (overshoulder)
{
	if( ps[myconnectindex].over_shoulder_on )
		ps[myconnectindex].over_shoulder_on = 0;
	else
	{
		ps[myconnectindex].over_shoulder_on = 1;
		cameradist = 0;
		cameraclock = totalclock;
	}
	FTA(109+ps[myconnectindex].over_shoulder_on,&ps[myconnectindex]);
}

CCMD (musicselector)
{
	if( MusicDevice != NumSoundCards )
	{
		sprintf(&fta_quotes[26][0], "%s.  USE SHIFT-F5 TO CHANGE.", &music_fn[0][music_select][0]);
		FTA(26,&ps[myconnectindex]);
	}
}

CCMD (musicselector_next)
{
	music_select++;
	if (VOLUMEALL)
	{
		if(music_select == 44) music_select = 0;
	}
	else
	{
		if(music_select == 6) music_select = 0;
	}
	playmusic(&music_fn[0][music_select][0]);
	sprintf(&fta_quotes[26][0], "PLAYING %s", &music_fn[0][music_select][0]);
	FTA(26,&ps[myconnectindex]);
}

CCMD (togglemessages)
{
	ud.fta_on = !ud.fta_on;
	if(ud.fta_on) FTA(23,&ps[myconnectindex]);
	else
	{
		ud.fta_on = 1;
		FTA(24,&ps[myconnectindex]);
		ud.fta_on = 0;
	}
}

CCMD (toggleautorun)
{
	ud.auto_run = 1-ud.auto_run;
	FTA(85+ud.auto_run,&ps[myconnectindex]);
}

CCMD (bumpgamma)
{
	if(SHIFTS_IS_PRESSED) ud.brightness-=4;
	else ud.brightness+=4;

	if (ud.brightness > (7<<2) )
		ud.brightness = 0;
	else if(ud.brightness < 0)
		ud.brightness = (7<<2);

	setbrightness(ud.brightness>>2,&ps[myconnectindex].palette[0]);
	if(ud.brightness < 20) FTA( 29 + (ud.brightness>>2) ,&ps[myconnectindex]);
	else if(ud.brightness < 40) FTA( 96 + (ud.brightness>>2) - 5,&ps[myconnectindex]);
}

CCMD (menu_quit)
{
	cmenu(500);
	FX_StopAllSounds();
	clearsoundlocks();
	ps[myconnectindex].gm |= MODE_MENU;
	if(ud.multimode < 2 && ud.recstat != 2)
	{
		ready2send = 0;
		totalclock = ototalclock;
	}
}

CCMD (togglecrosshair)
{
	ud.crosshair = 1-ud.crosshair;
	FTA(21-ud.crosshair,&ps[screenpeek]);
}

CCMD (mapfollow)
{
	if (ud.overhead_on)
	{
		ud.scrollmode = 1-ud.scrollmode;
		if(ud.scrollmode)
		{
			ud.folx = ps[screenpeek].oposx;
			ud.foly = ps[screenpeek].oposy;
			ud.fola = ps[screenpeek].oang;
		}
		FTA(83+ud.scrollmode,&ps[myconnectindex]);
	}
}

char screenshotname[] = "duke0000.pcx";

CCMD (screenshot)
{
	screencapture (screenshotname,0);
	FTA(103,&ps[myconnectindex]);
}

CCMD (spynext)
{
	if (ud.coop == 1 || ud.recstat == 2)
	{
		screenpeek = connectpoint2[screenpeek];
		if(screenpeek == -1) screenpeek = connecthead;
		restorepalette = 1;
	}
}

CCMD (showweapons)
{
	if (ud.multimode > 1)
	{
		ud.showweapons = 1-ud.showweapons;
		FTA(82-ud.showweapons,&ps[screenpeek]);
	}
}

CCMD (togglemap)
{
	if( ud.last_overhead != ud.overhead_on && ud.last_overhead)
	{
		ud.overhead_on = ud.last_overhead;
		ud.last_overhead = 0;
	}
	else
	{
		ud.overhead_on++;
		if(ud.overhead_on == 3 ) ud.overhead_on = 0;
		ud.last_overhead = ud.overhead_on;
	}
	restorepalette = 1;
	vscrn();
}

CCMD (messagemode)
{
	if( ud.multimode > 1 )
	{
		ps[myconnectindex].gm |= MODE_TYPE;
		typebuf[0] = 0;
		inputloc = 0;
	}
}

void comlinehelp(char **argv)
{
	Printf("Command line help.  %s [/flags...]\n",argv[0]);
	Printf(" ?, /?         This help message\n");
	Printf(" /l##          Level (1-11)\n");
	Printf(" /v#           Volume (1-4)\n");
	Printf(" /s#           Skill (1-4)\n");
	Printf(" /r            Record demo\n");
	Printf(" /dFILE        Start to play demo FILE\n");
	Printf(" /m            No monsters\n");
	Printf(" /ns           No sound\n");
	Printf(" /nm           No music\n");
	Printf(" /t#           Respawn, 1 = Monsters, 2 = Items, 3 = Inventory, x = All\n");
	Printf(" /c#           MP mode, 1 = DukeMatch(spawn), 2 = Coop, 3 = Dukematch(no spawn)\n");
	Printf(" /q#           Fake multiplayer (2-8 players)\n");
	Printf(" /a            Use player AI (fake multiplayer only)\n");
	Printf(" /i#           Network mode (1/0) (multiplayer only) (default == 1)\n");
	Printf(" /f#           Send fewer packets (1, 2, 4) (multiplayer only)\n");
	Printf(" /gFILE, /g... Use multiple group files (must be last on command line)\n");
	Printf(" /xFILE        Compile FILE (default GAME.CON)\n");
	Printf(" /u#########   User's favorite weapon order (default: 3425689071)\n");
	Printf(" /#            Load and run a game (slot 0-9)\n");
	Printf(" /z            Skip memory check\n");
	Printf(" -map FILE     Use a map FILE\n");
	Printf(" -name NAME    Foward NAME\n");
	Printf(" -net          Net mode game\n");
	Printf("\n\n");
}

void checkcommandline(int argc,char **argv)
{
	short i, j;
	char *c;

	i = 1;

	ud.fta_on = 1;
	ud.god = 0;
	ud.m_respawn_items = 0;
	ud.m_respawn_monsters = 0;
	ud.m_respawn_inventory = 0;
	ud.warp_on = 0;
	ud.cashman = 0;
	ud.m_player_skill = ud.player_skill = 2;

	if(argc > 1)
	{
		while(i < argc)
		{
			c = argv[i];

			if(*c == '?')
			{
				comlinehelp(argv);
				exit(-1);
			}

			if(*c == '/')
			{
				c++;
				switch(*c)
				{
				default:
					//                      printf("Unknown command line parameter '%s'\n",argv[i]);
				case '?':
					comlinehelp(argv);
					exit(0);
				case 'x':
				case 'X':
					c++;
					if(*c)
					{
						strcpy(confilename,c);
						if(SafeFileExists(c) == 0)
						{
							I_FatalError ("Could not find con file '%s'.\n",confilename );
						}
						else
						{
							Printf("Using con file: '%s'\n",confilename);
						}
					}
					break;
				case 'g':
				case 'G':
					c++;
					if(*c)
					{
						if( strchr(c,'.') == 0)
							strcat(c,".grp");

						j = initgroupfile(c);
						if( j == -1 )
							Printf("Could not find group file %s.\n",c);
						else
						{
							groupfile = j;
							Printf("Using group file %s.\n",c);
						}
					}

					break;
				case 'a':
				case 'A':
					ud.playerai = 1;
					Printf("Other player AI.\n");
					break;
				case 'n':
				case 'N':
					c++;
					if(*c == 's' || *c == 'S')
					{
						CommandSoundToggleOff = 2;
						Printf("Sound off.\n");
					}
					else if(*c == 'm' || *c == 'M')
					{
						CommandMusicToggleOff = 1;
						Printf("Music off.\n");
					}
					else
					{
						comlinehelp(argv);
						exit(-1);
					}
					break;
				case 'i':
				case 'I':
					c++;
					if(*c == '0') networkmode = 0;
					if(*c == '1') networkmode = 1;
					Printf("Network Mode %d\n",networkmode);
					break;
				case 'c':
				case 'C':
					c++;
					if(*c == '1' || *c == '2' || *c == '3' )
						ud.m_coop = *c - '0' - 1;
					else ud.m_coop = 0;

					switch(ud.m_coop)
					{
					case 0:
						Printf("Dukematch (spawn).\n");
						break;
					case 1:
						Printf("Cooperative play.\n");
						break;
					case 2:
						Printf("Dukematch (no spawn).\n");
						break;
					}

					break;
				case 'z':
				case 'Z':
					memorycheckoveride = 1;
					break;
				case 'f':
				case 'F':
					c++;
					if(*c == '1')
						movesperpacket = 1;
					if(*c == '2')
						movesperpacket = 2;
					if(*c == '4')
					{
						movesperpacket = 4;
						setpackettimeout(0x3fffffff,0x3fffffff);
					}
					break;
				case 't':
				case 'T':
					c++;
					if(*c == '1') ud.m_respawn_monsters = 1;
					else if(*c == '2') ud.m_respawn_items = 1;
					else if(*c == '3') ud.m_respawn_inventory = 1;
					else
					{
						ud.m_respawn_monsters = 1;
						ud.m_respawn_items = 1;
						ud.m_respawn_inventory = 1;
					}
					Printf("Respawn on.\n");
					break;
				case 'm':
				case 'M':
					if( *(c+1) != 'a' && *(c+1) != 'A' )
					{
						ud.m_monsters_off = 1;
						ud.m_player_skill = ud.player_skill = 0;
						Printf("Monsters off.\n");
					}
					break;
				case 'w':
				case 'W':
					ud.coords = 1;
					break;
				case 'q':
				case 'Q':
					Printf("Fake multiplayer mode.\n");
					if( *(++c) == 0) ud.multimode = 1;
					else ud.multimode = atol(c)%17;
					ud.m_coop = ud.coop = 0;
					ud.m_marker = ud.marker = 1;
					ud.m_respawn_monsters = ud.respawn_monsters = 1;
					ud.m_respawn_items = ud.respawn_items = 1;
					ud.m_respawn_inventory = ud.respawn_inventory = 1;

					break;
				case 'r':
				case 'R':
					ud.m_recstat = 1;
					Printf("Demo record mode on.\n");
					break;
				case 'd':
				case 'D':
					c++;
					if( strchr(c,'.') == 0)
						strcat(c,".dmo");
					Printf("Play demo %s.\n",c);
					strcpy(firstdemofile,c);
					break;
				case 'l':
				case 'L':
					ud.warp_on = 1;
					c++;
					ud.m_level_number = ud.level_number = (atol(c)-1)%11;
					break;
				case 'j':
				case 'J':
					if (VOLUMEALL)
					{
						Printf("Duke Nukem 3D (FULL VERSION) v%s\n",VERSION);
					}
					else
					{
						Printf("Duke Nukem 3D (SHAREWARE) v%s\n",VERSION);
					}

					exit(0);

				case 'v':
				case 'V':
					c++;
					ud.warp_on = 1;
					ud.m_volume_number = ud.volume_number = atol(c)-1;
					break;
				case 's':
				case 'S':
					c++;
					ud.m_player_skill = ud.player_skill = (atol(c)%5);
					if(ud.m_player_skill == 4)
						ud.m_respawn_monsters = ud.respawn_monsters = 1;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					ud.warp_on = 2 + (*c) - '0';
					break;
				case 'u':
				case 'U':
					c++;
					j = 0;
					if(*c)
					{
						Printf("Using favorite weapon order(s).\n");
						while(*c)
						{
							ud.wchoice[0][j] = *c-'0';
							c++;
							j++;
						}
						while(j < 10)
						{
							if(j == 9)
								ud.wchoice[0][9] = 1;
							else
								ud.wchoice[0][j] = 2;

							j++;
						}
					}
					else
					{
						Printf("Using default weapon orders.\n");
						ud.wchoice[0][0] = 3;
						ud.wchoice[0][1] = 4;
						ud.wchoice[0][2] = 5;
						ud.wchoice[0][3] = 7;
						ud.wchoice[0][4] = 8;
						ud.wchoice[0][5] = 6;
						ud.wchoice[0][6] = 0;
						ud.wchoice[0][7] = 2;
						ud.wchoice[0][8] = 9;
						ud.wchoice[0][9] = 1;
					}

					break;
				}
			}
			i++;
		}
	}
}



void printstr(short x, short y, char string[81], char attribute)
{
	Printf ("%s\n", string);
#if 0
	char character;
	short i, pos;

	pos = (y*80+x)<<1;
	i = 0;
	while (string[i] != 0)
	{
		character = string[i];
		printchrasm(0xb8000+(long)pos,1L,((long)attribute<<8)+(long)character);
		i++;
		pos+=2;
	}
#endif
}

/*
void cacheicon(void)
{
if(cachecount > 0)
{
if( (ps[myconnectindex].gm&MODE_MENU) == 0 )
rotatesprite((320-7)<<16,(200-23)<<16,32768L,0,SPINNINGNUKEICON,0,0,2,windowx1,windowy1,windowx2,windowy2);
cachecount = 0;
}
}
*/

bool SkipLogoTime;

bool Logo_Responder (const event_t *ev)
{
	if (ev->type == EV_KeyDown && C_GetSpecialBinding (ev->data1) == SBIND_Screenshot)
	{
		screencapture (screenshotname, 0);
		return true;
	}
	else if (ev->type == EV_KeyDown ||
		(ev->type == EV_GUI_Event && ev->subtype == EV_GUI_KeyDown))
	{
		SkipLogoTime = true;
		return true;
	}
	return false;
}

bool CheckSkipLogoTime ()
{
	if (SkipLogoTime)
	{
		return true;
	}
	I_StartTic ();
	D_ProcessEvents ();
	if (SkipLogoTime)
	{
		return true;
	}
	I_WaitClocks (1);	// relinquish the CPU for a little while
	return false;
}

void ClearSkipLogoTime ()
{
	I_StartTic ();
	D_ProcessEvents ();
	SkipLogoTime = false;
}

void Logo(void)
{
	short soundanm;

	D_AddResponder (Logo_Responder, 50);
	ClearSkipLogoTime ();

	soundanm = 0;

	ready2send = 0;

	setview(0,0,xdim-1,ydim-1);
	clearview(0L);
	palto(0,0,0,63);

	flushperms();
	nextpage();

	MUSIC_StopSong();

	if (VOLUMEALL)
	{
		if(!SkipLogoTime && nomorelogohack == 0)
		{
			getpackets();
			playanm("logo.anm",5);
			palto(0,0,0,63);
			ClearSkipLogoTime ();
		}

		clearview(0L);
		nextpage();
	}

	playmusic(&env_music_fn[0][0]);
	FadeOut (7);
	ps[myconnectindex].palette = drealms;
	palto(0,0,0,63);
	rotatesprite(0,0,65536L,0,DREALMS,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
	FadeIn (7);
	totalclock = 0;
	while( totalclock < (TICRATE*7) && !CheckSkipLogoTime() )
		getpackets();

	FadeOut (7);
	clearview(0L);

	ps[myconnectindex].palette = titlepal;
	flushperms();
	rotatesprite(0,0,65536L,0,BETASCREEN,0,0,2+8+16+64,0,0,xdim-1,ydim-1);
	ClearSkipLogoTime ();
	FadeIn (7);
	totalclock = 0;

	while(totalclock < (860+120) && !CheckSkipLogoTime())
	{
		rotatesprite(0,0,65536L,0,BETASCREEN,0,0,2+8+16+64,0,0,xdim-1,ydim-1);

		if( totalclock > 120 && totalclock < (120+60) )
		{
			if(soundanm == 0)
			{
				soundanm = 1;
				sound(PIPEBOMB_EXPLODE);
			}
			rotatesprite(160<<16,104<<16,(totalclock-120)<<10,0,DUKENUKEM,0,0,2+8,0,0,xdim-1,ydim-1);
		}
		else if( totalclock >= (120+60) )
			rotatesprite(160<<16,(104)<<16,60<<10,0,DUKENUKEM,0,0,2+8,0,0,xdim-1,ydim-1);

		if( totalclock > 220 && totalclock < (220+30) )
		{
			if( soundanm == 1)
			{
				soundanm = 2;
				sound(PIPEBOMB_EXPLODE);
			}

			rotatesprite(160<<16,(104)<<16,60<<10,0,DUKENUKEM,0,0,2+8,0,0,xdim-1,ydim-1);
			rotatesprite(160<<16,(129)<<16,(totalclock - 220 )<<11,0,THREEDEE,0,0,2+8,0,0,xdim-1,ydim-1);
		}
		else if( totalclock >= (220+30) )
			rotatesprite(160<<16,(129)<<16,30<<11,0,THREEDEE,0,0,2+8,0,0,xdim-1,ydim-1);

		if( totalclock >= 280 && totalclock < 395 )
		{
			rotatesprite(160<<16,(151)<<16,(410-totalclock)<<12,0,PLUTOPAKSPRITE+1,0,0,2+8,0,0,xdim-1,ydim-1);
			if(soundanm == 2)
			{
				soundanm = 3;
				sound(FLY_BY);
			}
		}
		else if( totalclock >= 395 )
		{
			if(soundanm == 3)
			{
				soundanm = 4;
				sound(PIPEBOMB_EXPLODE);
			}
			rotatesprite(160<<16,(151)<<16,30<<11,0,PLUTOPAKSPRITE+1,0,0,2+8,0,0,xdim-1,ydim-1);
		}

		getpackets();
		nextpage();
	}

	D_RemoveResponder (Logo_Responder);

	if(ud.multimode > 1)
	{
		rotatesprite(0,0,65536L,0,BETASCREEN,0,0,2+8+16+64,0,0,xdim-1,ydim-1);

		rotatesprite(160<<16,(104)<<16,60<<10,0,DUKENUKEM,0,0,2+8,0,0,xdim-1,ydim-1);
		rotatesprite(160<<16,(129)<<16,30<<11,0,THREEDEE,0,0,2+8,0,0,xdim-1,ydim-1);
		rotatesprite(160<<16,(151)<<16,30<<11,0,PLUTOPAKSPRITE+1,0,0,2+8,0,0,xdim-1,ydim-1);

		gametext(160,190,"WAITING FOR PLAYERS",14,2);
		nextpage();
	}

	waitforeverybody();

	flushperms();
	clearview(0L);
	nextpage();

	ps[myconnectindex].palette = palette;
	sound(NITEVISION_ONOFF);

	palto(0,0,0,0);
	clearview(0L);
	D_RemoveResponder (Logo_Responder);
}

void loadtmb(void)
{
	char tmb[8000];
	long fil, l;

	fil = kopen4load("d3dtimbr.tmb",0);
	if(fil == -1) return;
	l = kfilelength(fil);
	kread(fil,(char *)tmb,l);
	MUSIC_RegisterTimbreBank((unsigned char *)tmb);
	kclose(fil);
}

/*
===================
=
= ShutDown
=
===================
*/

void ShutDown( void )
{
	SoundShutdown();
	MusicShutdown();
	uninitengine();
	CONTROL_Shutdown();
	CONFIG_WriteSetup();
}

static char todd[] = "Duke Nukem 3D(tm) Copyright 1989, 1996 Todd Replogle and 3D Realms Entertainment";
static char trees[] = "I want to make a game with trees";
static char sixteen[] = "16 Possible Dukes";

/*
===================
=
= Startup
=
===================
*/

void compilecons(void)
{
	mymembuf = (char *)&hittype[0];
	labelcode = (long *)&sector[0];
	label = (char *)&sprite[0];

	//   printf("%ld %ld %ld\n",sizeof(hittype),sizeof(sector),sizeof(sprite));
	//   exit(0);

	loadefs(confilename,mymembuf);
	if( loadfromgrouponly )
	{
		Printf("  * Writing defaults to current directory.\n");
		loadefs(confilename,mymembuf);
	}
}


void Startup(void)
{
	int i;

	compilecons();

	CONFIG_GetSetupFilename();
	CONFIG_ReadSetup();

#ifdef AUSTRALIA
	ud.lockout = 1;
#endif

	if(CommandSoundToggleOff) SoundToggle = 0;
	if(CommandMusicToggleOff) MusicToggle = 0;

	if (VOLUMEONE)
	{
		Printf("\n*** You have run Duke Nukem 3D %ld times. ***\n\n",ud.executions);
		if(ud.executions >= 50) Printf("IT IS NOW TIME TO UPGRADE TO THE COMPLETE VERSION!!!\n\n");
	}

	CONTROL_Startup( (controltype)ControllerType, &GetTime, TICRATE );

	// CTW - MODIFICATION
	// initengine(ScreenMode,ScreenWidth,ScreenHeight);
	initengine();
	// CTW END - MODIFICATION

	Printf("* Hold Esc to Abort. *\n");
	Printf("Loading art header.\n");
	loadpics("tiles000.art");

	readsavenames();

	tilesizx[MIRROR] = tilesizy[MIRROR] = 0;

	for(i=0;i<MAXPLAYERS;i++) playerreadyflag[i] = 0;
	initmultiplayers(0,0,0);

	if(numplayers > 1)
		Printf("Multiplayer initialized.\n");

	ps[myconnectindex].palette = (char *) &palette[0];

	if(networkmode == 255)
		networkmode = 1;

	Printf("Checking music inits.\n");
	MusicStartup();
	Printf("Checking sound inits.\n");
	SoundStartup();
	S_Init ();
	loadtmb();
}


void sendscore(BYTE *s)
{
	if(numplayers > 1)
		genericmultifunction(-1,s,strlen((char *)s)+1,5);
}


void getnames(void)
{
	short i,l;

	for(l=0;myname[l];l++)
	{
		ud.user_name[myconnectindex][l] = toupper(myname[l]);
		buf[l+2] = toupper(myname[l]);
	}

	if(numplayers > 1)
	{
		buf[0] = 6;
		buf[1] = BYTEVERSION;

		buf[l+2] = 0;
		l += 3;

		for(i=connecthead;i>=0;i=connectpoint2[i])
			if( i != myconnectindex )
				sendpacket(i,(BYTE *)&buf[0],l);

		//      getpackets();

		l = 1;
		buf[0] = 9;

		for(i=0;i<10;i++)
		{
			ud.wchoice[myconnectindex][i] = ud.wchoice[0][i];
			buf[l] = (char) ud.wchoice[0][i];
			l++;
		}

		for(i=connecthead;i>=0;i=connectpoint2[i])
			if(i != myconnectindex)
				sendpacket(i,(BYTE *)&buf[0],11);

		//        getpackets();

		buf[0] = 10;
		buf[1] = ps[0].aim_mode;
		ps[myconnectindex].aim_mode = ps[0].aim_mode;

		for(i=connecthead;i>=0;i=connectpoint2[i])
			if(i != myconnectindex)
				sendpacket(i,(BYTE *)buf,2);

		//        getpackets();

		buf[0] = 125;

		for(i=connecthead;i>=0;i=connectpoint2[i])
			if(i != myconnectindex)
				sendpacket(i,(BYTE *)buf,1);

		getpackets();

		waitforeverybody();
	}
}

void writestring(long a1,long a2,long a3,short a4,long vx,long vy,long vz)
{

	FILE *fp;

	fp = (FILE *)fopen("debug.txt","rt+");

	fprintf(fp,"%ld %ld %ld %ld %ld %ld %ld\n",a1,a2,a3,a4,vx,vy,vz);

	fclose(fp);

}

void D_DoomMain (void)
{
	long i, j;
	int32 tempautorun;

	todd[0] = 'T';
	sixteen[0] = 'D';
	trees[0] = 'I';

	//    setvmode(0x03);

	D_AddResponder (C_Responder, 100);
	D_AddResponder (Type_Responder, 300);
	D_AddResponder (M_Responder, 400);
	D_AddResponder (Cheat_Responder, 500);
	D_AddResponder (NonShared_Responder, 560);
	D_AddResponder (C_DoKey, 600);
	D_AddResponder (G_Responder, 650);

	//printstr(0,0,"                                                                                ",79);

	initgroupfile("duke3d.grp");

	VOLUMEALL = VOLUMEONE = PLUTOPAK = ONELEVELDEMO = false;

	if (-1 != (i = kopen4load ("e2l1.map", true)))
	{
		VOLUMEALL = true;
		kclose (i);
	}
	if (-1 != (i = kopen4load ("e4l1.map", true)))
	{
		PLUTOPAK = true;
		kclose (i);
	}
	VOLUMEONE = !PLUTOPAK && !VOLUMEALL;
	if (VOLUMEONE && -1 == (i = kopen4load ("e1l2.map", true)))
	{
		ONELEVELDEMO = true;
	}
	else
	{
		kclose (i);
	}

	if (VOLUMEALL)
	{
#ifdef AUSTRALIA
		printstr(40-(strlen(HEAD2A)>>1),0,HEAD2A,79);
#else
		printstr(40-(strlen(HEAD2)>>1),0,PLUTOPAK?HEAD2P:HEAD2,79);
#endif
	}
	else
	{
#ifdef AUSTRALIA
		printstr(40-(strlen(HEADA)>>1),0,HEADA,79);
#else
		printstr(40-(strlen(HEAD)>>1),0,HEAD,79);
#endif
	}

	if (ONELEVELDEMO)
	{
		printstr(33,1,"ONE LEVEL DEMO",79);
	}

	ud.multimode = 1;
	printstr(0,1,"                   Copyright (c) 1996 3D Realms Entertainment                   ",79);

	//    printstr(0,2,"  ***     DUKE NUKEM v1.4 BETA VERSION.  USED FOR INTERNAL USE ONLY!!!     ***  ",79);

	//Printf("\n\n");

	I_Init ();

	checkcommandline(Args.NumArgs(),Args.GetArgList(0));

	totalmemory = Z_AvailHeap();

	if(memorycheckoveride == 0)
	{
		if(totalmemory < (3162000-350000))
		{
			I_FatalError ("You don't have enough free memory to run Duke Nukem 3D.\n"
				"The DOS \"mem\" command should report 6,800K (or 6.8 megs)\n"
				"of \"total memory free\".\n\n"
				"Duke Nukem 3D requires %ld more bytes to run.\n",3162000-350000-totalmemory);
		}
	}
	else
	{
		Printf("Using %ld bytes for heap.\n",totalmemory);
	}

#ifndef ONELEVELDEMO
	// CTW - REMOVED
	/*  if(movesperpacket == 4)
	TENtext();*/
	// CTW END - REMOVED
#endif

	RegisterShutdownFunction( ShutDown );

	if (VOLUMEONE)
	{
		Printf("Distribution of shareware Duke Nukem 3D is restricted in certain ways.\n");
		Printf("Please read LICENSE.DOC for more details.\n\n");
	}

	if (ONELEVELDEMO)
	{
		I_OkayMessage (
			"DUKE NUKEM 3D SINGLE-LEVEL PROMOTIONAL EDITION\n\n"
			"This single-level promotional edition of Duke Nukem 3D (tm) may not be\n"
			"distributed domestically (North America) by any publication other than\n"
			"Computer Gaming World, a Ziff-Davis publication.  It is a promotional\n"
			"version, licensed for a single month's run, and may not be redistributed\n"
			"by any online service, BBS, commercial publisher, magazine or distributor\n"
			"International distribution rights are reserved.\n\n"
			"Please read LICENSE.DOC for further information about this special version.\n"
			"NOTE: DUKE NUKEM 3D CONTAINS MATURE CONTENT.");
	}

	V_Init ();

	// Base systems have been inited; enable cvar callbacks
	FBaseCVar::EnableCallbacks ();

	Startup();

	if(numplayers > 1)
	{
		ud.multimode = numplayers;
		sendlogon();
	}
	else if(boardfilename[0] != 0)
	{
		ud.m_level_number = 7;
		ud.m_volume_number = 0;
		ud.warp_on = 1;
	}

	getnames();

	if(ud.multimode > 1)
	{
		playerswhenstarted = ud.multimode;

		if(ud.warp_on == 0)
		{
			ud.m_monsters_off = 1;
			ud.m_player_skill = 0;
		}
	}

	ud.last_level = -1;

	RTS_Init(ud.rtsname);
	if(numlumps) Printf("Using .RTS file:%s\n",ud.rtsname);

	if (CONTROL_JoystickEnabled)
		CONTROL_CenterJoystick
		(
		CenterCenter,
		UpperLeft,
		LowerRight,
		CenterThrottle,
		CenterRudder
		);

	Printf("Loading palette/lookups.\n");

	screen->Lock (false);
	ScreenWidth = screen->GetWidth ();
	ScreenHeight = screen->GetHeight ();
	ScreenMode = 1;
	setgamemode ((char*)screen->GetBuffer(), ScreenWidth, ScreenHeight, screen->GetPitch());
	screen->Unlock ();
	// CTW END - MODIFICATION

	genspriteremaps();

	if (VOLUMEONE && (numplayers > 4 || ud.multimode > 4))
		gameexit(" The full version of Duke Nukem 3D supports 5 or more players.");

	setbrightness(ud.brightness>>2,&ps[myconnectindex].palette[0]);
	C_HideConsole ();

	D_AddResponder (Escape_Grabber, 1);
	I_StartTic ();
	D_ProcessEvents ();
	D_RemoveResponder (Escape_Grabber);
	if (GotEscape)
	{
		qe = 1;
		gameexit (" ");
	}

	I_HideLogWindow ();
	FX_StopAllSounds();
	clearsoundlocks();

	if(ud.warp_on > 1 && ud.multimode < 2)
	{
		clearview(0L);
		ps[myconnectindex].palette = palette;
		palto(0,0,0,0);
		rotatesprite(320<<15,200<<15,65536L,0,LOADSCREEN,0,0,2+8+64,0,0,xdim-1,ydim-1);
		menutext(160,105,0,0,"LOADING SAVED GAME...");
		nextpage();

		j = loadplayer(ud.warp_on-2);
		if(j)
			ud.warp_on = 0;
	}

	//    getpackets();

MAIN_LOOP_RESTART:

	if(ud.warp_on == 0 && !(ps[myconnectindex].gm & MODE_MENU))
	{
		Logo();
	}
	else if(ud.warp_on == 1)
	{
		newgame(ud.m_volume_number,ud.m_level_number,ud.m_player_skill);
		enterlevel(MODE_GAME);
	}
	else vscrn();

	tempautorun = ud.auto_run;

	if( ud.warp_on == 0 && playback() )
	{
		FX_StopAllSounds();
		clearsoundlocks();
		nomorelogohack = 1;
		goto MAIN_LOOP_RESTART;
	}

	ud.auto_run = tempautorun;

	ud.warp_on = 0;

	while ( !(ps[myconnectindex].gm&MODE_END) ) //The whole loop!!!!!!!!!!!!!!!!!!
	{
		I_StartTic ();
		D_ProcessEvents ();

		if( ud.recstat == 2 || ud.multimode > 1 || ( ud.show_help == 0 && (ps[myconnectindex].gm&MODE_MENU) != MODE_MENU ) )
			if( ps[myconnectindex].gm&MODE_GAME )
				if( moveloop() ) continue;

		if( ps[myconnectindex].gm&MODE_EOL || ps[myconnectindex].gm&MODE_RESTART )
		{
			if( ps[myconnectindex].gm&MODE_EOL )
			{
				if (ONELEVELDEMO)
				{
					gameexit(" ");
				}

				closedemowrite();

				ready2send = 0;

				i = ud.screen_size;
				ud.screen_size = 0;
				vscrn();
				ud.screen_size = i;
				dobonus(0);

				if(ud.eog)
				{
					ud.eog = 0;
					if(ud.multimode < 2)
					{
						if (!VOLUMEALL)
						{
							doorders();
						}
						ps[myconnectindex].gm = MODE_MENU;
						cmenu(0);
						probey = 0;
						goto MAIN_LOOP_RESTART;
					}
					else
					{
						ud.m_level_number = 0;
						ud.level_number = 0;
					}
				}
			}

			ready2send = 0;
			if(numplayers > 1)
				ps[myconnectindex].gm = MODE_GAME;
			enterlevel(ps[myconnectindex].gm);
			continue;
		}

		nonsharedkeys();

		if( (ud.show_help == 0 && ud.multimode < 2 && !(ps[myconnectindex].gm&MODE_MENU) ) || ud.multimode > 1 || ud.recstat == 2)
			i = min(max((totalclock-ototalclock)*(65536L/TICSPERFRAME),0),65536);
		else
			i = 65536;

		displayrooms(screenpeek,i);
		displayrest(i);

		//        if( KB_KeyPressed(sc_F) )
		//        {
		//            KB_ClearKeyDown(sc_F);
		//            addplayer();
		//        }

		if(ps[myconnectindex].gm&MODE_DEMO)
			goto MAIN_LOOP_RESTART;

		if(debug_on) caches();

		checksync();

		if(VOLUMEONE && ud.show_help == 0 && show_shareware > 0 && (ps[myconnectindex].gm&MODE_MENU) == 0 )
			rotatesprite((320-50)<<16,9<<16,65536L,0,BETAVERSION,0,0,2+8+16+128,0,0,xdim-1,ydim-1);

		nextpage();
	}

	gameexit(" ");
}

char opendemoread(char which_demo) // 0 = mine
{
	char d[] = "demo_.dmo";
	char *fname = d;
	char ver;
	short i;

	if(which_demo == 10)
		d[4] = 'x';
	else
		d[4] = '0' + which_demo;

	ud.reccnt = 0;

	if(which_demo == 1 && firstdemofile[0] != 0)
	{
		fname = firstdemofile;
		if ((recfilep = kopen4load(firstdemofile,loadfromgrouponly)) == -1) return(0);
	}
	else
		if ((recfilep = kopen4load(d,loadfromgrouponly)) == -1) return(0);

	kread(recfilep,&ud.reccnt,sizeof(long));
	kread(recfilep,&ver,sizeof(char));
	if( (ver != BYTEVERSION) )// || (ud.reccnt < 512) )
	{
		Printf ("%s is a version %d demo, but we want version %d.\n",
			fname, (int)ver, (int)BYTEVERSION);
		kclose(recfilep);
		return 0;
	}
	kread(recfilep,(char *)&ud.volume_number,sizeof(char));
	kread(recfilep,(char *)&ud.level_number,sizeof(char));
	kread(recfilep,(char *)&ud.player_skill,sizeof(char));
	kread(recfilep,(char *)&ud.m_coop,sizeof(char));
	kread(recfilep,(char *)&ud.m_ffire,sizeof(char));
	kread(recfilep,(short *)&ud.multimode,sizeof(short));
	kread(recfilep,(short *)&ud.m_monsters_off,sizeof(short));
	kread(recfilep,(int32 *)&ud.m_respawn_monsters,sizeof(int32));
	kread(recfilep,(int32 *)&ud.m_respawn_items,sizeof(int32));
	kread(recfilep,(int32 *)&ud.m_respawn_inventory,sizeof(int32));
	kread(recfilep,(int32 *)&ud.playerai,sizeof(int32));
	kread(recfilep,(char *)&ud.user_name[0][0],sizeof(ud.user_name));
	kread(recfilep,(int32 *)&ud.auto_run,sizeof(int32));
	kread(recfilep,(char *)boardfilename,sizeof(boardfilename));
	if( boardfilename[0] != 0 )
	{
		ud.m_level_number = 7;
		ud.m_volume_number = 0;
	}

	for(i=0;i<ud.multimode;i++)
		kread(recfilep,(int32 *)&ps[i].aim_mode,sizeof(char));
	ud.god = ud.cashman = ud.eog = ud.showallmap = 0;
	ud.clipping = ud.scrollmode = ud.overhead_on = 0;
	ud.showweapons =  ud.pause_on = ud.auto_run = 0;

	newgame(ud.volume_number,ud.level_number,ud.player_skill);
	return(1);
}


void opendemowrite(void)
{
	char *d = "demo1.dmo";
	long dummylong = 0;
	char ver;
	short i;

	if(ud.recstat == 2) kclose(recfilep);

	ver = BYTEVERSION;

	// CTW - MODIFICATION
	//  if ((frecfilep = fopen(d,"wb")) == -1) return;
	if ((frecfilep = fopen(d,"wb")) == NULL) return;
	// CTW END - MODIFICATION
	fwrite(&dummylong,4,1,frecfilep);
	fwrite(&ver,sizeof(char),1,frecfilep);
	fwrite((char *)&ud.volume_number,sizeof(char),1,frecfilep);
	fwrite((char *)&ud.level_number,sizeof(char),1,frecfilep);
	fwrite((char *)&ud.player_skill,sizeof(char),1,frecfilep);
	fwrite((char *)&ud.m_coop,sizeof(char),1,frecfilep);
	fwrite((char *)&ud.m_ffire,sizeof(char),1,frecfilep);
	fwrite((short *)&ud.multimode,sizeof(short),1,frecfilep);
	fwrite((short *)&ud.m_monsters_off,sizeof(short),1,frecfilep);
	fwrite((int32 *)&ud.m_respawn_monsters,sizeof(int32),1,frecfilep);
	fwrite((int32 *)&ud.m_respawn_items,sizeof(int32),1,frecfilep);
	fwrite((int32 *)&ud.m_respawn_inventory,sizeof(int32),1,frecfilep);
	fwrite((int32 *)&ud.playerai,sizeof(int32),1,frecfilep);
	fwrite((char *)&ud.user_name[0][0],sizeof(ud.user_name),1,frecfilep);
	fwrite((int32 *)&ud.auto_run,sizeof(int32),1,frecfilep);
	fwrite((char *)boardfilename,sizeof(boardfilename),1,frecfilep);

	for(i=0;i<ud.multimode;i++)
		fwrite((int32 *)&ps[i].aim_mode,sizeof(char),1,frecfilep);

	totalreccnt = 0;
	ud.reccnt = 0;
}

void record(void)
{
	short i;

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		copybufbyte(&sync[i],&recsync[ud.reccnt],sizeof(input));
		ud.reccnt++;
		totalreccnt++;
		if (ud.reccnt >= RECSYNCBUFSIZ)
		{
			dfwrite(recsync,sizeof(input)*ud.multimode,ud.reccnt/ud.multimode,frecfilep);
			ud.reccnt = 0;
		}
	}
}

void closedemowrite(void)
{
	if (ud.recstat == 1)
	{
		if (ud.reccnt > 0)
		{
			dfwrite(recsync,sizeof(input)*ud.multimode,ud.reccnt/ud.multimode,frecfilep);

			fseek(frecfilep,SEEK_SET,0L);
			fwrite(&totalreccnt,sizeof(long),1,frecfilep);
			ud.recstat = ud.m_recstat = 0;
		}
		fclose(frecfilep);
	}
}

char which_demo = 1;

char in_menu = 0;

// extern long syncs[];
long playback(void)
{
	long i,j,k,l;
	char foundemo;

	if( ready2send ) return 0;

	foundemo = 0;

RECHECK:

	in_menu = ps[myconnectindex].gm&MODE_MENU;

	pub = NUMPAGES;
	pus = NUMPAGES;

	flushperms();

	if(numplayers < 2) foundemo = opendemoread(which_demo);

	if(foundemo == 0)
	{
		if(which_demo > 1)
		{
			which_demo = 1;
			goto RECHECK;
		}
		FadeOut (7);
		drawbackground();
		menus();
		ps[myconnectindex].palette = palette;
		nextpage();
		FadeIn (7);
		ud.reccnt = 0;
	}
	else
	{
		ud.recstat = 2;
		which_demo++;
		if(which_demo == 10) which_demo = 1;
		enterlevel(MODE_DEMO);
	}

	if(foundemo == 0 || in_menu || numplayers > 1)
	{
		FX_StopAllSounds();
		clearsoundlocks();
		ps[myconnectindex].gm |= MODE_MENU;
	}

	ready2send = 0;
	i = 0;
	k = 0;

	while (ud.reccnt > 0 || foundemo == 0)
	{
		I_StartTic ();
		D_ProcessEvents ();

		if(foundemo)
		{
			while ( totalclock >= (lockclock+TICSPERFRAME) )
			{
				if ((i == 0) || (i >= RECSYNCBUFSIZ))
				{
					i = 0;
					l = min(ud.reccnt,RECSYNCBUFSIZ);
					kdfread(recsync,sizeof(input)*ud.multimode,l/ud.multimode,recfilep);
				}

				for(j=connecthead;j>=0;j=connectpoint2[j])
				{
					copybufbyte(&recsync[i],&inputfifo[movefifoend[j]&(MOVEFIFOSIZ-1)][j],sizeof(input));
					movefifoend[j]++;
					i++;
					ud.reccnt--;
				}
				domovethings();
			}
		}

		if(foundemo == 0)
		{
			drawbackground();
		}
		else
		{
			nonsharedkeys();

			j = min(max((totalclock-lockclock)*(65536/TICSPERFRAME),0),65536);
			displayrooms(screenpeek,j);
			displayrest(j);

			if(ud.multimode > 1 && ps[myconnectindex].gm )
				getpackets();
		}

		if( (ps[myconnectindex].gm&MODE_MENU) && (ps[myconnectindex].gm&MODE_EOL) )
			goto RECHECK;

		if(ps[myconnectindex].gm&MODE_TYPE)
		{
			typemode();
			if((ps[myconnectindex].gm&MODE_TYPE) != MODE_TYPE)
				ps[myconnectindex].gm = MODE_MENU;
		}
		else
		{
			menus();
			if( ud.multimode > 1 )
			{
				ControlInfo noshareinfo;
				CONTROL_GetInput( &noshareinfo );
			}
		}

		operatefta();

		if(ud.last_camsprite != ud.camerasprite)
		{
			ud.last_camsprite = ud.camerasprite;
			ud.camera_time = totalclock+(TICRATE*2);
		}

		if( VOLUMEONE && ud.show_help == 0 && (ps[myconnectindex].gm&MODE_MENU) == 0 )
			rotatesprite((320-50)<<16,9<<16,65536L,0,BETAVERSION,0,0,2+8+16+128,0,0,xdim-1,ydim-1);

		getpackets();
		nextpage();

		if( ps[myconnectindex].gm==MODE_END || ps[myconnectindex].gm==MODE_GAME )
		{
			if(foundemo)
				kclose(recfilep);
			return 0;
		}
	}
	kclose(recfilep);
	if(ps[myconnectindex].gm&MODE_MENU) goto RECHECK;
	return 1;
}

char moveloop()
{
	long i;

	if (numplayers > 1)
		while (fakemovefifoplc < movefifoend[myconnectindex]) fakedomovethings();

	getpackets();

	if (numplayers < 2) bufferjitter = 0;
	while (movefifoend[myconnectindex]-movefifoplc > bufferjitter)
	{
		for(i=connecthead;i>=0;i=connectpoint2[i])
			if (movefifoplc == movefifoend[i]) break;
		if (i >= 0) break;
		if( domovethings() ) return 1;
	}
	return 0;
}

void fakedomovethingscorrect(void)
{
	long i;
	struct player_struct *p;

	if (numplayers < 2) return;

	i = ((movefifoplc-1)&(MOVEFIFOSIZ-1));
	p = &ps[myconnectindex];

	if (p->posx == myxbak[i] && p->posy == myybak[i] && p->posz == myzbak[i]
	&& p->horiz == myhorizbak[i] && p->ang == myangbak[i]) return;

	myx = p->posx; omyx = p->oposx; myxvel = p->posxv;
	myy = p->posy; omyy = p->oposy; myyvel = p->posyv;
	myz = p->posz; omyz = p->oposz; myzvel = p->poszv;
	myang = p->ang; omyang = p->oang;
	mycursectnum = p->cursectnum;
	myhoriz = p->horiz; omyhoriz = p->ohoriz;
	myhorizoff = p->horizoff; omyhorizoff = p->ohorizoff;
	myjumpingcounter = p->jumping_counter;
	myjumpingtoggle = p->jumping_toggle;
	myonground = p->on_ground;
	myhardlanding = p->hard_landing;
	myreturntocenter = p->return_to_center;

	fakemovefifoplc = movefifoplc;
	while (fakemovefifoplc < movefifoend[myconnectindex])
		fakedomovethings();

}

void fakedomovethings(void)
{
	input *syn;
	struct player_struct *p;
	long i, j, k, doubvel, fz, cz, hz, lz, x, y;
	unsigned long sb_snum;
	short psect, psectlotag, tempsect, backcstat;
	char shrunk, spritebridge;

	syn = (input *)&inputfifo[fakemovefifoplc&(MOVEFIFOSIZ-1)][myconnectindex];

	p = &ps[myconnectindex];

	backcstat = sprite[p->i].cstat;
	sprite[p->i].cstat &= ~CSTAT_ALLBLOCKING;

	sb_snum = syn->bits;

	psect = mycursectnum;
	psectlotag = sector[psect].lotag;
	spritebridge = 0;

	shrunk = (sprite[p->i].yrepeat < 32);

	if( ud.clipping == 0 && ( sector[psect].floorpicnum == MIRROR || psect < 0 || psect >= MAXSECTORS) )
	{
		myx = omyx;
		myy = omyy;
	}
	else
	{
		omyx = myx;
		omyy = myy;
	}

	omyhoriz = myhoriz;
	omyhorizoff = myhorizoff;
	omyz = myz;
	omyang = myang;

	getzrange(myx,myy,myz,psect,&cz,&hz,&fz,&lz,163L,CLIPMASK0);

	j = getflorzofslope(psect,myx,myy);

	if( (lz&49152) == 16384 && psectlotag == 1 && klabs(myz-j) > PHEIGHT+(16<<8) )
		psectlotag = 0;

	if( p->aim_mode == 0 && myonground && psectlotag != 2 && (sector[psect].floorstat&SSTAT_GROUDRAW) )
	{
		x = myx+(sintable[(myang+512)&2047]>>5);
		y = myy+(sintable[myang&2047]>>5);
		tempsect = psect;
		updatesector(x,y,&tempsect);
		if (tempsect >= 0)
		{
			k = getflorzofslope(psect,x,y);
			if (psect == tempsect)
				myhorizoff += mulscale16(j-k,160);
			else if (klabs(getflorzofslope(tempsect,x,y)-k) <= (4<<8))
				myhorizoff += mulscale16(j-k,160);
		}
	}
	if (myhorizoff > 0) myhorizoff -= ((myhorizoff>>3)+1);
	else if (myhorizoff < 0) myhorizoff += (((-myhorizoff)>>3)+1);

	if(hz >= 0 && (hz&49152) == 49152)
	{
		hz &= (MAXSPRITES-1);
		if (sprite[hz].statnum == 1 && sprite[hz].extra >= 0)
		{
			hz = 0;
			cz = getceilzofslope(psect,myx,myy);
		}
	}

	if(lz >= 0 && (lz&49152) == 49152)
	{
		j = lz&(MAXSPRITES-1);
		if ((sprite[j].cstat&(CSTAT_TYPEMASK|CSTAT_CLIPBLOCKING)) == (CSTAT_FLOOR|CSTAT_CLIPBLOCKING))
		{
			psectlotag = 0;
			spritebridge = 1;
		}
		if(badguy(&sprite[j]) && sprite[j].xrepeat > 24 && klabs(sprite[p->i].z-sprite[j].z) < (84<<8) )
		{
			j = getangle( sprite[j].x-myx,sprite[j].y-myy);
			myxvel -= sintable[(j+512)&2047]<<4;
			myyvel -= sintable[j&2047]<<4;
		}
	}

	if( sprite[p->i].extra <= 0 )
	{
		if( psectlotag == 2 )
		{
			if(p->on_warping_sector == 0)
			{
				if( klabs(myz-fz) > (PHEIGHT>>1))
					myz += 348;
			}
			clipmove(&myx,&myy,&myz,&mycursectnum,0,0,164L,(4L<<8),(4L<<8),CLIPMASK0);
		}

		updatesector(myx,myy,&mycursectnum);
		pushmove(&myx,&myy,&myz,&mycursectnum,128L,(4L<<8),(20L<<8),CLIPMASK0);

		myhoriz = 100;
		myhorizoff = 0;

		goto ENDFAKEPROCESSINPUT;
	}

	doubvel = TICSPERFRAME;

	if(p->on_crane >= 0) goto FAKEHORIZONLY;

	if(p->one_eighty_count < 0) myang += 128;

	i = 40;

	if( psectlotag == 2)
	{
		myjumpingcounter = 0;

		if ( sb_snum&1 )
		{
			if(myzvel > 0) myzvel = 0;
			myzvel -= 348;
			if(myzvel < -(256*6)) myzvel = -(256*6);
		}
		else if (sb_snum&(1<<1))
		{
			if(myzvel < 0) myzvel = 0;
			myzvel += 348;
			if(myzvel > (256*6)) myzvel = (256*6);
		}
		else
		{
			if(myzvel < 0)
			{
				myzvel += 256;
				if(myzvel > 0)
					myzvel = 0;
			}
			if(myzvel > 0)
			{
				myzvel -= 256;
				if(myzvel < 0)
					myzvel = 0;
			}
		}

		if(myzvel > 2048) myzvel >>= 1;

		myz += myzvel;

		if(myz > (fz-(15<<8)) )
			myz += ((fz-(15<<8))-myz)>>1;

		if(myz < (cz+(4<<8)) )
		{
			myz = cz+(4<<8);
			myzvel = 0;
		}
	}

	else if(p->jetpack_on)
	{
		myonground = 0;
		myjumpingcounter = 0;
		myhardlanding = 0;

		if(p->jetpack_on < 11)
			myz -= (p->jetpack_on<<7); //Goin up

		if(shrunk) j = 512;
		else j = 2048;

		if (sb_snum&1)                            //A
			myz -= j;
		if (sb_snum&(1<<1))                       //Z
			myz += j;

		if(shrunk == 0 && ( psectlotag == 0 || psectlotag == 2 ) ) k = 32;
		else k = 16;

		if(myz > (fz-(k<<8)) )
			myz += ((fz-(k<<8))-myz)>>1;
		if(myz < (cz+(18<<8)) )
			myz = cz+(18<<8);
	}
	else if( psectlotag != 2 )
	{
		if (psectlotag == 1 && p->spritebridge == 0)
		{
			if(shrunk == 0) i = 34;
			else i = 12;
		}
		if(myz < (fz-(i<<8)) && (floorspace(psect)|ceilingspace(psect)) == 0 ) //falling
		{
			if( (sb_snum&3) == 0 && myonground && (sector[psect].floorstat&SSTAT_GROUDRAW) && myz >= (fz-(i<<8)-(16<<8) ) )
				myz = fz-(i<<8);
			else
			{
				myonground = 0;

				myzvel += (gc+80);

				if(myzvel >= (4096+2048)) myzvel = (4096+2048);
			}
		}

		else
		{
			if(psectlotag != 1 && psectlotag != 2 && myonground == 0 && myzvel > (6144>>1))
				myhardlanding = myzvel>>10;
			myonground = 1;

			if(i==40)
			{
				//Smooth on the ground

				k = ((fz-(i<<8))-myz)>>1;
				if( klabs(k) < 256 ) k = 0;
				myz += k; // ((fz-(i<<8))-myz)>>1;
				myzvel -= 768; // 412;
				if(myzvel < 0) myzvel = 0;
			}
			else if(myjumpingcounter == 0)
			{
				myz += ((fz-(i<<7))-myz)>>1; //Smooth on the water
				if(p->on_warping_sector == 0 && myz > fz-(16<<8))
				{
					myz = fz-(16<<8);
					myzvel >>= 1;
				}
			}

			if( sb_snum&2 )
				myz += (2048+768);

			if( (sb_snum&1) == 0 && myjumpingtoggle == 1)
				myjumpingtoggle = 0;

			else if( (sb_snum&1) && myjumpingtoggle == 0 )
			{
				if( myjumpingcounter == 0 )
					if( (fz-cz) > (56<<8) )
					{
						myjumpingcounter = 1;
						myjumpingtoggle = 1;
					}
			}
			if( myjumpingcounter && (sb_snum&1) == 0 )
				myjumpingcounter = 0;
		}

		if(myjumpingcounter)
		{
			if( (sb_snum&1) == 0 && myjumpingtoggle == 1)
				myjumpingtoggle = 0;

			if( myjumpingcounter < (1024+256) )
			{
				if(psectlotag == 1 && myjumpingcounter > 768)
				{
					myjumpingcounter = 0;
					myzvel = -512;
				}
				else
				{
					myzvel -= (sintable[(2048-128+myjumpingcounter)&2047])/12;
					myjumpingcounter += 180;

					myonground = 0;
				}
			}
			else
			{
				myjumpingcounter = 0;
				myzvel = 0;
			}
		}

		myz += myzvel;

		if(myz < (cz+(4<<8)) )
		{
			myjumpingcounter = 0;
			if(myzvel < 0) myxvel = myyvel = 0;
			myzvel = 128;
			myz = cz+(4<<8);
		}

	}

	if ( p->fist_incs ||
		p->transporter_hold > 2 ||
		myhardlanding ||
		p->access_incs > 0 ||
		p->knee_incs > 0 ||
		(p->curr_weapon == TRIPBOMB_WEAPON &&
		p->kickback_pic > 1 &&
		p->kickback_pic < 4 ) )
	{
		doubvel = 0;
		myxvel = 0;
		myyvel = 0;
	}
	else if ( syn->avel )          //p->ang += syncangvel * constant
	{                         //ENGINE calculates angvel for you
		long tempang;

		tempang = syn->avel<<1;

		if(psectlotag == 2)
			myang += (tempang-(tempang>>3))*sgn(doubvel);
		else myang += (tempang)*sgn(doubvel);
		myang &= 2047;
	}

	if ( myxvel || myyvel || syn->fvel || syn->svel )
	{
		if(p->steroids_amount > 0 && p->steroids_amount < 400)
			doubvel <<= 1;

		myxvel += ((syn->fvel*doubvel)<<6);
		myyvel += ((syn->svel*doubvel)<<6);

		if( ( p->curr_weapon == KNEE_WEAPON && p->kickback_pic > 10 && myonground ) || ( myonground && (sb_snum&2) ) )
		{
			myxvel = mulscale16(myxvel,dukefriction-0x2000);
			myyvel = mulscale16(myyvel,dukefriction-0x2000);
		}
		else
		{
			if(psectlotag == 2)
			{
				myxvel = mulscale16(myxvel,dukefriction-0x1400);
				myyvel = mulscale16(myyvel,dukefriction-0x1400);
			}
			else
			{
				myxvel = mulscale16(myxvel,dukefriction);
				myyvel = mulscale16(myyvel,dukefriction);
			}
		}

		if( abs(myxvel) < 2048 && abs(myyvel) < 2048 )
			myxvel = myyvel = 0;

		if( shrunk )
		{
			myxvel =
				mulscale16(myxvel,(dukefriction)-(dukefriction>>1)+(dukefriction>>2));
			myyvel =
				mulscale16(myyvel,(dukefriction)-(dukefriction>>1)+(dukefriction>>2));
		}
	}

FAKEHORIZONLY:
	if(psectlotag == 1 || spritebridge == 1) i = (4L<<8); else i = (20L<<8);

	clipmove(&myx,&myy,&myz,&mycursectnum,myxvel,myyvel,164L,4L<<8,i,CLIPMASK0);
	pushmove(&myx,&myy,&myz,&mycursectnum,164L,4L<<8,4L<<8,CLIPMASK0);

	if( p->jetpack_on == 0 && psectlotag != 1 && psectlotag != 2 && shrunk)
		myz += 30<<8;

	if ((sb_snum&(1<<18)) || myhardlanding)
		myreturntocenter = 9;

	if (sb_snum&(1<<13))
	{
		myreturntocenter = 9;
		if (sb_snum&(1<<5)) myhoriz += 6;
		myhoriz += 6;
	}
	else if (sb_snum&(1<<14))
	{
		myreturntocenter = 9;
		if (sb_snum&(1<<5)) myhoriz -= 6;
		myhoriz -= 6;
	}
	else if (sb_snum&(1<<3))
	{
		if (sb_snum&(1<<5)) myhoriz += 6;
		myhoriz += 6;
	}
	else if (sb_snum&(1<<4))
	{
		if (sb_snum&(1<<5)) myhoriz -= 6;
		myhoriz -= 6;
	}

	if (myreturntocenter > 0)
	{
		if ((sb_snum&(1<<13)) == 0 && (sb_snum&(1<<14)) == 0)
		{
			myreturntocenter--;
			myhoriz += 33-(myhoriz/3);
		}
	}

	if(p->aim_mode)
		myhoriz += syn->horz>>1;
	else
	{
		if( myhoriz > 95 && myhoriz < 105) myhoriz = 100;
		if( myhorizoff > -5 && myhorizoff < 5) myhorizoff = 0;
	}

	if (myhardlanding > 0)
	{
		myhardlanding--;
		myhoriz -= (myhardlanding<<4);
	}

	if (myhoriz > 299) myhoriz = 299;
	else if (myhoriz < -99) myhoriz = -99;

	if(p->knee_incs > 0)
	{
		myhoriz -= 48;
		myreturntocenter = 9;
	}


ENDFAKEPROCESSINPUT:

	myxbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myx;
	myybak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myy;
	myzbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myz;
	myangbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myang;
	myhorizbak[fakemovefifoplc&(MOVEFIFOSIZ-1)] = myhoriz;
	fakemovefifoplc++;

	sprite[p->i].cstat = backcstat;
}


char domovethings(void)
{
	short i, j;
	char ch;

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		if( sync[i].bits&(1<<17) )
		{
			multiflag = 2;
			multiwhat = (sync[i].bits>>18)&1;
			multipos = (unsigned) (sync[i].bits>>19)&15;
			multiwho = i;

			if( multiwhat )
			{
				saveplayer( multipos );
				multiflag = 0;

				if(multiwho != myconnectindex)
				{
					sprintf(fta_quotes[122], "%s SAVED A MULTIPLAYER GAME", &ud.user_name[multiwho][0]);
					FTA(122,&ps[myconnectindex]);
				}
				else
				{
					strcpy(fta_quotes[122],"MULTIPLAYER GAME SAVED");
					FTA(122,&ps[myconnectindex]);
				}
				break;
			}
			else
			{
				//            waitforeverybody();

				j = loadplayer( multipos );

				multiflag = 0;

				if(j == 0)
				{
					if(multiwho != myconnectindex)
					{
						sprintf(fta_quotes[122], "%s LOADED A MULTIPLAYER GAME", &ud.user_name[multiwho][0]);
						FTA(122,&ps[myconnectindex]);
					}
					else
					{
						strcpy(fta_quotes[122],"MULTIPLAYER GAME LOADED");
						FTA(122,&ps[myconnectindex]);
					}
					return 1;
				}
			}
		}
	}

	ud.camerasprite = -1;
	lockclock += TICSPERFRAME;

	if(earthquaketime > 0) earthquaketime--;
	if(rtsplaying > 0) rtsplaying--;

	for(i=0;i<MAXUSERQUOTES;i++)
	{
		if (user_quote_time[i])
		{
			user_quote_time[i]--;
			if (!user_quote_time[i]) pub = NUMPAGES;
		}
	}
	if ((klabs(quotebotgoal-quotebot) <= 16) && (ud.screen_size <= 8))
		quotebot += ksgn(quotebotgoal-quotebot);
	else
		quotebot = quotebotgoal;

	if( show_shareware > 0 )
	{
		show_shareware--;
		if(show_shareware == 0)
		{
			pus = NUMPAGES;
			pub = NUMPAGES;
		}
	}

	everyothertime++;

	for(i=connecthead;i>=0;i=connectpoint2[i])
		copybufbyte(&inputfifo[movefifoplc&(MOVEFIFOSIZ-1)][i],&sync[i],sizeof(input));
	movefifoplc++;

	updateinterpolations();

	j = -1;
	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		if ((sync[i].bits&(1<<26)) == 0) { j = i; continue; }

		closedemowrite();

		if (i == myconnectindex) gameexit(" ");
		if (screenpeek == i)
		{
			screenpeek = connectpoint2[i];
			if (screenpeek < 0) screenpeek = connecthead;
		}

		if (i == connecthead) connecthead = connectpoint2[connecthead];
		else connectpoint2[j] = connectpoint2[i];

		numplayers--;
		ud.multimode--;

		if (numplayers < 2)
			sound(GENERIC_AMBIENCE17);

		pub = NUMPAGES;
		pus = NUMPAGES;
		vscrn();

		sprintf(buf,"%s is history!",ud.user_name[i]);

		quickkill(&ps[i]);
		deletesprite(ps[i].i);

		adduserquote(buf);

		if(j < 0 && networkmode == 0 )
			gameexit( " \nThe 'MASTER/First player' just quit the game.  All\nplayers are returned from the game. This only happens in 5-8\nplayer mode as a different network scheme is used.");
	}

	if ((numplayers >= 2) && ((movefifoplc&7) == 7))
	{
		ch = (char)(randomseed&255);
		for(i=connecthead;i>=0;i=connectpoint2[i])
			ch += ((ps[i].posx+ps[i].posy+ps[i].posz+ps[i].ang+ps[i].horiz)&255);
		syncval[myconnectindex][syncvalhead[myconnectindex]&(MOVEFIFOSIZ-1)] = ch;
		syncvalhead[myconnectindex]++;
	}

	if(ud.recstat == 1) record();

	if( ud.pause_on == 0 )
	{
		global_random = TRAND;
		movedummyplayers();//ST 13
	}

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		cheatkeys(i);

		if( ud.pause_on == 0 )
		{
			processinput(i);
			checksectors(i);
		}
	}

	if( ud.pause_on == 0 )
	{
		movefta();//ST 2
		moveweapons();          //ST 5 (must be last)
		movetransports();       //ST 9

		moveplayers();          //ST 10
		movefallers();          //ST 12
		moveexplosions();       //ST 4

		moveactors();           //ST 1
		moveeffectors();        //ST 3

		movestandables();       //ST 6
		doanimations();
		movefx();               //ST 11
	}

	fakedomovethingscorrect();

	if( (everyothertime&1) == 0)
	{
		animatewalls();
		movecyclers();
		pan3dsound();
	}

	C_Ticker();

	return 0;
}


void doorders(void)
{
	setview(0,0,xdim-1,ydim-1);

	D_AddResponder (Logo_Responder, 50);

	FadeOut (7);
	ps[myconnectindex].palette = palette;
	totalclock = 0;
	ClearSkipLogoTime ();
	rotatesprite(0,0,65536L,0,ORDERING,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
	FadeIn (7);
	totalclock = 0;while( !CheckSkipLogoTime() ) getpackets();

	FadeOut (7);
	totalclock = 0;
	ClearSkipLogoTime ();
	rotatesprite(0,0,65536L,0,ORDERING+1,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
	FadeIn (7);
	totalclock = 0;while( !CheckSkipLogoTime() ) getpackets();

	FadeOut (7);
	totalclock = 0;
	ClearSkipLogoTime ();
	rotatesprite(0,0,65536L,0,ORDERING+2,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
	FadeIn (7);
	totalclock = 0;while( !CheckSkipLogoTime() ) getpackets();

	FadeOut (7);
	totalclock = 0;
	ClearSkipLogoTime ();
	rotatesprite(0,0,65536L,0,ORDERING+3,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
	FadeIn (7);
	totalclock = 0;while( !CheckSkipLogoTime() ) getpackets();

	D_RemoveResponder (Logo_Responder);
}

void dobonus(char bonusonly)
{
	short t, tinc,gfx_offset;
	long i, y,xfragtotal,yfragtotal;
	short bonuscnt;

	long breathe[] =
	{
		0,  30,VICTORY1+1,176,59,
			30,  60,VICTORY1+2,176,59,
			60,  90,VICTORY1+1,176,59,
			90, 120,0         ,176,59
	};

	long bossmove[] =
	{
		0, 120,VICTORY1+3,86,59,
			220, 260,VICTORY1+4,86,59,
			260, 290,VICTORY1+5,86,59,
			290, 320,VICTORY1+6,86,59,
			320, 350,VICTORY1+7,86,59,
			350, 380,VICTORY1+8,86,59
	};

	bonuscnt = 0;
	D_AddResponder (Logo_Responder, 50);
	ClearSkipLogoTime ();

	FadeOut (7);
	setview(0,0,xdim-1,ydim-1);
	clearview(0L);
	nextpage();
	flushperms();

	FX_StopAllSounds();
	clearsoundlocks();
	FX_SetReverb(0L);

	if(bonusonly) goto FRAGBONUS;

	if(numplayers < 2 && ud.eog && ud.from_bonus == 0)
	{
		switch(ud.volume_number)
		{
		case 0:
			if(ud.lockout == 0)
			{
				clearview(0L);
				rotatesprite(0,50<<16,65536L,0,VICTORY1,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
				nextpage();
				ps[myconnectindex].palette = endingpal;
				FadeIn (1);

				ClearSkipLogoTime ();
				totalclock = 0; tinc = 0;
				while( 1 )
				{
					clearview(0L);
					rotatesprite(0,50<<16,65536L,0,VICTORY1,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);

					// boss
					if( totalclock > 390 && totalclock < 780 )
					{
						for(t=0;t<35;t+=5)
						{
							if( bossmove[t+2] && (totalclock%390) > bossmove[t] && (totalclock%390) <= bossmove[t+1] )
							{
								if(t==10 && bonuscnt == 1)
								{
									sound(SHOTGUN_FIRE);
									sound(SQUISHED);
									bonuscnt++;
								}
								rotatesprite(bossmove[t+3]<<16,bossmove[t+4]<<16,65536L,0,bossmove[t+2],0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
							}
						}

						// Breathe
						if( totalclock < 450 || totalclock >= 750 )
						{
							if(totalclock >= 750)
							{
								rotatesprite(86<<16,59<<16,65536L,0,VICTORY1+8,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
								if(totalclock >= 750 && bonuscnt == 2)
								{
									sound(DUKETALKTOBOSS);
									bonuscnt++;
								}
							}
							for(t=0;t<20;t+=5)
							{
								if( breathe[t+2] && (totalclock%120) > breathe[t] && (totalclock%120) <= breathe[t+1] )
								{
									if(t==5 && bonuscnt == 0)
									{
										sound(BOSSTALKTODUKE);
										bonuscnt++;
									}
									rotatesprite(breathe[t+3]<<16,breathe[t+4]<<16,65536L,0,breathe[t+2],0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
								}
							}
						}

					}
					getpackets();
					nextpage();
					if (CheckSkipLogoTime()) break;
				}
			}

			FadeOut (1);

			ClearSkipLogoTime ();
			ps[myconnectindex].palette = palette;

			rotatesprite(0,0,65536L,0,3292,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
			FadeIn (1);
			while( !CheckSkipLogoTime() ) getpackets();
			FadeOut (1);
			MUSIC_StopSong();
			FX_StopAllSounds();
			clearsoundlocks();
			break;
		case 1:
			MUSIC_StopSong();
			clearview(0L);
			nextpage();

			if(ud.lockout == 0)
			{
				playanm("cineov2.anm",1);
				ClearSkipLogoTime ();
				clearview(0L);
				nextpage();
			}

			sound(PIPEBOMB_EXPLODE);

			FadeOut (1);
			setview(0,0,xdim-1,ydim-1);
			ClearSkipLogoTime ();
			ps[myconnectindex].palette = palette;
			rotatesprite(0,0,65536L,0,3293,0,0,2+8+16+64, 0,0,xdim-1,ydim-1);
			FadeIn (1);
			while( !CheckSkipLogoTime() ) getpackets();
			FadeOut (1);

			break;

		case 3:

			setview(0,0,xdim-1,ydim-1);

			MUSIC_StopSong();
			clearview(0L);
			nextpage();

			if(ud.lockout == 0)
			{
				ClearSkipLogoTime ();
				playanm("vol4e1.anm",8);
				clearview(0L);
				nextpage();
				playanm("vol4e2.anm",10);
				clearview(0L);
				nextpage();
				playanm("vol4e3.anm",11);
				clearview(0L);
				nextpage();
			}

			FX_StopAllSounds();
			clearsoundlocks();
			sound(ENDSEQVOL3SND4);
			ClearSkipLogoTime ();

			ps[myconnectindex].palette = palette;
			palto(0,0,0,63);
			clearview(0L);
			menutext(160,60,0,0,"THANKS TO ALL OUR");
			menutext(160,60+16,0,0,"FANS FOR GIVING");
			menutext(160,60+16+16,0,0,"US BIG HEADS.");
			menutext(160,70+16+16+16,0,0,"LOOK FOR A DUKE NUKEM 3D");
			menutext(160,70+16+16+16+16,0,0,"SEQUEL SOON.");
			nextpage();

			FadeIn (3);
			ClearSkipLogoTime ();
			while(!CheckSkipLogoTime()) getpackets();
			FadeOut (3);

			clearview(0L);
			nextpage();

			playanm("DUKETEAM.ANM",4);

			ClearSkipLogoTime ();
			while(!CheckSkipLogoTime()) getpackets();

			clearview(0L);
			nextpage();
			palto(0,0,0,63);

			FX_StopAllSounds();
			clearsoundlocks();
			ClearSkipLogoTime ();

			break;

		case 2:

			MUSIC_StopSong();
			clearview(0L);
			nextpage();
			if(ud.lockout == 0)
			{
				FadeIn (1);
				playanm("cineov3.anm",2);
				ClearSkipLogoTime ();
				ototalclock = totalclock+200;
				while(totalclock < ototalclock)
				{
					getpackets();
					I_WaitClocks (1);
				}
				clearview(0L);
				nextpage();

				FX_StopAllSounds();
				clearsoundlocks();
			}

			playanm("RADLOGO.ANM",3);

			if( ud.lockout == 0 && !CheckSkipLogoTime() )
			{
				int v = sound(ENDSEQVOL3SND5);
				while(FX_SoundIsPlaying (v)) getpackets();
				if(CheckSkipLogoTime()) goto ENDANM;
				v = sound(ENDSEQVOL3SND6);
				while(FX_SoundIsPlaying (v)) getpackets();
				if(CheckSkipLogoTime()) goto ENDANM;
				v = sound(ENDSEQVOL3SND7);
				while(FX_SoundIsPlaying (v)) getpackets();
				if(CheckSkipLogoTime()) goto ENDANM;
				v = sound(ENDSEQVOL3SND8);
				while(FX_SoundIsPlaying (v)) getpackets();
				if(CheckSkipLogoTime()) goto ENDANM;
				v = sound(ENDSEQVOL3SND9);
				while(FX_SoundIsPlaying (v)) getpackets();
			}

			ClearSkipLogoTime ();
			totalclock = 0;
			while(!CheckSkipLogoTime() && totalclock < 120) getpackets();

ENDANM:

			FX_StopAllSounds();
			clearsoundlocks();

			ClearSkipLogoTime ();

			clearview(0L);

			break;
		}
	}

FRAGBONUS:

	ps[myconnectindex].palette = palette;
	ClearSkipLogoTime ();
	totalclock = 0; tinc = 0;
	bonuscnt = 0;

	MUSIC_StopSong();
	FX_StopAllSounds();
	clearsoundlocks();

	if(playerswhenstarted > 1 && ud.coop != 1 )
	{
		if(!(MusicToggle == 0 || MusicDevice == NumSoundCards))
			sound(BONUSMUSIC);

		rotatesprite(0,0,65536L,0,MENUSCREEN,16,0,2+8+16+64,0,0,xdim-1,ydim-1);
		rotatesprite(160<<16,34<<16,65536L,0,INGAMEDUKETHREEDEE,0,0,10,0,0,xdim-1,ydim-1);
		rotatesprite((260)<<16,36<<16,65536L,0,PLUTOPAKSPRITE+2,0,0,2+8,0,0,xdim-1,ydim-1);
		gametext(160,58+2,"MULTIPLAYER TOTALS",0,2+8+16);
		gametext(160,58+10,level_names[(ud.volume_number*11)+ud.last_level-1],0,2+8+16);

		gametext(160,165,"PRESS ANY KEY TO CONTINUE",0,2+8+16);


		t = 0;
		minitext(23,80,"   NAME                                           KILLS",8,2+8+16+128);
		for(i=0;i<playerswhenstarted;i++)
		{
			sprintf((char*)tempbuf,"%-4ld",i+1);
			minitext(92+(i*23),80,(char*)tempbuf,3,2+8+16+128);
		}

		for(i=0;i<playerswhenstarted;i++)
		{
			xfragtotal = 0;
			sprintf((char*)tempbuf,"%ld",i+1);

			minitext(30,90+t,(char*)tempbuf,0,2+8+16+128);
			minitext(38,90+t,ud.user_name[i],ps[i].palookup,2+8+16+128);

			for(y=0;y<playerswhenstarted;y++)
			{
				if(i == y)
				{
					sprintf((char*)tempbuf,"%-4ld",ps[y].fraggedself);
					minitext(92+(y*23),90+t,(char*)tempbuf,2,2+8+16+128);
					xfragtotal -= ps[y].fraggedself;
				}
				else
				{
					sprintf((char*)tempbuf,"%-4ld",frags[i][y]);
					minitext(92+(y*23),90+t,(char*)tempbuf,0,2+8+16+128);
					xfragtotal += frags[i][y];
				}

				if(myconnectindex == connecthead)
				{
					sprintf((char*)tempbuf,"stats %ld killed %ld %ld\n",i+1,y+1,frags[i][y]);
					sendscore(tempbuf);
				}
			}

			sprintf((char*)tempbuf,"%-4ld",xfragtotal);
			minitext(101+(8*23),90+t,(char*)tempbuf,2,2+8+16+128);

			t += 7;
		}

		for(y=0;y<playerswhenstarted;y++)
		{
			yfragtotal = 0;
			for(i=0;i<playerswhenstarted;i++)
			{
				if(i == y)
					yfragtotal += ps[i].fraggedself;
				yfragtotal += frags[i][y];
			}
			sprintf((char*)tempbuf,"%-4ld",yfragtotal);
			minitext(92+(y*23),96+(8*7),(char*)tempbuf,2,2+8+16+128);
		}

		minitext(45,96+(8*7),"DEATHS",8,2+8+16+128);
		nextpage();

		FadeIn (7);

		ClearSkipLogoTime ();
		while(!CheckSkipLogoTime()) getpackets();

		if(bonusonly || ud.multimode > 1)
		{
			D_RemoveResponder (Logo_Responder);
			return;
		}

		FadeOut (7);
	}

	if(bonusonly || ud.multimode > 1)
	{
		D_RemoveResponder (Logo_Responder);
		return;
	}


	switch(ud.volume_number)
	{
	case 1:
		gfx_offset = 5;
		break;
	default:
		gfx_offset = 0;
		break;
	}

	rotatesprite(0,0,65536L,0,BONUSSCREEN+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);

	menutext(160,20-6,0,0,&level_names[(ud.volume_number*11)+ud.last_level-1][0]);
	menutext(160,36-6,0,0,"COMPLETED");

	gametext(160,192,"PRESS ANY KEY TO CONTINUE",16,2+8+16);

	if(!(MusicToggle == 0 || MusicDevice == NumSoundCards))
		sound(BONUSMUSIC);

	nextpage();
	FadeIn (1);
	bonuscnt = 0;
	totalclock = 0; tinc = 0;

	ClearSkipLogoTime ();

	while( 1 )
	{
		if(ps[myconnectindex].gm&MODE_EOL)
		{
			rotatesprite(0,0,65536L,0,BONUSSCREEN+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);

			if( totalclock > (1000000000L) && totalclock < (1000000320L) )
			{
				switch( (totalclock>>4)%15 )
				{
				case 0:
					if(bonuscnt == 6)
					{
						bonuscnt++;
						sound(SHOTGUN_COCK);
						switch(rand()&3)
						{
						case 0:
							sound(BONUS_SPEECH1);
							break;
						case 1:
							sound(BONUS_SPEECH2);
							break;
						case 2:
							sound(BONUS_SPEECH3);
							break;
						case 3:
							sound(BONUS_SPEECH4);
							break;
						}
					}
				case 1:
				case 4:
				case 5:
					rotatesprite(199<<16,31<<16,65536L,0,BONUSSCREEN+3+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
					break;
				case 2:
				case 3:
					rotatesprite(199<<16,31<<16,65536L,0,BONUSSCREEN+4+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
					break;
				}
			}
			else if( totalclock > (10240+120L) ) break;
			else
			{
				switch( (totalclock>>5)&3 )
				{
				case 1:
				case 3:
					rotatesprite(199<<16,31<<16,65536L,0,BONUSSCREEN+1+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
					break;
				case 2:
					rotatesprite(199<<16,31<<16,65536L,0,BONUSSCREEN+2+gfx_offset,0,0,2+8+16+64+128,0,0,xdim-1,ydim-1);
					break;
				}
			}

			menutext(160,20-6,0,0,&level_names[(ud.volume_number*11)+ud.last_level-1][0]);
			menutext(160,36-6,0,0,"COMPLETED");

			gametext(160,192,"PRESS ANY KEY TO CONTINUE",16,2+8+16);

			if( totalclock > (60*3) )
			{
				gametext(10,59+9,"Your Time:",0,2+8+16);
				gametext(10,69+9,"Par time:",0,2+8+16);
				gametext(10,78+9,"3D Realms' Time:",0,2+8+16);
				if(bonuscnt == 0)
					bonuscnt++;

				if( totalclock > (60*4) )
				{
					if(bonuscnt == 1)
					{
						bonuscnt++;
						sound(PIPEBOMB_EXPLODE);
					}
					sprintf((char*)tempbuf,"%02ld:%02ld",
						(ps[myconnectindex].player_par/(26*60))%60,
						(ps[myconnectindex].player_par/26)%60);
					gametext((320>>2)+71,60+9,(char*)tempbuf,0,2+8+16);

					sprintf((char*)tempbuf,"%02ld:%02ld",
						(partime[ud.volume_number*11+ud.last_level-1]/(26*60))%60,
						(partime[ud.volume_number*11+ud.last_level-1]/26)%60);
					gametext((320>>2)+71,69+9,(char*)tempbuf,0,2+8+16);

					sprintf((char*)tempbuf,"%02ld:%02ld",
						(designertime[ud.volume_number*11+ud.last_level-1]/(26*60))%60,
						(designertime[ud.volume_number*11+ud.last_level-1]/26)%60);
					gametext((320>>2)+71,78+9,(char*)tempbuf,0,2+8+16);

				}
			}
			if( totalclock > (60*6) )
			{
				gametext(10,94+9,"Enemies Killed:",0,2+8+16);
				gametext(10,99+4+9,"Enemies Left:",0,2+8+16);

				if(bonuscnt == 2)
				{
					bonuscnt++;
					sound(FLY_BY);
				}

				if( totalclock > (60*7) )
				{
					if(bonuscnt == 3)
					{
						bonuscnt++;
						sound(PIPEBOMB_EXPLODE);
					}
					sprintf((char*)tempbuf,"%-3ld",ps[myconnectindex].actors_killed);
					gametext((320>>2)+70,93+9,(char*)tempbuf,0,2+8+16);
					if(ud.player_skill > 3 )
					{
						sprintf((char*)tempbuf,"N/A");
						gametext((320>>2)+70,99+4+9,(char*)tempbuf,0,2+8+16);
					}
					else
					{
						if( (ps[myconnectindex].max_actors_killed-ps[myconnectindex].actors_killed) < 0 )
							sprintf((char*)tempbuf,"%-3ld",0);
						else sprintf((char*)tempbuf,"%-3ld",ps[myconnectindex].max_actors_killed-ps[myconnectindex].actors_killed);
						gametext((320>>2)+70,99+4+9,(char*)tempbuf,0,2+8+16);
					}
				}
			}
			if( totalclock > (60*9) )
			{
				gametext(10,120+9,"Secrets Found:",0,2+8+16);
				gametext(10,130+9,"Secrets Missed:",0,2+8+16);
				if(bonuscnt == 4) bonuscnt++;

				if( totalclock > (60*10) )
				{
					if(bonuscnt == 5)
					{
						bonuscnt++;
						sound(PIPEBOMB_EXPLODE);
					}
					sprintf((char*)tempbuf,"%-3ld",ps[myconnectindex].secret_rooms);
					gametext((320>>2)+70,120+9,(char*)tempbuf,0,2+8+16);
					if( ps[myconnectindex].secret_rooms > 0 )
						sprintf((char*)tempbuf,"%-3ld%%",(100*ps[myconnectindex].secret_rooms/ps[myconnectindex].max_secret_rooms));
					// [RH] But look! You don't get to see it as a percentage!
					sprintf((char*)tempbuf,"%-3ld",ps[myconnectindex].max_secret_rooms-ps[myconnectindex].secret_rooms);
					gametext((320>>2)+70,130+9,(char*)tempbuf,0,2+8+16);
				}
			}

			if(totalclock > 10240 && totalclock < 10240+10240)
				totalclock = 1024;

			if( SkipLogoTime && totalclock > (60*2) )
			{
				if( totalclock < (60*13) )
				{
					ClearSkipLogoTime ();
					totalclock = (60*13);
				}
				else if( totalclock < (1000000000L))
				{
					totalclock = (1000000000L);
				}
			}
		}
		else break;
		nextpage();

		CheckSkipLogoTime();
		if (totalclock >= 1000000000L) ClearSkipLogoTime ();
	}
	D_RemoveResponder (Logo_Responder);
}


void cameratext(short i)
{
	char flipbits;
	long x , y;

	if(!hittype[i].temp_data[0])
	{
		rotatesprite(24<<16,33<<16,65536L,0,CAMCORNER,0,0,2,windowx1,windowy1,windowx2,windowy2);
		rotatesprite((320-26)<<16,34<<16,65536L,0,CAMCORNER+1,0,0,2,windowx1,windowy1,windowx2,windowy2);
		rotatesprite(22<<16,163<<16,65536L,512,CAMCORNER+1,0,0,2+4,windowx1,windowy1,windowx2,windowy2);
		rotatesprite((310-10)<<16,163<<16,65536L,512,CAMCORNER+1,0,0,2,windowx1,windowy1,windowx2,windowy2);
		if(totalclock&16)
			rotatesprite(46<<16,32<<16,65536L,0,CAMLIGHT,0,0,2,windowx1,windowy1,windowx2,windowy2);
	}
	else
	{
		flipbits = (totalclock<<1)&48;
		for(x=0;x<394;x+=64)
			for(y=0;y<200;y+=64)
				rotatesprite(x<<16,y<<16,65536L,0,STATIC,0,0,2+flipbits,windowx1,windowy1,windowx2,windowy2);
	}
}

void vglass(long x,long y,short a,short wn,short n)
{
	long z, zincs;
	short sect;

	sect = wall[wn].nextsector;
	if(sect == -1) return;
	zincs = ( sector[sect].floorz-sector[sect].ceilingz ) / n;

	for(z = sector[sect].ceilingz;z < sector[sect].floorz; z += zincs )
		EGS(sect,x,y,z-(TRAND&8191),GLASSPIECES+(z&(TRAND%3)),-32,36,36,a+128-(TRAND&255),16+(TRAND&31),0,-1,5);
}

void lotsofglass(short i,short wallnum,short n)
{
	long j, xv, yv, z, x1, y1;
	short sect, a;

	sect = -1;

	if(wallnum < 0)
	{
		for(j=n-1; j >= 0 ;j--)
		{
			a = sprite[i].ang-256+(TRAND&511)+1024;
			EGS(sprite[i].sectnum,sprite[i].x,sprite[i].y,sprite[i].z,GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),1024-(TRAND&1023),i,5);
		}
		return;
	}

	j = n+1;

	x1 = wall[wallnum].x;
	y1 = wall[wallnum].y;

	xv = wall[wall[wallnum].point2].x-x1;
	yv = wall[wall[wallnum].point2].y-y1;

	x1 -= ksgn(yv);
	y1 += ksgn(xv);

	xv /= j;
	yv /= j;

	for(j=n;j>0;j--)
	{
		x1 += xv;
		y1 += yv;

		updatesector(x1,y1,&sect);
		if(sect >= 0)
		{
			z = sector[sect].floorz-(TRAND&(klabs(sector[sect].ceilingz-sector[sect].floorz)));
			if( z < -(32<<8) || z > (32<<8) )
				z = sprite[i].z-(32<<8)+(TRAND&((64<<8)-1));
			a = sprite[i].ang-1024;
			EGS(sprite[i].sectnum,x1,y1,z,GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),-(TRAND&1023),i,5);
		}
	}
}

void spriteglass(short i,short n)
{
	long j, k, a, z;

	for(j=n;j>0;j--)
	{
		a = TRAND&2047;
		z = sprite[i].z-((TRAND&16)<<8);
		k = EGS(sprite[i].sectnum,sprite[i].x,sprite[i].y,z,GLASSPIECES+(j%3),TRAND&15,36,36,a,32+(TRAND&63),-512-(TRAND&2047),i,5);
		sprite[k].pal = sprite[i].pal;
	}
}

void ceilingglass(short i,short sectnum,short n)
{
	long j, xv, yv, z, x1, y1;
	short a,s, startwall,endwall;

	startwall = sector[sectnum].wallptr;
	endwall = startwall+sector[sectnum].wallnum;

	for(s=startwall;s<(endwall-1);s++)
	{
		x1 = wall[s].x;
		y1 = wall[s].y;

		xv = (wall[s+1].x-x1)/(n+1);
		yv = (wall[s+1].y-y1)/(n+1);

		for(j=n;j>0;j--)
		{
			x1 += xv;
			y1 += yv;
			a = TRAND&2047;
			z = sector[sectnum].ceilingz+((TRAND&15)<<8);
			EGS(sectnum,x1,y1,z,GLASSPIECES+(j%3),-32,36,36,a,(TRAND&31),0,i,5);
		}
	}
}



void lotsofcolourglass(short i,short wallnum,short n)
{
	long j, xv, yv, z, x1, y1;
	short sect = -1, a, k;

	if(wallnum < 0)
	{
		for(j=n-1; j >= 0 ;j--)
		{
			a = TRAND&2047;
			k = EGS(sprite[i].sectnum,sprite[i].x,sprite[i].y,sprite[i].z-(TRAND&(63<<8)),GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),1024-(TRAND&2047),i,5);
			sprite[k].pal = TRAND&15;
		}
		return;
	}

	j = n+1;
	x1 = wall[wallnum].x;
	y1 = wall[wallnum].y;

	xv = (wall[wall[wallnum].point2].x-wall[wallnum].x)/j;
	yv = (wall[wall[wallnum].point2].y-wall[wallnum].y)/j;

	for(j=n;j>0;j--)
	{
		x1 += xv;
		y1 += yv;

		updatesector(x1,y1,&sect);
		z = sector[sect].floorz-(TRAND&(klabs(sector[sect].ceilingz-sector[sect].floorz)));
		if( z < -(32<<8) || z > (32<<8) )
			z = sprite[i].z-(32<<8)+(TRAND&((64<<8)-1));
		a = sprite[i].ang-1024;
		k = EGS(sprite[i].sectnum,x1,y1,z,GLASSPIECES+(j%3),-32,36,36,a,32+(TRAND&63),-(TRAND&2047),i,5);
		sprite[k].pal = TRAND&7;
	}
}

/*
===================
=
= GetTime
=
===================
*/

long GetTime(void)
{
	return totalclock;
}


/*
===================
=
= CenterCenter
=
===================
*/

void CenterCenter(void)
{
	Printf("Center the joystick and press a button\n");
}

/*
===================
=
= UpperLeft
=
===================
*/

void UpperLeft(void)
{
	Printf("Move joystick to upper-left corner and press a button\n");
}

/*
===================
=
= LowerRight
=
===================
*/

void LowerRight(void)
{
	Printf("Move joystick to lower-right corner and press a button\n");
}

/*
===================
=
= CenterThrottle
=
===================
*/

void CenterThrottle(void)
{
	Printf("Center the throttle control and press a button\n");
}

/*
===================
=
= CenterRudder
=
===================
*/

void CenterRudder(void)
{
	Printf("Center the rudder control and press a button\n");
}

// Rare Multiplayer, when dead, total screen screwup back again!
// E3l1 (Coop /w monsters) sprite list corrupt 50%
// Univbe exit, instead, default to screen buffer.
// Check all caches bounds and memory usages
// Fix enlarger weapon selections to perfection
// Need sounds.c
// Spawning a couple of sounds at the same time
// Check Weapon Switching
// FIRE and FIRE2
// Where should I flash the screen white???
// Jittery on subs in mp?
// Check accurate memory amounts!
// Why squish sound at hit space when dead?
// Falling Counter Not reset in mp
// Wierd small freezer
// Double freeze on player?, still firing
// Do Mouse Flip option
// Save mouse aiming
// Laser bounce off mirrors
// GEORGE:   Ten in text screen.
// Alien:
// Freeze: change
// Press space holding player
// Press space
// tank broke
// 2d mode fucked in fake mp mode
// 207
// Mail not rolling up on conveyers
// Fix all alien animations
// Do episode names in .CONS
// do syntak check for "{?????"
// Make commline parms set approiate multiplayer flags

// Check all breakables to see if they are exploding properly
// Fix freezing palette on Alien

// Do a demo make run overnite
// Fix Super Duck
// Slime Guies, use quickkick.

// Make Lasers from trip bombs reflect off mirrors
// Remember for lockout of sound swears
// Pass sender in packed, NOT
// Fatal sync give no message for TEN
// Hitting TEN BUTTON(OPTION) no TEN SCreen
// Check multioperateswitches for se 31,32
// Fix pal for ceilings (SE#18)
// case 31: sprites up one high
// E1l1 No Kill All troops in room, sleep time

// Fifo for message list

// Bloodsplat on conveyers

// Meclanical
// Increase sound
// Mouse Delay at death
// Wierd slowdown

// Footprints on stuff floating

// Ken, The inside function is called a lot in -1 sectors
// No loading Univbe message rewrite
// Expander must cycle with rest of weapons
// Duck SHOOT PIPEBOMB, red wall

// Get commit source from mark

/*
1. fix pipebomb bug
2. check george maps
4. Save/Restore check (MP and SP)
5. Check TEN
6. Get Commit fixed
8. Is mail slow?
9. Cacheing
10. Blue out "PLAY ON TEN" in MULTIPLAYER
11. Eight Player test
12. Postal.voc not found.
13. All Monsters explode in arcade,
check SEENINE STRENGTH,
Change 28<<8 back to 16<<8 in hitradius
Compare 1.3d to 1.4
14. Check sounds/gfx for for parr lock
15. Player # Loaded a game
16. Replace Crane code 1.3d to 1.4
17. Fix Greenslime
18. Small Freeze sprite,below floor
19. Vesa message auto abort in mp?
20. Fucked Palette in my skip ahead in MP
21. Load in main menu
22. Rotated frag screen no game screen
23. Jibs sounds when killed other dukes
24. Ten code and /f4 mode
25. Fix All MP Glitches!!
26. Unrem Menues anim tenbn
27. buy groc,clothes,scanner
28. Why Double Defs in global and game, is so at work
29. Check that all .objs are erased
30. Check why 1.3ds gotweapon gamedef coop code no workie
31. Heavy mods to net code
32. Make sure all commline stuff works,
33. killed all waitfor???
34. 90k stack
35. double door probs
36: copy protection
* when you start a game the duke saying that is played when you choose a skill the sound is cut off.
* NEWBEASTJUMPING is not deleted at premap in multi-play
if(*c == '4') no work need objs ask ken, commit
{
movesperpacket = 4;
setpackettimeout(0x3fffffff,0x3fffffff);
}
remember, netcode load
*/
//  Ai Problem in god mode.
// Checkplayerhurtwall for forcefields bigforce
// Nuddie, posters. IMF
// Release commit.c to public?
// Document Save bug with mp
// Check moves per packet /f4 waitforeverybody over net?
// Kill IDF OBJ
// No shotguns under water @ tanker
// Unrem copyprotect
// Look for printf and puts
// Check con rewrites
// erase mmulti.c, or get newest objs
// Why nomonsters screwy in load menu in mp
// load last > 'y' == NOT
// Check xptr oos when dead rising to surface.
//    diaginal warping with shotguns
// Test white room.  Lasertripbomb arming crash
// The Bog
// Run Duke Out of windows
// Put Version number in con files
// Test diff. version playing together
// Reorganize dukecd
// Put out patch w/ two weeks testing
// Print draw3d
// Double Klick

/*
Duke Nukem V

Layout:

Settings:
Suburbs
Duke inflitrating neighborhoods inf. by aliens
Death Valley:
Sorta like a western.  Bull-skulls half buried in the sand
Military compound:  Aliens take over nuke-missle silo, duke
must destroy.
Abondend Aircraft field
Vegas:
Blast anything bright!  Alien lights camoflauged.
Alien Drug factory. The Blue Liquid
Mountainal Cave:
Interior cave battles.
Jungle:
Trees, canopee, animals, a mysterious hole in the earth with
gas seaping thru.
Penetencury:
Good use of spotlights:
Mental ward:
People whom have claimed to be slowly changing into an
alien species

Inventory:
Wood,
Metal,
Torch,
Rope,
Plastique,
Cloth,
Wiring,
Glue,
Cigars,
Food,
Duck Tape,
Nails,
Piping,
Petrol,
Uranium,
Gold,
Prism,
Power Cell,

Hand spikes (Limited usage, they become dull)
Oxygent     (Oxygen mixed with stimulant)


Player Skills:
R-Left,R-Right,Foward,Back
Strafe, Jump, Double Flip Jump for distance
Help, Escape
Fire/Use
Use Menu

After a brief resbit, Duke decides to get back to work.

Cmdr:   "Duke, we've got a lot of scared people down there.
Some reports even claim that people are already
slowly changing into aliens."
Duke:   "No problem, my speciality is in croud control."
Cmdr:   "Croud control, my ass!  Remember that incident
during the war?  You created nuthin' but death and
destruction."
Duke:   "Not destruction, justice."
Cmdr:   "I'll take no responsibility for your actions.  Your on
your own!  Behave your self, damnit!  You got that,
soldger?"
Duke:   "I've always been on my own...   Face it, it's ass kickin' time,
SIR!"
Cmdr:   "Get outta here...!"
(Duke gives the Cmdr a hard stair, then cocks his weapon and
walks out of the room)
Cmdr:   In a wisper: "Good luck, my friend."

(Cut to a scene where aliens are injecting genetic material
into an unconcious subject)

Programming:   ( the functions I need )
Images: Polys
Actors:
Multi-Object sections for change (head,arms,legs,torsoe,all change)
Facial expressions.  Pal lookup per poly?

struct imagetype
{
int *itable; // AngX,AngY,AngZ,Xoff,Yoff,Zoff;
int *idata;
struct imagetype *prev, *next;
}

*/


// Test frag screen name fuckup
// Test all xptrs
// Make Jibs stick to ceiling
// Save Game menu crash
// Cache len sum err
// Loading in main (MP), reset totalclock?
// White Room
// Sound hitch with repeat bits
// Rewrite saved menues so no crash
// Put a getpackets after loadplayer in menus
// Put "loading..." before waitfor in loadpla
// No ready2send = 0 for loading
// Test Joystick
// Ten
// Bog
// Test Blimp respawn
// move 1 in player???
