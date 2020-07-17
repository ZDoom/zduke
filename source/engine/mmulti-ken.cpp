// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <process.h>
#include <stdarg.h>
#include "pragmas.h"
#include "doomtype.h"
#include "multi.h"
#include "m_argv.h"

#define BAKSIZ 16384
#define SIMULATEERRORS 1
#define SHOWSENDPACKETS 1
#define SHOWGETPACKETS 1
#define PRINTERRORS 1

#define updatecrc16(crc,dat) crc = (((crc<<8)&65535)^crctable[((((unsigned short)crc)>>8)&65535)^dat])

void initcrc ();
void dosendpackets(long other);
void sendpacket(long other, BYTE *bufptr, long messleng);

extern void I_InitNetwork (void);
extern void I_NetCmd (void);

static long incnt[MAXPLAYERS], outcntplc[MAXPLAYERS], outcntend[MAXPLAYERS];
static BYTE errorgotnum[MAXPLAYERS];
static BYTE errorfixnum[MAXPLAYERS];
static BYTE errorresendnum[MAXPLAYERS];
#if (PRINTERRORS)
	static BYTE lasterrorgotnum[MAXPLAYERS];
#endif

long crctable[256];

static BYTE lastpacket[576], inlastpacket = 0;
static short lastpacketfrom, lastpacketleng;

extern volatile long totalclock;  //MUST EXTERN 1 ANNOYING VARIABLE FROM GAME
static long timeoutcount = 60, resendagaincount = 4, lastsendtime[MAXPLAYERS];

static short bakpacketptr[MAXPLAYERS][256], bakpacketlen[MAXPLAYERS][256];
static BYTE bakpacketbuf[BAKSIZ];
static long bakpacketplc = 0;

short myconnectindex, numplayers;
short connecthead, connectpoint2[MAXPLAYERS];
BYTE syncstate = 0;

extern int _argc;
extern char **_argv;

FILE *debugfile;

void callcommit()
{
	I_NetCmd ();
}

void initmultiplayers(BYTE damultioption, BYTE dacomrateoption, BYTE dapriority)
{
	int i;
	char *v;

	I_InitNetwork ();

	initcrc();
	for(i = 0; i < MAXPLAYERS; i++)
	{
		incnt[i] = 0L;
		outcntplc[i] = 0L;
		outcntend[i] = 0L;
		bakpacketlen[i][255] = -1;
	}

	if (gcom->numplayers == 1)
	{
		delete gcom;

		gcom = NULL;

		numplayers = 1; myconnectindex = 0;
		connecthead = 0; connectpoint2[0] = -1;
		return;
	}

	v = Args.CheckValue ("-debugfile");
	if (i > 0)
	{
		debugfile = fopen (v, "w");
	}

	numplayers = gcom->numplayers;
	myconnectindex = gcom->myconnectindex;
#if (SIMULATEERRORS != 0)
	srand(myconnectindex*24572457+345356);
#endif
	connecthead = 0;
	for(i=0;i<numplayers-1;i++) connectpoint2[i] = i+1;
	connectpoint2[numplayers-1] = -1;

	for(i=0;i<numplayers;i++) lastsendtime[i] = totalclock;
}

void initcrc()
{
	long i, j, k, a;

	for(j=0;j<256;j++)      //Calculate CRC table
	{
		k = (j<<8); a = 0;
		for(i=7;i>=0;i--)
		{
			if (((k^a)&0x8000) > 0)
				a = ((a<<1)&65535) ^ 0x1021;   //0x1021 = genpoly
			else
				a = ((a<<1)&65535);
			k = ((k<<1)&65535);
		}
		crctable[j] = (a&65535);
	}
}

void setpackettimeout(long datimeoutcount, long daresendagaincount)
{
	long i;

	timeoutcount = datimeoutcount;
	resendagaincount = daresendagaincount;
	for(i=0;i<numplayers;i++) lastsendtime[i] = totalclock;
}

long getcrc(BYTE *buffer, long bufleng)
{
	long i, j;

	j = 0;
	for(i=bufleng-1;i>=0;i--) updatecrc16(j,buffer[i]);
	return(j&65535);
}

void uninitmultiplayers()
{
	if (debugfile)
	{
		fclose (debugfile);
		debugfile = NULL;
	}
}

void sendlogon()
{
}

void sendlogoff()
{
	long i;
	BYTE tempbuf[2];

	tempbuf[0] = 255;
	tempbuf[1] = (BYTE)myconnectindex;
	for(i=connecthead;i>=0;i=connectpoint2[i])
		if (i != myconnectindex)
			sendpacket(i,tempbuf,2L);
}

int getoutputcirclesize()
{
	return(0);
}

void setsocket(short newsocket)
{
}

void sendpacket(long other, BYTE *bufptr, long messleng)
{
	long i, j;

	if (numplayers < 2) return;

	i = 0;
	if (bakpacketlen[other][(outcntend[other]-1)&255] == messleng)
	{
		j = bakpacketptr[other][(outcntend[other]-1)&255];
		for(i=messleng-1;i>=0;i--)
			if (bakpacketbuf[(i+j)&(BAKSIZ-1)] != bufptr[i]) break;
	}
	bakpacketlen[other][outcntend[other]&255] = (short)messleng;

	if (i < 0)   //Point to last packet to save space on bakpacketbuf
		bakpacketptr[other][outcntend[other]&255] = (short)j;
	else
	{
		bakpacketptr[other][outcntend[other]&255] = (short)bakpacketplc;
		for(i=0;i<messleng;i++)
			bakpacketbuf[(bakpacketplc+i)&(BAKSIZ-1)] = bufptr[i];
		bakpacketplc = ((bakpacketplc+messleng)&(BAKSIZ-1));
	}
	outcntend[other]++;

	lastsendtime[other] = totalclock;
	dosendpackets(other);
}

void dosendpackets(long other)
{
	long i, j, k, messleng;
	unsigned short dacrc;

	if (outcntplc[other] == outcntend[other]) return;

#if (PRINTERRORS)
	if (errorgotnum[other] > lasterrorgotnum[other])
	{
		lasterrorgotnum[other]++;
		if (debugfile)
			fprintf(debugfile, " MeWant %ld\n",incnt[other]&255);
	}
#endif

	if (outcntplc[other]+1 == outcntend[other])
	{     //Send 1 sub-packet
		k = 0;
		gcom->buffer[k++] = BYTE(outcntplc[other]&255);
		gcom->buffer[k++] = BYTE(errorgotnum[other]&7)+((errorresendnum[other]&7)<<3);
		gcom->buffer[k++] = BYTE(incnt[other]&255);

		j = bakpacketptr[other][outcntplc[other]&255];
		messleng = bakpacketlen[other][outcntplc[other]&255];
		for(i=0;i<messleng;i++)
			gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
		outcntplc[other]++;
	}
	else
	{     //Send 2 sub-packets
		k = 0;
		gcom->buffer[k++] = BYTE(outcntplc[other]&255);
		gcom->buffer[k++] = BYTE(errorgotnum[other]&7)+((errorresendnum[other]&7)<<3)+128;
		gcom->buffer[k++] = BYTE(incnt[other]&255);

			//First half-packet
		j = bakpacketptr[other][outcntplc[other]&255];
		messleng = bakpacketlen[other][outcntplc[other]&255];
		gcom->buffer[k++] = (BYTE)(messleng&255);
		gcom->buffer[k++] = (BYTE)(messleng>>8);
		for(i=0;i<messleng;i++)
			gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
		outcntplc[other]++;

			//Second half-packet
		j = bakpacketptr[other][outcntplc[other]&255];
		messleng = bakpacketlen[other][outcntplc[other]&255];
		for(i=0;i<messleng;i++)
			gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];
		outcntplc[other]++;

	}

	dacrc = (unsigned short)getcrc(gcom->buffer,k);
	gcom->buffer[k++] = (dacrc&255);
	gcom->buffer[k++] = (dacrc>>8);

	gcom->other = (short)other;
	gcom->numbytes = (short)k;

#if (SHOWSENDPACKETS)
	if (debugfile)
	{
		fprintf(debugfile, "Send(%ld): ",gcom->other);
		fprintf(debugfile, "[%3d %d-%d-%d %3d] ", gcom->buffer[0],
			gcom->buffer[1]>>7, (gcom->buffer[1]>>3)&7, gcom->buffer[1]&7,
			gcom->buffer[2]);
		for(i=3;i<gcom->numbytes;i++) fprintf(debugfile, "%2x ",gcom->buffer[i]);
		fprintf(debugfile, "\n");
	}
#endif

#if (SIMULATEERRORS != 0)
	if (!(rand()&SIMULATEERRORS)) gcom->buffer[rand()%gcom->numbytes] = (rand()&255);
	if (rand()&SIMULATEERRORS)
#endif
		{ gcom->command = 1; callcommit(); }
}

short getpacket (short *other, BYTE *bufptr)
{
	long i, messleng;
	unsigned short dacrc;

	if (numplayers < 2) return -1;

	for(i=connecthead;i>=0;i=connectpoint2[i])
		if (i != myconnectindex)
		{
			if (totalclock < lastsendtime[i]) lastsendtime[i] = totalclock;
			if (totalclock > lastsendtime[i]+timeoutcount)
			{
#if (PRINTERRORS)
				if (debugfile)
					fprintf(debugfile, " TimeOut!\n");
#endif
					errorgotnum[i] = errorfixnum[i]+1;

					if ((outcntplc[i] == outcntend[i]) && (outcntplc[i] > 0))
						{ outcntplc[i]--; lastsendtime[i] = totalclock; }
					else
						lastsendtime[i] += resendagaincount;
					dosendpackets(i);
				//}
			}
		}

	if (inlastpacket != 0)
	{
			//2ND half of good double-packet
		inlastpacket = 0;
		*other = lastpacketfrom;
		memcpy(bufptr,lastpacket,lastpacketleng);
		return(lastpacketleng);
	}

retry:
	gcom->command = 2;
	callcommit();

#if (SHOWGETPACKETS)
	if (gcom->other != -1 && debugfile)
	{
		fprintf(debugfile, " Get(%ld): ",gcom->other);
		fprintf(debugfile, "[%3d %d-%d-%d %3d] ", gcom->buffer[0],
			gcom->buffer[1]>>7, (gcom->buffer[1]>>3)&7, gcom->buffer[1]&7,
			gcom->buffer[2]);
		for(i=3;i<gcom->numbytes;i++) fprintf(debugfile, "%2x ",gcom->buffer[i]);
		fprintf(debugfile, "\n");
	}
#endif

	if (gcom->other < 0) return -1;

	messleng = gcom->numbytes;

	// [RH] Messages shorter than this can't possibly be packets we're
	// interested in.
	if (messleng < 5)
	{
#if PRINTERRORS
		if (debugfile)
			fprintf(debugfile, "Superfluous %d byte packet received from %d\n",
				messleng, gcom->other);
#endif
		goto retry;
	}

	*other = gcom->other;
	dacrc = ((unsigned short)gcom->buffer[messleng-2]);
	dacrc += (((unsigned short)gcom->buffer[messleng-1])<<8);
	if (dacrc != (unsigned short)getcrc(gcom->buffer,messleng-2))        //CRC check
	{
#if (PRINTERRORS)
		if (debugfile)
			fprintf(debugfile, "%ld CRC\n",gcom->buffer[0]);
#endif
		errorgotnum[*other] = errorfixnum[*other]+1;
		goto retry;
	}

	while ((errorfixnum[*other]&7) != ((gcom->buffer[1]>>3)&7))
		errorfixnum[*other]++;

	if ((gcom->buffer[1]&7) != (errorresendnum[*other]&7))
	{
		errorresendnum[*other]++;
		outcntplc[*other] = (outcntend[*other]&0xffffff00)+gcom->buffer[2];
		if (outcntplc[*other] > outcntend[*other]) outcntplc[*other] -= 256;
	}

	if (gcom->buffer[0] != (incnt[*other]&255))   //CNT check
	{
		if (((incnt[*other]-gcom->buffer[0])&255) > 32)
		{
			errorgotnum[*other] = errorfixnum[*other]+1;
#if (PRINTERRORS)
			if (debugfile)
				fprintf(debugfile, "%ld CNT\n",gcom->buffer[0]);
#endif
		}
#if (PRINTERRORS)
		else
		{
			if (!(gcom->buffer[1]&128))           //single else double packet
			{
				if (debugfile)
					fprintf(debugfile, "%ld cnt\n",gcom->buffer[0]);
			}
			else
			{
				if (((gcom->buffer[0]+1)&255) == (incnt[*other]&255))
				{
								 //GOOD! Take second half of double packet
					if (debugfile)
						fprintf(debugfile, "%ld-%ld .¬ \n",gcom->buffer[0],(gcom->buffer[0]+1)&255);

					messleng = ((long)gcom->buffer[3]) + (((long)gcom->buffer[4])<<8);
					lastpacketleng = (short)(gcom->numbytes-7-messleng);
					memcpy(bufptr,&gcom->buffer[messleng+5],lastpacketleng);
					incnt[*other]++;
					return(lastpacketleng);
				}
				else
				{
					if (debugfile)
						fprintf(debugfile, "%ld-%ld cnt \n",gcom->buffer[0],(gcom->buffer[0]+1)&255);
				}
			}
		}
#endif
		goto retry;
	}

		//PACKET WAS GOOD!
	if ((gcom->buffer[1]&128) == 0)           //Single packet
	{
#if (PRINTERRORS)
		if (debugfile)
			fprintf(debugfile, "%ld ¬  \n",gcom->buffer[0]);
#endif

		messleng = gcom->numbytes-5;

		memcpy(bufptr,&gcom->buffer[3],messleng);

		incnt[*other]++;
		return short(messleng);
	}

														 //Double packet
#if (PRINTERRORS)
	if (debugfile)
		fprintf(debugfile, "%ld-%ld ¬ \n",gcom->buffer[0],(gcom->buffer[0]+1)&255);
#endif

	messleng = ((long)gcom->buffer[3]) + (((long)gcom->buffer[4])<<8);
	lastpacketleng = (short)(gcom->numbytes-7-messleng);
	inlastpacket = 1; lastpacketfrom = *other;

	memcpy(bufptr,&gcom->buffer[5],messleng);
	memcpy(lastpacket,&gcom->buffer[messleng+5],lastpacketleng);

	incnt[*other] += 2;
	return short(messleng);
}

void flushpackets()
{
	/*long i;

	if (numplayers < 2) return;

	do
	{
		gcom->command = 2;
		callcommit();
	} while (gcom->other >= 0);

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
		incnt[i] = 0L;
		outcntplc[i] = 0L;
		outcntend[i] = 0L;
		errorgotnum[i] = 0;
		errorfixnum[i] = 0;
		errorresendnum[i] = 0;
		lastsendtime[i] = totalclock;
	}*/
}

void genericmultifunction(int other, BYTE *bufptr, int messleng, int command)
{
	if (numplayers < 2) return;

	gcom->command = command;
	gcom->numbytes = messleng < MAXPACKETSIZE ? messleng : MAXPACKETSIZE;
	copybuf(bufptr,gcom->buffer,(gcom->numbytes+3)>>2);
	gcom->other = other+1;
	callcommit();
}
