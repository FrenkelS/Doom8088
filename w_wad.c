/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2001 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023 by
 *  Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Handles WAD file header, directory, lump I/O.
 *
 *-----------------------------------------------------------------------------
 */

// use config.h if autoconf made one -- josh
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <fcntl.h>

#include "doomstat.h"
#include "d_net.h"
#include "doomtype.h"
#include "i_system.h"

#include "w_wad.h"

#include "globdata.h"

//#define BACKWARDS


//
// TYPES
//

typedef struct
{
  char identification[4]; // Should be "IWAD" or "PWAD".
  int16_t  numlumps;
  int16_t  filler;        // always zero
  int32_t  infotableofs;
} wadinfo_t;

typedef struct
{
  int32_t  filepos;
  int32_t  size;
  char name[8];
} filelump_t;


//
// GLOBALS
//

static FILE* fileWAD;

static wadinfo_t header;

static filelump_t fileinfo;

static void **lumpcache;

//
// LUMP BASED ROUTINES.
//

static const filelump_t* PUREFUNC W_FindLumpByNum(int16_t num)
{
#ifdef RANGECHECK
	if (!(0 <= num && num < header.numlumps))
		I_Error("W_FindLumpByNum: %i >= numlumps", num);
#endif

	fseek(fileWAD, header.infotableofs + num * sizeof(filelump_t), SEEK_SET);
	fread(&fileinfo, sizeof(filelump_t), 1, fileWAD);
	return &fileinfo;
}


// W_GetNumForName
// bombs out if not found.
//
int16_t PUREFUNC W_GetNumForName(const char *name)     // killough -- const added
{
	int64_t nameint;
	strncpy((char*)&nameint, name, 8);

#if BACKWARDS
	for (int16_t i = header.numlumps - 1; i >= 0; i--)
#else
	for (int16_t i = 0; i < header.numlumps; i++)
#endif
	{
		fseek(fileWAD, header.infotableofs + i * sizeof(filelump_t), SEEK_SET);
		fread(&fileinfo, sizeof(filelump_t), 1, fileWAD);

		if (nameint == *(int64_t*)fileinfo.name)
		{
			return i;
		}
	}

	I_Error("W_GetNumForName: %.8s not found", name);
	return -1;
}


const char* PUREFUNC W_GetNameForNum(int16_t num)
{
	const filelump_t* lump = W_FindLumpByNum(num);
	return lump->name;
}


void W_Init(void)
{
	printf("\tadding doom1.wad\n");
	printf("\tshareware version.\n");

	fileWAD = fopen("DOOM1.WAD", "rb");
	if (fileWAD == NULL)
		I_Error("Can't open DOOM1.WAD.");

	fseek(fileWAD, 0, SEEK_SET);
	fread(&header, sizeof(header), 1, fileWAD);

	lumpcache = Z_MallocStatic(header.numlumps * sizeof(*lumpcache));
	memset(lumpcache, 0, header.numlumps * sizeof(*lumpcache));
}


//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//

int32_t PUREFUNC W_LumpLength(int16_t num)
{
	const filelump_t* lump = W_FindLumpByNum(num);
	return lump->size;
}


static const filelump_t* PUREFUNC W_GetFileInfoForName(const char *name)
{
	int64_t nameint;
	strncpy((char*)&nameint, name, 8);

#if BACKWARDS
	for (int16_t i = header.numlumps - 1; i >= 0; i--)
#else
	for (int16_t i = 0; i < header.numlumps; i++)
#endif
	{
		fseek(fileWAD, header.infotableofs + i * sizeof(filelump_t), SEEK_SET);
		fread(&fileinfo, sizeof(filelump_t), 1, fileWAD);

		if (nameint == *(int64_t*)fileinfo.name)
		{
			return &fileinfo;
		}
	}

	I_Error("W_GetFileInfoForName: %.8s not found", name);
	return NULL;
}


static const void* PUREFUNC W_GetLump(const filelump_t* lump)
{
	void* ptr = Z_MallocStatic(lump->size);
	fseek(fileWAD, lump->filepos, SEEK_SET);
	fread(ptr, lump->size, 1, fileWAD);
	return ptr;
}


const void* PUREFUNC W_GetLumpByNum(int16_t num)
{
	if (lumpcache[num])
		Z_ChangeTagToStatic(lumpcache[num]);
	else
		lumpcache[num] = W_GetLumpByNumWithUser(num, &lumpcache[num]);

	return lumpcache[num];
}


static const void* PUREFUNC W_GetLumpByNumWithUser(int16_t num, void **user)
{
	const filelump_t* lump = W_FindLumpByNum(num);

	void* ptr = Z_MallocStaticWithUser(lump->size, user);

	fseek(fileWAD, lump->filepos, SEEK_SET);
	fread(ptr, lump->size, 1, fileWAD);
	return ptr;
}


const void* PUREFUNC W_GetLumpByNumAutoFree(int16_t num)
{
	const filelump_t* lump = W_FindLumpByNum(num);

	void* ptr = Z_MallocLevel(lump->size, NULL);

	fseek(fileWAD, lump->filepos, SEEK_SET);
	fread(ptr, lump->size, 1, fileWAD);
	return ptr;
}


const void* PUREFUNC W_GetLumpByName(const char *name)
{
	const filelump_t* lump = W_GetFileInfoForName(name);
	return W_GetLump(lump);
}


void W_ReadLumpByName(const char *name, void *ptr)
{
	const filelump_t* lump = W_GetFileInfoForName(name);
	fseek(fileWAD, lump->filepos, SEEK_SET);
	fread(ptr, lump->size, 1, fileWAD);
}
