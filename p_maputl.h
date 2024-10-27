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
 *      Map utility functions
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_MAPUTL__
#define __P_MAPUTL__

#include "r_defs.h"

/* mapblocks are used to check movement against lines and things */
#define MAPBLOCKUNITS   128
#define MAPBLOCKSIZE    (MAPBLOCKUNITS*FRACUNIT)
#define MAPBTOFRAC      7
#define MAPBLOCKSHIFT   (FRACBITS+MAPBTOFRAC)

#define PT_ADDLINES     1
#define PT_ADDTHINGS    2


typedef struct
{
  fixed_t     x;
  fixed_t     y;
  fixed_t     dx;
  fixed_t     dy;
} divline_t;

typedef struct {
  fixed_t     frac;           /* along trace line */
  boolean     isaline;
  union
  {
    mobj_t __far* thing;
    line_t __far* line;
  } d;
} intercept_t;


typedef boolean (*traverser_t)(intercept_t *in);

fixed_t CONSTFUNC P_AproxDistance (fixed_t dx, fixed_t dy);
int16_t     P_PointOnLineSide (fixed_t x, fixed_t y, const line_t __far* line);
int16_t     P_BoxOnLineSide (const fixed_t *tmbox, const line_t __far* ld);

void    P_LineOpening(const line_t __far* linedef);
void    P_UnsetThingPosition(mobj_t __far* thing);
void    P_SetThingPosition(mobj_t __far* thing);
boolean P_BlockLinesIterator (int16_t x, int16_t y, boolean func(line_t __far*));
boolean P_BlockThingsIterator(int16_t x, int16_t y, boolean func(mobj_t __far*));
boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
                       int16_t flags, boolean trav(intercept_t *));

#endif  /* __P_MAPUTL__ */
