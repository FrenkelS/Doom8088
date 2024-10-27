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
 *  Plats (i.e. elevator platforms) code, raising/lowering.
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "d_player.h"
#include "m_random.h"
#include "r_main.h"
#include "p_spec.h"
#include "p_tick.h"
#include "s_sound.h"
#include "sounds.h"

#include "globdata.h"


// New limit-free plat structure -- killough

typedef struct platlist {
  plat_t __far* plat;
  struct platlist __far* next;
  struct platlist __far*__far* prev;
} platlist_t;


static platlist_t __far* activeplats;


static void P_RemoveActivePlat(plat_t __far* plat);

//
// T_PlatRaise()
//
// Action routine to move a plat up and down
//
// Passed a plat structure containing all pertinent information about the move
// No return
//

void T_PlatRaise(plat_t __far* plat)
{
  result_e      res;

  // handle plat moving, up, down, or waiting
  switch(plat->status)
  {
    case up: // plat moving up
      res = T_MovePlaneFloor(plat->sector,plat->speed,plat->high,1);

      // if a pure raise type, make the plat moving sound
      if (plat->type == raiseToNearestAndChange)
      {
        if (!((int16_t)_g_leveltime&7))
          S_StartSound2(&plat->sector->soundorg, sfx_stnmov);
      }

      // if encountered an obstacle, and not a crush type, reverse direction
      if (res == crushed)
      {
        plat->count = plat->wait;
        plat->status = down;
        S_StartSound2(&plat->sector->soundorg, sfx_pstart);
      }
      else  // else handle reaching end of up stroke
      {
        if (res == pastdest) // end of stroke
        {
          // not an instant toggle type, wait, make plat stop sound
          plat->count = plat->wait;
          plat->status = waiting;
          S_StartSound2(&plat->sector->soundorg, sfx_pstop);

          // lift types and pure raise types are done at end of up stroke
          // only the perpetual type waits then goes back up
          switch(plat->type)
          {
            case downWaitUpStay:
            case raiseToNearestAndChange:
              P_RemoveActivePlat(plat);     // killough
            default:
              break;
          }
        }
      }
      break;

    case down: // plat moving down
      res = T_MovePlaneFloor(plat->sector,plat->speed,plat->low,-1);

      // handle reaching end of down stroke
      if (res == pastdest)
      {
        // not an instant toggle, start waiting, make plat stop sound
        plat->count = plat->wait;
        plat->status = waiting;
        S_StartSound2(&plat->sector->soundorg,sfx_pstop);

        //jff 1/26/98 remove the plat if it bounced so it can be tried again
        //only affects plats that raise and bounce
        //killough 1/31/98: relax compatibility to demo_compatibility

          switch(plat->type)
          {
            case raiseToNearestAndChange:
              P_RemoveActivePlat(plat);
            default:
              break;
          }

      }
      break;

    case waiting: // plat is waiting
      if (!--plat->count)  // downcount and check for delay elapsed
      {
        if (plat->sector->floorheight == plat->low)
          plat->status = up;     // if at bottom, start up
        else
          plat->status = down;   // if at top, start down

        // make plat start sound
        S_StartSound2(&plat->sector->soundorg,sfx_pstart);
      }
      break;
  }
}


//
// EV_DoPlat
//
// Handle Plat linedef types
//
// Passed the linedef that activated the plat and the type of plat action.
// Returns true if a thinker is started
//

static void P_AddActivePlat(plat_t __far* plat);

boolean EV_DoPlat(const line_t __far* line, plattype_e type)
{
  plat_t __far* plat;
  int16_t             secnum;
  boolean             rtn;
  sector_t __far*       sec;

  secnum = -1;
  rtn = false;

  // act on all sectors tagged the same as the activating linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &_g_sectors[secnum];

    // don't start a second floor function if already moving
    if (sec->floordata != NULL) // multiple thinkers
      continue;

    // Create a thinker
    rtn = true;
    plat = Z_CallocLevSpec(sizeof(*plat));
    P_AddThinker(&plat->thinker);

    plat->type = type;
    plat->sector = sec;
    plat->sector->floordata = plat; //jff 2/23/98 multiple thinkers
    plat->thinker.function = T_PlatRaise;
    plat->tag = line->tag;

    //jff 1/26/98 Avoid raise plat bouncing a head off a ceiling and then
    //going down forever -- default low to plat height when triggered
    plat->low = sec->floorheight;

    // set up plat according to type
    switch(type)
    {
      case raiseToNearestAndChange:
        plat->speed = PLATSPEED/2;
        sec->floorpic = _g_sides[line->sidenum[0]].sector->floorpic;
        plat->high = P_FindNextHighestFloor(sec);
        plat->wait = 0;
        plat->status = up;
        sec->special = 0;
        //jff 3/14/98 clear old field as well
        sec->oldspecial = 0;

        S_StartSound2(&sec->soundorg,sfx_stnmov);
        break;

      case downWaitUpStay:
        plat->speed = PLATSPEED * 4;
        plat->low = P_FindLowestFloorSurrounding(sec);

        if (plat->low > sec->floorheight)
          plat->low = sec->floorheight;

        plat->high = sec->floorheight;
        plat->wait = 35*PLATWAIT;
        plat->status = down;
        S_StartSound2(&sec->soundorg,sfx_pstart);
        break;

      default:
        break;
    }
    P_AddActivePlat(plat);  // add plat to list of active plats
  }
  return rtn;
}

// The following were all rewritten by Lee Killough
// to use the new structure which places no limits
// on active plats. It also avoids spending as much
// time searching for active plats. Previously a
// fixed-size array was used, with NULL indicating
// empty entries, while now a doubly-linked list
// is used.

//
// P_AddActivePlat()
//
// Add a plat to the head of the active plat list
//
// Passed a pointer to the plat to add
// Returns nothing
//
static void P_AddActivePlat(plat_t __far* plat)
{
    platlist_t __far* old_head = activeplats;

    platlist_t __far* list = activeplats = Z_MallocLevel(sizeof *list, (void __far*__far*)&activeplats);
    list->plat = plat;
    plat->list = list;
    if ((list->next = old_head))
        list->next->prev = &list->next;

    list->prev = old_head;
}

//
// P_RemoveActivePlat()
//
// Remove a plat from the active plat list
//
// Passed a pointer to the plat to remove
// Returns nothing
//
static void P_RemoveActivePlat(plat_t __far* plat)
{
  platlist_t __far* list = plat->list;
  plat->sector->floordata = NULL; //jff 2/23/98 multiple thinkers

  P_RemoveThinker(&plat->thinker);

  if (list->prev && (*list->prev = list->next))
    list->next->prev = list->prev;

  Z_Free(list);
}

//
// P_RemoveAllActivePlats()
//
// Remove all plats from the active plat list
//
// Passed nothing, returns nothing
//
void P_RemoveAllActivePlats(void)
{
  while (activeplats)
  {
    platlist_t __far* next = activeplats->next;
    Z_Free(activeplats);
    activeplats = next;
  }
}
