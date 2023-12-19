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

//
// Silent TELEPORTATION, by Lee Killough
// Primarily for rooms-over-rooms etc.
//

boolean EV_SilentTeleport(const line_t __far* line, int16_t side, mobj_t __far* thing)
{
  mobj_t    __far* m;

  // don't teleport missiles
  // Don't teleport if hit back of line,
  // so you can get out of teleporter.

  if (side || thing->flags & MF_MISSILE)
    return false;

  if ((m = P_TeleportDestination(line)) != NULL)
        {
          // Height of thing above ground, in case of mid-air teleports:
          fixed_t z = thing->z - thing->floorz;

          // Get the angle between the exit thing and source linedef.
          // Rotate 90 degrees, so that walking perpendicularly across
          // teleporter linedef causes thing to exit in the direction
          // indicated by the exit thing.
          angle_t angle =
            R_PointToAngle2(0, 0, (fixed_t)line->dx<<FRACBITS, (fixed_t)line->dy<<FRACBITS) - m->angle + ANG90;

          // Sine, cosine of angle adjustment
          fixed_t s = finesine(  angle>>ANGLETOFINESHIFT);
          fixed_t c = finecosine(angle>>ANGLETOFINESHIFT);

          // Momentum of thing crossing teleporter linedef
          fixed_t momx = thing->momx;
          fixed_t momy = thing->momy;

          // Whether this is a player, and if so, a pointer to its player_t
          player_t *player = P_MobjIsPlayer(thing);

          // Attempt to teleport, aborting if blocked
          if (!P_TeleportMove(thing, m->x, m->y, false)) /* killough 8/9/98 */
            return false;

          // Rotate thing according to difference in angles
          thing->angle += angle;

          // Adjust z position to be same height above ground as before
          thing->z = z + thing->floorz;

          // Rotate thing's momentum to come out of exit just like it entered
          thing->momx = FixedMul(momx, c) - FixedMul(momy, s);
          thing->momy = FixedMul(momy, c) + FixedMul(momx, s);

          // Adjust player's view, in case there has been a height change
          // Voodoo dolls are excluded by making sure player->mo == thing.
          if (player && player->mo == thing)
            {
              // Save the current deltaviewheight, used in stepping
              fixed_t deltaviewheight = player->deltaviewheight;

              // Clear deltaviewheight, since we don't want any changes
              player->deltaviewheight = 0;

              // Set player's view according to the newly set parameters
              P_CalcHeight(player);

              // Reset the delta to have the same dynamics as before
              player->deltaviewheight = deltaviewheight;
            }
          


          return true;
        }
  return false;
}
