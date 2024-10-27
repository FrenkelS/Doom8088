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
 *  all external data is defined here
 *  most of the data is loaded into different structures at run time
 *  some internal structures shared by many modules are here
 *
 *-----------------------------------------------------------------------------*/

#ifndef __DOOMDATA__
#define __DOOMDATA__

// The most basic types we use, portability.
#include "config.h"
#include "doomtype.h"

//
// Map level types.
// The following data structures define the persistent format
// used in the lumps of the WAD files.
//


#define NO_INDEX ((uint16_t)-1)
#define NO_INDEX8 (0xff)

//
// LineDef attributes.
//

// Solid, is an obstacle.
#define ML_BLOCKING             1

// Blocks monsters only.
#define ML_BLOCKMONSTERS        2

// Backside will not be drawn if not two sided.
#define ML_TWOSIDED             4

// If a texture is pegged, the texture will have
// the end exposed to air held constant at the
// top or bottom of the texture (stairs or pulled
// down things) and will move with a height change
// of one of the neighbor sectors.
// Unpegged textures always have the first row of
// the texture at the top pixel of the line for both
// top and bottom textures (use next to windows).

// upper texture unpegged
#define ML_DONTPEGTOP           8

// lower texture unpegged
#define ML_DONTPEGBOTTOM        16

// In AutoMap: don't map as two sided: IT'S A SECRET!
#define ML_SECRET               32

// Sound rendering: don't let sound cross two of these.
#define ML_SOUNDBLOCK           64

// Don't draw on the automap at all.
#define ML_DONTDRAW             128

// Set if already seen, thus drawn in automap.
#define ML_MAPPED               256


// BSP node structure.

// Indicate a leaf.
#define NF_SUBSECTOR    0x8000

// Thing definition, position, orientation and type,
// plus skill/visibility flags and attributes.
typedef struct {
  int16_t x;
  int16_t y;
  int16_t type;
  int8_t angle;
  int8_t options;
} mapthing_t;

typedef char assertMapthingSize[sizeof(mapthing_t) == 8 ? 1 : -1];


/* Bounding box coordinate storage. */
enum
{
  BOXTOP,
  BOXBOTTOM,
  BOXLEFT,
  BOXRIGHT
};  /* bbox coordinates */


//This is used at runtime so not packed.
//compiler uses byte access on packed structs.

typedef struct {
  int16_t x;  // Partition line from (x,y) to x+dx,y+dy)
  int16_t y;
  int16_t dx;
  int16_t dy;
  // Bounding box for each child, clip against view frustum.
  int16_t bbox[2][4];
  // If NF_SUBSECTOR its a subsector, else it's a node of another subtree.
  uint16_t children[2];
} mapnode_t;

typedef char assertMapnodeSize[sizeof(mapnode_t) == 28 ? 1 : -1];



#endif // __DOOMDATA__
