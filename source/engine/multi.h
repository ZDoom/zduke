void sendpacket (long otherconnectindex, BYTE *bufptr, long messleng);
short getpacket (short *otherconnectindex, BYTE *bufptr);
int getoutputcirclesize ();

// mmulti.c
void flushpackets();
void setpackettimeout (long datimeoutcount, long daresendagaincount);
void initmultiplayers(BYTE damultioption, BYTE docomrateoption, BYTE dapriority);
void uninitmultiplayers ();
void genericmultifunction(int other, BYTE *bufptr, int messleng, int command);
void sendlogon();
void sendlogoff();

#define MAXPLAYERS 16

//#define MAXPACKETSIZE 2048
#define MAXPACKETSIZE 1400		// [RH] Maximum size of a UDP packet
struct gcomtype
{
	short intnum;                //communication between Game and the driver
	short command;               //1-send, 2-get
	short other;                 //dest for send, set by get (-1 = no packet)
	short numbytes;
	short myconnectindex;
	short numplayers;
	short gametype;              //gametype: 1-serial,2-modem,3-net
	short filler;
	BYTE buffer[MAXPACKETSIZE];
	long longcalladdress;
};
extern gcomtype *gcom;

extern bool CheckAbort ();