/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2025 Frenkel Smeijers
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
 *      Enemy thinking, AI.
 *
 *-----------------------------------------------------------------------------*/

#include "m_fixed.h"
#include "p_enemy.h"
#include "p_map.h"

#include "globdata.h"


//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//

static const fixed_t xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
static const fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

boolean P_Move(mobj_t __far* actor)
{
  if (actor->movedir == DI_NODIR)
    return false;

#ifdef RANGECHECK
  if (actor->movedir >= 8)
    I_Error ("P_Move: Weird actor->movedir!");
#endif

  int32_t speed = mobjinfo[actor->type].speed;

  fixed_t tryx = actor->x + speed * xspeed[actor->movedir];
  fixed_t tryy = actor->y + speed * yspeed[actor->movedir];

  boolean try_ok = P_TryMove(actor, tryx, tryy);

  if (!try_ok)
    {      // open any specials
      if (!_g_numspechit)
        return false;

      actor->movedir = DI_NODIR;

      /* if the special is not a door that can be opened, return false
       *
       * killough 8/9/98: this is what caused monsters to get stuck in
       * doortracks, because it thought that the monster freed itself
       * by opening a door, even if it was moving towards the doortrack,
       * and not the door itself.
       *
       * killough 9/9/98: If a line blocking the monster is activated,
       * return true 90% of the time. If a line blocking the monster is
       * not activated, but some other line is, return false 90% of the
       * time. A bit of randomness is needed to ensure it's free from
       * lockups, but for most cases, it returns the correct result.
       *
       * Do NOT simply return false 1/4th of the time (causes monsters to
       * back out when they shouldn't, and creates secondary stickiness).
       */

      boolean good = false;
      for ( ; _g_numspechit--; )
        if (P_UseSpecialLine(actor, _g_spechit[_g_numspechit]))
          good = true;

      /* cph - compatibility maze here
       * Boom v2.01 and orig. Doom return "good"
       * Boom v2.02 and LxDoom return good && (P_Random(pr_trywalk)&3)
       * MBF plays even more games
       */
      return good;
    }

  /* fall more slowly, under gravity */
  actor->z = actor->floorz;

  return true;
}
