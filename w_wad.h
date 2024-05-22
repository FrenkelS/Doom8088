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
 *  Copyright 2023, 2024 by
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
 *      WAD I/O functions.
 *
 *-----------------------------------------------------------------------------*/


#ifndef __W_WAD__
#define __W_WAD__

#include "doomtype.h"


void W_Init(void);
void W_Shutdown(void);

int16_t           PUREFUNC W_GetNumForName(const char *name);
const char __far* PUREFUNC W_GetNameForNum(       int16_t num);
uint16_t          PUREFUNC W_LumpLength(          int16_t num);
boolean           PUREFUNC W_IsLumpCached(        int16_t num);
int16_t                    W_GetFirstInt16(       int16_t num);
const void __far* PUREFUNC W_GetLumpByNum(        int16_t num);
const void __far* PUREFUNC W_TryGetLumpByNum(     int16_t num);
const void __far* PUREFUNC W_GetLumpByNumAutoFree(int16_t num);
void                       W_ReadLumpByNum(       int16_t num, void __far* ptr);

#define W_GetLumpByName(x)    W_GetLumpByNum(W_GetNumForName(x))

#endif
