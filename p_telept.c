/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2002 by
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
 *      Teleportation.
 *
 *-----------------------------------------------------------------------------*/

#include "doomdef.h"
#include "d_player.h"
#include "p_spec.h"
#include "p_maputl.h"
#include "p_map.h"
#include "r_main.h"
#include "p_tick.h"
#include "s_sound.h"
#include "sounds.h"
#include "p_user.h"

#include "globdata.h"

static mobj_t __far* P_TeleportDestination(const line_t __far* line)
{
  int16_t i;
  for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;) {
    thinker_t __far* th = NULL;
    while ((th = P_NextThinker(th)) != NULL)
      if (th->function == P_MobjThinker) {
        mobj_t __far* m = (mobj_t __far*)th;
        if (m->type == MT_TELEPORTMAN  &&
            m->subsector->sector-_g_sectors == i)
            return m;
      }
  }
  return NULL;
}
//
// TELEPORTATION
//

boolean EV_Teleport(const line_t __far* line, int16_t side, mobj_t __far* thing)
{
  mobj_t    __far* m;

  // don't teleport missiles
  // Don't teleport if hit back of line,
  //  so you can get out of teleporter.
  if (side || thing->flags & MF_MISSILE)
    return false;

  // killough 1/31/98: improve performance by using
  // P_FindSectorFromLineTag instead of simple linear search.

  if ((m = P_TeleportDestination(line)) != NULL)
        {
          fixed_t oldx = thing->x, oldy = thing->y, oldz = thing->z;
          player_t *player = P_MobjIsPlayer(thing);

          // killough 5/12/98: exclude voodoo dolls:
          if (player && player->mo != thing)
            player = NULL;

          if (!P_TeleportMove(thing, m->x, m->y, false)) /* killough 8/9/98 */
            return false;

          thing->z = thing->floorz;

          if (player)
            player->viewz = thing->z + player->viewheight;

          // spawn teleport fog and emit sound at source
          S_StartSound(P_SpawnMobj(oldx, oldy, oldz, MT_TFOG), sfx_telept);

          // spawn teleport fog and emit sound at destination
          S_StartSound(P_SpawnMobj(m->x + 20*finecosine(m->angle>>ANGLETOFINESHIFT),
                                   m->y + 20*finesine(  m->angle>>ANGLETOFINESHIFT),
                                   thing->z, MT_TFOG),
                       sfx_telept);

    /* don't move for a bit
     * cph - DEMOSYNC - BOOM had (player) here? */
          if (P_MobjIsPlayer(thing))
            thing->reactiontime = 18;

          thing->angle = m->angle;

          thing->momx = thing->momy = thing->momz = 0;

    /* killough 10/98: kill all bobbing momentum too */
    if (player)
      player->momx = player->momy = 0;



          return true;
        }
  return false;
}
