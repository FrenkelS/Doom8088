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
  // Not really a cheat, just a debug aid.
  CF_NOMOMENTUM       = 4,

  //You played goldeneye right?
  CF_ENEMY_ROCKETS    = 8

} cheat_t;


//
// Extended player object info: player_t
//
typedef struct player_s
{
  mobj_t*             mo;
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
  int32_t                 health;
  int32_t                 armorpoints;
  // Armor type is 0-2.
  int32_t                 armortype;

  // Power ups. invinc and invis are tic counters.
  int32_t                 powers[NUMPOWERS];
  boolean             cards[NUMCARDS];
  boolean             backpack;

  // Frags, kills of other players.
  weapontype_t        readyweapon;

  // Is wp_nochange if not changing.
  weapontype_t        pendingweapon;

  int32_t                 weaponowned[NUMWEAPONS];
  int32_t                 ammo[NUMAMMO];
  int32_t                 maxammo[NUMAMMO];

  // True if button down last tic.
  int32_t                 attackdown;
  int32_t                 usedown;

  // Bit flags, for cheats and debug.
  // See cheat_t, above.
  int32_t                 cheats;

  // Refired shots are less accurate.
  int32_t                 refire;

   // For intermission stats.
  int32_t                 killcount;
  int32_t                 itemcount;
  int32_t                 secretcount;

  // Hint messages. // CPhipps - const
  const char*         message;

  // For screen flashing (red or bright).
  int32_t                 damagecount;
  int32_t                 bonuscount;

  // Who did damage (NULL for floors/ceilings).
  mobj_t __far*             attacker;

  // So gun flashes light up areas.
  int32_t                 extralight;

  // Current PLAYPAL, ???
  //  can be set to REDCOLORMAP for pain, etc.
  int32_t                 fixedcolormap;

  // Player skin colorshift,
  //  0-3 for which color to draw player.
  int32_t                 colormap;

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
  boolean     in;     // whether the player is in game

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
  int32_t         last;
  int32_t         next;

  int32_t         maxkills;
  int32_t         maxitems;
  int32_t         maxsecret;

  // the par time
  int32_t         partime;

  // index of this player in game
  int32_t         pnum;

  wbplayerstruct_t    plyr[MAXPLAYERS];

  // CPhipps - total game time for completed levels so far
  int32_t         totaltimes;

} wbstartstruct_t;


#endif
