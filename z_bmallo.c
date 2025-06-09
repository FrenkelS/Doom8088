/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023-2025 by
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
 * This is designed to be a fast allocator for small, regularly used block sizes
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "doomtype.h"
#include "compiler.h"
#include "z_zone.h"
#include "z_bmallo.h"
#include "i_system.h"

typedef struct bmalpool_s {
	struct bmalpool_s __far* nextpool;
	size_t             blocks;
	byte               used[];
} bmalpool_t;


inline static void __far* getelem(bmalpool_t __far* p, size_t size, size_t n)
{
	return (((byte __far*)p) + sizeof(bmalpool_t) + sizeof(byte) * (p->blocks) + size * n);
}


inline static PUREFUNC int16_t iselem(const bmalpool_t __far* pool, size_t size, const void __far* p)
{
#if defined _M_I86
	if (D_FP_SEG(p) != D_FP_SEG(pool))
		return -1;

	uint16_t dif = D_FP_OFF(p) - D_FP_OFF(pool);
	dif -= sizeof(bmalpool_t);
	dif -= pool->blocks;
	dif /= size;
	return dif;
#else
	int32_t dif = (const char*)p - (const char*)pool;

	dif -= sizeof(bmalpool_t);
	dif -= pool->blocks;
	if (dif < 0)
		return -1;

	dif /= size;
	return (((size_t)dif >= pool->blocks) ? -1 : dif);
#endif
}


enum { unused_block = 0, used_block = 1};


void __far* Z_BMalloc(struct block_memory_alloc_s *pzone)
{
	bmalpool_t __far*__far* pool = (bmalpool_t __far*__far*)&(pzone->firstpool);
	while (*pool != NULL) {
		byte __far* p = _fmemchr((*pool)->used, unused_block, (*pool)->blocks); // Scan for unused marker
		if (p) {
			int16_t n = p - (*pool)->used;
			(*pool)->used[n] = used_block;
			return getelem(*pool, pzone->size, n);
		} else
			pool = &((*pool)->nextpool);
	}

	// Nothing available, must allocate a new pool
	bmalpool_t __far* newpool;

	// CPhipps: Allocate new memory, initialised to 0

	*pool = newpool = Z_CallocLevel(sizeof(*newpool) + (sizeof(byte) + pzone->size) * (pzone->perpool));
	newpool->nextpool = NULL; // NULL = (void*)0 so this is redundant

	// Return element 0 from this pool to satisfy the request
	newpool->used[0] = used_block;
	newpool->blocks = pzone->perpool;
	return getelem(newpool, pzone->size, 0);
}


void Z_BFree(struct block_memory_alloc_s *pzone, void __far* p)
{
	bmalpool_t __far*__far* pool = (bmalpool_t __far*__far*)&(pzone->firstpool);

	while (*pool != NULL) {
		int16_t n = iselem(*pool, pzone->size, p);
		if (n >= 0) {
			(*pool)->used[n] = unused_block;
			if (_fmemchr(((*pool)->used), used_block, (*pool)->blocks) == NULL) {
				// Block is all unused, can be freed
				bmalpool_t __far* oldpool = *pool;
				*pool = (*pool)->nextpool;
				Z_Free(oldpool);
			}
			return;
		} else
			pool = &((*pool)->nextpool);
	}

	I_Error("Z_BFree: Free not in zone");
}
