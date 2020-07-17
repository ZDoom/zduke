// "Build Engine & Tools" Copyright (c) 1993-1997 Ken Silverman
// Ken Silverman's official web site: "http://www.advsys.net/ken"
// See the included license file "BUILDLIC.TXT" for license info.
//
// This file is based on pragmas.h from Ken Silverman's original release
// but is meant for use with Visual C++ instead of Watcom C.
//
// Some of the inline assembly has been turned into C code, because VC++
// is smart enough to produce code at least as good as Ken's inlines.
// The more used functions are still inline assembly, because they do
// things that can't really be done in C. (I consider this a bad thing,
// because VC++ has considerably poorer support for inline assembly than
// Watcom, so it's better to rely on its C optimizer to produce fast code.)
//
// The following #pragmas shouldn't really be used under Windows:
//		int5
//		setvmode
//		setupmouse
//		readmousexy
//		readmousebstatus
//		setcolor16
//		koutp
//		koutpw
//		kinp
//		limitrate
//		readtimer
//		redbluebit
//		chainblit
//		inittimer1mhz
//		uninittimer1mhz
//		gettime1mhz
//		deltatime1mhz
//
// The following #pragmas work with the editor's 2D screen, and need
// to be recoded for linear frame buffers:
//		vlin16first
//		vlin16
//		drawpixel16
//		fillscreen16

#pragma warning (disable: 4035)

#include <string.h>

static long dmval;

__inline long sqr (long num)
{
	return num*num;
}

__inline long scale (long a, long b, long c)
{
	__asm mov eax,a
	__asm mov ecx,c
	__asm imul b
	__asm idiv ecx
}

__inline long mulscale (long a, long b, long c)
{
	__asm mov eax,a
	__asm mov ecx,c
	__asm imul b
	__asm shrd eax,edx,cl
}

#define MAKECONSTMULSCALE(s) \
	__inline long mulscale##s (long a, long b) \
	{ \
		__asm mov eax,a \
		__asm imul b \
		__asm shrd eax,edx,s \
	}
MAKECONSTMULSCALE(1)
MAKECONSTMULSCALE(2)
MAKECONSTMULSCALE(3)
MAKECONSTMULSCALE(4)
MAKECONSTMULSCALE(5)
MAKECONSTMULSCALE(6)
MAKECONSTMULSCALE(7)
MAKECONSTMULSCALE(8)
MAKECONSTMULSCALE(9)
MAKECONSTMULSCALE(10)
MAKECONSTMULSCALE(11)
MAKECONSTMULSCALE(12)
MAKECONSTMULSCALE(13)
MAKECONSTMULSCALE(14)
MAKECONSTMULSCALE(15)
MAKECONSTMULSCALE(16)
MAKECONSTMULSCALE(17)
MAKECONSTMULSCALE(18)
MAKECONSTMULSCALE(19)
MAKECONSTMULSCALE(20)
MAKECONSTMULSCALE(21)
MAKECONSTMULSCALE(22)
MAKECONSTMULSCALE(23)
MAKECONSTMULSCALE(24)
MAKECONSTMULSCALE(25)
MAKECONSTMULSCALE(26)
MAKECONSTMULSCALE(27)
MAKECONSTMULSCALE(28)
MAKECONSTMULSCALE(29)
MAKECONSTMULSCALE(30)
MAKECONSTMULSCALE(31)
#undef MAKECONSTMULSCALE

__inline long mulscale32 (long a, long b)
{
	__asm mov eax,a
	__asm imul b
	__asm mov eax,edx
}

__inline long dmulscale (long a, long b, long c, long d, long s)
{
	__asm mov eax,a
	__asm imul b
	__asm mov ebx,eax
	__asm mov eax,c
	__asm mov esi,edx
	__asm mov ecx,s
	__asm imul d
	__asm add eax,ebx
	__asm adc edx,esi
	__asm shrd eax,edx,cl
}

#define MAKECONSTDMULSCALE(s) \
	__inline long dmulscale##s (long a, long b, long c, long d) \
	{ \
		__asm mov eax,a \
		__asm imul b \
		__asm mov ebx,eax \
		__asm mov eax,c \
		__asm mov esi,edx \
		__asm imul d \
		__asm add eax,ebx \
		__asm adc edx,esi \
		__asm shrd eax,edx,s \
	}

MAKECONSTDMULSCALE(1)
MAKECONSTDMULSCALE(2)
MAKECONSTDMULSCALE(3)
MAKECONSTDMULSCALE(4)
MAKECONSTDMULSCALE(5)
MAKECONSTDMULSCALE(6)
MAKECONSTDMULSCALE(7)
MAKECONSTDMULSCALE(8)
MAKECONSTDMULSCALE(9)
MAKECONSTDMULSCALE(10)
MAKECONSTDMULSCALE(11)
MAKECONSTDMULSCALE(12)
MAKECONSTDMULSCALE(13)
MAKECONSTDMULSCALE(14)
MAKECONSTDMULSCALE(15)
MAKECONSTDMULSCALE(16)
MAKECONSTDMULSCALE(17)
MAKECONSTDMULSCALE(18)
MAKECONSTDMULSCALE(19)
MAKECONSTDMULSCALE(20)
MAKECONSTDMULSCALE(21)
MAKECONSTDMULSCALE(22)
MAKECONSTDMULSCALE(23)
MAKECONSTDMULSCALE(24)
MAKECONSTDMULSCALE(25)
MAKECONSTDMULSCALE(26)
MAKECONSTDMULSCALE(27)
MAKECONSTDMULSCALE(28)
MAKECONSTDMULSCALE(29)
MAKECONSTDMULSCALE(30)
MAKECONSTDMULSCALE(31)
#undef MAKCONSTDMULSCALE

__inline long dmulscale32 (long a, long b, long c, long d)
{
	__asm mov eax,a
	__asm imul b
	__asm mov ebx,eax
	__asm mov eax,c
	__asm mov esi,edx
	__asm imul d
	__asm add eax,ebx
	__asm adc edx,esi
	__asm mov eax,edx
}

#define MAKECONSTTMULSCALE(s) \
	__inline long tmulscale##s (long a, long b, long c, long d, long e, long f) \
	{ \
		__asm mov eax,a \
		__asm imul b \
		__asm mov ebx,eax \
		__asm mov eax,d \
		__asm mov ecx,edx \
		__asm imul c \
		__asm add ebx,eax \
		__asm mov eax,e \
		__asm adc ecx,edx \
		__asm imul f \
		__asm add eax,ebx \
		__asm adc edx,ecx \
		__asm shrd eax,edx,s \
	}

MAKECONSTTMULSCALE(1)
MAKECONSTTMULSCALE(2)
MAKECONSTTMULSCALE(3)
MAKECONSTTMULSCALE(4)
MAKECONSTTMULSCALE(5)
MAKECONSTTMULSCALE(6)
MAKECONSTTMULSCALE(7)
MAKECONSTTMULSCALE(8)
MAKECONSTTMULSCALE(9)
MAKECONSTTMULSCALE(10)
MAKECONSTTMULSCALE(11)
MAKECONSTTMULSCALE(12)
MAKECONSTTMULSCALE(13)
MAKECONSTTMULSCALE(14)
MAKECONSTTMULSCALE(15)
MAKECONSTTMULSCALE(16)
MAKECONSTTMULSCALE(17)
MAKECONSTTMULSCALE(18)
MAKECONSTTMULSCALE(19)
MAKECONSTTMULSCALE(20)
MAKECONSTTMULSCALE(21)
MAKECONSTTMULSCALE(22)
MAKECONSTTMULSCALE(23)
MAKECONSTTMULSCALE(24)
MAKECONSTTMULSCALE(25)
MAKECONSTTMULSCALE(26)
MAKECONSTTMULSCALE(27)
MAKECONSTTMULSCALE(28)
MAKECONSTTMULSCALE(29)
MAKECONSTTMULSCALE(30)
MAKECONSTTMULSCALE(31)
#undef MAKECONSTTMULSCALE

__inline long tmulscale32 (long a, long b, long c, long d, long e, long f)
{
	__asm mov eax,a
	__asm imul b
	__asm mov ebx,eax
	__asm mov eax,c
	__asm mov ecx,edx
	__asm imul d
	__asm add ebx,eax
	__asm mov eax,e
	__asm adc ecx,edx
	__asm imul f
	__asm add eax,ebx
	__asm adc edx,ecx
	__asm mov eax,edx
}

__inline long boundmulscale (long a, long b, long c)
{
	__asm mov eax,a
	__asm imul b
	__asm mov ebx,edx
	__asm mov ecx,c
	__asm shrd eax,edx,cl
	__asm sar edx,cl
	__asm xor edx,eax
	__asm js checkit
	__asm xor edx,eax
	__asm jz skipboundit
	__asm cmp edx,0xffffffff
	__asm je skipboundit
checkit:
	__asm mov eax,ebx
	__asm sar eax,31
	__asm xor eax,0x7fffffff
skipboundit:
	;
}

__inline long divscale (long a, long b, long c)
{
	__asm mov eax,a
	__asm mov ecx,c
	__asm shl eax,cl
	__asm mov edx,a
	__asm neg cl
	__asm sar edx,cl
	__asm idiv b
}

__inline long divscale1 (long a, long b)
{
	__asm mov eax,a
	__asm add eax,eax
	__asm sbb edx,edx
	__asm idiv b
}

#define MAKECONSTDIVSCALE(s) \
	__inline long divscale##s (long a, long b) \
	{ \
		__asm mov edx,a \
		__asm sar edx,32-s \
		__asm mov eax,a \
		__asm shl eax,s \
		__asm idiv b \
	}

MAKECONSTDIVSCALE(2)
MAKECONSTDIVSCALE(3)
MAKECONSTDIVSCALE(4)
MAKECONSTDIVSCALE(5)
MAKECONSTDIVSCALE(6)
MAKECONSTDIVSCALE(7)
MAKECONSTDIVSCALE(8)
MAKECONSTDIVSCALE(9)
MAKECONSTDIVSCALE(10)
MAKECONSTDIVSCALE(11)
MAKECONSTDIVSCALE(12)
MAKECONSTDIVSCALE(13)
MAKECONSTDIVSCALE(14)
MAKECONSTDIVSCALE(15)
MAKECONSTDIVSCALE(16)
MAKECONSTDIVSCALE(17)
MAKECONSTDIVSCALE(18)
MAKECONSTDIVSCALE(19)
MAKECONSTDIVSCALE(20)
MAKECONSTDIVSCALE(21)
MAKECONSTDIVSCALE(22)
MAKECONSTDIVSCALE(23)
MAKECONSTDIVSCALE(24)
MAKECONSTDIVSCALE(25)
MAKECONSTDIVSCALE(26)
MAKECONSTDIVSCALE(27)
MAKECONSTDIVSCALE(28)
MAKECONSTDIVSCALE(29)
MAKECONSTDIVSCALE(30)
MAKECONSTDIVSCALE(31)
#undef MAKECONSTDIVSCALE

__inline long divscale32 (long a, long b)
{
	__asm mov edx,a
	__asm xor eax,eax
	__asm idiv b
}

__inline char readpixel (const void *mem)
{
	return *((char *)mem);
}

__inline void drawpixel (void *mem, char pixel)
{
	*((char *)mem) = pixel;
}

__inline void drawpixels (void *mem, short pixels)
{
	*((short *)mem) = pixels;
}

__inline void drawpixelses (void *mem, long pixelses)
{
	*((long *)mem) = pixelses;
}

__inline void clearbuf (void *buff, long count, long clear)
{
	long *b2 = (long *)buff;
	while (count--)
		*b2++ = clear;
}

__forceinline void clearbufshort (void *buff, unsigned int count, int clear)
{
	if (!count)
		return;
	short *b2 = (short *)buff;
	do
	{
		*b2++ = (short)clear;
	} while (--count);
}

#define clearbufbyte(buff,len,byte) memset(buff,byte,len)
#define copybuf(from,to,count) memcpy(to,from,(count)*4)
#define copybufbyte(from,to,len) memcpy(to,from,len)

__inline void copybufreverse (const void *f, void *t, long len)
{
	char *from = (char *)f;
	char *to = (char *)t;
	while (len--)
		*to++ = *from--;
}

__inline void qinterpolatedown16 (long *out, long count, long val, long delta)
{
	long odd = count;
	if ((count >>= 1) != 0)
	{
		do
		{
			long temp = val + delta;
			out[0] = val >> 16;
			val = temp + delta;
			out[1] = temp >> 16;
			out += 2;
		} while (--count);
		if (!(odd & 1))
			return;
	}
	*out = val >> 16;
}

__inline void qinterpolatedown16short (short *out, long count, long val, long delta)
{
	if (count == 0)
		return;
	if ((long)out & 2)
	{ // align to dword boundary
		*out++ = (short)(val >> 16);
		val += delta;
		if (--count == 0)
			return;
	}
	while ((count -= 2) >= 0)
	{
		long temp = val + delta;
		*((long *)out) = (temp & 0xffff0000) + (val >> 16);
		val = temp + delta;
		out += 2;
	}
	if (count & 1)
	{
		*out = (short)(val >> 16);
	}
}

__inline long mul3 (long a)
{
	return a*3;
}

__inline long mul5 (long a)
{
	return a*5;
}

__inline long mul9 (long a)
{
	return a*9;
}

	//returns num/den, dmval = num%den
__inline long divmod (unsigned long num, long den)
{
	__asm mov eax,num
	__asm xor edx,edx
	__asm div den
	__asm mov dmval,edx
}

	//returns num%den, dmval = num/den
__inline long moddiv (unsigned long num, long den)
{
	__asm mov eax,num
	__asm xor edx,edx
	__asm div den
	__asm mov dmval,eax
	__asm mov eax,edx
}

#define klabs(a) abs(a)

__inline long ksgn (long a)
{
	__asm mov edx,a
	__asm add edx,edx
	__asm sbb eax,eax
	__asm cmp eax,edx
	__asm adc eax,0
}

__inline unsigned long umin (unsigned long a, unsigned long b)
{
	__asm mov eax,a
	__asm sub eax,b
	__asm sbb edx,edx
	__asm and eax,edx
	__asm add eax,b
}

__inline unsigned long umax (unsigned long a, unsigned long b)
{
	__asm mov eax,a
	__asm sub eax,b
	__asm sbb edx,edx
	__asm xor edx,0xffffffff
	__asm and eax,edx
	__asm add eax,b
}

__inline long kmin (long a, long b)
{
	return a < b ? a : b;
}

__inline long kmax (long a, long b)
{
	return a > b ? a : b;
}

__inline void swapchar (char *a, char *b)
{
	char t = *a;
	*a = *b;
	*b = t;
}

__inline void swapshort (short *a, short *b)
{
	short t = *a;
	*a = *b;
	*b = t;
}

__inline void swaplong (long *a, long *b)
{
	long t = *a;
	*a = *b;
	*b = t;
}

__inline void swapbuf4 (long *a, long *b, long c)
{
	do
	{
		long t = *a;
		*a++ = *b;
		*b++ = t;
	} while (--c);
}

__inline void swap64bit (__int64 *a, __int64 *b)
{
	__int64 t = *a;
	*a = *b;
	*b = t;
}

	//swapchar2(ptr1,ptr2,xsiz); is the same as:
	//swapchar(ptr1,ptr2); swapchar(ptr1+1,ptr2+xsiz);
__inline void swapchar2 (char *ptr1, char *ptr2, long xsiz)
{ 
	char t = *ptr2;
	*ptr2 = *ptr1;
	*ptr1 = t;
	t = ptr2[xsiz];
	ptr2[xsiz] = ptr1[1];
	ptr1[1] = t;
}

#pragma warning (default: 4035)
