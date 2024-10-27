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
 *  Player state structure.
 *
 *-----------------------------------------------------------------------------*/


#ifndef __D_PLAYER__
#define __D_PLAYER__


// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).
#include "d_items.h"
#include "p_pspr.h"

// In addition, the player is just a special
// case of the generic moving object/actor.
#include "p_mobj.h"

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.
#include "d_ticcmd.h"


//
// Player states.
//
typedef enum
{
  // Playing or camping.
  PST_LIVE,
  // Dead on the ground, view follows killer.
  PST_DEAD,
  // Ready to restart/respawn???
  PST_REBORN

} playerstate_t;


//
// Player internal flags, for cheats and debug.
//
typedef enum
{
  // No clipping, walk through barriers.
  CF_NOCLIP           = 1,
  // No damage, no health loss.
  CF_GODMODE          = 2,
  //You played goldeneye right?
  CF_ENEMY_ROCKETS    = 4

} cheat_t;


//
// Extended player object info: player_t
//
typedef struct player_s
{
  mobj_t __far*             mo;
  playerstate_t       playerstate;
  ticcmd_t            cmd;

  // Determine POV,
  //  including viewpoint bobbing during movement.
  // Focal origin above r.z
  fixed_t             viewz;
  // Base height above floor for viewz.
  fixed_t             viewheight;
  // Bob/squat speed.
  fixed_t             deltaviewheight;
  // bounded/scaled total momentum.
  fixed_t             bob;

  /* killough 10/98: used for realistic bobbing (i.e. not simply overall speed)
   * mo->momx and mo->momy represent true momenta experienced by player.
   * This only represents the thrust that the player applies himself.
   * This avoids anomolies with such things as Boom ice and conveyors.
   */
  fixed_t            momx, momy;      // killough 10/98

  // This is only used between levels,
  // mo->health is used during levels.
  int16_t                 health;
  int16_t                 armorpoints;
  // Armor type is 0-2.
  int16_t                 armortype;

  // Power ups. invinc and invis are tic counters.
  int16_t                 powers[NUMPOWERS];
  boolean             cards[NUMCARDS];
  boolean             backpack;

  // Frags, kills of other players.
  weapontype_t        readyweapon;

  // Is wp_nochange if not changing.
  weapontype_t        pendingweapon;

  int16_t                 weaponowned[NUMWEAPONS];
  int16_t                 ammo[NUMAMMO];
  int16_t                 maxammo[NUMAMMO];

  // True if button down last tic.
  boolean                 attackdown;
  boolean                 usedown;

  // Bit flags, for cheats and debug.
  // See cheat_t, above.
  int16_t                 cheats;

  // Refired shots are less accurate.
  int16_t                 refire;

   // For intermission stats.
  int16_t                 killcount;
  int16_t                 itemcount;
  int16_t                 secretcount;

  // Hint messages. // CPhipps - const
  const char*         message;

  // For screen flashing (red or bright).
  int16_t                 damagecount;
  int16_t                 bonuscount;

  // Who did damage (NULL for floors/ceilings).
  mobj_t __far*             attacker;

  // So gun flashes light up areas.
  int16_t                 extralight;

  // Current PLAYPAL, ???
  //  can be set to REDCOLORMAP for pain, etc.
  int16_t                 fixedcolormap;

  // Overlay view sprites (gun, etc).
  pspdef_t            psprites[NUMPSPRITES];

  // True if secret level has been done.
  boolean             didsecret;

} player_t;


//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
typedef struct
{
  // Player stats, kills, collected items etc.
  int32_t         skills;
  int32_t         sitems;
  int32_t         ssecret;
  int32_t         stime;
} wbplayerstruct_t;

typedef struct
{
  // if true, splash the secret level
  boolean     didsecret;

  // previous and next levels, origin 0
  int16_t         last;
  int16_t         next;

  int32_t         maxkills;
  int32_t         maxitems;
  int32_t         maxsecret;

  // the par time
  int16_t         partime;

  wbplayerstruct_t    plyr[MAXPLAYERS];

  // CPhipps - total game time for completed levels so far
  int32_t         totaltimes;

} wbstartstruct_t;


#endif
