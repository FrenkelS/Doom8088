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
 *   Ceiling aninmation (lowering, crushing, raising)
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "d_player.h"
#include "r_main.h"
#include "p_spec.h"
#include "p_tick.h"
#include "s_sound.h"
#include "sounds.h"

#include "globdata.h"


// the list of ceilings moving currently, including crushers
static ceilinglist_t __far* activeceilings;


static void P_RemoveActiveCeiling(ceiling_t __far* ceiling);

/////////////////////////////////////////////////////////////////
//
// Ceiling action routine and linedef type handler
//
/////////////////////////////////////////////////////////////////

//
// T_MoveCeiling
//
// Action routine that moves ceilings. Called once per tick.
//
// Passed a ceiling_t structure that contains all the info about the move.
// see P_SPEC.H for fields. No return.
//
// jff 02/08/98 all cases with labels beginning with gen added to support
// generalized line type behaviors.
//
void T_MoveCeiling (ceiling_t __far* ceiling)
{
  result_e  res;

  switch(ceiling->direction)
  {
    case 0:
      // If ceiling in stasis, do nothing
      break;

    case 1:
      // Ceiling is moving up
      res = T_MovePlane
            (
              ceiling->sector,
              ceiling->speed,
              ceiling->topheight,
              false,
              1,
              ceiling->direction
            );

      // if not a silent crusher, make moving sound
      if (!(_g_leveltime&7))
      {
        switch(ceiling->type)
        {
          case silentCrushAndRaise:
          case genSilentCrusher:
            break;
          default:
            S_StartSound2(&ceiling->sector->soundorg,sfx_stnmov);
            break;
        }
      }

      // handle reaching destination height
      if (res == pastdest)
      {
        switch(ceiling->type)
        {
          // plain movers are just removed
          case raiseToHighest:
          case genCeiling:
            P_RemoveActiveCeiling(ceiling);
            break;

          // movers with texture change, change the texture then get removed
          case genCeilingChgT:
          case genCeilingChg0:
            ceiling->sector->special = ceiling->newspecial;
            //jff 3/14/98 transfer old special field as well
            ceiling->sector->oldspecial = ceiling->oldspecial;
          case genCeilingChg:
            ceiling->sector->ceilingpic = ceiling->texture;
            P_RemoveActiveCeiling(ceiling);
            break;

          // crushers reverse direction at the top
          case silentCrushAndRaise:
            S_StartSound2(&ceiling->sector->soundorg,sfx_pstop);
          case genSilentCrusher:
          case genCrusher:
          case fastCrushAndRaise:
          case crushAndRaise:
            ceiling->direction = -1;
            break;

          default:
            break;
        }
      }
      break;

    case -1:
      // Ceiling moving down
      res = T_MovePlane
            (
              ceiling->sector,
              ceiling->speed,
              ceiling->bottomheight,
              ceiling->crush,
              1,
              ceiling->direction
            );

      // if not silent crusher type make moving sound
      if (!(_g_leveltime&7))
      {
        switch(ceiling->type)
        {
          case silentCrushAndRaise:
          case genSilentCrusher:
            break;
          default:
            S_StartSound2(&ceiling->sector->soundorg,sfx_stnmov);
        }
      }

      // handle reaching destination height
      if (res == pastdest)
      {
        switch(ceiling->type)
        {
          // 02/09/98 jff change slow crushers' speed back to normal
          // start back up
          case genSilentCrusher:
          case genCrusher:
            if (ceiling->oldspeed<CEILSPEED*3)
              ceiling->speed = ceiling->oldspeed;
            ceiling->direction = 1; //jff 2/22/98 make it go back up!
            break;

          // make platform stop at bottom of all crusher strokes
          // except generalized ones, reset speed, start back up
          case silentCrushAndRaise:
            S_StartSound2(&ceiling->sector->soundorg,sfx_pstop);
          case crushAndRaise:
            ceiling->speed = CEILSPEED;
          case fastCrushAndRaise:
            ceiling->direction = 1;
            break;

          // in the case of ceiling mover/changer, change the texture
          // then remove the active ceiling
          case genCeilingChgT:
          case genCeilingChg0:
            ceiling->sector->special = ceiling->newspecial;
            //jff add to fix bug in special transfers from changes
            ceiling->sector->oldspecial = ceiling->oldspecial;
          case genCeilingChg:
            ceiling->sector->ceilingpic = ceiling->texture;
            P_RemoveActiveCeiling(ceiling);
            break;

          // all other case, just remove the active ceiling
          case lowerAndCrush:
          case lowerToFloor:
          case lowerToLowest:
          case lowerToMaxFloor:
          case genCeiling:
            P_RemoveActiveCeiling(ceiling);
            break;

          default:
            break;
        }
      }
      else // ( res != pastdest )
      {
        // handle the crusher encountering an obstacle
        if (res == crushed)
        {
          switch(ceiling->type)
          {
            //jff 02/08/98 slow down slow crushers on obstacle
            case genCrusher:
            case genSilentCrusher:
              if (ceiling->oldspeed < CEILSPEED*3)
                ceiling->speed = CEILSPEED / 8;
              break;
            case silentCrushAndRaise:
            case crushAndRaise:
            case lowerAndCrush:
                ceiling->speed = CEILSPEED / 8;
              break;

            default:
              break;
          }
        }
      }
      break;
  }
}


//////////////////////////////////////////////////////////////////////
//
// Active ceiling list primitives
//
/////////////////////////////////////////////////////////////////////

// jff 2/22/98 - modified Lee's plat code to work for ceilings
//
// The following were all rewritten by Lee Killough
// to use the new structure which places no limits
// on active ceilings. It also avoids spending as much
// time searching for active ceilings. Previously a
// fixed-size array was used, with NULL indicating
// empty entries, while now a doubly-linked list
// is used.

//
// P_RemoveActiveCeiling()
//
// Removes a ceiling from the list of active ceilings
//
// Passed the ceiling motion structure
// Returns nothing
//
static void P_RemoveActiveCeiling(ceiling_t __far* ceiling)
{
  ceilinglist_t __far* list = ceiling->list;
  ceiling->sector->ceilingdata = NULL;  //jff 2/22/98

  P_RemoveThinker(&ceiling->thinker);

  if ((list->prev && (*list->prev = list->next)))
    list->next->prev = list->prev;

  Z_Free(list);
}

//
// P_RemoveAllActiveCeilings()
//
// Removes all ceilings from the active ceiling list
//
// Passed nothing, returns nothing
//
void P_RemoveAllActiveCeilings(void)
{
  while (activeceilings)
  {
    ceilinglist_t __far* next = activeceilings->next;
    Z_Free(activeceilings);
    activeceilings = next;
  }
}
