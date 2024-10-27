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
 * DESCRIPTION:  definitions, declarations and prototypes for specials
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_SPEC__
#define __P_SPEC__

#include "r_defs.h"
#include "d_player.h"


// p_floor

#define FLOORSPEED     FRACUNIT

// p_ceilng

#define CEILSPEED   FRACUNIT


// p_doors

#define VDOORSPEED  (FRACUNIT*2)
#define VDOORWAIT   150

// p_plats

#define PLATWAIT    3
#define PLATSPEED   FRACUNIT

// p_switch

// 4 players, 4 buttons each at once, max.
// killough 2/14/98: redefine in terms of MAXPLAYERS
#define MAXBUTTONS    (MAXPLAYERS*4)


// p_lights

#define GLOWSPEED       8
#define STROBEBRIGHT    5
#define FASTDARK        15
#define SLOWDARK        35


//////////////////////////////////////////////////////////////////
//
// enums for classes of linedef triggers
//
//////////////////////////////////////////////////////////////////

// p_plats

typedef enum
{
  up,
  down,
  waiting,
} plat_e;

typedef enum
{
  downWaitUpStay,
  raiseToNearestAndChange,
} plattype_e;

// p_doors

typedef enum
{
  normal,
  close30ThenOpen,
  dopen,
} vldoor_e;


// p_floor

typedef enum
{
  // lower floor to highest surrounding floor
  lowerFloor,

  // lower floor to lowest surrounding floor
  lowerFloorToLowest,

  // lower floor to highest surrounding floor VERY FAST
  turboLower,

  // raise floor to lowest surrounding CEILING
  raiseFloor,

  // raise floor to next highest surrounding floor
  raiseFloorToNearest,

  donutRaise,

  buildStair,
} floor_e;


//////////////////////////////////////////////////////////////////
//
// general enums
//
//////////////////////////////////////////////////////////////////

// texture type enum
typedef enum
{
    top,
    middle,
    bottom

} bwhere_e;

// crush check returns
typedef enum
{
  ok,
  crushed,
  pastdest
} result_e;

//////////////////////////////////////////////////////////////////
//
// linedef and sector special data types
//
//////////////////////////////////////////////////////////////////

// p_switch

typedef struct
{
  const line_t __far* line;
  bwhere_e where;
  int16_t   btexture;
  int16_t   btimer;
  degenmobj_t __far* soundorg;

} button_t;

// p_plats

typedef struct
{
  thinker_t thinker;
  sector_t __far* sector;
  fixed_t speed;
  fixed_t low;
  fixed_t high;
  int16_t wait;
  int16_t count;
  plat_e status;
  int16_t tag;
  plattype_e type;

  struct platlist __far* list;
} plat_t;


// p_ceilng

typedef struct
{
  thinker_t thinker;
  vldoor_e type;
  sector_t __far* sector;
  fixed_t topheight;
  fixed_t speed;

  // 1 = up, 0 = waiting at top, -1 = down
  int16_t direction;

  // tics to wait at the top
  int16_t topwait;
  // (keep in case a door going down is reset)
  // when it reaches 0, start going down
  int16_t topcountdown;

  //jff 1/31/98 keep track of line door is triggered by
  const line_t __far* line;

  /* killough 10/98: sector tag for gradual lighting effects */
  int16_t lighttag;
} vldoor_t;


////////////////////////////////////////////////////////////////
//
// Linedef and sector special utility function prototypes
//
////////////////////////////////////////////////////////////////

fixed_t P_FindLowestFloorSurrounding(sector_t __far* sec);

fixed_t P_FindHighestFloorSurrounding(sector_t __far* sec);

fixed_t P_FindNextHighestFloor(sector_t __far* sec);

fixed_t P_FindLowestCeilingSurrounding(sector_t __far* sec);

int16_t P_FindSectorFromLineTag(const line_t __far* line, int16_t start);

sector_t __far* getNextSector(const line_t __far* line, sector_t __far* sec);

boolean P_CheckTag(const line_t __far* line);

void P_ChangeSwitchTexture(line_t __far* line, boolean useAgain);

////////////////////////////////////////////////////////////////
//
// Linedef and sector special action function prototypes
//
////////////////////////////////////////////////////////////////

// p_plats

void T_PlatRaise(plat_t __far* plat);

// p_doors

void T_VerticalDoor(vldoor_t __far* door);

// p_floor

result_e T_MovePlaneFloor  (sector_t __far* sector, fixed_t speed, fixed_t dest, int16_t direction);
result_e T_MovePlaneCeiling(sector_t __far* sector, fixed_t speed, fixed_t dest, int16_t direction);


////////////////////////////////////////////////////////////////
//
// Linedef and sector special handler prototypes
//
////////////////////////////////////////////////////////////////

// p_telept

boolean EV_Teleport(const line_t __far* line, int16_t side, mobj_t __far* thing);


// p_floor

boolean EV_BuildStairs(const line_t __far* line);

boolean EV_DoFloor(const line_t __far* line, floor_e floortype);

// p_doors

boolean EV_DoDoor(const line_t __far* line, vldoor_e type);

// p_lights

void EV_LightTurnOn(const line_t __far* line, int16_t bright);

// p_floor

boolean EV_DoDonut(const line_t __far* line);

// p_plats

boolean EV_DoPlat(const line_t __far* line, plattype_e type);


////////////////////////////////////////////////////////////////
//
// Linedef and sector special thinker spawning
//
////////////////////////////////////////////////////////////////

// at game start
void P_InitPicAnims(void);

void P_InitSwitchList
( void );

// at map load
void P_SpawnSpecials
( void );

// every tic
void P_UpdateSpecials(void);
void P_UpdateAnimatedFlat(void);

// when needed
boolean P_UseSpecialLine(mobj_t __far* thing, line_t __far* line);

// p_lights

void P_SpawnLightFlash(sector_t __far* sector);

void P_SpawnStrobeFlash(sector_t __far* sector, int16_t fastOrSlow, boolean inSync);

void P_SpawnGlowingLight(sector_t __far* sector);

// p_plats

void P_RemoveAllActivePlats
( void );    // killough


#endif
