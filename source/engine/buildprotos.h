// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file was not in Ken Silverman's original release

#ifndef __PROTOS_H__
#define __PROTOS_H__

#include <stdio.h>

#include "doomtype.h"

#ifdef _MSC_VER
#define FP_OFF(x)	(x)		// What does FP_OFF do?
#else
#endif

#pragma pack(push,1)

//ceilingstat/floorstat:
//   bit 0: 1 = parallaxing, 0 = not                                 "P"
//   bit 1: 1 = groudraw, 0 = not
//   bit 2: 1 = swap x&y, 0 = not                                    "F"
//   bit 3: 1 = double smooshiness                                   "E"
//   bit 4: 1 = x-flip                                               "F"
//   bit 5: 1 = y-flip                                               "F"
//   bit 6: 1 = Align texture to first wall of sector                "R"
//   bits 7-8:                                                       "T"
//          00 = normal floors
//          01 = masked floors
//          10 = transluscent masked floors
//          11 = reverse transluscent masked floors
//   bits 9-15: reserved

// [RH] I like names for these bits so I know what they are without
// looking at the header file.
#define SSTAT_PARALLAX			0x0001
#define SSTAT_GROUDRAW			0x0002
#define SSTAT_SWAPXY			0x0004
#define SSTAT_DOUBLESMOOSHINESS	0x0008
#define SSTAT_XFLIP				0x0010
#define SSTAT_YFLIP				0x0020
#define SSTAT_ALIGNTOWALL		0x0040

#define SSTAT_NORMAL			0x0000
#define SSTAT_MASKED			0x0080
#define SSTAT_TRANSMASKED		0x0100
#define SSTAT_REVTRANSMASKED	0x0180
#define SSTAT_MASKMASK			0x0180

	//40 bytes
typedef struct
{
	short wallptr, wallnum;
	long ceilingz, floorz;
	unsigned short ceilingstat, floorstat;
	short ceilingpicnum, ceilingheinum;
	signed char ceilingshade;
	unsigned char ceilingpal, ceilingxpanning, ceilingypanning;
	short floorpicnum, floorheinum;
	signed char floorshade;
	unsigned char floorpal, floorxpanning, floorypanning;
	unsigned char visibility, filler;
	short lotag, hitag, extra;
} sectortype;

//cstat:
//   bit 0: 1 = Blocking wall (use with clipmove, getzrange)         "B"
//   bit 1: 1 = bottoms of invisible walls swapped, 0 = not          "2"
//   bit 2: 1 = align picture on bottom (for doors), 0 = top         "O"
//   bit 3: 1 = x-flipped, 0 = normal                                "F"
//   bit 4: 1 = masking wall, 0 = not                                "M"
//   bit 5: 1 = 1-way wall, 0 = not                                  "1"
//   bit 6: 1 = Blocking wall (use with hitscan / cliptype 1)        "H"
//   bit 7: 1 = Transluscence, 0 = not                               "T"
//   bit 8: 1 = y-flipped, 0 = normal                                "F"
//   bit 9: 1 = Transluscence reversing, 0 = normal                  "T"
//   bits 10-15: reserved

#define WSTAT_CLIPBLOCKING		0x0001
#define WSTAT_SWAPBOTTOMS		0x0002
#define WSTAT_ALIGNBOTTOM		0x0004
#define WSTAT_XFLIP				0x0008
#define WSTAT_MASKED			0x0010
#define WSTAT_ONEWAY			0x0020
#define WSTAT_HITBLOCKING		0x0040
#define WSTAT_TRANS				0x0080
#define WSTAT_YFLIP				0x0100
#define WSTAT_REVTRANS			0x0200

#define WSTAT_ALLBLOCKING		(WSTAT_CLIPBLOCKING|WSTAT_HITBLOCKING)

	//32 bytes
typedef struct
{
	long x, y;
	short point2, nextwall, nextsector;
	unsigned short cstat;
	short picnum, overpicnum;
	signed char shade;
	unsigned char pal, xrepeat, yrepeat, xpanning, ypanning;
	short lotag, hitag, extra;
} walltype;

//cstat:
//   bit 0: 1 = Blocking sprite (use with clipmove, getzrange)       "B"
//   bit 1: 1 = transluscence, 0 = normal                            "T"
//   bit 2: 1 = x-flipped, 0 = normal                                "F"
//   bit 3: 1 = y-flipped, 0 = normal                                "F"
//   bits 5-4: 00 = FACE sprite (default)                            "R"
//             01 = WALL sprite (like masked walls)
//             10 = FLOOR sprite (parallel to ceilings&floors)
//   bit 6: 1 = 1-sided sprite, 0 = normal                           "1"
//   bit 7: 1 = Real centered centering, 0 = foot center             "C"
//   bit 8: 1 = Blocking sprite (use with hitscan / cliptype 1)      "H"
//   bit 9: 1 = Transluscence reversing, 0 = normal                  "T"
//   bits 10-14: reserved
//   bit 15: 1 = Invisible sprite, 0 = not invisible

#define CSTAT_CLIPBLOCKING		0x0001
#define CSTAT_TRANS				0x0002
#define CSTAT_XFLIP				0x0004
#define CSTAT_YFLIP				0x0008
#define CSTAT_ONESIDED			0x0040
#define CSTAT_CENTERED			0x0080
#define CSTAT_HITBLOCKING		0x0100
#define CSTAT_REVTRANS			0x0200
#define CSTAT_INVISIBLE			0x8000

#define CSTAT_FACE				0x0000
#define CSTAT_WALL				0x0010		// 16
#define CSTAT_FLOOR				0x0020		// 32
#define CSTAT_VOXEL				0x0030		// 48
#define CSTAT_TYPEMASK			0x0030

#define CSTAT_ALLBLOCKING		(CSTAT_CLIPBLOCKING|CSTAT_HITBLOCKING)

#define CSTATB_CLIPBLOCKING		0
#define CSTATB_TRANS			1
#define CSTATB_XFLIP			2
#define CSTATB_YFLIP			3
#define CSTATB_ONESIDED			4
#define CSTATB_CENTERED			5
#define CSTATB_HITBLOCKING		8
#define CSTATB_REVTRANS			9
#define CSTATB_INVISIBLE		15

	//44 bytes
typedef struct
{
	long x, y, z;
	unsigned short cstat;
	short picnum;
	signed char shade;
	unsigned char pal, clipdist, filler;
	unsigned char xrepeat, yrepeat;
	signed char xoffset, yoffset;
	short sectnum, statnum;
	short ang, owner, xvel, yvel, zvel;
	short lotag, hitag, extra;
} spritetype;

#pragma pack(pop)

void *kmalloc (size_t size);
void kfree (void *buffer);

void drawrooms (long daposx, long daposy, long daposz,
			 short daang, long dahoriz, short dacursectnum);
void scansector (short sectnum);
long wallfront (long l1, long l2);
long spritewallfront (spritetype *s, long w);
long bunchfront (long b1, long b2);
void drawalls (long bunch);
void prepwall (long z, walltype *wal);
void ceilscan (long x1, long x2, long sectnum);
void florscan (long x1, long x2, long sectnum);
void wallscan (long x1, long x2, short *uwal, short *dwal, long *swal, long *lwal);
void maskwallscan (long x1, long x2, short *uwal, short *dwal, long *swal, long *lwal);
void transmaskvline (long x);
void transmaskvline2 (long x);
void transmaskwallscan (long x1, long x2);
long loadboard (char *filename, long *daposx, long *daposy, long *daposz,
				short *daang, short *dacursectnum);
long saveboard (char *filename, long *daposx, long *daposy, long *daposz,
				short *daang, short *dacursectnum);
void loadtables (void);
void loadpalette (void);
long setgamemode (char *daframeplace, long daxdim, long daydim, long dabytesperline);
void hline (long xr, long yp);
void slowhline (long xr, long yp);
void initengine (void);
void uninitengine (void);
void nextpage (void);
void loadtile (short tilenume);
unsigned char *allocatepermanenttile (short tilenume, long xsiz, long ysiz);
long loadpics (char *filename);
void qloadkvx (long voxindex, char *filename);
long clipinsidebox (long x, long y, short wallnum, long walldist);
long clipinsideboxline (long x, long y, long x1, long y1, long x2, long y2, long walldist);
long readpixel16 (long p);
long screencapture (char *filename, char inverseit);
long inside (long x, long y, short sectnum);
long getangle (long xvect, long yvect);
long ksqrt (long num);
long krecip (long num);
void initksqrt (void);
void copytilepiece (long tilenume1, long sx1, long sy1, long xsiz, long ysiz,
					long tilenume2, long sx2, long sy2);
void drawmasks (void);
void drawmaskwall (short damaskwallcnt);
void drawsprite (long snum);
void drawvox (long dasprx, long daspry, long dasprz, long dasprang, long daxscale, long dayscale, char daindex, signed char dashade,
			  char dapal, long *daumost, long *dadmost);
void ceilspritescan (long x1, long x2);
void ceilspritehline (long x2, long y);
long setsprite (short spritenum, long newx, long newy, long newz);
long animateoffs (short tilenum, short fakevar);
void initspritelists (void);
short insertsprite (short sectnum, short statnum);
short insertspritesect (short sectnum);
short insertspritestat (short statnum);
long deletesprite (short spritenum);
long deletespritesect (short deleteme);
long deletespritestat (short deleteme);
long changespritesect (short spritenum, short newsectnum);
long changespritestat (short spritenum, short newstatnum);
long nextsectorneighborz (short sectnum, long thez, short topbottom, short direction);
long cansee (long x1, long y1, long z1, short sect1, long x2, long y2, long z2, short sect2);
long hitscan (long xs, long ys, long zs, short sectnum, long vx, long vy, long vz,
	short *hitsect, short *hitwall, short *hitsprite,
	long *hitx, long *hity, long *hitz, unsigned long cliptype);
long neartag (long xs, long ys, long zs, short sectnum, short ange, short *neartagsector, short *neartagwall, short *neartagsprite, long *neartaghitdist, long neartagrange, char tagsearch);
long lintersect(long x1, long y1, long z1, long x2, long y2, long z2, long x3,
			  long y3, long x4, long y4, long *intx, long *inty, long *intz);
long rintersect(long x1, long y1, long z1, long vx, long vy, long vz, long x3,
			  long y3, long x4, long y4, long *intx, long *inty, long *intz);
void dragpoint(short pointhighlight, long dax, long day);
short lastwall(short point);
long clipmove (long *x, long *y, long *z, short *sectnum,
			 long xvect, long yvect,
			 long walldist, long ceildist, long flordist, unsigned long cliptype);
void keepaway (long *x, long *y, long w);
long raytrace (long x3, long y3, long *x4, long *y4);
long pushmove (long *x, long *y, long *z, short *sectnum,
			   long walldist, long ceildist, long flordist, unsigned long cliptype);
void updatesector (long x, long y, short *sectnum);
void rotatepoint (long xpivot, long ypivot, long x, long y, short daang, long *x2, long *y2);
char initmouse (void);
void getmousevalues (short *mousx, short *mousy, short *bstatus);
void printscreeninterrupt (void);
void drawline256 (long x1, long y1, long x2, long y2, char col);
void drawline16 (long x1, long y1, long x2, long y2, char col);
void qsetmode640350 (void);
void qsetmode640480 (void);
void clear2dscreen (void);
void draw2dgrid (long posxe, long posye, short ange, long zoome, short gride);
void draw2dscreen(long posxe, long posye, short ange, long zoome, short gride);
void printext16(long xpos, long ypos, short col, short backcol, char name[82], char fontsize);
void printext256(long xpos, long ypos, short col, short backcol, char name[82], char fontsize);
long krand (void);
void getzrange (long x, long y, long z, short sectnum,
			 long *ceilz, long *ceilhit, long *florz, long *florhit,
			 long walldist, unsigned long cliptype);
void setview (long x1, long y1, long x2, long y2);
void setaspect (long daxrange, long daaspect);
void dosetaspect (void);
void flushperms (void);
void rotatesprite (long sx, long sy, long z, short a, short picnum, signed char dashade, char dapalnum, char dastat, long cx1, long cy1, long cx2, long cy2);
void dorotatesprite (long sx, long sy, long z, short a, short picnum, signed char dashade, char dapalnum, char dastat, long cx1, long cy1, long cx2, long cy2);
long clippoly4 (long cx1, long cy1, long cx2, long cy2);
void makepalookup (long palnum, char *remapbuf, signed char r, signed char g, signed char b, char dastat);
void initfastcolorlookup (long rscale, long gscale, long bscale);
long getclosestcol (long r, long g, long b);
void setbrightness (char dabrightness, char *dapal);
void drawmapview (long dax, long day, long zoome, short ang);
long clippoly (long npoints, long clipstat);
void fillpolygon (long npoints);
void clearview (long dacol);
void clearallviews (long dacol);
void plotpixel (long x, long y, char col);
char getpixel (long x, long y);
void setviewtotile (short tilenume, long xsiz, long ysiz);
void setviewback (void);
void squarerotatetile (short tilenume);
void preparemirror (long dax, long day, long daz, short daang, long dahoriz, short dawall, short dasector, long *tposx, long *tposy, short *tang);
void completemirror (void);
long sectorofwall (short theline);
long getceilzofslope (short secnum, long dax, long day);
long getflorzofslope (short sectnum, long dax, long day);
void getzsofslope (short sectnum, long dax, long day, long *ceilz, long *florz);
void alignceilslope (short dasect, long x, long y, long z);
void alignflorslope (short dasect, long x, long y, long z);
long owallmost (short *mostbuf, long w, long z);
long wallmost (short *mostbuf, long w, long sectnum, char dastat);
void grouscan (long dax1, long dax2, long sectnum, char dastat);
long getpalookup (long davis, long dashade);
void parascan (long dax1, long dax2, long sectnum, char dastat, long bunch);
void *engconvalloc32 (unsigned long size);
long loopnumofsector (short sectnum, short wallnum);
void setfirstwall (short sectnum, short newfirstwall);

void initcache (char *dacachestart, long dacachesize);
void allocache (void **newhandle, long newbytes, unsigned char *newlockptr);
void suckcache (long *suckptr);
void agecache (void);
void reportandexit (char *errormessage);
long initgroupfile (char *filename);
void uninitgroupfile (void);
long kopen4load (const char *filename, char searchfirst);
long kread (long handle, void *buffer, long leng);
long klseek (long handle, long offset, long whence);
long ktell (long handle);
long kfilelength (long handle);
void kclose (long handle);
void kdfread (void *buffer, size_t dasizeof, size_t count, long fil);
void dfread (void *buffer, size_t dasizeof, size_t count, FILE *fil);
void dfwrite (void *buffer, size_t dasizeof, size_t count, FILE *fil);
long compress (char *lzwinbuf, long uncompleng, char *lzwoutbuf);
long uncompress (char *lzwinbuf, long compleng, char *lzwoutbuf);

long setvesa (long x, long y);
void uninitvesa ();
void getvalidvesamodes ();
void setactivepage (long dapagenum);
void setvisualpage (long dapagenum);
long VBE_setPalette (long start, long num, char *dapal);
long VBE_getPalette (long start, long num, char *dapal);

#ifdef NEED_DDVID_CALLS
long InitSurfaces (HWND hWnd, DWORD width, DWORD height);
void ReleaseAllSurfaces (HWND hWnd);
void BeginDrawing (void);
void FinishDrawing (void);
#endif

extern "C"
{
extern long mmxoverlay();
extern long sethlinesizes(long,long,char *);
extern long setpalookupaddress(char *);
extern long setuphlineasm4(long,long);
extern long hlineasm4(long,long,long,long,long,char *);
extern long setuprhlineasm4(long,long,long,char *,long,long);
extern long rhlineasm4(long,char *,long,long,long,char *);
extern long setuprmhlineasm4(long,long,long,char *,long,long);
extern long rmhlineasm4(long,char *,long,long,long,char *);
extern long setupqrhlineasm4(long,long,long,char *,long,long);
extern long qrhlineasm4(long,char *,long,long,long,char *);
extern long setvlinebpl(long);
extern long fixtransluscence(char *);
extern long prevlineasm1(long,char *,long,long,char *,char *);
extern long vlineasm1(long,char *,long,long,char *,char *);
extern long setuptvlineasm(long);
extern long tvlineasm1(long,char *,long,long,char *,char *);
extern long setuptvlineasm2(long,char *,char *);
extern long tvlineasm2(long,long,char *,char *,long,char *);
extern long mvlineasm1(long,char *,long,long,char *,char *);
extern long setupvlineasm(long);
extern long vlineasm4(long,char *);
extern long setupmvlineasm(long);
extern long mvlineasm4(long,char *);
extern long mhline(char *,long,long,long,long,char *);
extern long mhlineskipmodify(long,long,long,long,long,long);
extern long msethlineshift(long,long);
extern long thline(char *,long,long,long,long,char *);
extern long thlineskipmodify(long,long,long,long,long,char *);
extern long tsethlineshift(long,long);
extern long setupslopevlin(long,char *,long);
extern long slopevlin(char *,long,long,long,long,long);
extern long settransnormal();
extern long settransreverse();
extern long setupdrawslab(long,char *);
extern long drawslab(long,long,long,long,long,char *);
#ifdef _MSC_VER
extern void setupspritevline(char *,long,long,long,long);
extern void spritevline(long,long,long,char *,char *);
extern void msetupspritevline(char *,long,long,long,long);
extern void mspritevline(long,long,long,char *,char *);
extern void tsetupspritevline(char *,long,long,long,long);
extern void tspritevline(long,long,long,char *,char *);
#define setupspritevline(a,b,c,d,e,f) setupspritevline(a,b,c,d,e) // last parm ignored
#define spritevline(a,b,c,d,e,f) spritevline(b,c,d,e,f)	// first parm ignored
#define msetupspritevline(a,b,c,d,e,f) msetupspritevline(a,b,c,d,e) // last parm ignored
#define mspritevline(a,b,c,d,e,f) mspritevline(b,c,d,e,f) // first parm ignored
#define tsetupspritevline(a,b,c,d,e,f) tsetupspritevline(a,b,c,d,e) // last parm ignored
#define tspritevline(a,b,c,d,e,f) tspritevline(b,c,d,e,f) // first parm ignored
#else
extern void setupspritevline(long,long,long,long,long,long);
extern void spritevline(long,long,long,long,long,long);
extern void msetupspritevline(long,long,long,long,long,long);
extern void mspritevline(long,long,long,long,long,long);
extern void tsetupspritevline(long,long,long,long,long,long);
extern void tspritevline(long,long,long,long,long,long);
#pragma aux mmxoverlay modify [eax ebx ecx edx];
#pragma aux sethlinesizes parm [eax][ebx][ecx];
#pragma aux setpalookupaddress parm [eax];
#pragma aux setuphlineasm4 parm [eax][ebx];
#pragma aux hlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux setuprhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux rhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux setuprmhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux rmhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux setupqrhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux qrhlineasm4 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux setvlinebpl parm [eax];
#pragma aux fixtransluscence parm [eax];
#pragma aux prevlineasm1 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux vlineasm1 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux setuptvlineasm parm [eax];
#pragma aux tvlineasm1 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux setuptvlineasm2 parm [eax][ebx][ecx];
#pragma aux tvlineasm2 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux mvlineasm1 parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux setupvlineasm parm [eax];
#pragma aux vlineasm4 parm [ecx][edi] modify [eax ebx ecx edx esi edi];
#pragma aux setupmvlineasm parm [eax];
#pragma aux mvlineasm4 parm [ecx][edi] modify [eax ebx ecx edx esi edi];
#pragma aux setupspritevline parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux spritevline parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux msetupspritevline parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux mspritevline parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux tsetupspritevline parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux tspritevline parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux mhline parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux mhlineskipmodify parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux msethlineshift parm [eax][ebx];
#pragma aux thline parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux thlineskipmodify parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux tsethlineshift parm [eax][ebx];
#pragma aux setupslopevlin parm [eax][ebx][ecx] modify [edx];
#pragma aux slopevlin parm [eax][ebx][ecx][edx][esi][edi];
#pragma aux settransnormal parm;
#pragma aux settransreverse parm;
#pragma aux setupdrawslab parm [eax][ebx];
#pragma aux drawslab parm [eax][ebx][ecx][edx][esi][edi];
#endif
}

#endif	// __PROTOS_H__