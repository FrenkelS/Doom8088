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
 *  Thing events, and dehacked specified numbers controlling them.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_INTER__
#define __P_INTER__

#include "d_player.h"
#include "p_mobj.h"


/* follow a player exclusively for 3 seconds */
#define BASETHRESHOLD   (100)

boolean P_GivePower(player_t *, powertype_t);
void P_TouchSpecialThing(mobj_t __far* special, mobj_t __far* toucher);
void P_DamageMobj(mobj_t __far* target, mobj_t __far* inflictor, mobj_t __far* source, int32_t damage);

/* killough 5/2/98: moved from d_deh.c, g_game.c, m_misc.c, others: */

extern const int32_t god_health;   /* Ty 03/09/98 - deh support, see also p_inter.c */
extern const int32_t idfa_armor;
extern const int32_t idfa_armor_class;  /* Ty - end */
/* Ty 03/13/98 - externalized initial settings for respawned player */
extern const int32_t initial_health;
extern const int32_t initial_bullets;
extern const int32_t bfgcells;
extern const int32_t maxammo[];

#endif
