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

//jff 3/14/98 add bits and shifts for generalized sector types

#define DAMAGE_MASK     0x60
#define DAMAGE_SHIFT    5
#define SECRET_MASK     0x80


//////////////////////////////////////////////////////////////////
//
// enums for classes of linedef triggers
//
//////////////////////////////////////////////////////////////////

//jff 2/23/98 identify the special classes that can share sectors

typedef enum
{
  floor_special,
  ceiling_special,
} special_e;


// p_plats

typedef enum
{
  up,
  down,
  waiting,
  in_stasis
} plat_e;

typedef enum
{
  perpetualRaise,
  downWaitUpStay,
  raiseAndChange,
  raiseToNearestAndChange,
  blazeDWUS,
  genLift,      //jff added to support generalized Plat types
  genPerpetual,
  toggleUpDn,   //jff 3/14/98 added to support instant toggle type

} plattype_e;

// p_doors

typedef enum
{
  normal,
  close30ThenOpen,
  dclose,
  dopen,
  raiseIn5Mins,
  blazeRaise,
  blazeOpen,
  blazeClose,

  //jff 02/05/98 add generalize door types
  genRaise,
  genBlazeRaise,
  genOpen,
  genBlazeOpen,
  genClose,
  genBlazeClose,
  genCdO,
  genBlazeCdO,
} vldoor_e;

// p_ceilng

typedef enum
{
  lowerToFloor,
  raiseToHighest,
  lowerToLowest,
  lowerToMaxFloor,
  lowerAndCrush,
  crushAndRaise,
  fastCrushAndRaise,
  silentCrushAndRaise,

  //jff 02/04/98 add types for generalized ceiling mover
  genCeiling,
  genCeilingChg,
  genCeilingChg0,
  genCeilingChgT,

  //jff 02/05/98 add types for generalized ceiling mover
  genCrusher,
  genSilentCrusher,

} ceiling_e;

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

  //jff 02/03/98 lower floor to next lowest neighbor
  lowerFloorToNearest,

  // raise floor to shortest height texture around it
  raiseToTexture,

  // lower floor to lowest surrounding floor
  //  and change floorpic
  lowerAndChange,

  raiseFloor24,

  raiseFloor24AndChange,
  raiseFloorCrush,

  // raise to next highest floor, turbo-speed
  raiseFloorTurbo,
  donutRaise,
  raiseFloor512,

  //jff 02/04/98  add types for generalized floor mover
  genFloor,
  genFloorChg,
  genFloorChg0,
  genFloorChgT,

  //new types for stair builders
  buildStair,
  genBuildStair,
} floor_e;

typedef enum
{
  build8, // slowly build by 8
  turbo16 // quickly build by 16

} stair_e;

typedef enum
{
  elevateUp,
  elevateDown,
  elevateCurrent,
} elevator_e;

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
  plat_e oldstatus;
  boolean crush;
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

// p_doors

typedef struct
{
  thinker_t thinker;
  ceiling_e type;
  sector_t __far* sector;
  fixed_t bottomheight;
  fixed_t topheight;
  fixed_t speed;
  fixed_t oldspeed;
  boolean crush;

  //jff 02/04/98 add these to support ceiling changers
  int16_t newspecial;
  int16_t oldspecial; //jff 3/14/98 add to fix bug in change transfers
  int16_t texture;

  // 1 = up, 0 = waiting, -1 = down
  int16_t direction;

  // ID
  int16_t tag;
  int16_t olddirection;
  struct ceilinglist __far* list;
} ceiling_t;

typedef struct ceilinglist {
  ceiling_t __far* ceiling;
  struct ceilinglist __far* next;
  struct ceilinglist __far*__far* prev;
} ceilinglist_t;

// p_floor

typedef struct
{
  thinker_t thinker;
  floor_e type;
  boolean crush;
  sector_t __far* sector;
  int16_t direction;
  int16_t newspecial;
  int16_t oldspecial;   //jff 3/14/98 add to fix bug in change transfers
  int16_t texture;
  fixed_t floordestheight;
  fixed_t speed;

} floormove_t;


////////////////////////////////////////////////////////////////
//
// Linedef and sector special utility function prototypes
//
////////////////////////////////////////////////////////////////

fixed_t P_FindLowestFloorSurrounding(sector_t __far* sec);

fixed_t P_FindHighestFloorSurrounding(sector_t __far* sec);

fixed_t P_FindNextHighestFloor(sector_t __far* sec);

fixed_t P_FindNextLowestFloor(sector_t __far* sec, fixed_t currentheight);

fixed_t P_FindLowestCeilingSurrounding(sector_t __far* sec);

int16_t P_FindSectorFromLineTag(const line_t __far* line, int16_t start);

sector_t __far* getNextSector(const line_t __far* line, sector_t __far* sec);

boolean P_CheckTag(const line_t __far* line);

boolean P_SectorActive(special_e t, const sector_t __far* s);


void P_ChangeSwitchTexture(const line_t __far* line, boolean useAgain);

////////////////////////////////////////////////////////////////
//
// Linedef and sector special action function prototypes
//
////////////////////////////////////////////////////////////////

// p_plats

void T_PlatRaise(plat_t __far* plat);

// p_doors

void T_VerticalDoor(vldoor_t __far* door);

// p_ceilng

void T_MoveCeiling(ceiling_t __far* ceiling);

// p_floor

result_e T_MovePlane(sector_t __far* sector, fixed_t speed, fixed_t dest, boolean crush, int16_t floorOrCeiling, int16_t direction);


////////////////////////////////////////////////////////////////
//
// Linedef and sector special handler prototypes
//
////////////////////////////////////////////////////////////////

// p_telept

boolean EV_Teleport(const line_t __far* line, int16_t side, mobj_t __far* thing);


// p_floor

boolean EV_BuildStairs(const line_t __far* line, stair_e type);

boolean EV_DoFloor(const line_t __far* line, floor_e floortype);

// p_doors

boolean EV_DoDoor(const line_t __far* line, vldoor_e type);

// p_lights

void EV_LightTurnOn(const line_t __far* line, int16_t bright);

// p_floor

boolean EV_DoDonut(const line_t __far* line);

// p_plats

boolean EV_DoPlat(const line_t __far* line, plattype_e type, int16_t amount);


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
boolean P_UseSpecialLine(mobj_t __far* thing, const line_t __far* line);

// p_lights

void P_SpawnLightFlash(sector_t __far* sector);

void P_SpawnStrobeFlash(sector_t __far* sector, int16_t fastOrSlow, boolean inSync);

void P_SpawnGlowingLight(sector_t __far* sector);

// p_plats

void P_RemoveAllActivePlats
( void );    // killough


// p_ceilng

void P_RemoveAllActiveCeilings
( void );                //jff 2/22/98

#endif
