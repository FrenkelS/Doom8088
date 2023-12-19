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
 *  Refresh module, data I/O, caching, retrieval of graphics
 *  by name.
 *
 *-----------------------------------------------------------------------------*/


#ifndef __R_DATA__
#define __R_DATA__

#include "r_defs.h"
#include "d_player.h"


// A single patch from a texture definition, basically
// a rectangular area within the texture rectangle.
typedef struct
{
  int16_t originx, originy;  // Block origin, which has already accounted
  int16_t patch_num;    // for the internal origin of the patch.
  int16_t patch_width;
} texpatch_t;

//
// Texture definition.
// A DOOM wall texture is a list of patches
// which are to be combined in a predefined order.
//

typedef struct
{
  //const char*  name;         // Keep name for switch changing, etc.
  //int32_t   next, index;     // killough 1/31/98: used in hashing algorithm
  // CPhipps - moved arrays with per-texture entries to elements here
  uint16_t  widthmask;
  // CPhipps - end of additions
  int16_t width, height;

  uint8_t overlapped;
  uint8_t patchcount;      // All the patches[patchcount] are drawn
  texpatch_t patches[1]; // back-to-front into the cached texture.
} texture_t;



// I/O, setting up the stuff.
void R_Init(void);
void R_InitFlats(void);
void R_InitSky(void);


// Retrieval.
// Floor/ceiling opaque texture tiles,
// lookup by name. For animation?
int16_t R_FlatNumForName (const char* name);   // killough -- const added


// R_*TextureNumForName returns the texture number for the texture name, or NO_TEXTURE if 
//  there is no texture (i.e. "-") specified.
/* cph 2006/07/23 - defined value for no-texture marker (texture "-" in the WAD file) */

int16_t R_CheckTextureNumForName (const char *name);

const texture_t __far* R_GetTexture(int16_t texture);
void P_LoadTexture(int16_t texture);


#endif
