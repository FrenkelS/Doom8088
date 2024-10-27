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
 *  Internally used data structures for virtually everything,
 *   key definitions, lots of other stuff.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __DOOMDEF__
#define __DOOMDEF__

/* use config.h if autoconf made one -- josh */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>

// This must come first, since it redefines malloc(), free(), etc. -- killough:
#include "z_zone.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "m_swap.h"


#define UNUSED(x)	(x = x)	// for pesky compiler / lint warnings


#define LOBYTE(w)	(((uint8_t *)&w)[0])
#define HIBYTE(w)	(((uint8_t *)&w)[1])


#if !defined VIEWWINDOWWIDTH
#define VIEWWINDOWWIDTH  60
#endif

#if !defined VIEWWINDOWHEIGHT
#define VIEWWINDOWHEIGHT 128
#endif


// SCREENWIDTH and SCREENHEIGHT define the visible size
#define SCREENWIDTH  240u
#define SCREENHEIGHT 160 /*(VIEWWINDOWHEIGHT+ST_HEIGHT)*/

#define SCREENWIDTH_VGA  320
#define SCREENHEIGHT_VGA 200


// The maximum number of players, multiplayer/networking.
#define MAXPLAYERS       1


// State updates, number of tics / second.
#define TICRATE          35


int16_t M_CheckParm(char *check);


// The current state of the game: whether we are playing, gazing
// at the intermission screen, the game final animation, or a demo.

typedef enum {
  GS_LEVEL,
  GS_INTERMISSION,
  GS_FINALE,
  GS_DEMOSCREEN
} gamestate_t;

//
// Difficulty/skill settings/filters.
//

typedef enum {
  sk_none=-1, //jff 3/24/98 create unpicked skill setting
  sk_baby=0,
  sk_easy,
  sk_medium,
  sk_hard,
  sk_nightmare
} skill_t;

//
// Key cards.
//

typedef enum {
  it_bluecard,
  it_yellowcard,
  it_redcard,
  NUMCARDS
} card_t;

// The defined weapons, including a marker
// indicating user has not changed weapon.
typedef enum {
  wp_fist,
  wp_pistol,
  wp_shotgun,
  wp_chaingun,
  wp_missile,
  wp_plasma,
  wp_bfg,
  wp_chainsaw,
  wp_supershotgun,

  NUMWEAPONS,
  wp_nochange              // No pending weapon change.
} weapontype_t;

// Ammunition types defined.
typedef enum {
  am_clip,    // Pistol / chaingun ammo.
  am_shell,   // Shotgun / double barreled shotgun.
  am_misl,    // Missile launcher.
  am_cell,    // Plasma rifle, BFG.
  NUMAMMO,
  am_noammo   // Unlimited for chainsaw / fist.
} ammotype_t;

// Power up artifacts.
typedef enum {
  pw_invulnerability,
  pw_strength,
  pw_invisibility,
  pw_ironfeet,
  pw_allmap,
  pw_infrared,
  NUMPOWERS
} powertype_t;

// Power up durations (how many seconds till expiration).
typedef enum {
  INVULNTICS  = (30*TICRATE),
  INVISTICS   = (60*TICRATE),
  INFRATICS   = (120*TICRATE),
  IRONTICS    = (60*TICRATE)
} powerduration_t;


//GBA Keys
#define KEYD_SPEED          0
#define KEYD_A              1
#define KEYD_B              2
#define KEYD_L              3
#define KEYD_R              4
#define KEYD_UP             5
#define KEYD_DOWN           6
#define KEYD_LEFT           7
#define KEYD_RIGHT          8
#define KEYD_START          9
#define KEYD_SELECT        10
#define KEYD_MINUS         11
#define KEYD_PLUS          12
#define KEYD_BRACKET_LEFT  13
#define KEYD_BRACKET_RIGHT 14
#define KEYD_STRAFE        15

#define NUMKEYS   16

//
// Player friction is variable, based on controlling
// linedefs. More friction can create mud, sludge,
// magnetized floors, etc. Less friction can create ice.

#define ORIG_FRICTION          0xE800      // original value
#define ORIG_FRICTION_FACTOR   2048L       // original value

#endif          // __DOOMDEF__
