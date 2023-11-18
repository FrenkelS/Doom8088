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
#include <stdint.h>

#include "compiler.h"
#include "d_player.h"
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
  int32_t  filepos;
  int32_t  size;
  char name[8];
} filelump_t;


//
// GLOBALS
//

static FILE* fileWAD;

static int16_t numlumps;

static filelump_t __far* fileinfo;

static void __far*__far* lumpcache;

//
// LUMP BASED ROUTINES.
//

#define BUFFERSIZE 512

static void _ffread(void __far* ptr, uint16_t size, FILE* fp)
{
	uint8_t __far* dest = ptr;
	uint8_t* buffer = alloca(BUFFERSIZE);

	while (size >= BUFFERSIZE)
	{
		fread(buffer, BUFFERSIZE, 1, fp);
		_fmemcpy(dest, buffer, BUFFERSIZE);
		dest += BUFFERSIZE;
		size -= BUFFERSIZE;
	}

	if (size > 0)
	{
		fread(buffer, size, 1, fp);
		_fmemcpy(dest, buffer, size);
	}
}

typedef struct
{
  char identification[4]; // Should be "IWAD" or "PWAD".
  int16_t  numlumps;
  int16_t  filler;        // always zero
  int32_t  infotableofs;
} wadinfo_t;

void W_Init(void)
{
	printf("\tadding doom1.wad\n");
	printf("\tshareware version.\n");

	fileWAD = fopen("DOOM1.WAD", "rb");
	if (fileWAD == NULL)
		I_Error("Can't open DOOM1.WAD.");

	wadinfo_t header;
	fseek(fileWAD, 0, SEEK_SET);
	fread(&header, sizeof(header), 1, fileWAD);

	fileinfo = Z_MallocStatic(header.numlumps * sizeof(filelump_t));
	fseek(fileWAD, header.infotableofs, SEEK_SET);
	_ffread(fileinfo, sizeof(filelump_t) * header.numlumps, fileWAD);

	lumpcache = Z_MallocStatic(header.numlumps * sizeof(*lumpcache));
	_fmemset(lumpcache, 0, header.numlumps * sizeof(*lumpcache));

	numlumps = header.numlumps;
}


const char __far* PUREFUNC W_GetNameForNum(int16_t num)
{
	return fileinfo[num].name;
}


//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//

int32_t PUREFUNC W_LumpLength(int16_t num)
{
	return fileinfo[num].size;
}


// W_GetNumForName
// bombs out if not found.
//
int16_t PUREFUNC W_GetNumForName(const char *name)
{
	int64_t nameint;
	strncpy((char*)&nameint, name, 8);

#if BACKWARDS
	for (int16_t i = numlumps - 1; i >= 0; i--)
#else
	for (int16_t i = 0; i < numlumps; i++)
#endif
	{
		if (nameint == *(int64_t __far*)(fileinfo[i].name))
		{
			return i;
		}
	}

	I_Error("W_GetNumForName: %.8s not found", name);
	return -1;
}


void W_ReadLumpByNum(int16_t num, void __far* ptr)
{
	const filelump_t __far* lump = &fileinfo[num];
	fseek(fileWAD, lump->filepos, SEEK_SET);
	_ffread(ptr, lump->size, fileWAD);
}


const void __far* PUREFUNC W_GetLumpByNumAutoFree(int16_t num)
{
	const filelump_t __far* lump = &fileinfo[num];

	void __far* ptr = Z_MallocLevel(lump->size, NULL);

	fseek(fileWAD, lump->filepos, SEEK_SET);
	_ffread(ptr, lump->size, fileWAD);
	return ptr;
}


static void __far* PUREFUNC W_GetLumpByNumWithUser(int16_t num, void __far*__far* user)
{
	const filelump_t __far* lump = &fileinfo[num];

	void __far* ptr = Z_MallocStaticWithUser(lump->size, user);

	fseek(fileWAD, lump->filepos, SEEK_SET);
	_ffread(ptr, lump->size, fileWAD);
	return ptr;
}


int16_t W_GetFirstInt16(int16_t num)
{
	const filelump_t __far* lump = &fileinfo[num];

	int16_t firstInt16;

	fseek(fileWAD, lump->filepos, SEEK_SET);
	fread(&firstInt16, sizeof(int16_t), 1, fileWAD);
	return firstInt16;;
}


const void __far* PUREFUNC W_GetLumpByNum(int16_t num)
{
	if (lumpcache[num])
		Z_ChangeTagToStatic(lumpcache[num]);
	else
		lumpcache[num] = W_GetLumpByNumWithUser(num, &lumpcache[num]);

	return lumpcache[num];
}


boolean PUREFUNC W_IsLumpCached(int16_t num)
{
	return lumpcache[num] != NULL;
}


const void __far* PUREFUNC W_TryGetLumpByNum(int16_t num)
{
	if (W_IsLumpCached(num) || Z_IsEnoughFreeMemory(W_LumpLength(num)))
		return W_GetLumpByNum(num);
	else
		return NULL;
}
