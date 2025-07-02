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
 *      Rendering main loop and setup functions,
 *       utility functions (BSP, geometry, trigonometry).
 *      See tables.c, too.
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "compiler.h"
#include "d_player.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_things.h"
#include "m_fixed.h"
#include "st_stuff.h"
#include "i_system.h"
#include "g_game.h"
#include "m_random.h"

#include "globdata.h"


// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
#define SIL_NONE    0
#define SIL_BOTTOM  1
#define SIL_TOP     2
#define SIL_BOTH    3

typedef struct drawseg_s
{
  const seg_t __far* curline;
  int16_t x1, x2;
  fixed_t scale1, scale2, scalestep;
  int16_t silhouette;                       // 0=none, 1=bottom, 2=top, 3=both
  fixed_t bsilheight;                   // do not clip sprites above this
  fixed_t tsilheight;                   // do not clip sprites below this

  // Pointers to lists for sprite clipping,
  // all three adjusted so [x1] is first value.

  int16_t *sprtopclip, *sprbottomclip;
  int16_t *maskedtexturecol; // dropoff overflow
} drawseg_t;

#define MAXDRAWSEGS   128

static drawseg_t _s_drawsegs[MAXDRAWSEGS];


#define MAXOPENINGS (VIEWWINDOWWIDTH*16)

static int16_t openings[MAXOPENINGS];
static int16_t* lastopening;


#if VIEWWINDOWWIDTH == 240
#define VIEWANGLETOXMAX 1029
#elif VIEWWINDOWWIDTH == 120
#define VIEWANGLETOXMAX 1034
#elif VIEWWINDOWWIDTH == 80
#define VIEWANGLETOXMAX 1040
#elif VIEWWINDOWWIDTH == 60
#define VIEWANGLETOXMAX 1046
#elif VIEWWINDOWWIDTH == 40
#define VIEWANGLETOXMAX 1057
#elif VIEWWINDOWWIDTH == 30
#define VIEWANGLETOXMAX 1068
#else
#error unsupported VIEWWINDOWWIDTH value
#endif
static const uint8_t viewangletoxTable[4096 - 1023 - VIEWANGLETOXMAX];


static uint8_t viewangletox(int16_t va)
{
#ifdef RANGECHECK
	if (va < 0)
		I_Error("viewangletox: va < 0: %i", va);
	else if (va >= 4096)
		I_Error("viewangletox: va >= 4096: %i", va);
#endif

	if (va < VIEWANGLETOXMAX)	//               0 <= va < VIEWANGLETOXMAX
		return VIEWWINDOWWIDTH;
	else if (3073 <= va)		//            3073 <= va < 4096
		return 0;
	else						// VIEWANGLETOXMAX <= va < 3073
		return viewangletoxTable[va - VIEWANGLETOXMAX];
}


static const angle_t tantoangleTable[2049];

#if BYTE_ORDER == LITTLE_ENDIAN
static const angle16_t* tantoangle16Table = ((angle16_t*)&tantoangleTable[0]) + 1;
#elif BYTE_ORDER == BIG_ENDIAN
static const angle16_t* tantoangle16Table = ((angle16_t*)&tantoangleTable[0]);
#else
#error unknown byte order
#endif

#define tantoangle(t) tantoangleTable[t]
#define tantoangle16(t) tantoangle16Table[(t)*2]


static const uint16_t finetangentTable_part_3[1024];
#if VIEWWINDOWWIDTH == 240
static const fixed_t  __far finetangentTable_part_4[1024];
#else
static const fixed_t  finetangentTable_part_4[1024];
#endif

static int16_t floorclip[VIEWWINDOWWIDTH];
static int16_t ceilingclip[VIEWWINDOWWIDTH];


static int16_t screenheightarray[VIEWWINDOWWIDTH] =
{
#if VIEWWINDOWWIDTH == 240
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT
#elif VIEWWINDOWWIDTH == 120
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT
#elif VIEWWINDOWWIDTH == 80
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT
#elif VIEWWINDOWWIDTH == 60
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT
#elif VIEWWINDOWWIDTH == 40
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,

	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT
#elif VIEWWINDOWWIDTH == 30
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT,
	VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT, VIEWWINDOWHEIGHT
#else
#error unsupported VIEWWINDOWWIDTH value
#endif
};

static int16_t negonearray[VIEWWINDOWWIDTH] =
{
#if VIEWWINDOWWIDTH == 240
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1
#elif VIEWWINDOWWIDTH == 120
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1
#elif VIEWWINDOWWIDTH == 80
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,

	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,

	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,

	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1
#elif VIEWWINDOWWIDTH == 60
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,

	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1
#elif VIEWWINDOWWIDTH == 40
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,

	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1
#elif VIEWWINDOWWIDTH == 30
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1
#else
#error unsupported VIEWWINDOWWIDTH value
#endif
};


//*****************************************
//Globals.
//*****************************************

int16_t numnodes;
const mapnode_t __far* nodes;

#if defined FLAT_SPAN
static fixed_t  viewx, viewy, viewz;
static fixed_t  viewcos, viewsin;
#else
fixed_t  viewx, viewy, viewz;
fixed_t  viewcos, viewsin;
#endif

angle_t  viewangle;
static angle16_t viewangle16;

static byte solidcol[VIEWWINDOWWIDTH];

static const seg_t     __far* curline;
static side_t    __far* sidedef;
static line_t    __far* linedef;
static sector_t  __far* frontsector;
static sector_t  __far* backsector;
static drawseg_t *ds_p;

#if defined FLAT_SPAN
static int16_t floorplane_color;
static int16_t ceilingplane_color;
#else
static visplane_t __far* floorplane;
static visplane_t __far* ceilingplane;
#endif

static angle16_t             rw_angle1;

static angle16_t         rw_normalangle; // angle to line origin
static int16_t         rw_distance;

static int16_t      rw_stopx;

static fixed_t  rw_scale;
static fixed_t  rw_scalestep;

static int32_t      worldtop;
static int32_t      worldbottom;

static boolean didsolidcol; /* True if at least one column was marked solid */

static boolean  maskedtexture;
static int16_t      toptexture;
static int16_t      bottomtexture;
static int16_t      midtexture;
static const texture_t __far* textoptexture;
static const texture_t __far* texbottomtexture;
static const texture_t __far* texmidtexture;

static fixed_t  rw_midtexturemid;
static fixed_t  rw_toptexturemid;
static fixed_t  rw_bottomtexturemid;

const uint8_t fullcolormap[256 * 34];
const uint8_t* fixedcolormap;

static int16_t extralight;                           // bumped light from gun blasts


static int16_t   *mfloorclip;   // dropoff overflow
static int16_t   *mceilingclip; // dropoff overflow
static fixed_t spryscale;
static fixed_t sprtopscreen;

static angle16_t  rw_centerangle;
static int16_t  rw_offset;
static int16_t      rw_lightlevel;

static int16_t      *maskedtexturecol; // dropoff overflow

int16_t   __far* textureheight; //needed for texture pegging (and TFE fix - killough)

int16_t       __far* texturetranslation;


static fixed_t  topfrac;
static fixed_t  topstep;
static fixed_t  bottomfrac;
static fixed_t  bottomstep;

static fixed_t  pixhigh;
static fixed_t  pixlow;

static fixed_t  pixhighstep;
static fixed_t  pixlowstep;

static int32_t      worldhigh;
static int32_t      worldlow;


uint16_t validcount = 1;         // increment every time a check is made

//*****************************************
// Constants
//*****************************************

#define COLEXTRABITS (8 - 1)

static const int16_t CENTERX = VIEWWINDOWWIDTH  / 2;
       const int16_t CENTERY = VIEWWINDOWHEIGHT / 2;

static const fixed_t PROJECTION = (VIEWWINDOWWIDTH / 2L) << FRACBITS;

static const uint16_t PSPRITESCALE  = FRACUNIT * VIEWWINDOWWIDTH / SCREENWIDTH_VGA;
static const fixed_t  PSPRITEISCALE = FRACUNIT * SCREENWIDTH_VGA / VIEWWINDOWWIDTH; // = FixedReciprocal(PSPRITESCALE)

static const uint16_t PSPRITEYSCALE = FRACUNIT * (VIEWWINDOWHEIGHT * 5 / 4) / SCREENHEIGHT_VGA;
static const uint16_t PSPRITEYFRACSTEP = (FRACUNIT * SCREENHEIGHT_VGA / (VIEWWINDOWHEIGHT * 5 / 4)) >> COLEXTRABITS; // = FixedReciprocal(PSPRITEYSCALE) >> COLEXTRABITS

static const angle16_t clipangle = 0x2008; // = xtoviewangleTable[0]


#if defined __WATCOMC__
//
#else
inline
#endif
fixed_t CONSTFUNC FixedMul(fixed_t a, fixed_t b)
{
	uint16_t alw = a;
	 int16_t ahw = a >> FRACBITS;
	uint16_t blw = b;
	 int16_t bhw = b >> FRACBITS;

	if (bhw == 0) {
		uint32_t ll = (uint32_t) alw * blw;
		 int32_t hl = ( int32_t) ahw * blw;
		return (ll >> FRACBITS) + hl;
	} else if (alw == 0) {
		//return ahw * b;
		 int32_t hl = ( int32_t) ahw * blw;
		 int32_t hh = ( int32_t) ahw * bhw;
		return hl + (hh << FRACBITS);
	} else {
		uint32_t ll = (uint32_t) alw * blw;
		 int32_t hl = ( int32_t) ahw * blw;
		return (a * bhw) + (ll >> FRACBITS) + hl;
	}
}


inline static fixed_t CONSTFUNC FixedMul3232(fixed_t a, fixed_t b)
{
	uint16_t alw = a;
	 int16_t ahw = a >> FRACBITS;
	uint16_t blw = b;
	 int16_t bhw = b >> FRACBITS;

	uint32_t ll = (uint32_t) alw * blw;
	 int32_t hl = ( int32_t) ahw * blw;
	return (a * bhw) + (ll >> FRACBITS) + hl;
}


//
// FixedMulAngle
// b should be coming from finesine() or finecosine(), so its high word is either 0x0000 or 0xffff
//
#if defined __WATCOMC__
//
#else
inline
#endif
fixed_t CONSTFUNC FixedMulAngle(fixed_t a, fixed_t b)
{
	uint16_t alw = a;
	 int16_t ahw = a >> FRACBITS;
	uint16_t blw = b;

	uint32_t ll = (uint32_t) alw * blw;
	 int32_t hl = ( int32_t) ahw * blw;
	fixed_t r = (ll >> FRACBITS) + hl;

	if (b < 0)
		r -= a;

	return r;
}


#if defined __WATCOMC__
//
#else
inline
#endif
fixed_t CONSTFUNC FixedMul3216(fixed_t a, uint16_t blw)
{
	uint16_t alw = a;
	 int16_t ahw = a >> FRACBITS;

	uint32_t ll = (uint32_t) alw * blw;
	 int32_t hl = ( int32_t) ahw * blw;
	return (ll >> FRACBITS) + hl;
}


//Approx fixed point divide of a/b using reciprocal. -> a * (1/b).
#if defined __WATCOMC__
//
#else
inline
#endif
fixed_t CONSTFUNC FixedApproxDiv(fixed_t a, fixed_t b)
{
	if (b <= 0xffffu)
		return FixedMul3232(a, FixedReciprocalSmall(b));
	else
		return FixedMul3216(a, FixedReciprocalBig(b));
}


//
// R_PointOnSide
// Traverse BSP (sub) tree,
//  check point against partition plane.
// Returns side 0 (front) or 1 (back).
//

static PUREFUNC int8_t R_PointOnSide(fixed_t x, fixed_t y, const mapnode_t __far* node)
{
	int16_t ix = x >> FRACBITS;

	if (!node->dx)
		return ix <= node->x ? node->dy > 0 : node->dy < 0;

	int16_t iy = y >> FRACBITS;

	if (!node->dy)
		return iy <= node->y ? node->dx < 0 : node->dx > 0;

	x -= (fixed_t)node->x << FRACBITS;
	y -= (fixed_t)node->y << FRACBITS;

	ix = x >> FRACBITS;
	iy = y >> FRACBITS;

	// Try to quickly decide by looking at sign bits.
	if ((node->dy ^ node->dx ^ ix ^ iy) < 0)
		return (node->dy ^ ix) < 0;  // (left is negative)

	//return FixedMul(y, node->dx) >= FixedMul(x, node->dy);
	return (y >> 8) * node->dx >= (x >> 8) * node->dy;
}

//
// R_PointInSubsector
//

subsector_t __far* R_PointInSubsector(fixed_t x, fixed_t y)
{
	static fixed_t prevx;
	static fixed_t prevy;
	static subsector_t __far* prevr;

	if (prevx == x && prevy == y)
		return prevr;

	prevx = x;
	prevy = y;

	int16_t nodenum = numnodes-1;

	// special case for trivial maps (single subsector, no nodes)
	if (numnodes == 0)
	{
		prevr = _g_subsectors;
		return prevr;
	}

	while (!(nodenum & NF_SUBSECTOR))
		nodenum = nodes[nodenum].children[R_PointOnSide(x, y, nodes+nodenum)];

	prevr = &_g_subsectors[(int16_t)(nodenum & ~NF_SUBSECTOR)];
	return prevr;
}


#define SLOPERANGE 2048

static CONSTFUNC int16_t SlopeDiv(uint32_t num, uint32_t den)
{
    den = den >> 8;

    if (den == 0)
        return SLOPERANGE;

    const uint16_t ans = (num << 3) / den;//FixedApproxDiv(num << 3, den) >> FRACBITS;

    return (ans <= SLOPERANGE) ? ans : SLOPERANGE;
}


static CONSTFUNC int16_t SlopeDiv16(uint16_t n, uint16_t d)
{
	if (d == 0)
		return SLOPERANGE;

	const uint16_t ans = ((uint32_t)n * SLOPERANGE) / d;

	return (ans <= SLOPERANGE) ? ans : SLOPERANGE;
}


//
// R_PointToAngle
// To get a global angle from cartesian coordinates,
//  the coordinates are flipped until they are in
//  the first octant of the coordinate system, then
//  the y (<=x) is scaled and divided by x to get a
//  tangent (slope) value which is looked up in the
//  tantoangleTable[] table.
//


CONSTFUNC angle_t R_PointToAngle3(fixed_t x, fixed_t y)
{
    if ( (!x) && (!y) )
        return 0;

    if (x>= 0)
    {
        // x >=0
        if (y>= 0)
        {
            // y>= 0

            if (x>y)
            {
                // octant 0
                return tantoangle(SlopeDiv(y,x));
            }
            else
            {
                // octant 1
                return ANG90-1-tantoangle(SlopeDiv(x,y));
            }
        }
        else
        {
            // y<0
            y = -y;

            if (x>y)
            {
                // octant 8
                return -tantoangle(SlopeDiv(y,x));
            }
            else
            {
                // octant 7
                return ANG270+tantoangle(SlopeDiv(x,y));
            }
        }
    }
    else
    {
        // x<0
        x = -x;

        if (y>= 0)
        {
            // y>= 0
            if (x>y)
            {
                // octant 3
                return ANG180-1-tantoangle(SlopeDiv(y,x));
            }
            else
            {
                // octant 2
                return ANG90+ tantoangle(SlopeDiv(x,y));
            }
        }
        else
        {
            // y<0
            y = -y;

            if (x>y)
            {
                // octant 4
                return ANG180+tantoangle(SlopeDiv(y,x));
            }
            else
            {
                // octant 5
                return ANG270-1-tantoangle(SlopeDiv(x,y));
            }
        }
    }
}


#define R_PointToAngle(x,y) R_PointToAngle16((x)>>FRACBITS,(y)>>FRACBITS)


static angle16_t R_PointToAngle16(int16_t x, int16_t y)
{
    x = x - (viewx >> FRACBITS);
    y = y - (viewy >> FRACBITS);

    if (!x && !y)
        return 0;

    if (x >= 0)
    {
        // x >= 0
        if (y >= 0)
        {
            // y >= 0

            if (x > y)
            {
                // octant 0
                return tantoangle16(SlopeDiv16(y, x));
            }
            else
            {
                // octant 1
                return ANG90_16 - 1 - tantoangle16(SlopeDiv16(x, y));
            }
        }
        else
        {
            // y < 0
            y = -y;

            if (x > y)
            {
                // octant 8
                return -tantoangle16(SlopeDiv16(y, x));
            }
            else
            {
                // octant 7
                return ANG270_16 + tantoangle16(SlopeDiv16(x, y));
            }
        }
    }
    else
    {
        // x < 0
        x = -x;

        if (y >= 0)
        {
            // y >= 0
            if (x > y)
            {
                // octant 3
                return ANG180_16 - 1 - tantoangle16(SlopeDiv16(y, x));
            }
            else
            {
                // octant 2
                return ANG90_16 + tantoangle16(SlopeDiv16(x, y));
            }
        }
        else
        {
            // y < 0
            y = -y;

            if (x > y)
            {
                // octant 4
                return ANG180_16 + tantoangle16(SlopeDiv16(y, x));
            }
            else
            {
                // octant 5
                return ANG270_16 - 1 - tantoangle16(SlopeDiv16(x, y));
            }
        }
    }
}


#define SLOPEBITS    11
#define DBITS      (FRACBITS-SLOPEBITS)

static CONSTFUNC int16_t R_PointToDist(int16_t x, int16_t y)
{
    if (viewx == (fixed_t)x << FRACBITS && viewy == (fixed_t)y << FRACBITS)
        return 0;

    fixed_t dx = D_abs(((fixed_t)x << FRACBITS) - viewx);
    fixed_t dy = D_abs(((fixed_t)y << FRACBITS) - viewy);

    if (dy > dx)
    {
        fixed_t t = dx;
        dx = dy;
        dy = t;
    }

    return dx / finecosineapprox((FixedApproxDiv(dy,dx) >> DBITS) / 2);
}


// Lighting constants.

#define LIGHTSEGSHIFT      4


// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.

#define NUMCOLORMAPS 32


const uint8_t* R_LoadColorMap(int16_t lightlevel)
{
    if (fixedcolormap)
        return fixedcolormap;
    else
    {
        if (curline)
        {
            if (curline->v1.y == curline->v2.y)
                lightlevel -= 1 << LIGHTSEGSHIFT;
            else if (curline->v1.x == curline->v2.x)
                lightlevel += 1 << LIGHTSEGSHIFT;
        }

        lightlevel += (extralight +_g_gamma) << LIGHTSEGSHIFT;

        int16_t cm = ((256-lightlevel)>>2) - 24;

        if(cm >= NUMCOLORMAPS)
            cm = NUMCOLORMAPS-1;
        else if(cm < 0)
            cm = 0;

        return fullcolormap + cm*256;
    }
}


//
// R_DrawMaskedColumn
// Used for sprites and masked mid textures.
// Masked means: partly transparent, i.e. stored
//  in posts/runs of opaque pixels.
//

typedef void (*R_DrawColumn_f)(const draw_column_vars_t *dcvars);

static void R_DrawMaskedColumn(R_DrawColumn_f colfunc, draw_column_vars_t *dcvars, const column_t __far* column)
{
    const fixed_t basetexturemid = dcvars->texturemid;

    const int16_t fclip_x = mfloorclip[dcvars->x];
    const int16_t cclip_x = mceilingclip[dcvars->x];

    while (column->topdelta != 0xff)
    {
        // calculate unclipped screen coordinates for post
        const int32_t topscreen = sprtopscreen + spryscale*column->topdelta;
        const int32_t bottomscreen = topscreen + spryscale*column->length;

        int16_t yh = (bottomscreen-1)>>FRACBITS;
        int16_t yl = (topscreen+FRACUNIT-1)>>FRACBITS;

        if (yh >= fclip_x)
            yh = fclip_x - 1;

        if (yl <= cclip_x)
            yl = cclip_x + 1;

        // killough 3/2/98, 3/27/98: Failsafe against overflow/crash:
        if (yl <= yh && yh < VIEWWINDOWHEIGHT)
        {
            dcvars->source =  (const byte __far*)column + 3;

            dcvars->texturemid = basetexturemid - (((int32_t)column->topdelta)<<FRACBITS);

            dcvars->yh = yh;
            dcvars->yl = yl;

            // Drawn by either R_DrawColumn or (SHADOW) R_DrawFuzzColumn.
            colfunc (dcvars);
        }

        column = (const column_t __far*)((const byte __far*)column + column->length + 4);
    }

    dcvars->texturemid = basetexturemid;
}


//
// R_InitColormaps
//
void R_InitColormaps(void)
{
	W_ReadLumpByNum(W_GetNumForName("COLORMAP"), (uint8_t __far*)fullcolormap);
}


//
// A vissprite_t is a thing that will be drawn during a refresh.
// i.e. a sprite object that is partly visible.
//

typedef struct vissprite_s
{
  int16_t x1, x2;
  fixed_t gx, gy;              // for line side calculation
  fixed_t gz;                   // global bottom for silhouette clipping
  fixed_t startfrac;           // horizontal position of x1
  fixed_t scale;
  fixed_t xiscale;             // negative if flipped
  fixed_t texturemid;
  uint16_t fracstep;

  int16_t lump_num;
  int16_t patch_topoffset;

  // for color translation and shadow draw, maxbright frames as well
  const uint8_t* colormap;

} vissprite_t;


void R_DrawFuzzColumn (const draw_column_vars_t *dcvars);


//
// R_DrawVisSprite
//  mfloorclip and mceilingclip should also be set.
//
// CPhipps - new wad lump handling, *'s to const*'s
static void R_DrawVisSprite(const vissprite_t *vis)
{
    fixed_t  frac;

    R_DrawColumn_f colfunc = R_DrawColumnSprite;
    draw_column_vars_t dcvars;
    dcvars.colormap = vis->colormap;

    // killough 4/11/98: rearrange and handle translucent sprites
    // mixed with translucent/non-translucenct 2s normals

    if (!dcvars.colormap)   // NULL colormap = shadow draw
        colfunc = R_DrawFuzzColumn;    // killough 3/14/98

    // proff 11/06/98: Changed for high-res
    dcvars.fracstep = vis->fracstep;
    dcvars.texturemid = vis->texturemid;
    frac = vis->startfrac;

    spryscale = vis->scale;
    sprtopscreen = CENTERY * FRACUNIT - FixedMul(dcvars.texturemid, spryscale);


    const patch_t __far* patch = W_GetLumpByNum(vis->lump_num);

    dcvars.x = vis->x1;

    while (dcvars.x < VIEWWINDOWWIDTH)
    {
        const column_t __far* column = (const column_t __far*) ((const byte __far*)patch + (uint16_t)patch->columnofs[frac >> FRACBITS]);
        R_DrawMaskedColumn(colfunc, &dcvars, column);

        frac += vis->xiscale;

        if(((frac >> FRACBITS) >= patch->width) || frac < 0)
            break;

        dcvars.x++;
    }

    Z_ChangeTagToCache(patch);
}


static void R_GetColumn(const texture_t __far* texture, int16_t texcolumn, int16_t* patch_num, int16_t* x_c)
{
	const uint8_t patchcount = texture->patchcount;

	const int16_t xc = texcolumn & texture->widthmask;

	if (patchcount == 1)
	{
		//simple texture.
		*patch_num = texture->patches[0].patch_num;
		*x_c = xc;
	}
	else
	{
		uint8_t i = 0;

		do
		{
			const texpatch_t __far* patch = &texture->patches[i];

			int16_t x = xc - patch->originx;
			if (0 <= x && x < patch->patch_width)
			{
				*patch_num = patch->patch_num;
				*x_c = x;
				break;
			}
		} while (++i < patchcount);
	}
}


//
// R_RenderMaskedSegRange
//

static void R_RenderMaskedSegRange(const drawseg_t *ds, int16_t x1, int16_t x2)
{
	draw_column_vars_t dcvars;

	// Calculate light table.
	// Use different light tables
	//   for horizontal / vertical / diagonal. Diagonal?

	curline = ds->curline;  // OPTIMIZE: get rid of LIGHTSEGSHIFT globally

	frontsector = &_g_sectors[curline->frontsectornum];
	backsector  = &_g_sectors[curline->backsectornum];

	int16_t texnum = texturetranslation[_g_sides[curline->sidenum].midtexture];

	// killough 4/13/98: get correct lightlevel for 2s normal textures
	rw_lightlevel = frontsector->lightlevel;

	maskedtexturecol = ds->maskedtexturecol;
	rw_scalestep     = ds->scalestep;
	spryscale        = ds->scale1 + (x1 - ds->x1) * rw_scalestep;
	mfloorclip       = ds->sprbottomclip;
	mceilingclip     = ds->sprtopclip;

	// find positioning
	if (_g_lines[curline->linenum].flags & ML_DONTPEGBOTTOM)
	{
		dcvars.texturemid = frontsector->floorheight > backsector->floorheight ? frontsector->floorheight : backsector->floorheight;
		dcvars.texturemid = dcvars.texturemid + ((int32_t)textureheight[texnum] << FRACBITS) - viewz;
	}
	else
	{
		dcvars.texturemid =frontsector->ceilingheight<backsector->ceilingheight ? frontsector->ceilingheight : backsector->ceilingheight;
		dcvars.texturemid = dcvars.texturemid - viewz;
	}

	dcvars.texturemid += (((int32_t)_g_sides[curline->sidenum].rowoffset) << FRACBITS);

	dcvars.colormap = R_LoadColorMap(rw_lightlevel);

	const texture_t __far* texture = R_GetTexture(texnum);

	const uint16_t widthmask = texture->widthmask;

	// draw the columns
	// simple texture == 1 patch
	const patch_t __far* patch = W_GetLumpByNum(texture->patches[0].patch_num);

	for (dcvars.x = x1 ; dcvars.x <= x2 ; dcvars.x++, spryscale += rw_scalestep)
	{
		int16_t xc = maskedtexturecol[dcvars.x];

		if (xc != SHRT_MAX) // dropoff overflow
		{
			xc &= widthmask;

			sprtopscreen = CENTERY * FRACUNIT - FixedMul(dcvars.texturemid, spryscale);

			dcvars.fracstep = FixedReciprocal((uint32_t)spryscale) >> COLEXTRABITS;

			// draw the texture
			const column_t __far* column = (const column_t __far*) ((const byte __far*)patch + (uint16_t)patch->columnofs[xc]);

			R_DrawMaskedColumn(R_DrawColumnWall, &dcvars, column);
			maskedtexturecol[dcvars.x] = SHRT_MAX; // dropoff overflow
		}
	}

	Z_ChangeTagToCache(patch);

	curline = NULL; /* cph 2001/11/18 - must clear curline now we're done with it, so R_LoadColorMap doesn't try using it for other things */
}


static PUREFUNC boolean R_PointOnSegSide(fixed_t x, fixed_t y, const seg_t __far* line)
{
    const int16_t lx = line->v1.x;
    const int16_t ly = line->v1.y;
    const int16_t ldx = line->v2.x - lx;
    const int16_t ldy = line->v2.y - ly;

    if (!ldx)
        return x <= (fixed_t)lx << FRACBITS ? ldy > 0 : ldy < 0;

    if (!ldy)
        return y <= (fixed_t)ly << FRACBITS ? ldx < 0 : ldx > 0;

    x -= (fixed_t)lx << FRACBITS;
    y -= (fixed_t)ly << FRACBITS;

    // Try to quickly decide by looking at sign bits.
    if ((ldy ^ ldx ^ (x >> FRACBITS) ^ (y >> FRACBITS)) < 0)
        return (ldy ^ (x >> FRACBITS)) < 0;          // (left is negative)

    return FixedMul3216(y, ldx) >= FixedMul3216(x, ldy);
}


//
// R_DrawSprite
//

static void R_DrawSprite (const vissprite_t* spr)
{
    int16_t* clipbot = floorclip;
    int16_t* cliptop = ceilingclip;

    fixed_t scale;
    fixed_t lowscale;

    for (int16_t x = spr->x1; x <= spr->x2; x++)
    {
        clipbot[x] = VIEWWINDOWHEIGHT;
        cliptop[x] = -1;
    }


    // Scan drawsegs from end to start for obscuring segs.
    // The first drawseg that has a greater scale is the clip seg.

    // Modified by Lee Killough:
    // (pointer check was originally nonportable
    // and buggy, by going past LEFT end of array):

    const drawseg_t* drawsegs  =_s_drawsegs;

    for (const drawseg_t* ds = ds_p; ds-- > drawsegs; )  // new -- killough
    {
        // determine if the drawseg obscures the sprite
        if (ds->x1 > spr->x2 || ds->x2 < spr->x1 || (!ds->silhouette && !ds->maskedtexturecol))
            continue;      // does not cover sprite

        const int16_t r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
        const int16_t r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

        if (ds->scale1 > ds->scale2)
        {
            lowscale = ds->scale2;
            scale    = ds->scale1;
        }
        else
        {
            lowscale = ds->scale1;
            scale    = ds->scale2;
        }

        if (scale < spr->scale || (lowscale < spr->scale && !R_PointOnSegSide (spr->gx, spr->gy, ds->curline)))
        {
            if (ds->maskedtexturecol)       // masked mid texture?
                R_RenderMaskedSegRange(ds, r1, r2);

            continue;               // seg is behind sprite
        }

        // clip this piece of the sprite
        // killough 3/27/98: optimized and made much shorter

        fixed_t gzt = spr->gz + (((int32_t)spr->patch_topoffset) << FRACBITS);

        if ((ds->silhouette & SIL_BOTTOM && spr->gz < ds->bsilheight)  // bottom sil
         && (ds->silhouette & SIL_TOP    && gzt     > ds->tsilheight)) // top sil
        {
            for (int16_t x = r1; x <= r2; x++)
            {
                if (clipbot[x] == VIEWWINDOWHEIGHT)
                    clipbot[x] = ds->sprbottomclip[x];

                if (cliptop[x] == -1)
                    cliptop[x] = ds->sprtopclip[x];
            }
        }
        else if (ds->silhouette & SIL_BOTTOM && spr->gz < ds->bsilheight) // bottom sil
        {
            for (int16_t x = r1; x <= r2; x++)
            {
                if (clipbot[x] == VIEWWINDOWHEIGHT)
                    clipbot[x] = ds->sprbottomclip[x];
            }
        }
        else if (ds->silhouette & SIL_TOP && gzt > ds->tsilheight) // top sil
        {
            for (int16_t x = r1; x <= r2; x++)
            {
                if (cliptop[x] == -1)
                    cliptop[x] = ds->sprtopclip[x];
            }
        }
    }

    // all clipping has been performed, so draw the sprite
    mfloorclip   = clipbot;
    mceilingclip = cliptop;
    R_DrawVisSprite (spr);
}


/*
 * Frame flags:
 * handles maximum brightness (torches, muzzle flare, light sources)
 */

#define FF_FULLBRIGHT   0x8000  /* flag in thing->frame */
#define FF_FRAMEMASK    0x7fff


//
// R_DrawPSprite
//

#define BASEXCENTER (SCREENWIDTH_VGA  / 2)
#define BASEYCENTER (SCREENHEIGHT_VGA / 2L)

static void R_DrawPSprite (pspdef_t *psp, int16_t lightlevel)
{
    int16_t           x1, x2;
    uint32_t hl;
    spritedef_t   __far* sprdef;
    spriteframe_t __far* sprframe;
    vissprite_t   *vis;
    vissprite_t   avis;
    fixed_t       topoffset;

    // decide which patch to use
    sprdef = &sprites[psp->state->sprite];

    sprframe = &sprdef->spriteframes[psp->state->frame & FF_FRAMEMASK];

    const patch_t __far* patch = W_GetLumpByNum(sprframe->lump[0]);
    // calculate edges of the shape
    int16_t tx = psp->sx - BASEXCENTER;

    tx -= patch->leftoffset;
    hl = (uint32_t) tx * PSPRITESCALE;
    x1 = CENTERX + (hl >> FRACBITS);

    tx += patch->width;
    hl = (uint32_t) tx * PSPRITESCALE;
    x2 = CENTERX + (hl >> FRACBITS) - 1;

    // off the side
    if (x2 < 0 || x1 > VIEWWINDOWWIDTH)
    {
        Z_ChangeTagToCache(patch);
        return;
    }

    topoffset = ((int32_t)patch->topoffset) << FRACBITS;
    Z_ChangeTagToCache(patch);

    // store information in a vissprite
    vis = &avis;
    // killough 12/98: fix psprite positioning problem
    vis->texturemid = (BASEYCENTER<<FRACBITS) /* +  FRACUNIT/2 */ -
            (psp->sy-topoffset);
    vis->x1 = x1 < 0 ? 0 : x1;
    vis->x2 = x2 >= VIEWWINDOWWIDTH ? VIEWWINDOWWIDTH - 1 : x2;
    // proff 11/06/98: Added for high-res
    vis->scale = PSPRITEYSCALE;
    vis->fracstep = PSPRITEYFRACSTEP;

    vis->xiscale = PSPRITEISCALE;
    vis->startfrac = 0;

    if (vis->x1 > x1)
        vis->startfrac = vis->xiscale*(vis->x1-x1);

    vis->lump_num = sprframe->lump[0];

    if (_g_player.powers[pw_invisibility] > 4*32 || _g_player.powers[pw_invisibility] & 8)
        vis->colormap = NULL;                    // shadow draw
    else if (fixedcolormap)
        vis->colormap = fixedcolormap;           // fixed color
    else if (psp->state->frame & FF_FULLBRIGHT)
        vis->colormap = fullcolormap;            // full bright // killough 3/20/98
    else
        vis->colormap = R_LoadColorMap(lightlevel);  // local light

    R_DrawVisSprite(vis);
}



//
// R_DrawPlayerSprites
//

static void R_DrawPlayerSprites(void)
{

  int16_t i, lightlevel = _g_player.mo->subsector->sector->lightlevel;
  pspdef_t *psp;

  // clip to screen bounds
  mfloorclip   = screenheightarray;
  mceilingclip = negonearray;

  // add all active psprites
  for (i=0, psp=_g_player.psprites; i<NUMPSPRITES; i++,psp++)
    if (psp->state)
      R_DrawPSprite (psp, lightlevel);
}


//
// R_SortVisSprites
//

// insertion sort
static void isort(vissprite_t **s, int16_t n)
{
	for (int16_t i = 1; i < n; i++)
	{
		vissprite_t *temp = s[i];
		if (s[i - 1]->scale < temp->scale)
		{
			int16_t j = i;
			while ((s[j] = s[j - 1])->scale < temp->scale && --j)
				;
			s[j] = temp;
		}
	}
}

#define MAXVISSPRITES 80
static int16_t num_vissprite;
static vissprite_t vissprites[MAXVISSPRITES];
static vissprite_t* vissprite_ptrs[MAXVISSPRITES];

static void R_SortVisSprites (void)
{
    int16_t i = num_vissprite;

    if (i)
    {
        while (--i >= 0)
            vissprite_ptrs[i] = vissprites + i;

        isort(vissprite_ptrs, num_vissprite);
    }
}

//
// R_DrawMasked
//

static void R_DrawMasked(void)
{
    drawseg_t *ds;
    drawseg_t* drawsegs = _s_drawsegs;


    R_SortVisSprites();

    // draw all vissprites back to front
    for (int16_t i = num_vissprite; --i >= 0; )
        R_DrawSprite(vissprite_ptrs[i]);

    // render any remaining masked mid textures

    for (ds=ds_p ; ds-- > drawsegs ; )
        if (ds->maskedtexturecol)
            R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

    R_DrawPlayerSprites ();
}


//
// R_NewVisSprite
//
static vissprite_t *R_NewVisSprite(void)
{
    if (num_vissprite >= MAXVISSPRITES)
    {
#ifdef RANGECHECK
        I_Error("Vissprite overflow.");
#endif
        return NULL;
    }

    return vissprites + num_vissprite++;
}


//
// R_ClearSprites
// Called at frame start.
//

static void R_ClearSprites(void)
{
    num_vissprite = 0;
}


//*******************************************

//
// R_ScaleFromGlobalAngle
// Returns the texture mapping scale
//  for the current line (horizontal span)
//  at the given angle.
// rw_distance must be calculated first.
//

static fixed_t R_ScaleFromGlobalAngle(int16_t x)
{
  int16_t anglea = ANG90_16 + xtoviewangleTable[x];
  int16_t angleb = anglea + viewangle16 - rw_normalangle;

  fixed_t den = rw_distance * finesineapprox(anglea >> ANGLETOFINESHIFT_16);

// proff 11/06/98: Changed for high-res
  fixed_t num = VIEWWINDOWHEIGHT * finesineapprox(angleb >> ANGLETOFINESHIFT_16);

  return den > num>>16 ? (num = FixedApproxDiv(num, den)) > 64*FRACUNIT ?
    64*FRACUNIT : num < 256 ? 256 : num : 64*FRACUNIT;
}


//
// R_ProjectSprite
// Generates a vissprite for a thing if it might be visible.
//

#define MINZ        (FRACUNIT*4)
#define MAXZ        (FRACUNIT*1280)

#define SPR_FLIPPED(s, r) (s->flipmask & (1 << r))

static void R_ProjectSprite (mobj_t __far* thing, int16_t lightlevel)
{
    const fixed_t fx = thing->x;
    const fixed_t fy = thing->y;
    const fixed_t fz = thing->z;

    const fixed_t tr_x = fx - viewx;
    const fixed_t tr_y = fy - viewy;

    fixed_t xc = FixedMulAngle(tr_x, viewcos);
    fixed_t ys = FixedMulAngle(tr_y, viewsin);
    const fixed_t tz = xc - (-ys);

    // thing is behind view plane?
    if (tz < MINZ)
        return;

    //Too far away.
    if(tz > MAXZ)
        return;

    fixed_t yc = FixedMulAngle(tr_y, viewcos);
    fixed_t xs = FixedMulAngle(tr_x, viewsin);
    fixed_t tx = -(yc + (-xs));

    // too far off the side?
    if (D_abs(tx)>(tz<<2))
        return;

    // decide which patch to use for sprite relative to player
    const spritedef_t __far*   sprdef   = &sprites[thing->sprite];
    const spriteframe_t __far* sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

    uint16_t rot = 0;

    if (sprframe->rotate)
    {
        // choose a different rotation based on player view
        angle16_t ang = R_PointToAngle(fx, fy);
        rot = (angle16_t)(ang - (angle16_t)(thing->angle >> FRACBITS) + (angle16_t)(ANG45_16 / 2) * 9) >> 13;
    }

    const boolean flip = (boolean)SPR_FLIPPED(sprframe, rot);
    const patch_t __far* patch = W_GetLumpByNum(sprframe->lump[rot]);

    /* calculate edges of the shape
     * cph 2003/08/1 - fraggle points out that this offset must be flipped
     * if the sprite is flipped; e.g. FreeDoom imp is messed up by this. */
    if (flip)
        tx -= ((int32_t)(patch->width - patch->leftoffset)) << FRACBITS;
    else
        tx -= ((int32_t)patch->leftoffset) << FRACBITS;

    //const fixed_t xscale = FixedDiv(PROJECTION, tz);
    const fixed_t xscale = PROJECTION / (tz >> FRACBITS);

    fixed_t xl = CENTERX * FRACUNIT + FixedMul(tx,xscale);
    const int16_t x1 = (xl >> FRACBITS);

    // off the side?
    if (x1 > VIEWWINDOWWIDTH)
    {
        Z_ChangeTagToCache(patch);
        return;
    }

    fixed_t xr = CENTERX * FRACUNIT - FRACUNIT + FixedMul(tx + (((int32_t)patch->width) << FRACBITS), xscale);
    const int16_t x2 = (xr >> FRACBITS);

    // off the side?
    if (xr < 0)
    {
        Z_ChangeTagToCache(patch);
        return;
    }

    //Too small.
    if (xr <= (xl + (FRACUNIT >> 2)))
    {
        Z_ChangeTagToCache(patch);
        return;
    }


    // store information in a vissprite
    vissprite_t* vis = R_NewVisSprite ();

    //No more vissprites.
    if(!vis)
    {
        Z_ChangeTagToCache(patch);
        return;
    }

    //vis->scale           = FixedDiv(PROJECTIONY, tz);
    vis->scale           = (VIEWWINDOWHEIGHT * FRACUNIT) / (tz >> FRACBITS);
    vis->fracstep        = tz / (VIEWWINDOWHEIGHT << COLEXTRABITS);
    vis->lump_num        = sprframe->lump[rot];
    vis->patch_topoffset = patch->topoffset;
    vis->gx              = fx;
    vis->gy              = fy;
    vis->gz              = fz;
    vis->texturemid      = (fz + (((int32_t)patch->topoffset) << FRACBITS)) - viewz;
    vis->x1              = x1 < 0 ? 0 : x1;
    vis->x2              = x2 >= VIEWWINDOWWIDTH ? VIEWWINDOWWIDTH - 1 : x2;


    const fixed_t iscale = FixedReciprocal(xscale);

    if (flip)
    {
        vis->startfrac = (((int32_t)patch->width)<<FRACBITS)-1;
        vis->xiscale = -iscale;
    }
    else
    {
        vis->startfrac = 0;
        vis->xiscale = iscale;
    }

    Z_ChangeTagToCache(patch);

    if (vis->x1 > x1)
        vis->startfrac += vis->xiscale*(vis->x1-x1);

    // get light level
    if (thing->flags & MF_SHADOW)
        vis->colormap = NULL;             // shadow draw
    else if (fixedcolormap)
        vis->colormap = fixedcolormap;      // fixed map
    else if (thing->frame & FF_FULLBRIGHT)
        vis->colormap = fullcolormap;     // full bright  // killough 3/20/98
    else
        vis->colormap = R_LoadColorMap(lightlevel); // diminished light
}

//
// R_AddSprites
// During BSP traversal, this adds sprites by sector.
//
// killough 9/18/98: add lightlevel as parameter, fixing underwater lighting
static void R_AddSprites(subsector_t __far* subsec, int16_t lightlevel)
{
  sector_t __far* sec=subsec->sector;
  mobj_t __far* thing;

  // BSP is traversed by subsector.
  // A sector might have been split into several
  //  subsectors during BSP building.
  // Thus we check whether its already added.

  if (sec->validcount == validcount)
    return;

  // Well, now it will be done.
  sec->validcount = validcount;

  // Handle all things in sector.

  for (thing = sec->thinglist; thing; thing = thing->snext)
    R_ProjectSprite(thing, lightlevel);
}


#if defined FLAT_WALL
#define R_DrawSegTextureColumn(w,x,y,z) R_DrawColumnFlat(x,z)
#else
static void R_DrawColumnInCache(const column_t __far* patch, byte* cache, int16_t originy, int16_t cacheheight)
{
    while (patch->topdelta != 0xff)
    {
        const byte __far* source = (const byte __far*)patch + 3;
        int16_t count = patch->length;
        int16_t position = originy + patch->topdelta;

        if (position < 0)
        {
            count += position;
            position = 0;
        }

        if (position + count > cacheheight)
            count = cacheheight - position;

        if (count > 0)
            _fmemcpy(cache + position, source, count);

        patch = (const column_t __far*)((const byte __far*)patch + patch->length + 4);
    }
}

/*
 * Draw a column of pixels of the specified texture.
 * If the texture is simple (1 patch, full height) then just draw
 * straight from const patch_t*.
*/

#define MAX_CACHE_ENTRIES 128
#define MAX_CACHE_TRIES 4

static uint16_t CACHE_ENTRY(int16_t column, int16_t texture)
{
	return column | (texture << 8);
}

static byte __far columnCache[MAX_CACHE_ENTRIES*128];
static uint16_t columnCacheEntries[MAX_CACHE_ENTRIES];

static uint16_t FindColumnCacheItem(int16_t texture, int16_t column)
{
	uint16_t hash = ((column >> 2) ^ (texture * 71)) & (MAX_CACHE_ENTRIES - 1);
	uint16_t key = hash;

	uint16_t cx = CACHE_ENTRY(column, texture);

	for (int16_t i = 0; i < MAX_CACHE_TRIES; i++)
	{
		if (columnCacheEntries[key] == 0 || columnCacheEntries[key] == cx)
			return key;

		key += 119;
		key &= (MAX_CACHE_ENTRIES - 1);
	}

	return hash;
}


static const byte __far* R_ComposeColumn(const int16_t texture, const texture_t __far* tex, int16_t texcolumn)
{
#if defined HIGH_DETAIL
    const int16_t xc = texcolumn & tex->widthmask;
#else
    const int16_t xc = (texcolumn & 0xfffc) & tex->widthmask;
#endif

    uint16_t cachekey = FindColumnCacheItem(texture, xc);

    byte __far* colcache = &columnCache[cachekey*128];
    uint16_t cacheEntry = columnCacheEntries[cachekey];

    //total++;

    if (cacheEntry != CACHE_ENTRY(xc, texture))
    {
        //misses++;
        static byte tmpCache[128];

        uint8_t i = 0;
        uint8_t patchcount = tex->patchcount;

        do
        {
            const texpatch_t __far* patch = &tex->patches[i];

            const int16_t x1 = patch->originx;

            if (xc < x1)
                continue;

            const patch_t __far* realpatch = W_TryGetLumpByNum(patch->patch_num);
            if (realpatch == NULL)
                return NULL;

            const int16_t x2 = x1 + realpatch->width;

            if (xc < x2)
            {
                const column_t __far* patchcol = (const column_t __far*)((const byte __far*)realpatch + (uint16_t)realpatch->columnofs[xc - x1]);

                R_DrawColumnInCache (patchcol, tmpCache, patch->originy, tex->height);
            }
            Z_ChangeTagToCache(realpatch);
        } while(++i < patchcount);

        //Block copy will drop low 2 bits of len.
        _fmemcpy(colcache, tmpCache, (tex->height + 3) & ~3);

        columnCacheEntries[cachekey] = CACHE_ENTRY(xc, texture);
    }

    return colcache;
}

static void R_DrawSegTextureColumn(const texture_t __far* tex, int16_t texture, int16_t texcolumn, draw_column_vars_t* dcvars)
{
    if (!tex->overlapped)
    {
        int16_t patch_num;
        int16_t x_c;
        R_GetColumn(tex, texcolumn, &patch_num, &x_c);

        const patch_t __far* patch = W_TryGetLumpByNum(patch_num);
        if (patch == NULL)
            R_DrawColumnFlat(texture, dcvars);
        else
        {
            const column_t __far* column = (const column_t __far*) ((const byte __far*)patch + (uint16_t)patch->columnofs[x_c]);

            dcvars->source = (const byte __far*)column + 3;
            R_DrawColumnWall(dcvars);
            Z_ChangeTagToCache(patch);
        }
    }
    else
    {
        const byte __far* source = R_ComposeColumn(texture, tex, texcolumn);
        if (source == NULL)
            R_DrawColumnFlat(texture, dcvars);
        else
        {
            dcvars->source = source;
            R_DrawColumnWall(dcvars);
        }
    }
}
#endif

//
// R_RenderSegLoop
// Draws zero, one, or two textures (and possibly a masked texture) for walls.
// Can draw or mark the starting pixel of floor and ceiling textures.
// boolean segtextured is true if any of the segs textures might be visible.
// boolean markfloor is false if the back side is the same plane.
// CALLED: CORE LOOPING ROUTINE.
//

static void R_RenderSegLoop(int16_t rw_x, boolean segtextured, boolean markfloor, boolean markceiling)
{
    draw_column_vars_t dcvars;
    int16_t  texturecolumn = 0;   // shut up compiler warning

    dcvars.colormap = R_LoadColorMap(rw_lightlevel);

    for ( ; rw_x < rw_stopx ; rw_x++)
    {
        // mark floor / ceiling areas

        int16_t yh = bottomfrac>>FRACBITS;
        int16_t yl = (topfrac+FRACUNIT-1)>>FRACBITS;

        int16_t cc_rwx = ceilingclip[rw_x];
        int16_t fc_rwx = floorclip[rw_x];

        // no space above wall?
        int16_t bottom,top = cc_rwx+1;

        dcvars.x  = rw_x;

        if (yl < top)
            yl = top;

        if (markceiling)
        {
            bottom = yl-1;

            if (bottom >= fc_rwx)
                bottom = fc_rwx-1;

            if (top <= bottom)
            {
#if defined FLAT_SPAN
                dcvars.yl = top;
                dcvars.yh = bottom;
                if (ceilingplane_color == -2)
                    R_DrawSky(&dcvars);
                else
                    R_DrawColumnFlat(ceilingplane_color, &dcvars);
#else
                ceilingplane->top[rw_x] = top;
                ceilingplane->bottom[rw_x] = bottom;
                ceilingplane->modified = true;
#endif
            }
            // SoM: this should be set here
            cc_rwx = bottom;
        }

        bottom = fc_rwx-1;
        if (yh > bottom)
            yh = bottom;

        if (markfloor)
        {

            top  = yh < cc_rwx ? cc_rwx : yh;

            if (++top <= bottom)
            {
#if defined FLAT_SPAN
                dcvars.yl = top;
                dcvars.yh = bottom;
                R_DrawColumnFlat(floorplane_color, &dcvars);
#else
                floorplane->top[rw_x] = top;
                floorplane->bottom[rw_x] = bottom;
                floorplane->modified = true;
#endif
            }
            // SoM: This should be set here to prevent overdraw
            fc_rwx = top;
        }

        // texturecolumn and lighting are independent of wall tiers
        if (segtextured)
        {
            // calculate texture offset
#if !defined FLAT_WALL
			texturecolumn = rw_offset;
			int16_t ang = (angle16_t)(rw_centerangle + xtoviewangleTable[rw_x]) >> ANGLETOFINESHIFT_16;
			if (ang < 1024) {			//    0 <= ang < 1024
				fixed_t tan = finetangentTable_part_4[1023 - ang];
				texturecolumn += (rw_distance * tan) >> FRACBITS;
			} else if (ang < 2048) {	// 1024 <= ang < 2048
				fixed_t tan = finetangentTable_part_3[1023 - (ang - 1024)];
				texturecolumn += (rw_distance * tan) >> FRACBITS;
			} else if (ang < 3072) {	// 2048 <= ang < 3072
				fixed_t tan = finetangentTable_part_3[ang - 2048];
				texturecolumn -= (rw_distance * tan) >> FRACBITS;
			} else {					// 3072 <= ang < 4096
				fixed_t tan = finetangentTable_part_4[ang - 3072];
				texturecolumn -= (rw_distance * tan) >> FRACBITS;
			}
#endif

            dcvars.fracstep = FixedReciprocal((uint32_t)rw_scale) >> COLEXTRABITS;
        }

        // draw the wall tiers
        if (midtexture)
        {

            dcvars.yl = yl;     // single sided line
            dcvars.yh = yh;
            dcvars.texturemid = rw_midtexturemid;
            //

            R_DrawSegTextureColumn(texmidtexture, midtexture, texturecolumn, &dcvars);

            cc_rwx = VIEWWINDOWHEIGHT;
            fc_rwx = -1;
        }
        else
        {

            // two sided line
            if (toptexture)
            {
                // top wall
                int16_t mid = pixhigh>>FRACBITS;
                pixhigh += pixhighstep;

                if (mid >= fc_rwx)
                    mid = fc_rwx-1;

                if (mid >= yl)
                {
                    dcvars.yl = yl;
                    dcvars.yh = mid;
                    dcvars.texturemid = rw_toptexturemid;

                    R_DrawSegTextureColumn(textoptexture, toptexture, texturecolumn, &dcvars);

                    cc_rwx = mid;
                }
                else
                    cc_rwx = yl-1;
            }
            else  // no top wall
            {

                if (markceiling)
                    cc_rwx = yl-1;
            }

            if (bottomtexture)          // bottom wall
            {
                int16_t mid = (pixlow+FRACUNIT-1)>>FRACBITS;
                pixlow += pixlowstep;

                // no space above wall?
                if (mid <= cc_rwx)
                    mid = cc_rwx+1;

                if (mid <= yh)
                {
                    dcvars.yl = mid;
                    dcvars.yh = yh;
                    dcvars.texturemid = rw_bottomtexturemid;

                    R_DrawSegTextureColumn(texbottomtexture, bottomtexture, texturecolumn, &dcvars);

                    fc_rwx = mid;
                }
                else
                    fc_rwx = yh+1;
            }
            else        // no bottom wall
            {
                if (markfloor)
                    fc_rwx = yh+1;
            }

            // cph - if we completely blocked further sight through this column,
            // add this info to the solid columns array for r_bsp.c
            if ((markceiling || markfloor) && (fc_rwx <= cc_rwx + 1))
            {
                solidcol[rw_x] = 1;
                didsolidcol = true;
            }

            // save texturecol for backdrawing of masked mid texture
            if (maskedtexture)
                maskedtexturecol[rw_x] = texturecolumn;
        }

        rw_scale += rw_scalestep;
        topfrac += topstep;
        bottomfrac += bottomstep;

        floorclip[rw_x] = fc_rwx;
        ceilingclip[rw_x] = cc_rwx;
    }
}

static boolean R_CheckOpenings(const int16_t start)
{
    int16_t pos = lastopening - openings;
    int16_t need = (rw_stopx - start)*sizeof(int16_t) + pos;

#ifdef RANGECHECK
    if(need > MAXOPENINGS)
        I_Error("Openings overflow. Need = %ld", need);
#endif

    return need <= MAXOPENINGS;
}


static void R_ClearOpeningClippingDetermination(void)
{
	// opening / clipping determination
	for (uint8_t i = 0; i < VIEWWINDOWWIDTH; i++)
		floorclip[i] = VIEWWINDOWHEIGHT, ceilingclip[i] = -1;
}


static void R_ClearOpenings(void)
{
	lastopening = openings;
}


/* CPhipps -
 * Mod - returns a % b, guaranteeing 0 <= a < b
 * (notice that the C standard for % does not guarantee this)
 */

inline static int16_t CONSTFUNC Mod(int16_t a, int16_t b)
{
    if(!a)
        return 0;

    if (b & (b-1))
    {
        int16_t r = a % b;
        return ((r<0) ? r+b : r);
    }
    else
        return (a & (b-1));
}


//
// R_StoreWallRange
// A wall segment will be drawn
//  between start and stop pixels (inclusive).
//
static void R_StoreWallRange(const int16_t start, const int16_t stop)
{
    // don't overflow and crash
    if (ds_p == &_s_drawsegs[MAXDRAWSEGS])
    {
#ifdef RANGECHECK
        I_Error("Drawsegs overflow.");
#endif
        return;
    }


    sidedef = &_g_sides[curline->sidenum];
    linedef = &_g_lines[curline->linenum];

    // mark the segment as visible for auto map
    linedef->r_flags |= ML_MAPPED;

    // calculate rw_distance for scale calculation
    rw_normalangle = curline->angle;

    angle16_t offsetangle = rw_normalangle - rw_angle1;

#if defined _M_I86
    if (abs(offsetangle) > ANG90_16)
        offsetangle = ANG90_16;
#else
    if (D_abs((angle_t)offsetangle << FRACBITS) > ANG90)
        offsetangle = ANG90_16;
#endif

    int16_t hyp = R_PointToDist(curline->v1.x, curline->v1.y);

    rw_distance = (hyp * finecosineapprox(offsetangle >> ANGLETOFINESHIFT_16)) >> FRACBITS;

    int16_t rw_x = ds_p->x1 = start;
    ds_p->x2 = stop;
    ds_p->curline = curline;
    rw_stopx = stop+1;

    //Openings overflow. Nevermind.
    if(!R_CheckOpenings(start))
        return;

    // calculate scale at both ends and step
    ds_p->scale1 = rw_scale = R_ScaleFromGlobalAngle(start);

    if (stop > start)
    {
        ds_p->scale2 = R_ScaleFromGlobalAngle(stop);
        ds_p->scalestep = rw_scalestep = (ds_p->scale2 - rw_scale) / (stop - start);
    }
    else
        ds_p->scale2 = ds_p->scale1;

    // calculate texture boundaries
    //  and decide if floor / ceiling marks are needed

    worldtop = frontsector->ceilingheight - viewz;
    worldbottom = frontsector->floorheight - viewz;

    midtexture = toptexture = bottomtexture = maskedtexture = 0;
    ds_p->maskedtexturecol = NULL;

    boolean markfloor, markceiling;

    if (!backsector)
    {
        // single sided line
        midtexture = texturetranslation[sidedef->midtexture];
        texmidtexture = R_GetTexture(midtexture);

        // a single sided line is terminal, so it must mark ends
        markfloor = markceiling = true;

        if (linedef->flags & ML_DONTPEGBOTTOM)
        {         // bottom of texture at bottom
            fixed_t vtop = frontsector->floorheight + ((int32_t)textureheight[sidedef->midtexture] << FRACBITS);
            rw_midtexturemid = vtop - viewz;
        }
        else        // top of texture at top
            rw_midtexturemid = worldtop;

        rw_midtexturemid += ((int32_t)Mod(sidedef->rowoffset, textureheight[midtexture])) << FRACBITS;

        ds_p->silhouette = SIL_BOTH;
        ds_p->sprtopclip = screenheightarray;
        ds_p->sprbottomclip = negonearray;
        ds_p->bsilheight = INT32_MAX;
        ds_p->tsilheight = INT32_MIN;
    }
    else      // two sided line
    {
        ds_p->sprtopclip = ds_p->sprbottomclip = NULL;
        ds_p->silhouette = SIL_NONE;

        if(linedef->r_flags & RF_CLOSED)
        { /* cph - closed 2S line e.g. door */
            // cph - killough's (outdated) comment follows - this deals with both
            // "automap fixes", his and mine
            // killough 1/17/98: this test is required if the fix
            // for the automap bug (r_bsp.c) is used, or else some
            // sprites will be displayed behind closed doors. That
            // fix prevents lines behind closed doors with dropoffs
            // from being displayed on the automap.

            ds_p->silhouette = SIL_BOTH;
            ds_p->sprtopclip = screenheightarray;
            ds_p->sprbottomclip = negonearray;
            ds_p->bsilheight = INT32_MAX;
            ds_p->tsilheight = INT32_MIN;

        }
        else
        { /* not solid - old code */

            if (frontsector->floorheight > backsector->floorheight)
            {
                ds_p->silhouette = SIL_BOTTOM;
                ds_p->bsilheight = frontsector->floorheight;
            }
            else
                if (backsector->floorheight > viewz)
                {
                    ds_p->silhouette = SIL_BOTTOM;
                    ds_p->bsilheight = INT32_MAX;
                }

            if (frontsector->ceilingheight < backsector->ceilingheight)
            {
                ds_p->silhouette |= SIL_TOP;
                ds_p->tsilheight = frontsector->ceilingheight;
            }
            else
                if (backsector->ceilingheight < viewz)
                {
                    ds_p->silhouette |= SIL_TOP;
                    ds_p->tsilheight = INT32_MIN;
                }
        }

        worldhigh = backsector->ceilingheight - viewz;
        worldlow = backsector->floorheight - viewz;

        // hack to allow height changes in outdoor areas
        if (frontsector->ceilingpic == skyflatnum && backsector->ceilingpic == skyflatnum)
            worldtop = worldhigh;

        markfloor = worldlow != worldbottom
                || backsector->floorpic != frontsector->floorpic
                || backsector->lightlevel != frontsector->lightlevel
                ;

        markceiling = worldhigh != worldtop
                || backsector->ceilingpic != frontsector->ceilingpic
                || backsector->lightlevel != frontsector->lightlevel
                ;

        if (backsector->ceilingheight <= frontsector->floorheight || backsector->floorheight >= frontsector->ceilingheight)
            markceiling = markfloor = true;   // closed door

        if (worldhigh < worldtop)   // top texture
        {
            toptexture = texturetranslation[sidedef->toptexture];
            textoptexture = R_GetTexture(toptexture);
            rw_toptexturemid = linedef->flags & ML_DONTPEGTOP ? worldtop :
                                                                        backsector->ceilingheight + ((int32_t)textureheight[sidedef->toptexture] << FRACBITS) - viewz;
            rw_toptexturemid += ((int32_t)Mod(sidedef->rowoffset, textureheight[toptexture])) << FRACBITS;
        }

        if (worldlow > worldbottom) // bottom texture
        {
            bottomtexture = texturetranslation[sidedef->bottomtexture];
            texbottomtexture = R_GetTexture(bottomtexture);
            rw_bottomtexturemid = linedef->flags & ML_DONTPEGBOTTOM ? worldtop : worldlow;

            rw_bottomtexturemid += ((int32_t)Mod(sidedef->rowoffset, textureheight[bottomtexture])) << FRACBITS;
        }

        // allocate space for masked texture tables
        if (sidedef->midtexture)    // masked midtexture
        {
            maskedtexture = true;
            ds_p->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
            lastopening += rw_stopx - rw_x;
        }
    }

    // calculate rw_offset (only needed for textured lines)
    boolean segtextured = ((midtexture | toptexture | bottomtexture | maskedtexture) > 0);

    if (segtextured)
    {
        fixed_t rw_offset32 = hyp * -finesineapprox(offsetangle >> ANGLETOFINESHIFT_16);
        rw_offset = rw_offset32 >> FRACBITS;
        rw_offset += sidedef->textureoffset + curline->offset;

        rw_centerangle = ANG90_16 + viewangle16 - rw_normalangle;

        rw_lightlevel = frontsector->lightlevel;
    }

    // if a floor / ceiling plane is on the wrong side of the view
    // plane, it is definitely invisible and doesn't need to be marked.
    if (frontsector->floorheight >= viewz)       // above view plane
        markfloor = false;
    if (frontsector->ceilingheight <= viewz &&
            frontsector->ceilingpic != skyflatnum)   // below view plane
        markceiling = false;

    // calculate incremental stepping values for texture edges
    topstep = -FixedMul(worldtop, rw_scalestep);
    topfrac = (CENTERY * FRACUNIT) - FixedMul(worldtop, rw_scale);

    bottomstep = -FixedMul(worldbottom, rw_scalestep);
    bottomfrac = (CENTERY * FRACUNIT) - FixedMul(worldbottom, rw_scale);

    if (backsector)
    {
        if (worldhigh < worldtop)
        {
            pixhigh = (CENTERY * FRACUNIT) - FixedMul(worldhigh, rw_scale);
            pixhighstep = -FixedMul(worldhigh, rw_scalestep);
        }
        if (worldlow > worldbottom)
        {
            pixlow = (CENTERY * FRACUNIT) - FixedMul(worldlow, rw_scale);
            pixlowstep = -FixedMul(worldlow, rw_scalestep);
        }
    }

    // render it
    if (markceiling)
    {
#if defined FLAT_SPAN
        if (ceilingplane_color == -1)
            markceiling = false;
#else
        if (ceilingplane)   // killough 4/11/98: add NULL ptr checks
            ceilingplane = R_CheckPlane (ceilingplane, rw_x, rw_stopx-1);
        else
            markceiling = false;
#endif
    }

    if (markfloor)
    {
#if defined FLAT_SPAN
        if (floorplane_color == -1)
            markfloor = false;
#else
        if (floorplane)     // killough 4/11/98: add NULL ptr checks
            /* cph 2003/04/18  - ceilingplane and floorplane might be the same
       * visplane (e.g. if both skies); R_CheckPlane doesn't know about
       * modifications to the plane that might happen in parallel with the check
       * being made, so we have to override it and split them anyway if that is
       * a possibility, otherwise the floor marking would overwrite the ceiling
       * marking, resulting in HOM. */
            if (markceiling && ceilingplane == floorplane)
                floorplane = R_DupPlane (floorplane, rw_x, rw_stopx-1);
            else
                floorplane = R_CheckPlane (floorplane, rw_x, rw_stopx-1);
        else
            markfloor = false;
#endif
    }

    didsolidcol = false;
    R_RenderSegLoop(rw_x, segtextured, markfloor, markceiling);

    /* cph - if a column was made solid by this wall, we _must_ save full clipping info */
    if (backsector && didsolidcol)
    {
        if (!(ds_p->silhouette & SIL_BOTTOM))
        {
            ds_p->silhouette |= SIL_BOTTOM;
            ds_p->bsilheight = backsector->floorheight;
        }
        if (!(ds_p->silhouette & SIL_TOP))
        {
            ds_p->silhouette |= SIL_TOP;
            ds_p->tsilheight = backsector->ceilingheight;
        }
    }

    // save sprite clipping info
    if ((ds_p->silhouette & SIL_TOP || maskedtexture) && !ds_p->sprtopclip)
    {
        memcpy((byte*)lastopening, (const byte*)(ceilingclip+start), sizeof(int16_t)*(rw_stopx-start));
        ds_p->sprtopclip = lastopening - start;
        lastopening += rw_stopx - start;
    }

    if ((ds_p->silhouette & SIL_BOTTOM || maskedtexture) && !ds_p->sprbottomclip)
    {
        memcpy((byte*)lastopening, (const byte*)(floorclip+start), sizeof(int16_t)*(rw_stopx-start));
        ds_p->sprbottomclip = lastopening - start;
        lastopening += rw_stopx - start;
    }

    if (maskedtexture && !(ds_p->silhouette & SIL_TOP))
    {
        ds_p->silhouette |= SIL_TOP;
        ds_p->tsilheight = INT32_MIN;
    }

    if (maskedtexture && !(ds_p->silhouette & SIL_BOTTOM))
    {
        ds_p->silhouette |= SIL_BOTTOM;
        ds_p->bsilheight = INT32_MAX;
    }

    ds_p++;
}


// killough 1/18/98 -- This function is used to fix the automap bug which
// showed lines behind closed doors simply because the door had a dropoff.
//
// cph - converted to R_RecalcLineFlags. This recalculates all the flags for
// a line, including closure and texture tiling.

static void R_RecalcLineFlags(void)
{
    const side_t __far* side = &_g_sides[curline->sidenum];

    linedef->r_validcount = _g_gametic;

    /* First decide if the line is closed, normal, or invisible */
    if (!(linedef->flags & ML_TWOSIDED)
            || backsector->ceilingheight <= frontsector->floorheight
            || backsector->floorheight >= frontsector->ceilingheight
            || (
                // if door is closed because back is shut:
                backsector->ceilingheight <= backsector->floorheight

                // preserve a kind of transparent door/lift special effect:
                && (backsector->ceilingheight >= frontsector->ceilingheight ||
                    side->toptexture)

                && (backsector->floorheight <= frontsector->floorheight ||
                    side->bottomtexture)

                // properly render skies (consider door "open" if both ceilings are sky):
                && (backsector->ceilingpic != skyflatnum ||
                    frontsector->ceilingpic!= skyflatnum)
                )
            )
        linedef->r_flags = (RF_CLOSED | (linedef->r_flags & ML_MAPPED));
    else
    {
        // Reject empty lines used for triggers
        //  and special events.
        // Identical floor and ceiling on both sides,
        // identical light levels on both sides,
        // and no middle texture.
        // CPhipps - recode for speed, not certain if this is portable though
        if (backsector->ceilingheight != frontsector->ceilingheight
                || backsector->floorheight != frontsector->floorheight
                || side->midtexture
                || backsector->ceilingpic != frontsector->ceilingpic
                || backsector->floorpic != frontsector->floorpic
                || backsector->lightlevel != frontsector->lightlevel)
        {
            linedef->r_flags = (linedef->r_flags & ML_MAPPED);
        } else
            linedef->r_flags = (RF_IGNORE | (linedef->r_flags & ML_MAPPED));
    }
}


// CPhipps -
// R_ClipWallSegment
//
// Replaces the old R_Clip*WallSegment functions. It draws bits of walls in those
// columns which aren't solid, and updates the solidcol[] array appropriately

static void R_ClipWallSegment(int16_t first, int16_t last, const boolean solid)
{
    byte *p;
    while (first < last)
    {
        if (solidcol[first])
        {
            if (!(p = memchr(solidcol+first, 0, last-first)))
                return; // All solid

            first = p - solidcol;
        }
        else
        {
            int16_t to;
            if (!(p = memchr(solidcol+first, 1, last-first)))
                to = last;
            else
                to = p - solidcol;

            R_StoreWallRange(first, to-1);

            if (solid)
            {
                memset(solidcol + first, 1, to - first);
            }

            first = to;
        }
    }
}

//
// R_ClearClipSegs
//

//
// R_AddLine
// Clips the given segment
// and adds any visible pieces to the line list.
//

static void R_AddLine(const seg_t __far* line)
{
    curline = line;

    angle16_t angle1 = R_PointToAngle16(line->v1.x, line->v1.y);
    angle16_t angle2 = R_PointToAngle16(line->v2.x, line->v2.y);

    // Clip to view edges.
    angle16_t span = angle1 - angle2;

    // Back side, i.e. backface culling
    if (span >= ANG180_16)
        return;

    // Global angle needed by segcalc.
    rw_angle1 = angle1;
    angle1 -= viewangle16;
    angle2 -= viewangle16;

    angle16_t tspan = angle1 + clipangle;
    if (tspan > 2 * clipangle)
    {
        tspan -= 2 * clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return;

        angle1 = clipangle;
    }

    tspan = clipangle - angle2;
    if (tspan > 2 * clipangle)
    {
        tspan -= 2 * clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return;
        angle2 = -clipangle;
    }

    // The seg is in the view range,
    // but not necessarily visible.

    // killough 1/31/98: Here is where "slime trails" can SOMETIMES occur:
    uint8_t x1 = viewangletox((angle16_t)(angle1 + ANG90_16) >> ANGLETOFINESHIFT_16);
    uint8_t x2 = viewangletox((angle16_t)(angle2 + ANG90_16) >> ANGLETOFINESHIFT_16);

    // Does not cross a pixel?
    if (x1 >= x2)       // killough 1/31/98 -- change == to >= for robustness
        return;

    backsector = line->backsectornum != NO_INDEX8 ? &_g_sectors[line->backsectornum] : NULL;

    /* cph - roll up linedef properties in flags */
    linedef = &_g_lines[curline->linenum];

    if (linedef->r_validcount != (uint16_t)_g_gametic)
        R_RecalcLineFlags();

    if (!(linedef->r_flags & RF_IGNORE))
    {
        R_ClipWallSegment (x1, x2, linedef->r_flags & RF_CLOSED);
    }
}

//
// R_Subsector
// Determine floor/ceiling planes.
// Add sprites of things in sector.
// Draw one or more line segments.
//

static void R_Subsector(int16_t num)
{
    int16_t         count;
    const seg_t       __far* line;
    subsector_t __far* sub;

    sub = &_g_subsectors[num];
    frontsector = sub->sector;
    count = sub->numlines;
    line = &_g_segs[sub->firstline];

#if defined FLAT_SPAN
    if (frontsector->floorheight < viewz)
        floorplane_color = R_GetPlaneColor(frontsector->floorpic, frontsector->lightlevel);
    else
        floorplane_color = -1;
#else
    if(frontsector->floorheight < viewz)
    {
        floorplane = R_FindPlane(frontsector->floorheight,
                                     frontsector->floorpic,
                                     frontsector->lightlevel                // killough 3/16/98
                                     );
    }
    else
    {
        floorplane = NULL;
    }
#endif


#if defined FLAT_SPAN
    if (frontsector->ceilingpic == skyflatnum) {
        ceilingplane_color = -2;
        R_LoadSkyPatch();
    } else if (frontsector->ceilingheight > viewz)
        ceilingplane_color = R_GetPlaneColor(frontsector->ceilingpic, frontsector->lightlevel);
    else
        ceilingplane_color = -1;
#else
    if(frontsector->ceilingheight > viewz || (frontsector->ceilingpic == skyflatnum))
    {
        ceilingplane = R_FindPlane(frontsector->ceilingheight,     // killough 3/8/98
                                       frontsector->ceilingpic,
                                       frontsector->lightlevel
                                       );
    }
    else
    {
        ceilingplane = NULL;
    }
#endif

    R_AddSprites(sub, frontsector->lightlevel);
    while (count--)
    {
        R_AddLine (line);
        line++;
        curline = NULL; /* cph 2001/11/18 - must clear curline now we're done with it, so R_LoadColorMap doesn't try using it for other things */
    }
}

//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//

static const byte checkcoord[12][4] =
{
  {3,0,2,1},
  {3,0,2,0},
  {3,1,2,0},
  {0},
  {2,0,2,1},
  {0,0,0,0},
  {3,1,3,0},
  {0},
  {2,0,3,1},
  {2,1,3,1},
  {2,1,3,0}
};


static boolean R_CheckBBox(const int16_t __far* bspcoord)
{
    // Find the corners of the box
    // that define the edges from current viewpoint.
    int16_t boxpos = (viewx <= ((fixed_t)bspcoord[BOXLEFT]<<FRACBITS) ? 0 : viewx < ((fixed_t)bspcoord[BOXRIGHT]<<FRACBITS) ? 1 : 2) +
            (viewy >= ((fixed_t)bspcoord[BOXTOP]<<FRACBITS) ? 0 : viewy > ((fixed_t)bspcoord[BOXBOTTOM]<<FRACBITS) ? 4 : 8);

    if (boxpos == 5)
        return true;

    const byte* check = checkcoord[boxpos];
    angle16_t angle1 = R_PointToAngle16(bspcoord[check[0]], bspcoord[check[1]]) - viewangle16;
    angle16_t angle2 = R_PointToAngle16(bspcoord[check[2]], bspcoord[check[3]]) - viewangle16;


    // cph - replaced old code, which was unclear and badly commented
    // Much more efficient code now
    if ((int16_t)angle1 < (int16_t)angle2)
    { /* it's "behind" us */
        /* Either angle1 or angle2 is behind us, so it doesn't matter if we
     * change it to the corect sign
     */
        if (ANG180_16 <= angle1 && angle1 < ANG270_16)
            angle1 = INT16_MAX; /* which is ANG180_16 - 1 */
        else
            angle2 = INT16_MIN;
    }

    if ((int16_t)angle2 >=  (int16_t)clipangle) return false; // Both off left edge
    if ((int16_t)angle1 <= -(int16_t)clipangle) return false; // Both off right edge
    if ((int16_t)angle1 >=  (int16_t)clipangle) angle1 =  clipangle; // Clip at left edge
    if ((int16_t)angle2 <= -(int16_t)clipangle) angle2 = -clipangle; // Clip at right edge

    // Find the first clippost
    //  that touches the source post
    //  (adjacent pixels are touching).

    uint8_t sx1 = viewangletox((angle16_t)(angle1 + ANG90_16) >> ANGLETOFINESHIFT_16);
    uint8_t sx2 = viewangletox((angle16_t)(angle2 + ANG90_16) >> ANGLETOFINESHIFT_16);
    //    const cliprange_t *start;

    // Does not cross a pixel.
    if (sx1 == sx2)
        return false;

    if (!memchr(solidcol+sx1, 0, sx2-sx1)) return false;
    // All columns it covers are already solidly covered


    return true;
}

//Render a BSP subsector if bspnum is a leaf node.
//Return false if bspnum is frame node.





static boolean R_RenderBspSubsector(int16_t bspnum)
{
    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
        if (bspnum == -1)
            R_Subsector (0);
        else
            R_Subsector (bspnum & (~NF_SUBSECTOR));

        return true;
    }

    return false;
}

// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.

#if defined PROFILING
//Non recursive version.
//constant stack space used and easier to
//performance profile.
#define MAX_BSP_DEPTH 64

static void R_RenderBSPNode(int16_t bspnum)
{
    static int16_t stack_bsp[MAX_BSP_DEPTH];
    static int8_t  stack_side[MAX_BSP_DEPTH];
    int16_t sp = 0;

    const mapnode_t __far* bsp;
    int8_t side;

    while (true)
    {
        //Front sides.
        while (!R_RenderBspSubsector(bspnum))
        {
            if (sp == MAX_BSP_DEPTH)
                break;

            bsp = &nodes[bspnum];
            side = R_PointOnSide(viewx, viewy, bsp);

            stack_bsp[sp]  = bspnum;
            stack_side[sp] = side ^ 1;
            sp++;

            bspnum = bsp->children[side];
        }

        if (sp == 0)
        {
            //back at root node and not visible. All done!
            return;
        }

        //Back sides.
        --sp;
        side   = stack_side[sp];
        bspnum = stack_bsp[sp];
        bsp    = &nodes[bspnum];

        // Possibly divide back space.
        //Walk back up the tree until we find
        //a node that has a visible backspace.
        while (!R_CheckBBox(bsp->bbox[side]))
        {
            if (sp == 0)
            {
                //back at root node and not visible. All done!
                return;
            }

            //Back side next.
            --sp;
            side   = stack_side[sp];
            bspnum = stack_bsp[sp];

            bsp = &nodes[bspnum];
        }

        bspnum = bsp->children[side];
    }
}
#else
static void R_RenderBSPNode(int16_t bspnum)
{
	if (R_RenderBspSubsector(bspnum))
		return;

	const mapnode_t __far* bsp = &nodes[bspnum];

//
// decide which side the view point is on
//
	int8_t side = R_PointOnSide(viewx, viewy, bsp);

	R_RenderBSPNode(bsp->children[side]); // recursively divide front space

	if (R_CheckBBox(bsp->bbox[side ^ 1]))	// possibly divide back space
		R_RenderBSPNode(bsp->children[side ^ 1]);
}
#endif


static void R_ClearDrawSegs(void)
{
    ds_p = _s_drawsegs;
}

static void R_ClearClipSegs (void)
{
    memset(solidcol, 0, VIEWWINDOWWIDTH);
}


//
// R_SetupFrame
//

static void R_SetupFrame (player_t *player)
{
    viewx = player->mo->x;
    viewy = player->mo->y;
    viewz = player->viewz;
    viewangle = player->mo->angle;
    viewangle16 = viewangle >> FRACBITS;

    extralight = player->extralight;

    viewsin = finesineapprox(  viewangle16 >> ANGLETOFINESHIFT_16);
    viewcos = finecosineapprox(viewangle16 >> ANGLETOFINESHIFT_16);

    if (player->fixedcolormap)
    {
        fixedcolormap = fullcolormap   // killough 3/20/98: use fullcolormap
                + player->fixedcolormap*256;
    }
    else
        fixedcolormap = NULL;

    validcount++;
}


//
// R_RenderView
//
void R_RenderPlayerView (player_t* player)
{
    R_SetupFrame (player);

    // Clear buffers.
    R_ClearClipSegs ();
    R_ClearDrawSegs ();
    R_ClearOpeningClippingDetermination ();

#if !defined FLAT_SPAN
    R_ClearPlanes ();
#endif

    R_ClearOpenings ();
    R_ClearSprites ();

    // The head node is the last node output.
    R_RenderBSPNode (numnodes-1);

#if defined FLAT_SPAN
    R_FreeSkyPatch ();
#else
    R_DrawPlanes ();
#endif

    R_DrawMasked ();
}


static const uint8_t viewangletoxTable[4096 - 1023 - VIEWANGLETOXMAX] =
{
#if VIEWWINDOWWIDTH == 240
    239, 239, 239, 239, 239,
    238, 238, 238, 238, 238, 238,
    237, 237, 237, 237, 237, 237,
    236, 236, 236, 236, 236,
    235, 235, 235, 235, 235, 235,
    234, 234, 234, 234, 234, 234,
    233, 233, 233, 233, 233,
    232, 232, 232, 232, 232, 232,
    231, 231, 231, 231, 231, 231,
    230, 230, 230, 230, 230, 230,
    229, 229, 229, 229, 229, 229,
    228, 228, 228, 228, 228, 228,
    227, 227, 227, 227, 227, 227,
    226, 226, 226, 226, 226, 226,
    225, 225, 225, 225, 225, 225,
    224, 224, 224, 224, 224, 224, 224,
    223, 223, 223, 223, 223, 223,
    222, 222, 222, 222, 222, 222,
    221, 221, 221, 221, 221, 221, 221,
    220, 220, 220, 220, 220, 220,
    219, 219, 219, 219, 219, 219, 219,
    218, 218, 218, 218, 218, 218,
    217, 217, 217, 217, 217, 217, 217,
    216, 216, 216, 216, 216, 216,
    215, 215, 215, 215, 215, 215, 215,
    214, 214, 214, 214, 214, 214, 214,
    213, 213, 213, 213, 213, 213, 213,
    212, 212, 212, 212, 212, 212, 212,
    211, 211, 211, 211, 211, 211, 211,
    210, 210, 210, 210, 210, 210, 210,
    209, 209, 209, 209, 209, 209, 209,
    208, 208, 208, 208, 208, 208, 208,
    207, 207, 207, 207, 207, 207, 207,
    206, 206, 206, 206, 206, 206, 206,
    205, 205, 205, 205, 205, 205, 205,
    204, 204, 204, 204, 204, 204, 204, 204,
    203, 203, 203, 203, 203, 203, 203,
    202, 202, 202, 202, 202, 202, 202,
    201, 201, 201, 201, 201, 201, 201, 201,
    200, 200, 200, 200, 200, 200, 200,
    199, 199, 199, 199, 199, 199, 199, 199,
    198, 198, 198, 198, 198, 198, 198, 198,
    197, 197, 197, 197, 197, 197, 197,
    196, 196, 196, 196, 196, 196, 196, 196,
    195, 195, 195, 195, 195, 195, 195, 195,
    194, 194, 194, 194, 194, 194, 194, 194,
    193, 193, 193, 193, 193, 193, 193, 193,
    192, 192, 192, 192, 192, 192, 192, 192,
    191, 191, 191, 191, 191, 191, 191, 191,
    190, 190, 190, 190, 190, 190, 190, 190,
    189, 189, 189, 189, 189, 189, 189, 189,
    188, 188, 188, 188, 188, 188, 188, 188, 188,
    187, 187, 187, 187, 187, 187, 187, 187,
    186, 186, 186, 186, 186, 186, 186, 186,
    185, 185, 185, 185, 185, 185, 185, 185, 185,
    184, 184, 184, 184, 184, 184, 184, 184,
    183, 183, 183, 183, 183, 183, 183, 183, 183,
    182, 182, 182, 182, 182, 182, 182, 182,
    181, 181, 181, 181, 181, 181, 181, 181, 181,
    180, 180, 180, 180, 180, 180, 180, 180, 180,
    179, 179, 179, 179, 179, 179, 179, 179, 179,
    178, 178, 178, 178, 178, 178, 178, 178,
    177, 177, 177, 177, 177, 177, 177, 177, 177,
    176, 176, 176, 176, 176, 176, 176, 176, 176,
    175, 175, 175, 175, 175, 175, 175, 175, 175,
    174, 174, 174, 174, 174, 174, 174, 174, 174,
    173, 173, 173, 173, 173, 173, 173, 173, 173,
    172, 172, 172, 172, 172, 172, 172, 172, 172, 172,
    171, 171, 171, 171, 171, 171, 171, 171, 171,
    170, 170, 170, 170, 170, 170, 170, 170, 170,
    169, 169, 169, 169, 169, 169, 169, 169, 169, 169,
    168, 168, 168, 168, 168, 168, 168, 168, 168,
    167, 167, 167, 167, 167, 167, 167, 167, 167,
    166, 166, 166, 166, 166, 166, 166, 166, 166, 166,
    165, 165, 165, 165, 165, 165, 165, 165, 165,
    164, 164, 164, 164, 164, 164, 164, 164, 164, 164,
    163, 163, 163, 163, 163, 163, 163, 163, 163, 163,
    162, 162, 162, 162, 162, 162, 162, 162, 162,
    161, 161, 161, 161, 161, 161, 161, 161, 161, 161,
    160, 160, 160, 160, 160, 160, 160, 160, 160, 160,
    159, 159, 159, 159, 159, 159, 159, 159, 159, 159,
    158, 158, 158, 158, 158, 158, 158, 158, 158, 158,
    157, 157, 157, 157, 157, 157, 157, 157, 157, 157,
    156, 156, 156, 156, 156, 156, 156, 156, 156, 156,
    155, 155, 155, 155, 155, 155, 155, 155, 155, 155,
    154, 154, 154, 154, 154, 154, 154, 154, 154, 154,
    153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
    152, 152, 152, 152, 152, 152, 152, 152, 152, 152,
    151, 151, 151, 151, 151, 151, 151, 151, 151, 151,
    150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150,
    149, 149, 149, 149, 149, 149, 149, 149, 149, 149,
    148, 148, 148, 148, 148, 148, 148, 148, 148, 148,
    147, 147, 147, 147, 147, 147, 147, 147, 147, 147, 147,
    146, 146, 146, 146, 146, 146, 146, 146, 146, 146,
    145, 145, 145, 145, 145, 145, 145, 145, 145, 145,
    144, 144, 144, 144, 144, 144, 144, 144, 144, 144, 144,
    143, 143, 143, 143, 143, 143, 143, 143, 143, 143,
    142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142,
    141, 141, 141, 141, 141, 141, 141, 141, 141, 141,
    140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140,
    139, 139, 139, 139, 139, 139, 139, 139, 139, 139, 139,
    138, 138, 138, 138, 138, 138, 138, 138, 138, 138,
    137, 137, 137, 137, 137, 137, 137, 137, 137, 137, 137,
    136, 136, 136, 136, 136, 136, 136, 136, 136, 136, 136,
    135, 135, 135, 135, 135, 135, 135, 135, 135, 135,
    134, 134, 134, 134, 134, 134, 134, 134, 134, 134, 134,
    133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133,
    132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132,
    131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131,
    130, 130, 130, 130, 130, 130, 130, 130, 130, 130,
    129, 129, 129, 129, 129, 129, 129, 129, 129, 129, 129,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    126, 126, 126, 126, 126, 126, 126, 126, 126, 126, 126,
    125, 125, 125, 125, 125, 125, 125, 125, 125, 125, 125,
    124, 124, 124, 124, 124, 124, 124, 124, 124, 124,
    123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123,
    122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 122,
    121, 121, 121, 121, 121, 121, 121, 121, 121, 121, 121,
    120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120,
    119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119,
    118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118,
    117, 117, 117, 117, 117, 117, 117, 117, 117, 117,
    116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116,
    115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
    114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114,
    113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113,
    112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112,
    111, 111, 111, 111, 111, 111, 111, 111, 111, 111,
    110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110,
    109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
    108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108,
    107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107,
    106, 106, 106, 106, 106, 106, 106, 106, 106, 106,
    105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105,
    104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104,
    103, 103, 103, 103, 103, 103, 103, 103, 103, 103,
    102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
    101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
    97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
    96, 96, 96, 96, 96, 96, 96, 96, 96, 96,
    95, 95, 95, 95, 95, 95, 95, 95, 95, 95,
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
    93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
    92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
    91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
    90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
    89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
    88, 88, 88, 88, 88, 88, 88, 88, 88, 88,
    87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
    86, 86, 86, 86, 86, 86, 86, 86, 86, 86,
    85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
    84, 84, 84, 84, 84, 84, 84, 84, 84, 84,
    83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
    82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
    81, 81, 81, 81, 81, 81, 81, 81, 81, 81,
    80, 80, 80, 80, 80, 80, 80, 80, 80, 80,
    79, 79, 79, 79, 79, 79, 79, 79, 79,
    78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
    77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
    76, 76, 76, 76, 76, 76, 76, 76, 76,
    75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
    74, 74, 74, 74, 74, 74, 74, 74, 74,
    73, 73, 73, 73, 73, 73, 73, 73, 73,
    72, 72, 72, 72, 72, 72, 72, 72, 72, 72,
    71, 71, 71, 71, 71, 71, 71, 71, 71,
    70, 70, 70, 70, 70, 70, 70, 70, 70,
    69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
    68, 68, 68, 68, 68, 68, 68, 68, 68,
    67, 67, 67, 67, 67, 67, 67, 67, 67,
    66, 66, 66, 66, 66, 66, 66, 66, 66,
    65, 65, 65, 65, 65, 65, 65, 65, 65,
    64, 64, 64, 64, 64, 64, 64, 64, 64,
    63, 63, 63, 63, 63, 63, 63, 63,
    62, 62, 62, 62, 62, 62, 62, 62, 62,
    61, 61, 61, 61, 61, 61, 61, 61, 61,
    60, 60, 60, 60, 60, 60, 60, 60, 60,
    59, 59, 59, 59, 59, 59, 59, 59,
    58, 58, 58, 58, 58, 58, 58, 58, 58,
    57, 57, 57, 57, 57, 57, 57, 57,
    56, 56, 56, 56, 56, 56, 56, 56, 56,
    55, 55, 55, 55, 55, 55, 55, 55,
    54, 54, 54, 54, 54, 54, 54, 54,
    53, 53, 53, 53, 53, 53, 53, 53, 53,
    52, 52, 52, 52, 52, 52, 52, 52,
    51, 51, 51, 51, 51, 51, 51, 51,
    50, 50, 50, 50, 50, 50, 50, 50,
    49, 49, 49, 49, 49, 49, 49, 49,
    48, 48, 48, 48, 48, 48, 48, 48,
    47, 47, 47, 47, 47, 47, 47, 47,
    46, 46, 46, 46, 46, 46, 46, 46,
    45, 45, 45, 45, 45, 45, 45, 45,
    44, 44, 44, 44, 44, 44, 44,
    43, 43, 43, 43, 43, 43, 43, 43,
    42, 42, 42, 42, 42, 42, 42, 42,
    41, 41, 41, 41, 41, 41, 41,
    40, 40, 40, 40, 40, 40, 40, 40,
    39, 39, 39, 39, 39, 39, 39,
    38, 38, 38, 38, 38, 38, 38,
    37, 37, 37, 37, 37, 37, 37, 37,
    36, 36, 36, 36, 36, 36, 36,
    35, 35, 35, 35, 35, 35, 35,
    34, 34, 34, 34, 34, 34, 34,
    33, 33, 33, 33, 33, 33, 33,
    32, 32, 32, 32, 32, 32, 32,
    31, 31, 31, 31, 31, 31, 31,
    30, 30, 30, 30, 30, 30, 30,
    29, 29, 29, 29, 29, 29, 29,
    28, 28, 28, 28, 28, 28, 28,
    27, 27, 27, 27, 27, 27, 27,
    26, 26, 26, 26, 26, 26, 26,
    25, 25, 25, 25, 25, 25,
    24, 24, 24, 24, 24, 24, 24,
    23, 23, 23, 23, 23, 23,
    22, 22, 22, 22, 22, 22, 22,
    21, 21, 21, 21, 21, 21,
    20, 20, 20, 20, 20, 20, 20,
    19, 19, 19, 19, 19, 19,
    18, 18, 18, 18, 18, 18,
    17, 17, 17, 17, 17, 17, 17,
    16, 16, 16, 16, 16, 16,
    15, 15, 15, 15, 15, 15,
    14, 14, 14, 14, 14, 14,
    13, 13, 13, 13, 13, 13,
    12, 12, 12, 12, 12, 12,
    11, 11, 11, 11, 11, 11,
    10, 10, 10, 10, 10, 10,
    9, 9, 9, 9, 9, 9,
    8, 8, 8, 8, 8,
    7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1
#elif VIEWWINDOWWIDTH == 120
    119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 119,
    118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118,
    117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117,
    116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116,
    115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
    114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114,
    113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113,
    112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112,
    111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111,
    110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110,
    109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
    108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108,
    107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107,
    106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106,
    105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105,
    104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104, 104,
    103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103, 103,
    102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102, 102,
    101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98, 98,
    97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97, 97,
    96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96, 96,
    95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95, 95,
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
    93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
    92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92,
    91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91,
    90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
    89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
    88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88, 88,
    87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
    86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86,
    85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
    84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84,
    83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83,
    82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 82,
    81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81,
    80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80,
    79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
    78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
    76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76,
    75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
    74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74,
    73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73,
    72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72,
    71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71,
    70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
    69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
    68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
    67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
    61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
    60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
    59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57,
    56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
    55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
    54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
    53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
    52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
    51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
    50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
    47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
    45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
    44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
    43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
    42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
    41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
    40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
    39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
    38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
    37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
    34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
    33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
#elif VIEWWINDOWWIDTH == 80
    79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
    78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
    77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
    76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76,
    75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75, 75,
    74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74, 74,
    73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73, 73,
    72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72,
    71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71,
    70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70,
    69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69,
    68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
    67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67,
    66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
    61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61,
    60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
    59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57,
    56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
    55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
    54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
    53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
    52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
    51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
    50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
    47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
    45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
    44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
    43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
    42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
    41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
    40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
    39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
    38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
    37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
    34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
    33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
#elif VIEWWINDOWWIDTH == 60
    59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59,
    58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
    57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57,
    56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
    55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
    54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
    53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
    52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
    51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
    50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
    47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
    45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
    44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
    43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
    42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
    41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
    40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
    39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
    38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
    37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
    34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
    33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
#elif VIEWWINDOWWIDTH == 40
    39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
    38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
    37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
    34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
    33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
#elif VIEWWINDOWWIDTH == 30
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
#else
#error unsupported VIEWWINDOWWIDTH value
#endif
};


// ArcTan LUT,
//  maps tan(angle) to angle fast. Gotta search.
//
// Effective size is 2049;
// The +1 size is to handle the case when x==y without additional checking.
static const angle_t tantoangleTable[2049] =
{
    0,333772,667544,1001315,1335086,1668857,2002626,2336395,
    2670163,3003929,3337694,3671457,4005219,4338979,4672736,5006492,
    5340245,5673995,6007743,6341488,6675230,7008968,7342704,7676435,
    8010164,8343888,8677609,9011325,9345037,9678744,10012447,10346145,
    10679838,11013526,11347209,11680887,12014558,12348225,12681885,13015539,
    13349187,13682829,14016464,14350092,14683714,15017328,15350936,15684536,
    16018129,16351714,16685291,17018860,17352422,17685974,18019518,18353054,
    18686582,19020100,19353610,19687110,20020600,20354080,20687552,21021014,
    21354466,21687906,22021338,22354758,22688168,23021568,23354956,23688332,
    24021698,24355052,24688396,25021726,25355046,25688352,26021648,26354930,
    26688200,27021456,27354702,27687932,28021150,28354356,28687548,29020724,
    29353888,29687038,30020174,30353296,30686404,31019496,31352574,31685636,
    32018684,32351718,32684734,33017736,33350722,33683692,34016648,34349584,
    34682508,35015412,35348300,35681172,36014028,36346868,36679688,37012492,
    37345276,37678044,38010792,38343524,38676240,39008936,39341612,39674272,
    40006912,40339532,40672132,41004716,41337276,41669820,42002344,42334848,
    42667332,42999796,43332236,43664660,43997060,44329444,44661800,44994140,
    45326456,45658752,45991028,46323280,46655512,46987720,47319908,47652072,
    47984212,48316332,48648428,48980500,49312548,49644576,49976580,50308556,
    50640512,50972444,51304352,51636236,51968096,52299928,52631740,52963524,
    53295284,53627020,53958728,54290412,54622068,54953704,55285308,55616888,
    55948444,56279972,56611472,56942948,57274396,57605816,57937212,58268576,
    58599916,58931228,59262512,59593768,59924992,60256192,60587364,60918508,
    61249620,61580704,61911760,62242788,62573788,62904756,63235692,63566604,
    63897480,64228332,64559148,64889940,65220696,65551424,65882120,66212788,
    66543420,66874024,67204600,67535136,67865648,68196120,68526568,68856984,
    69187360,69517712,69848024,70178304,70508560,70838776,71168960,71499112,
    71829224,72159312,72489360,72819376,73149360,73479304,73809216,74139096,
    74468936,74798744,75128520,75458264,75787968,76117632,76447264,76776864,
    77106424,77435952,77765440,78094888,78424304,78753688,79083032,79412336,
    79741608,80070840,80400032,80729192,81058312,81387392,81716432,82045440,
    82374408,82703336,83032224,83361080,83689896,84018664,84347400,84676096,
    85004760,85333376,85661952,85990488,86318984,86647448,86975864,87304240,
    87632576,87960872,88289128,88617344,88945520,89273648,89601736,89929792,
    90257792,90585760,90913688,91241568,91569408,91897200,92224960,92552672,
    92880336,93207968,93535552,93863088,94190584,94518040,94845448,95172816,
    95500136,95827416,96154648,96481832,96808976,97136080,97463136,97790144,
    98117112,98444032,98770904,99097736,99424520,99751256,100077944,100404592,
    100731192,101057744,101384248,101710712,102037128,102363488,102689808,103016080,
    103342312,103668488,103994616,104320696,104646736,104972720,105298656,105624552,
    105950392,106276184,106601928,106927624,107253272,107578872,107904416,108229920,
    108555368,108880768,109206120,109531416,109856664,110181872,110507016,110832120,
    111157168,111482168,111807112,112132008,112456856,112781648,113106392,113431080,
    113755720,114080312,114404848,114729328,115053760,115378136,115702464,116026744,
    116350960,116675128,116999248,117323312,117647320,117971272,118295176,118619024,
    118942816,119266560,119590248,119913880,120237456,120560984,120884456,121207864,
    121531224,121854528,122177784,122500976,122824112,123147200,123470224,123793200,
    124116120,124438976,124761784,125084528,125407224,125729856,126052432,126374960,
    126697424,127019832,127342184,127664472,127986712,128308888,128631008,128953072,
    129275080,129597024,129918912,130240744,130562520,130884232,131205888,131527480,
    131849016,132170496,132491912,132813272,133134576,133455816,133776992,134098120,
    134419184,134740176,135061120,135382000,135702816,136023584,136344272,136664912,
    136985488,137306016,137626464,137946864,138267184,138587456,138907664,139227808,
    139547904,139867920,140187888,140507776,140827616,141147392,141467104,141786752,
    142106336,142425856,142745312,143064720,143384048,143703312,144022512,144341664,
    144660736,144979744,145298704,145617584,145936400,146255168,146573856,146892480,
    147211040,147529536,147847968,148166336,148484640,148802880,149121056,149439152,
    149757200,150075168,150393072,150710912,151028688,151346400,151664048,151981616,
    152299136,152616576,152933952,153251264,153568496,153885680,154202784,154519824,
    154836784,155153696,155470528,155787296,156104000,156420624,156737200,157053696,
    157370112,157686480,158002768,158318976,158635136,158951216,159267232,159583168,
    159899040,160214848,160530592,160846256,161161840,161477376,161792832,162108208,
    162423520,162738768,163053952,163369040,163684080,163999040,164313936,164628752,
    164943504,165258176,165572784,165887312,166201776,166516160,166830480,167144736,
    167458912,167773008,168087040,168400992,168714880,169028688,169342432,169656096,
    169969696,170283216,170596672,170910032,171223344,171536576,171849728,172162800,
    172475808,172788736,173101600,173414384,173727104,174039728,174352288,174664784,
    174977200,175289536,175601792,175913984,176226096,176538144,176850096,177161984,
    177473792,177785536,178097200,178408784,178720288,179031728,179343088,179654368,
    179965568,180276704,180587744,180898720,181209616,181520448,181831184,182141856,
    182452448,182762960,183073408,183383760,183694048,184004240,184314368,184624416,
    184934400,185244288,185554096,185863840,186173504,186483072,186792576,187102000,
    187411344,187720608,188029808,188338912,188647936,188956896,189265760,189574560,
    189883264,190191904,190500448,190808928,191117312,191425632,191733872,192042016,
    192350096,192658096,192966000,193273840,193581584,193889264,194196848,194504352,
    194811792,195119136,195426400,195733584,196040688,196347712,196654656,196961520,
    197268304,197574992,197881616,198188144,198494592,198800960,199107248,199413456,
    199719584,200025616,200331584,200637456,200943248,201248960,201554576,201860128,
    202165584,202470960,202776256,203081456,203386592,203691632,203996592,204301472,
    204606256,204910976,205215600,205520144,205824592,206128960,206433248,206737456,
    207041584,207345616,207649568,207953424,208257216,208560912,208864512,209168048,
    209471488,209774832,210078112,210381296,210684384,210987408,211290336,211593184,
    211895936,212198608,212501184,212803680,213106096,213408432,213710672,214012816,
    214314880,214616864,214918768,215220576,215522288,215823920,216125472,216426928,
    216728304,217029584,217330784,217631904,217932928,218233856,218534704,218835472,
    219136144,219436720,219737216,220037632,220337952,220638192,220938336,221238384,
    221538352,221838240,222138032,222437728,222737344,223036880,223336304,223635664,
    223934912,224234096,224533168,224832160,225131072,225429872,225728608,226027232,
    226325776,226624240,226922608,227220880,227519056,227817152,228115168,228413088,
    228710912,229008640,229306288,229603840,229901312,230198688,230495968,230793152,
    231090256,231387280,231684192,231981024,232277760,232574416,232870960,233167440,
    233463808,233760096,234056288,234352384,234648384,234944304,235240128,235535872,
    235831504,236127056,236422512,236717888,237013152,237308336,237603424,237898416,
    238193328,238488144,238782864,239077488,239372016,239666464,239960816,240255072,
    240549232,240843312,241137280,241431168,241724960,242018656,242312256,242605776,
    242899200,243192512,243485744,243778896,244071936,244364880,244657744,244950496,
    245243168,245535744,245828224,246120608,246412912,246705104,246997216,247289216,
    247581136,247872960,248164688,248456320,248747856,249039296,249330640,249621904,
    249913056,250204128,250495088,250785968,251076736,251367424,251658016,251948512,
    252238912,252529200,252819408,253109520,253399536,253689456,253979280,254269008,
    254558640,254848176,255137632,255426976,255716224,256005376,256294432,256583392,
    256872256,257161024,257449696,257738272,258026752,258315136,258603424,258891600,
    259179696,259467696,259755600,260043392,260331104,260618704,260906224,261193632,
    261480960,261768176,262055296,262342320,262629248,262916080,263202816,263489456,
    263776000,264062432,264348784,264635024,264921168,265207216,265493168,265779024,
    266064784,266350448,266636000,266921472,267206832,267492096,267777264,268062336,
    268347312,268632192,268916960,269201632,269486208,269770688,270055072,270339360,
    270623552,270907616,271191616,271475488,271759296,272042976,272326560,272610048,
    272893440,273176736,273459936,273743040,274026048,274308928,274591744,274874432,
    275157024,275439520,275721920,276004224,276286432,276568512,276850528,277132416,
    277414240,277695936,277977536,278259040,278540448,278821728,279102944,279384032,
    279665056,279945952,280226752,280507456,280788064,281068544,281348960,281629248,
    281909472,282189568,282469568,282749440,283029248,283308960,283588544,283868032,
    284147424,284426720,284705920,284985024,285264000,285542912,285821696,286100384,
    286378976,286657440,286935840,287214112,287492320,287770400,288048384,288326240,
    288604032,288881696,289159264,289436768,289714112,289991392,290268576,290545632,
    290822592,291099456,291376224,291652896,291929440,292205888,292482272,292758528,
    293034656,293310720,293586656,293862496,294138240,294413888,294689440,294964864,
    295240192,295515424,295790560,296065600,296340512,296615360,296890080,297164704,
    297439200,297713632,297987936,298262144,298536256,298810240,299084160,299357952,
    299631648,299905248,300178720,300452128,300725408,300998592,301271680,301544640,
    301817536,302090304,302362976,302635520,302908000,303180352,303452608,303724768,
    303996800,304268768,304540608,304812320,305083968,305355520,305626944,305898272,
    306169472,306440608,306711616,306982528,307253344,307524064,307794656,308065152,
    308335552,308605856,308876032,309146112,309416096,309685984,309955744,310225408,
    310494976,310764448,311033824,311303072,311572224,311841280,312110208,312379040,
    312647776,312916416,313184960,313453376,313721696,313989920,314258016,314526016,
    314793920,315061728,315329408,315597024,315864512,316131872,316399168,316666336,
    316933408,317200384,317467232,317733984,318000640,318267200,318533632,318799968,
    319066208,319332352,319598368,319864288,320130112,320395808,320661408,320926912,
    321192320,321457632,321722816,321987904,322252864,322517760,322782528,323047200,
    323311744,323576192,323840544,324104800,324368928,324632992,324896928,325160736,
    325424448,325688096,325951584,326215008,326478304,326741504,327004608,327267584,
    327530464,327793248,328055904,328318496,328580960,328843296,329105568,329367712,
    329629760,329891680,330153536,330415264,330676864,330938400,331199808,331461120,
    331722304,331983392,332244384,332505280,332766048,333026752,333287296,333547776,
    333808128,334068384,334328544,334588576,334848512,335108352,335368064,335627712,
    335887200,336146624,336405920,336665120,336924224,337183200,337442112,337700864,
    337959552,338218112,338476576,338734944,338993184,339251328,339509376,339767296,
    340025120,340282848,340540480,340797984,341055392,341312704,341569888,341826976,
    342083968,342340832,342597600,342854272,343110848,343367296,343623648,343879904,
    344136032,344392064,344648000,344903808,345159520,345415136,345670656,345926048,
    346181344,346436512,346691616,346946592,347201440,347456224,347710880,347965440,
    348219872,348474208,348728448,348982592,349236608,349490528,349744320,349998048,
    350251648,350505152,350758528,351011808,351264992,351518048,351771040,352023872,
    352276640,352529280,352781824,353034272,353286592,353538816,353790944,354042944,
    354294880,354546656,354798368,355049952,355301440,355552800,355804096,356055264,
    356306304,356557280,356808128,357058848,357309504,357560032,357810464,358060768,
    358311008,358561088,358811104,359060992,359310784,359560480,359810048,360059520,
    360308896,360558144,360807296,361056352,361305312,361554144,361802880,362051488,
    362300032,362548448,362796736,363044960,363293056,363541024,363788928,364036704,
    364284384,364531936,364779392,365026752,365274016,365521152,365768192,366015136,
    366261952,366508672,366755296,367001792,367248192,367494496,367740704,367986784,
    368232768,368478656,368724416,368970080,369215648,369461088,369706432,369951680,
    370196800,370441824,370686752,370931584,371176288,371420896,371665408,371909792,
    372154080,372398272,372642336,372886304,373130176,373373952,373617600,373861152,
    374104608,374347936,374591168,374834304,375077312,375320224,375563040,375805760,
    376048352,376290848,376533248,376775520,377017696,377259776,377501728,377743584,
    377985344,378227008,378468544,378709984,378951328,379192544,379433664,379674688,
    379915584,380156416,380397088,380637696,380878176,381118560,381358848,381599040,
    381839104,382079072,382318912,382558656,382798304,383037856,383277280,383516640,
    383755840,383994976,384233984,384472896,384711712,384950400,385188992,385427488,
    385665888,385904160,386142336,386380384,386618368,386856224,387093984,387331616,
    387569152,387806592,388043936,388281152,388518272,388755296,388992224,389229024,
    389465728,389702336,389938816,390175200,390411488,390647680,390883744,391119712,
    391355584,391591328,391826976,392062528,392297984,392533312,392768544,393003680,
    393238720,393473632,393708448,393943168,394177760,394412256,394646656,394880960,
    395115136,395349216,395583200,395817088,396050848,396284512,396518080,396751520,
    396984864,397218112,397451264,397684288,397917248,398150080,398382784,398615424,
    398847936,399080320,399312640,399544832,399776928,400008928,400240832,400472608,
    400704288,400935872,401167328,401398720,401629984,401861120,402092192,402323136,
    402553984,402784736,403015360,403245888,403476320,403706656,403936896,404167008,
    404397024,404626944,404856736,405086432,405316032,405545536,405774912,406004224,
    406233408,406462464,406691456,406920320,407149088,407377760,407606336,407834784,
    408063136,408291392,408519520,408747584,408975520,409203360,409431072,409658720,
    409886240,410113664,410340992,410568192,410795296,411022304,411249216,411476032,
    411702720,411929312,412155808,412382176,412608480,412834656,413060736,413286720,
    413512576,413738336,413964000,414189568,414415040,414640384,414865632,415090784,
    415315840,415540800,415765632,415990368,416215008,416439552,416663968,416888288,
    417112512,417336640,417560672,417784576,418008384,418232096,418455712,418679200,
    418902624,419125920,419349120,419572192,419795200,420018080,420240864,420463552,
    420686144,420908608,421130976,421353280,421575424,421797504,422019488,422241344,
    422463104,422684768,422906336,423127776,423349120,423570400,423791520,424012576,
    424233536,424454368,424675104,424895744,425116288,425336736,425557056,425777280,
    425997408,426217440,426437376,426657184,426876928,427096544,427316064,427535488,
    427754784,427974016,428193120,428412128,428631040,428849856,429068544,429287168,
    429505664,429724064,429942368,430160576,430378656,430596672,430814560,431032352,
    431250048,431467616,431685120,431902496,432119808,432336992,432554080,432771040,
    432987936,433204736,433421408,433637984,433854464,434070848,434287104,434503296,
    434719360,434935360,435151232,435367008,435582656,435798240,436013696,436229088,
    436444352,436659520,436874592,437089568,437304416,437519200,437733856,437948416,
    438162880,438377248,438591520,438805696,439019744,439233728,439447584,439661344,
    439875008,440088576,440302048,440515392,440728672,440941824,441154880,441367872,
    441580736,441793472,442006144,442218720,442431168,442643552,442855808,443067968,
    443280032,443492000,443703872,443915648,444127296,444338880,444550336,444761696,
    444972992,445184160,445395232,445606176,445817056,446027840,446238496,446449088,
    446659552,446869920,447080192,447290400,447500448,447710432,447920320,448130112,
    448339776,448549376,448758848,448968224,449177536,449386720,449595808,449804800,
    450013664,450222464,450431168,450639776,450848256,451056640,451264960,451473152,
    451681248,451889248,452097152,452304960,452512672,452720288,452927808,453135232,
    453342528,453549760,453756864,453963904,454170816,454377632,454584384,454791008,
    454997536,455203968,455410304,455616544,455822688,456028704,456234656,456440512,
    456646240,456851904,457057472,457262912,457468256,457673536,457878688,458083744,
    458288736,458493600,458698368,458903040,459107616,459312096,459516480,459720768,
    459924960,460129056,460333056,460536960,460740736,460944448,461148064,461351584,
    461554976,461758304,461961536,462164640,462367680,462570592,462773440,462976160,
    463178816,463381344,463583776,463786144,463988384,464190560,464392608,464594560,
    464796448,464998208,465199872,465401472,465602944,465804320,466005600,466206816,
    466407904,466608896,466809824,467010624,467211328,467411936,467612480,467812896,
    468013216,468213440,468413600,468613632,468813568,469013440,469213184,469412832,
    469612416,469811872,470011232,470210528,470409696,470608800,470807776,471006688,
    471205472,471404192,471602784,471801312,471999712,472198048,472396288,472594400,
    472792448,472990400,473188256,473385984,473583648,473781216,473978688,474176064,
    474373344,474570528,474767616,474964608,475161504,475358336,475555040,475751648,
    475948192,476144608,476340928,476537184,476733312,476929376,477125344,477321184,
    477516960,477712640,477908224,478103712,478299104,478494400,478689600,478884704,
    479079744,479274656,479469504,479664224,479858880,480053408,480247872,480442240,
    480636512,480830656,481024736,481218752,481412640,481606432,481800128,481993760,
    482187264,482380704,482574016,482767264,482960416,483153472,483346432,483539296,
    483732064,483924768,484117344,484309856,484502240,484694560,484886784,485078912,
    485270944,485462880,485654720,485846464,486038144,486229696,486421184,486612576,
    486803840,486995040,487186176,487377184,487568096,487758912,487949664,488140320,
    488330880,488521312,488711712,488901984,489092160,489282240,489472256,489662176,
    489851968,490041696,490231328,490420896,490610336,490799712,490988960,491178144,
    491367232,491556224,491745120,491933920,492122656,492311264,492499808,492688256,
    492876608,493064864,493253056,493441120,493629120,493817024,494004832,494192544,
    494380160,494567712,494755136,494942496,495129760,495316928,495504000,495691008,
    495877888,496064704,496251424,496438048,496624608,496811040,496997408,497183680,
    497369856,497555936,497741920,497927840,498113632,498299360,498484992,498670560,
    498856000,499041376,499226656,499411840,499596928,499781920,499966848,500151680,
    500336416,500521056,500705600,500890080,501074464,501258752,501442944,501627040,
    501811072,501995008,502178848,502362592,502546240,502729824,502913312,503096704,
    503280000,503463232,503646368,503829408,504012352,504195200,504377984,504560672,
    504743264,504925760,505108192,505290496,505472736,505654912,505836960,506018944,
    506200832,506382624,506564320,506745952,506927488,507108928,507290272,507471552,
    507652736,507833824,508014816,508195744,508376576,508557312,508737952,508918528,
    509099008,509279392,509459680,509639904,509820032,510000064,510180000,510359872,
    510539648,510719328,510898944,511078432,511257856,511437216,511616448,511795616,
    511974688,512153664,512332576,512511392,512690112,512868768,513047296,513225792,
    513404160,513582432,513760640,513938784,514116800,514294752,514472608,514650368,
    514828064,515005664,515183168,515360608,515537952,515715200,515892352,516069440,
    516246432,516423328,516600160,516776896,516953536,517130112,517306592,517482976,
    517659264,517835488,518011616,518187680,518363648,518539520,518715296,518891008,
    519066624,519242144,519417600,519592960,519768256,519943424,520118528,520293568,
    520468480,520643328,520818112,520992800,521167392,521341888,521516320,521690656,
    521864896,522039072,522213152,522387168,522561056,522734912,522908640,523082304,
    523255872,523429376,523602784,523776096,523949312,524122464,524295552,524468512,
    524641440,524814240,524986976,525159616,525332192,525504640,525677056,525849344,
    526021568,526193728,526365792,526537760,526709632,526881440,527053152,527224800,
    527396352,527567840,527739200,527910528,528081728,528252864,528423936,528594880,
    528765760,528936576,529107296,529277920,529448480,529618944,529789344,529959648,
    530129856,530300000,530470048,530640000,530809888,530979712,531149440,531319072,
    531488608,531658080,531827488,531996800,532166016,532335168,532504224,532673184,
    532842080,533010912,533179616,533348288,533516832,533685312,533853728,534022048,
    534190272,534358432,534526496,534694496,534862400,535030240,535197984,535365632,
    535533216,535700704,535868128,536035456,536202720,536369888,536536992,536704000,
    536870912
};


// Tangens LUT.
// Should work with BAM fairly well (12 of 16bit, effectively, by shifting).
static const uint16_t finetangentTable_part_3[1024] =
{
    25,75,125,175,226,276,326,376,
    427,477,527,578,628,678,728,779,
    829,879,929,980,1030,1080,1131,1181,
    1231,1281,1332,1382,1432,1483,1533,1583,
    1633,1684,1734,1784,1835,1885,1935,1986,
    2036,2086,2137,2187,2237,2288,2338,2388,
    2439,2489,2539,2590,2640,2690,2741,2791,
    2841,2892,2942,2992,3043,3093,3144,3194,
    3244,3295,3345,3395,3446,3496,3547,3597,
    3648,3698,3748,3799,3849,3900,3950,4001,
    4051,4101,4152,4202,4253,4303,4354,4404,
    4455,4505,4556,4606,4657,4707,4758,4808,
    4859,4910,4960,5011,5061,5112,5162,5213,
    5264,5314,5365,5415,5466,5517,5567,5618,
    5668,5719,5770,5820,5871,5922,5972,6023,
    6074,6124,6175,6226,6277,6327,6378,6429,
    6480,6530,6581,6632,6683,6733,6784,6835,
    6886,6937,6988,7038,7089,7140,7191,7242,
    7293,7344,7395,7445,7496,7547,7598,7649,
    7700,7751,7802,7853,7904,7955,8006,8057,
    8108,8159,8210,8261,8312,8363,8414,8466,
    8517,8568,8619,8670,8721,8772,8824,8875,
    8926,8977,9028,9080,9131,9182,9233,9285,
    9336,9387,9438,9490,9541,9592,9644,9695,
    9747,9798,9849,9901,9952,10004,10055,10106,
    10158,10209,10261,10312,10364,10415,10467,10519,
    10570,10622,10673,10725,10777,10828,10880,10931,
    10983,11035,11086,11138,11190,11242,11293,11345,
    11397,11449,11501,11552,11604,11656,11708,11760,
    11812,11864,11916,11967,12019,12071,12123,12175,
    12227,12279,12331,12383,12436,12488,12540,12592,
    12644,12696,12748,12800,12853,12905,12957,13009,
    13062,13114,13166,13218,13271,13323,13375,13428,
    13480,13533,13585,13637,13690,13742,13795,13847,
    13900,13952,14005,14057,14110,14163,14215,14268,
    14321,14373,14426,14479,14531,14584,14637,14690,
    14743,14795,14848,14901,14954,15007,15060,15113,
    15166,15219,15272,15325,15378,15431,15484,15537,
    15590,15643,15696,15749,15802,15856,15909,15962,
    16015,16069,16122,16175,16229,16282,16335,16389,
    16442,16496,16549,16603,16656,16710,16763,16817,
    16870,16924,16977,17031,17085,17138,17192,17246,
    17300,17353,17407,17461,17515,17569,17623,17677,
    17731,17784,17838,17892,17946,18001,18055,18109,
    18163,18217,18271,18325,18380,18434,18488,18542,
    18597,18651,18705,18760,18814,18868,18923,18977,
    19032,19086,19141,19195,19250,19305,19359,19414,
    19469,19523,19578,19633,19688,19742,19797,19852,
    19907,19962,20017,20072,20127,20182,20237,20292,
    20347,20402,20457,20513,20568,20623,20678,20734,
    20789,20844,20900,20955,21010,21066,21121,21177,
    21232,21288,21343,21399,21455,21510,21566,21622,
    21678,21733,21789,21845,21901,21957,22013,22069,
    22125,22181,22237,22293,22349,22405,22461,22517,
    22573,22630,22686,22742,22799,22855,22911,22968,
    23024,23081,23137,23194,23250,23307,23364,23420,
    23477,23534,23591,23647,23704,23761,23818,23875,
    23932,23989,24046,24103,24160,24217,24274,24331,
    24389,24446,24503,24560,24618,24675,24732,24790,
    24847,24905,24962,25020,25078,25135,25193,25251,
    25308,25366,25424,25482,25540,25598,25656,25714,
    25772,25830,25888,25946,26004,26062,26120,26179,
    26237,26295,26354,26412,26471,26529,26588,26646,
    26705,26763,26822,26881,26940,26998,27057,27116,
    27175,27234,27293,27352,27411,27470,27529,27588,
    27647,27707,27766,27825,27884,27944,28003,28063,
    28122,28182,28241,28301,28361,28420,28480,28540,
    28600,28660,28719,28779,28839,28899,28959,29020,
    29080,29140,29200,29260,29321,29381,29441,29502,
    29562,29623,29683,29744,29805,29865,29926,29987,
    30048,30108,30169,30230,30291,30352,30413,30474,
    30536,30597,30658,30719,30781,30842,30904,30965,
    31026,31088,31150,31211,31273,31335,31396,31458,
    31520,31582,31644,31706,31768,31830,31892,31955,
    32017,32079,32141,32204,32266,32329,32391,32454,
    32516,32579,32642,32705,32767,32830,32893,32956,
    33019,33082,33145,33208,33272,33335,33398,33461,
    33525,33588,33652,33715,33779,33843,33906,33970,
    34034,34098,34162,34225,34289,34354,34418,34482,
    34546,34610,34675,34739,34803,34868,34932,34997,
    35062,35126,35191,35256,35321,35385,35450,35515,
    35580,35646,35711,35776,35841,35907,35972,36037,
    36103,36168,36234,36300,36365,36431,36497,36563,
    36629,36695,36761,36827,36893,36959,37026,37092,
    37158,37225,37291,37358,37425,37491,37558,37625,
    37692,37759,37826,37893,37960,38027,38094,38161,
    38229,38296,38364,38431,38499,38566,38634,38702,
    38770,38837,38905,38973,39042,39110,39178,39246,
    39314,39383,39451,39520,39588,39657,39726,39794,
    39863,39932,40001,40070,40139,40208,40278,40347,
    40416,40486,40555,40625,40694,40764,40834,40904,
    40973,41043,41113,41184,41254,41324,41394,41465,
    41535,41605,41676,41747,41817,41888,41959,42030,
    42101,42172,42243,42314,42385,42457,42528,42600,
    42671,42743,42814,42886,42958,43030,43102,43174,
    43246,43318,43390,43463,43535,43608,43680,43753,
    43826,43898,43971,44044,44117,44190,44263,44337,
    44410,44483,44557,44630,44704,44778,44851,44925,
    44999,45073,45147,45221,45296,45370,45444,45519,
    45593,45668,45743,45818,45892,45967,46042,46118,
    46193,46268,46343,46419,46494,46570,46646,46721,
    46797,46873,46949,47025,47102,47178,47254,47331,
    47407,47484,47560,47637,47714,47791,47868,47945,
    48022,48100,48177,48255,48332,48410,48488,48565,
    48643,48721,48799,48878,48956,49034,49113,49191,
    49270,49349,49427,49506,49585,49664,49744,49823,
    49902,49982,50061,50141,50221,50300,50380,50460,
    50540,50621,50701,50781,50862,50942,51023,51104,
    51185,51266,51347,51428,51509,51591,51672,51754,
    51835,51917,51999,52081,52163,52245,52327,52410,
    52492,52575,52657,52740,52823,52906,52989,53072,
    53156,53239,53322,53406,53490,53574,53657,53741,
    53826,53910,53994,54079,54163,54248,54333,54417,
    54502,54587,54673,54758,54843,54929,55015,55100,
    55186,55272,55358,55444,55531,55617,55704,55790,
    55877,55964,56051,56138,56225,56312,56400,56487,
    56575,56663,56751,56839,56927,57015,57104,57192,
    57281,57369,57458,57547,57636,57725,57815,57904,
    57994,58083,58173,58263,58353,58443,58534,58624,
    58715,58805,58896,58987,59078,59169,59261,59352,
    59444,59535,59627,59719,59811,59903,59996,60088,
    60181,60273,60366,60459,60552,60646,60739,60833,
    60926,61020,61114,61208,61302,61396,61491,61585,
    61680,61775,61870,61965,62060,62156,62251,62347,
    62443,62539,62635,62731,62828,62924,63021,63118,
    63215,63312,63409,63506,63604,63702,63799,63897,
    63996,64094,64192,64291,64389,64488,64587,64687,
    64786,64885,64985,65085,65185,65285,65385,65485
};

#if VIEWWINDOWWIDTH == 240
static const fixed_t __far finetangentTable_part_4[1024] =
#else
static const fixed_t finetangentTable_part_4[1024] =
#endif
{
    65586,65686,65787,65888,65989,66091,66192,66294,
    66396,66498,66600,66702,66804,66907,67010,67113,
    67216,67319,67422,67526,67629,67733,67837,67942,
    68046,68151,68255,68360,68465,68570,68676,68781,
    68887,68993,69099,69205,69312,69418,69525,69632,
    69739,69846,69954,70061,70169,70277,70385,70494,
    70602,70711,70820,70929,71038,71147,71257,71367,
    71477,71587,71697,71808,71918,72029,72140,72252,
    72363,72475,72587,72699,72811,72923,73036,73149,
    73262,73375,73488,73602,73715,73829,73944,74058,
    74172,74287,74402,74517,74633,74748,74864,74980,
    75096,75213,75329,75446,75563,75680,75797,75915,
    76033,76151,76269,76388,76506,76625,76744,76864,
    76983,77103,77223,77343,77463,77584,77705,77826,
    77947,78068,78190,78312,78434,78557,78679,78802,
    78925,79048,79172,79296,79420,79544,79668,79793,
    79918,80043,80168,80294,80420,80546,80672,80799,
    80925,81053,81180,81307,81435,81563,81691,81820,
    81949,82078,82207,82336,82466,82596,82726,82857,
    82987,83118,83250,83381,83513,83645,83777,83910,
    84043,84176,84309,84443,84576,84710,84845,84980,
    85114,85250,85385,85521,85657,85793,85930,86066,
    86204,86341,86479,86616,86755,86893,87032,87171,
    87310,87450,87590,87730,87871,88011,88152,88294,
    88435,88577,88720,88862,89005,89148,89292,89435,
    89579,89724,89868,90013,90158,90304,90450,90596,
    90742,90889,91036,91184,91332,91480,91628,91777,
    91926,92075,92225,92375,92525,92675,92826,92978,
    93129,93281,93434,93586,93739,93892,94046,94200,
    94354,94509,94664,94819,94975,95131,95287,95444,
    95601,95758,95916,96074,96233,96391,96551,96710,
    96870,97030,97191,97352,97513,97675,97837,98000,
    98163,98326,98489,98653,98818,98982,99148,99313,
    99479,99645,99812,99979,100146,100314,100482,100651,
    100820,100990,101159,101330,101500,101671,101843,102015,
    102187,102360,102533,102706,102880,103054,103229,103404,
    103580,103756,103933,104109,104287,104465,104643,104821,
    105000,105180,105360,105540,105721,105902,106084,106266,
    106449,106632,106816,107000,107184,107369,107555,107741,
    107927,108114,108301,108489,108677,108866,109055,109245,
    109435,109626,109817,110008,110200,110393,110586,110780,
    110974,111169,111364,111560,111756,111952,112150,112347,
    112546,112744,112944,113143,113344,113545,113746,113948,
    114151,114354,114557,114761,114966,115171,115377,115583,
    115790,115998,116206,116414,116623,116833,117044,117254,
    117466,117678,117891,118104,118318,118532,118747,118963,
    119179,119396,119613,119831,120050,120269,120489,120709,
    120930,121152,121374,121597,121821,122045,122270,122496,
    122722,122949,123176,123404,123633,123863,124093,124324,
    124555,124787,125020,125254,125488,125723,125959,126195,
    126432,126669,126908,127147,127387,127627,127869,128111,
    128353,128597,128841,129086,129332,129578,129825,130073,
    130322,130571,130821,131072,131324,131576,131830,132084,
    132339,132594,132851,133108,133366,133625,133884,134145,
    134406,134668,134931,135195,135459,135725,135991,136258,
    136526,136795,137065,137335,137607,137879,138152,138426,
    138701,138977,139254,139532,139810,140090,140370,140651,
    140934,141217,141501,141786,142072,142359,142647,142936,
    143226,143517,143808,144101,144395,144690,144986,145282,
    145580,145879,146179,146480,146782,147084,147388,147693,
    148000,148307,148615,148924,149235,149546,149859,150172,
    150487,150803,151120,151438,151757,152077,152399,152722,
    153045,153370,153697,154024,154352,154682,155013,155345,
    155678,156013,156349,156686,157024,157363,157704,158046,
    158389,158734,159079,159427,159775,160125,160476,160828,
    161182,161537,161893,162251,162610,162970,163332,163695,
    164060,164426,164793,165162,165532,165904,166277,166651,
    167027,167405,167784,168164,168546,168930,169315,169701,
    170089,170479,170870,171263,171657,172053,172451,172850,
    173251,173653,174057,174463,174870,175279,175690,176102,
    176516,176932,177349,177769,178190,178612,179037,179463,
    179891,180321,180753,181186,181622,182059,182498,182939,
    183382,183827,184274,184722,185173,185625,186080,186536,
    186995,187455,187918,188382,188849,189318,189789,190261,
    190736,191213,191693,192174,192658,193143,193631,194122,
    194614,195109,195606,196105,196606,197110,197616,198125,
    198636,199149,199664,200182,200703,201226,201751,202279,
    202809,203342,203878,204416,204956,205500,206045,206594,
    207145,207699,208255,208815,209376,209941,210509,211079,
    211652,212228,212807,213389,213973,214561,215151,215745,
    216341,216941,217544,218149,218758,219370,219985,220603,
    221225,221849,222477,223108,223743,224381,225022,225666,
    226314,226966,227621,228279,228941,229606,230275,230948,
    231624,232304,232988,233676,234367,235062,235761,236463,
    237170,237881,238595,239314,240036,240763,241493,242228,
    242967,243711,244458,245210,245966,246727,247492,248261,
    249035,249813,250596,251384,252176,252973,253774,254581,
    255392,256208,257029,257855,258686,259522,260363,261209,
    262060,262917,263779,264646,265519,266397,267280,268169,
    269064,269965,270871,271782,272700,273624,274553,275489,
    276430,277378,278332,279292,280258,281231,282210,283195,
    284188,285186,286192,287204,288223,289249,290282,291322,
    292369,293423,294485,295554,296630,297714,298805,299904,
    301011,302126,303248,304379,305517,306664,307819,308983,
    310154,311335,312524,313721,314928,316143,317368,318601,
    319844,321097,322358,323629,324910,326201,327502,328812,
    330133,331464,332805,334157,335519,336892,338276,339671,
    341078,342495,343924,345364,346816,348280,349756,351244,
    352744,354257,355783,357321,358872,360436,362013,363604,
    365208,366826,368459,370105,371765,373440,375130,376835,
    378555,380290,382040,383807,385589,387387,389202,391034,
    392882,394747,396630,398530,400448,402384,404338,406311,
    408303,410314,412344,414395,416465,418555,420666,422798,
    424951,427125,429321,431540,433781,436045,438332,440643,
    442978,445337,447720,450129,452564,455024,457511,460024,
    462565,465133,467730,470355,473009,475692,478406,481150,
    483925,486732,489571,492443,495348,498287,501261,504269,
    507313,510394,513512,516667,519861,523094,526366,529680,
    533034,536431,539870,543354,546881,550455,554074,557741,
    561456,565221,569035,572901,576818,580789,584815,588896,
    593033,597229,601483,605798,610174,614613,619117,623686,
    628323,633028,637803,642651,647572,652568,657640,662792,
    668024,673338,678737,684223,689797,695462,701219,707072,
    713023,719074,725227,731486,737853,744331,750922,757631,
    764460,771411,778490,785699,793041,800521,808143,815910,
    823827,831898,840127,848520,857081,865817,874730,883829,
    893117,902602,912289,922186,932298,942633,953199,964003,
    975054,986361,997931,1009774,1021901,1034322,1047046,1060087,
    1073455,1087164,1101225,1115654,1130465,1145673,1161294,1177345,
    1193846,1210813,1228269,1246234,1264730,1283783,1303416,1323658,
    1344537,1366084,1388330,1411312,1435065,1459630,1485049,1511367,
    1538632,1566898,1596220,1626658,1658278,1691149,1725348,1760956,
    1798063,1836758,1877161,1919378,1963536,2009771,2058233,2109087,
    2162516,2218719,2277919,2340362,2406322,2476104,2550052,2628549,
    2712030,2800983,2895966,2997613,3106651,3223918,3350381,3487165,
    3635590,3797206,3973855,4167737,4381502,4618375,4882318,5178251,
    5512368,5892567,6329090,6835455,7429880,8137527,8994149,10052327,
    11392683,13145455,15535599,18988036,24413316,34178904,56965752,170910304
};
