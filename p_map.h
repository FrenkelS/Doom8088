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
 *      Map functions
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_MAP__
#define __P_MAP__

#include "r_defs.h"
#include "d_player.h"

#define USERANGE        (64*FRACUNIT)
#define MELEERANGE      (64*FRACUNIT)
#define MISSILERANGE    (32*64*FRACUNIT)

boolean P_IsAttackRangeMeleeRange(void);

boolean P_TryMove(mobj_t __far* thing, fixed_t x, fixed_t y);

// killough 8/9/98: extra argument for telefragging
boolean P_TeleportMove(mobj_t __far* thing, fixed_t x, fixed_t y, boolean boss);
boolean P_CheckSight(mobj_t __far* t1, mobj_t __far* t2);
void    P_UseLines(player_t *player);

fixed_t P_AimLineAttack(mobj_t __far*t1, angle_t angle, fixed_t distance);

void    P_LineAttack(mobj_t __far* t1, angle_t angle, fixed_t distance, fixed_t slope, int16_t damage);
void    P_RadiusAttack(mobj_t __far* spot, mobj_t __far* source, int16_t damage);
boolean P_CheckPosition(mobj_t __far* thing, fixed_t x, fixed_t y);


void    P_SetSecnodeFirstpoolToNull(void);
void    P_DelSeclist(void);
void    P_SetSeclist(msecnode_t __far* sectorList);
void    P_CreateSecNodeList(mobj_t __far*);


void	P_MapEnd(void);

#endif // __P_MAP__
