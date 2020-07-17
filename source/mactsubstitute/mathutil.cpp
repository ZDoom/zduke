#include "stdlib.h"

int FindDistance2D (int ix, int iy)
{
	int t;

	ix = abs (ix);
	iy = abs (iy);
	if (ix == 0)
		return iy;
	if (iy == 0)
		return ix;
	if (ix<iy)
	{
		t = ix;
		ix = iy;
		iy = t;
	}
	t = iy + (iy >> 1);
	return (ix - (ix>>5) - (ix>>7) + (t>>2) + (t>>6));
}

int FindDistance3D (int ix, int iy, int iz)
{
	int t;

	ix = abs (ix);
	iy = abs (iy);
	iz = abs (iz);

	if (ix < iy)
	{
		t = ix;
		ix = iy;
		iy = t;
	}
	if (ix < iz)
	{
		t = ix;
		ix = iz;
		iz = t;
	}
	t = iy + iz;
	return (ix - (ix>>4) + (t>>2) + (t>>3));
}
