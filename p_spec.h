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
 * DESCRIPTION:  definitions, declarations and prototypes for specials
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_SPEC__
#define __P_SPEC__

#include "r_defs.h"
#include "d_player.h"

//      Define values for map objects
#define MO_TELEPORTMAN  14

// p_floor

#define ELEVATORSPEED (FRACUNIT*4)
#define FLOORSPEED     FRACUNIT

// p_ceilng

#define CEILSPEED   FRACUNIT
#define CEILWAIT    150

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
#define MAXSWITCHES		50


// 1 second, in ticks.
#define BUTTONTIME  TICRATE

// p_lights

#define GLOWSPEED       8
#define STROBEBRIGHT    5
#define FASTDARK        15
#define SLOWDARK        35

//jff 3/14/98 add bits and shifts for generalized sector types

#define DAMAGE_MASK     0x60
#define DAMAGE_SHIFT    5
#define SECRET_MASK     0x80
#define SECRET_SHIFT    7
#define FRICTION_MASK   0x100
#define FRICTION_SHIFT  8
#define PUSH_MASK       0x200
#define PUSH_SHIFT      9

//jff 02/04/98 Define masks, shifts, for fields in
// generalized linedef types

#define GenEnd                0x8000
#define GenFloorBase          0x6000
#define GenCeilingBase        0x4000
#define GenDoorBase           0x3c00
#define GenLockedBase         0x3800
#define GenLiftBase           0x3400
#define GenStairsBase         0x3000
#define GenCrusherBase        0x2F80

#define TriggerType           0x0007
#define TriggerTypeShift      0

// define masks and shifts for the floor type fields

#define FloorCrush            0x1000
#define FloorChange           0x0c00
#define FloorTarget           0x0380
#define FloorDirection        0x0040
#define FloorModel            0x0020
#define FloorSpeed            0x0018

#define FloorCrushShift           12
#define FloorChangeShift          10
#define FloorTargetShift           7
#define FloorDirectionShift        6
#define FloorModelShift            5
#define FloorSpeedShift            3

// define masks and shifts for the ceiling type fields

#define CeilingCrush          0x1000
#define CeilingChange         0x0c00
#define CeilingTarget         0x0380
#define CeilingDirection      0x0040
#define CeilingModel          0x0020
#define CeilingSpeed          0x0018

#define CeilingCrushShift         12
#define CeilingChangeShift        10
#define CeilingTargetShift         7
#define CeilingDirectionShift      6
#define CeilingModelShift          5
#define CeilingSpeedShift          3

// define masks and shifts for the lift type fields

#define LiftTarget            0x0300
#define LiftDelay             0x00c0
#define LiftMonster           0x0020
#define LiftSpeed             0x0018

#define LiftTargetShift            8
#define LiftDelayShift             6
#define LiftMonsterShift           5
#define LiftSpeedShift             3

// define masks and shifts for the stairs type fields

#define StairIgnore           0x0200
#define StairDirection        0x0100
#define StairStep             0x00c0
#define StairMonster          0x0020
#define StairSpeed            0x0018

#define StairIgnoreShift           9
#define StairDirectionShift        8
#define StairStepShift             6
#define StairMonsterShift          5
#define StairSpeedShift            3

// define masks and shifts for the crusher type fields

#define CrusherSilent         0x0040
#define CrusherMonster        0x0020
#define CrusherSpeed          0x0018

#define CrusherSilentShift         6
#define CrusherMonsterShift        5
#define CrusherSpeedShift          3

// define masks and shifts for the door type fields

#define DoorDelay             0x0300
#define DoorMonster           0x0080
#define DoorKind              0x0060
#define DoorSpeed             0x0018

#define DoorDelayShift             8
#define DoorMonsterShift           7
#define DoorKindShift              5
#define DoorSpeedShift             3

// define masks and shifts for the locked door type fields

#define LockedNKeys           0x0200
#define LockedKey             0x01c0
#define LockedKind            0x0020
#define LockedSpeed           0x0018

#define LockedNKeysShift           9
#define LockedKeyShift             6
#define LockedKindShift            5
#define LockedSpeedShift           3

// define names for the TriggerType field of the general linedefs

typedef enum
{
  WalkOnce,
  WalkMany,
  SwitchOnce,
  SwitchMany,
  GunOnce,
  GunMany,
  PushOnce,
  PushMany,
} triggertype_e;

// define names for the Speed field of the general linedefs

typedef enum
{
  SpeedSlow,
  SpeedNormal,
  SpeedFast,
  SpeedTurbo,
} motionspeed_e;

// define names for the Target field of the general floor

typedef enum
{
  FtoHnF,
  FtoLnF,
  FtoNnF,
  FtoLnC,
  FtoC,
  FbyST,
  Fby24,
  Fby32,
} floortarget_e;

// define names for the Changer Type field of the general floor

typedef enum
{
  FNoChg,
  FChgZero,
  FChgTxt,
  FChgTyp,
} floorchange_e;

// define names for the Change Model field of the general floor

typedef enum
{
  FTriggerModel,
  FNumericModel,
} floormodel_t;

// define names for the Target field of the general ceiling

typedef enum
{
  CtoHnC,
  CtoLnC,
  CtoNnC,
  CtoHnF,
  CtoF,
  CbyST,
  Cby24,
  Cby32,
} ceilingtarget_e;

// define names for the Changer Type field of the general ceiling

typedef enum
{
  CNoChg,
  CChgZero,
  CChgTxt,
  CChgTyp,
} ceilingchange_e;

// define names for the Change Model field of the general ceiling

typedef enum
{
  CTriggerModel,
  CNumericModel,
} ceilingmodel_t;

// define names for the Target field of the general lift

typedef enum
{
  F2LnF,
  F2NnF,
  F2LnC,
  LnF2HnF,
} lifttarget_e;

// define names for the door Kind field of the general ceiling

typedef enum
{
  OdCDoor,
  ODoor,
  CdODoor,
  CDoor,
} doorkind_e;

// define names for the locked door Kind field of the general ceiling

typedef enum
{
  AnyKey,
  RCard,
  BCard,
  YCard,
  RSkull,
  BSkull,
  YSkull,
  AllKeys,
} keykind_e;

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
  lighting_special,
} special_e;

//jff 3/15/98 pure texture/type change for better generalized support
typedef enum
{
  trigChangeOnly,
  numChangeOnly,
} change_e;

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

  //jff 02/03/98 lower floor 24 absolute
  lowerFloor24,

  //jff 02/03/98 lower floor 32 absolute
  lowerFloor32Turbo,

  // raise floor to shortest height texture around it
  raiseToTexture,

  // lower floor to lowest surrounding floor
  //  and change floorpic
  lowerAndChange,

  raiseFloor24,

  //jff 02/03/98 raise floor 32 absolute
  raiseFloor32Turbo,

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

// switch animation structure type

typedef PACKEDATTR_PRE struct
{
  char name1[9];
  char name2[9];
  int16_t episode;
} PACKEDATTR_POST switchlist_t; //jff 3/23/98 pack to read from memory

typedef struct
{
  const line_t __far* line;
  bwhere_e where;
  int32_t   btexture;
  int32_t   btimer;
  degenmobj_t __far* soundorg;

} button_t;

// p_lights

typedef struct
{
  thinker_t thinker;
  sector_t __far* sector;
  int32_t count;
  int32_t maxlight;
  int32_t minlight;

} fireflicker_t;


typedef struct
{
  thinker_t thinker;
  sector_t __far* sector;
  int32_t minlight;
  int32_t maxlight;
  int32_t direction;

} glow_t;

// p_plats

typedef struct
{
  thinker_t thinker;
  sector_t __far* sector;
  fixed_t speed;
  fixed_t low;
  fixed_t high;
  int32_t wait;
  int32_t count;
  plat_e status;
  plat_e oldstatus;
  boolean crush;
  int32_t tag;
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
  int32_t direction;

  // tics to wait at the top
  int32_t topwait;
  // (keep in case a door going down is reset)
  // when it reaches 0, start going down
  int32_t topcountdown;

  //jff 1/31/98 keep track of line door is triggered by
  const line_t __far* line;

  /* killough 10/98: sector tag for gradual lighting effects */
  int32_t lighttag;
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
  int32_t newspecial;
  int32_t oldspecial; //jff 3/14/98 add to fix bug in change transfers
  int16_t texture;

  // 1 = up, 0 = waiting, -1 = down
  int32_t direction;

  // ID
  int32_t tag;
  int32_t olddirection;
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
  int32_t direction;
  int32_t newspecial;
  int32_t oldspecial;   //jff 3/14/98 add to fix bug in change transfers
  int16_t texture;
  fixed_t floordestheight;
  fixed_t speed;

} floormove_t;

typedef struct
{
  thinker_t thinker;
  elevator_e type;
  sector_t __far* sector;
  int32_t direction;
  fixed_t floordestheight;
  fixed_t ceilingdestheight;
  fixed_t speed;
} elevator_t;

// p_spec

// killough 3/7/98: Add generalized scroll effects

typedef struct {
  thinker_t thinker;   // Thinker structure for scrolling
  int32_t affectee;        // Number of affected sidedef, sector, tag, or whatever
} scroll_t;

// phares 3/12/98: added new model of friction for ice/sludge effects

typedef struct {
  //thinker_t thinker;   // Thinker structure for friction
  int32_t friction;        // friction value (E800 = normal)
  //int32_t movefactor;      // inertia factor when adding to momentum
  int32_t affectee;        // Number of affected sector
} friction_t;

// phares 3/20/98: added new model of Pushers for push/pull effects

typedef struct {
  thinker_t thinker;   // Thinker structure for Pusher
  enum
  {
    p_push,
    p_pull,
    p_wind,
    p_current,
  } type;
  mobj_t* source;      // Point source if point pusher
  int32_t x_mag;           // X Strength
  int32_t y_mag;           // Y Strength
  int32_t magnitude;       // Vector strength for point pusher
  int32_t radius;          // Effective radius for point pusher
  int32_t x;               // X of point source if point pusher
  int32_t y;               // Y of point source if point pusher
  int32_t affectee;        // Number of affected sector
} pusher_t;


////////////////////////////////////////////////////////////////
//
// Linedef and sector special utility function prototypes
//
////////////////////////////////////////////////////////////////

boolean twoSided(int16_t sector, int16_t line);

side_t __far* getSide(int16_t currentSector, int16_t line, int16_t side);

fixed_t P_FindLowestFloorSurrounding(sector_t __far* sec);

fixed_t P_FindHighestFloorSurrounding(sector_t __far* sec);

fixed_t P_FindNextHighestFloor(sector_t __far* sec);

fixed_t P_FindNextLowestFloor(sector_t __far* sec, fixed_t currentheight);

fixed_t P_FindLowestCeilingSurrounding(sector_t __far* sec);

fixed_t P_FindHighestCeilingSurrounding(sector_t __far* sec);

sector_t __far* P_FindModelFloorSector(fixed_t floordestheight, int16_t secnum);

int16_t P_FindSectorFromLineTag(const line_t __far* line, int16_t start);

sector_t __far* getNextSector(const line_t __far* line, sector_t __far* sec);

boolean P_CheckTag(const line_t __far* line);

boolean P_CanUnlockGenDoor(const line_t __far* line, player_t* player);

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

void T_MoveFloor(floormove_t __far* floor);

////////////////////////////////////////////////////////////////
//
// Linedef and sector special handler prototypes
//
////////////////////////////////////////////////////////////////

// p_telept

boolean EV_Teleport(const line_t __far* line, int16_t side, mobj_t __far* thing);

// killough 2/14/98: Add silent teleporter
boolean EV_SilentTeleport(const line_t __far* line, int16_t side, mobj_t __far* thing);

// p_floor

boolean EV_DoElevator(const line_t __far* line, elevator_e type);

boolean EV_BuildStairs(const line_t __far* line, stair_e type);

boolean EV_DoFloor(const line_t __far* line, floor_e floortype);

// p_ceilng

boolean EV_DoCeiling(const line_t __far* line, ceiling_e type);

boolean EV_CeilingCrushStop(const line_t __far* line);

// p_doors

boolean EV_DoDoor(const line_t __far* line, vldoor_e type);

// p_lights

void EV_StartLightStrobing(const line_t __far* line);

void EV_TurnTagLightsOff(const line_t __far* line);

void EV_LightTurnOn(const line_t __far* line, int32_t bright);

// p_floor

boolean EV_DoChange(const line_t __far* line, change_e changetype);

boolean EV_DoDonut(const line_t __far* line);

// p_plats

boolean EV_DoPlat(const line_t __far* line, plattype_e type, int32_t amount);

void EV_StopPlat(const line_t __far* line);

// p_genlin

boolean EV_DoGenFloor(const line_t __far* line);

boolean EV_DoGenCeiling(const line_t __far* line);

boolean EV_DoGenLift(const line_t __far* line);

boolean EV_DoGenStairs(const line_t __far* line);

boolean EV_DoGenCrusher(const line_t __far* line);

boolean EV_DoGenDoor(const line_t __far* line);

boolean EV_DoGenLockedDoor(const line_t __far* line);

////////////////////////////////////////////////////////////////
//
// Linedef and sector special thinker spawning
//
////////////////////////////////////////////////////////////////

// at game start
void P_InitPicAnims(void);
void P_InitAnimatedFlat(void);

void P_InitSwitchList
( void );

// at map load
void P_SpawnSpecials
( void );

// every tic
void P_UpdateSpecials(void);
void P_UpdateAnimatedFlat(void);

// when needed
boolean P_UseSpecialLine(mobj_t __far* thing, const line_t __far* line, int16_t side);

// p_lights

void P_SpawnFireFlicker(sector_t __far* sector);

void P_SpawnLightFlash(sector_t __far* sector);

void P_SpawnStrobeFlash(sector_t __far* sector, int16_t fastOrSlow, boolean inSync);

void P_SpawnGlowingLight(sector_t __far* sector);

// p_plats

void P_AddActivePlat(plat_t __far* plat);

void P_RemoveAllActivePlats
( void );    // killough

void P_ActivateInStasis
( int32_t tag );

// p_doors

void P_SpawnDoorCloseIn30(sector_t __far* sec);

void P_SpawnDoorRaiseIn5Mins(sector_t __far* sec);

// p_ceilng

void P_RemoveAllActiveCeilings
( void );                //jff 2/22/98

void P_AddActiveCeiling(ceiling_t __far* c);

boolean P_ActivateInStasisCeiling(const line_t __far* line);

#endif
