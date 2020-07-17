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

#include "duke3d.h"
#include "s_sound.h"

#define IntelLong(x) (x)

//=============
// STATICS
//=============

int32 numlumps;
static lumpinfo_t *lumpinfo;              // location of each lump on disk
static bool RTS_Started = false;
int RTS_FirstSound = -1;

/*
============================================================================

LUMP BASED ROUTINES

============================================================================
*/

/*
====================
=
= RTS_AddFile
=
= All files are optional, but at least one file must be found
= Files with a .rts extension are wadlink files with multiple lumps
= Other files are single lumps with the base filename for the lump name
=
====================
*/

void RTS_AddFile (char *filename)
{
	wadinfo_t  header;
	lumpinfo_t *lump_p;
	int32     i;
	int32      handle, length;
	int32      startlump;
	filelump_t *fileinfo;

	//
	// read the entire file in
	//      FIXME: shared opens

	handle = SafeOpenRead( filename, filetype_binary );

	startlump = numlumps;

	// WAD file
	Printf("    Adding %s.\n",filename);
	SafeRead( handle, &header, sizeof( header ) );
	if (strncmp(header.identification,"IWAD",4))
		I_FatalError ("RTS file %s doesn't have IWAD id\n",filename);
	header.numlumps = IntelLong(header.numlumps);
	header.infotableofs = IntelLong(header.infotableofs);
	length = header.numlumps*sizeof(filelump_t);
	fileinfo = (filelump_t *)alloca (length);
	if (!fileinfo)
		I_FatalError ("RTS file could not allocate header info on stack");
	lseek (handle, header.infotableofs, SEEK_SET);
	SafeRead (handle, fileinfo, length);
	numlumps += header.numlumps;

	//
	// Fill in lumpinfo
	//
	SafeRealloc((void**)&lumpinfo,numlumps*sizeof(lumpinfo_t));
	lump_p = &lumpinfo[startlump];

	for (i=startlump ; i<numlumps ; i++,lump_p++, fileinfo++)
	{
		lump_p->handle = handle;
		lump_p->position = IntelLong(fileinfo->filepos);
		lump_p->size = IntelLong(fileinfo->size);
		strncpy (lump_p->name, fileinfo->name, 8);
	}
}

/*
====================
=
= RTS_Init
=
= Files with a .rts extension are idlink files with multiple lumps
=
====================
*/

void RTS_Init (char *filename)
{
	//
	// open all the files, load headers, and count lumps
	//
	numlumps = 0;
	lumpinfo = (lumpinfo_t*)SafeMalloc(5);   // will be realloced as lumps are added

	Printf("RTS Manager Started.\n");
	if (SafeFileExists(filename))
		RTS_AddFile (filename);

	if (!numlumps) return;

	//
	// set up caching
	//
	RTS_FirstSound = S_sfx.Size();
	sfxinfo_t sfx = { 0, };

	sfx.bWadLump = true;
	sfx.Priority = 254;
	sfx.link = sfxinfo_t::NO_LINK;

	// REMOSTRT is the first lump, and REMOSTOP is the last lump, so don't
	// add them to the sound effects list

	for (int i = 1; i < numlumps-1; ++i)
	{
		sfx.LumpNum = i;
		S_sfx.Push (sfx);
	}

	RTS_Started = true;
}


/*
====================
=
= RTS_NumSounds
=
====================
*/

int32 RTS_NumSounds (void)
{
	return numlumps-1;
}

/*
====================
=
= RTS_SoundLength
=
= Returns the buffer size needed to load the given lump
=
====================
*/

int32 RTS_SoundLength (int32 lump)
{
	if (lump >= numlumps)
		I_FatalError ("RTS_SoundLength: %i >= numlumps",lump);
	return lumpinfo[lump].size;
}

/*
====================
=
= RTS_GetSoundName
=
====================
*/

char * RTS_GetSoundName (int32 i)
{
	if (i>=numlumps)
		I_FatalError ("RTS_GetSoundName: %i >= numlumps",i);
	return &(lumpinfo[i].name[0]);
}

/*
====================
=
= RTS_ReadLump
=
= Loads the lump into the given buffer, which must be >= RTS_SoundLength()
=
====================
*/
void RTS_ReadLump (int32 lump, void *dest)
{
	lumpinfo_t *l;

	if (lump >= numlumps)
		I_FatalError ("RTS_ReadLump: %i >= numlumps",lump);
	if (lump < 0)
		I_FatalError ("RTS_ReadLump: %i < 0",lump);
	l = lumpinfo+lump;
	lseek (l->handle, l->position, SEEK_SET);
	SafeRead(l->handle,dest,l->size);
}

/*
====================
=
= RTS_OpenLump
=
= "Opens" the lump for reading
=
====================
*/
int RTS_OpenLump (int32 lump)
{
	lumpinfo_t *l;

	if (lump >= numlumps)
		I_FatalError ("RTS_ReadLump: %i >= numlumps",lump);
	if (lump < 0)
		I_FatalError ("RTS_ReadLump: %i < 0",lump);
	l = lumpinfo+lump;
	lseek (l->handle, l->position, SEEK_SET);
	return l->handle;
}
