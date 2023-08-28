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
 *  Something to do with weapon sprite frames. Don't ask me.
 *
 *-----------------------------------------------------------------------------
 */

// We are referring to sprite numbers.
#include "doomtype.h"
#include "info.h"

#include "d_items.h"


//
// PSPRITE ACTIONS for waepons.
// This struct controls the weapon animations.
//
// Each entry is:
//  ammo/amunition type
//  upstate
//  downstate
//  readystate
//  atkstate, i.e. attack/fire/hit frame
//  flashstate, muzzle flash
//
const weaponinfo_t weaponinfo[NUMWEAPONS] =
{
    {
        // fist
        am_noammo,
        S_PUNCHUP,
        S_PUNCHDOWN,
        S_PUNCH,
        S_PUNCH1,
        S_NULL
    },
    {
        // pistol
        am_clip,
        S_PISTOLUP,
        S_PISTOLDOWN,
        S_PISTOL,
        S_PISTOL1,
        S_PISTOLFLASH
    },
    {
        // shotgun
        am_shell,
        S_SGUNUP,
        S_SGUNDOWN,
        S_SGUN,
        S_SGUN1,
        S_SGUNFLASH1
    },
    {
        // chaingun
        am_clip,
        S_CHAINUP,
        S_CHAINDOWN,
        S_CHAIN,
        S_CHAIN1,
        S_CHAINFLASH1
    },
    {
        // missile launcher
        am_misl,
        S_MISSILEUP,
        S_MISSILEDOWN,
        S_MISSILE,
        S_MISSILE1,
        S_MISSILEFLASH1
    },
    {
        // chaingun, was plasma rifle
        am_clip,
        S_CHAINUP,
        S_CHAINDOWN,
        S_CHAIN,
        S_CHAIN1,
        S_CHAINFLASH1
    },
    {
        // missile launcher, was bfg 9000
        am_misl,
        S_MISSILEUP,
        S_MISSILEDOWN,
        S_MISSILE,
        S_MISSILE1,
        S_MISSILEFLASH1
    },
    {
        // chainsaw
        am_noammo,
        S_SAWUP,
        S_SAWDOWN,
        S_SAW,
        S_SAW1,
        S_NULL
    },
    {
        // shotgun, was super shotgun
        am_shell,
        S_SGUNUP,
        S_SGUNDOWN,
        S_SGUN,
        S_SGUN1,
        S_SGUNFLASH1
    },
};
