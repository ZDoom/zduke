// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// [RH] Modified the packet headers:
//
// It used to be: AA BB CC ... YY ZZ
//    Where AA = Sending subpacket num
//          BB = 1-bit subpacket count + Some error correcting stuff I don't understand
//          CC = Last received subpacket num
//       YY ZZ = CRC-16
//
// It's now this: AA BB [CC] [[EE [FF]] ...]
//    Where AA = Sending subpacket num (low 8 bits)
//          BB = Bits 0-6: Number of subpackets
//               Bit    7: Retransmit request is present if set
//          CC = Retransmit subpacket num (low 8 bits; only present if bit 7 of BB is set)
//
//    Every subpacket except for the last one is prefaced by a 1- or 2-byte length count:
//          EE = Bits 0-6: Low 7 bits of subpacket length
//               Bit    7: FF is present if set
//          FF = Bits 7-15 of subpacket length (only present if bit 7 of EE is set)
//
//    The last subpacket does not have a length prefix because its length can be
//    deduced from the size of the packet.
//
//    There is no CRC because UDP already has attaches its own checksum to the packet.

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
#include "i_system.h"

#define BAKSIZ 16384
#define RESENDCOUNT 10

#define SIMULATEERRORS 1
#define SHOWSENDPACKETS 1
#define SHOWGETPACKETS 1
#define PRINTERRORS 1

void dosendpackets(long other);
void sendpacket(long other, BYTE *bufptr, long messleng);

extern void I_InitNetwork (void);
extern void I_NetCmd (void);

static int incnt[MAXPLAYERS], outcntplc[MAXPLAYERS], outcntend[MAXPLAYERS];
static int misscnt[MAXPLAYERS];
static int lastcntsent[MAXPLAYERS];	// Last cnt the other node is known to have received

static bool RemoteResend[MAXPLAYERS];
static int ResendCount[MAXPLAYERS];

static BYTE errorgotnum[MAXPLAYERS];
static BYTE errorfixnum[MAXPLAYERS];
static BYTE errorresendnum[MAXPLAYERS];

static BYTE lastpacket[MAXPACKETSIZE], inlastpacket = 0;
static int lastpacketpos = 0;
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
	if (v != NULL)
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

void setpackettimeout(long datimeoutcount, long daresendagaincount)
{
	long i;

	timeoutcount = datimeoutcount;
	resendagaincount = daresendagaincount;
	for(i=0;i<numplayers;i++) lastsendtime[i] = totalclock;
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
	int i, j, k, messleng, count;
	BYTE t;

	count = outcntend[other] - outcntplc[other];

	if (count <= 0) return;

	if (count > 128)
		count = 128;

	k = 0;
	//gcom->buffer[k++] = outcntplc[other] & 255;
	int foo = outcntplc[other];
	gcom->buffer[k++] = outcntplc[other] & 255;
	gcom->buffer[k++] = (outcntplc[other] >> 8) & 255;
	gcom->buffer[k++] = (outcntplc[other] >> 16) & 255;
	gcom->buffer[k++] = (outcntplc[other] >> 24) & 255;
	t = count - 1;
	if (RemoteResend[other])
	{
	#if (PRINTERRORS)
		if (debugfile)
			fprintf(debugfile, " MeWant %ld\n",incnt[other]&255);
	#endif
		t |= 128;
	}
	gcom->buffer[k++] = t;
	//if (t & 128)
	{
	//	gcom->buffer[k++] = incnt[other] & 255;
	gcom->buffer[k++] = incnt[other] & 255;
	gcom->buffer[k++] = (incnt[other] >> 8) & 255;
	gcom->buffer[k++] = (incnt[other] >> 16) & 255;
	gcom->buffer[k++] = (incnt[other] >> 24) & 255;
	}

	while (count > 0)
	{
		j = bakpacketptr[other][outcntplc[other]&255];
		messleng = bakpacketlen[other][outcntplc[other]&255];

		// If not the last subpacket, prepend a length to it
		if (count > 1)
		{
			t = messleng & 127;
			if (messleng > 127)
			{
				t |= 128;
			}
			gcom->buffer[k++] = t;
			if (t & 128)
			{
				gcom->buffer[k++] = messleng >> 7;
			}
		}

		for(i = 0; i < messleng; i++)
			gcom->buffer[k++] = bakpacketbuf[(i+j)&(BAKSIZ-1)];

		outcntplc[other]++;
		count--;
	}

	gcom->other = (short)other;
	gcom->numbytes = (short)k;

#if (SHOWSENDPACKETS)
	if (debugfile)
	{
		fprintf(debugfile, "%8ld:Send(%ld): ", totalclock, gcom->other);
		fprintf(debugfile, "[%5d+%2d %5d] ", foo,
			(gcom->buffer[1]&127) + 1,
			incnt[other]);
		for(i=0;i<gcom->numbytes;i++) fprintf(debugfile, "%2x ",gcom->buffer[i]);
		fprintf(debugfile, "\n");
	}
#endif

#if (SIMULATEERRORS != 0)
	//if (!(rand()&SIMULATEERRORS)) gcom->buffer[rand()%gcom->numbytes] = (rand()&255);
	int n = rand()&SIMULATEERRORS;
	if (n && debugfile)
	{
		fprintf(debugfile, "simulating packet drop\n");
	}
	if (n)
#endif
		{ gcom->command = 1; callcommit(); }
}

int ExpandTics (int maketic, int low)
{
	int delta;

	delta = low - (maketic&0xff);

	if (delta >= -64 && delta <= 64)
		return (maketic&~0xff) + low;
	if (delta > 64)
		return (maketic&~0xff) - 256 + low;
	if (delta < -64)
		return (maketic&~0xff) + 256 + low;

	I_Error ("ExpandTics: strange value %i at maketic %i", low, maketic);
	return 0;
}

short getpacket (short *other, BYTE *bufptr)
{
	int i, k, messleng;
	int recvcnt, recvpackets;

	if (numplayers < 2) return -1;

	for(i=connecthead;i>=0;i=connectpoint2[i])
	{
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
			}
		}
	}

	if (inlastpacket > 0)
	{
		// Get next subpacket from previously received packet
		int len;

		if (--inlastpacket > 0)
		{
			len = lastpacket[lastpacketpos++];
			if (len & 128)
			{
				len = (len & 127) | (lastpacket[lastpacketpos++] << 7);
			}
		}
		else
		{
			len = lastpacketleng - lastpacketpos;
		}

		*other = lastpacketfrom;
		memcpy (bufptr, lastpacket + lastpacketpos, len);
		lastpacketpos += len;
		return len;
	}

retry:
	gcom->command = 2;
	callcommit();

	if (gcom->other < 0) return -1;

	messleng = gcom->numbytes;

	// [RH] Make sure packet is long enough
	if (messleng < 2 || messleng < 2 + (gcom->buffer[1] & 127) + (gcom->buffer[1] >> 7))
	{
#if PRINTERRORS
		if (debugfile)
			fprintf(debugfile, "Superfluous %d byte packet received from %d\n",
				messleng, gcom->other);
#endif
		goto retry;
	}

	*other = gcom->other;

	/*
	recvpackets = (gcom->buffer[1] & 127) + 1;
	if (recvpackets > 1)
	{
		recvcnt = ExpandTics (incnt[*other], gcom->buffer[0]);
	}
	else
	{
		recvcnt = ExpandTics (misscnt[*other], gcom->buffer[0]);
	}
	misscnt[*other] = recvcnt;
	lastcntsent[*other] = ExpandTics (lastcntsent[*other], gcom->buffer[2]);
	k = 3;
	*/
	recvpackets = (gcom->buffer[4] & 127) + 1;
	recvcnt = gcom->buffer[0] | (gcom->buffer[1]<<8) | (gcom->buffer[2]<<16) | (gcom->buffer[3]<<24);
	lastcntsent[*other] = gcom->buffer[5] | (gcom->buffer[6]<<8) | (gcom->buffer[7]<<16) | (gcom->buffer[8]<<24);
	k = 9;

#if (SHOWGETPACKETS)
	if (debugfile)
	{
		fprintf(debugfile, "%8ld: Get(%ld): ", totalclock, gcom->other);
		fprintf(debugfile, "[%5d %2d %5d] ", recvcnt, recvpackets, lastcntsent[*other]);
		for(i=0;i<gcom->numbytes;i++) fprintf(debugfile, "%2x ",gcom->buffer[i]);
		fprintf(debugfile, "\n");
	}
#endif

	// check for retransmit request
	if (ResendCount[*other] <= 0 && (gcom->buffer[4] & 128))
	{
		outcntplc[*other] = lastcntsent[*other];
		if (debugfile)
			fprintf (debugfile,"retransmit from %i\n", outcntplc[*other]);
		ResendCount[*other] = RESENDCOUNT;
	}
	else
	{
		ResendCount[*other]--;
	}

	// Check for out-of-order/duplicated packet
	if (recvcnt + recvpackets <= incnt[*other])
	{
		if (debugfile)
		{
			if (recvcnt + recvpackets < incnt[*other])
			{
				fprintf (debugfile, "Out of order packet from %d: %d + %d\n",
					*other, recvcnt, recvpackets);
			}
			else
			{
				fprintf (debugfile, "Duplicate packet from %d: %d + %d\n",
					*other, recvcnt, recvpackets);
			}
		}
		goto retry;
	}

	// Check for missing packet
	if (recvcnt > incnt[*other])
	{
		if (debugfile)
		{
			fprintf (debugfile, "Missed packet(s) from %d: [%d,%d]\n",
				*other, incnt[*other], recvcnt - 1);
		}
		RemoteResend[*other] = true;
		goto retry;
	}

	// It's good!
	RemoteResend[*other] = false;

	if (recvpackets > 1)
	{
		messleng = gcom->buffer[k++];
		if (messleng & 128)
		{
			messleng = (messleng & 127) | (gcom->buffer[k++] << 7);
		}
	}
	else
	{
		messleng = gcom->numbytes - k;
	}

	memcpy (bufptr, &gcom->buffer[k], messleng);

	if (recvpackets > 1)
	{
		k += messleng;
		lastpacketleng = gcom->numbytes - k;
		inlastpacket = recvpackets - 1;
		lastpacketfrom = *other;
		lastpacketpos = 0;
		memcpy (lastpacket, &gcom->buffer[k], lastpacketleng);
	}

	incnt[*other] += recvpackets;
	return messleng;
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
