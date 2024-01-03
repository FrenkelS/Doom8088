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
 *   Door animation code (opening/closing)
 *
 *-----------------------------------------------------------------------------*/

#include "d_player.h"
#include "p_spec.h"
#include "p_tick.h"
#include "s_sound.h"
#include "sounds.h"
#include "r_main.h"
#include "d_englsh.h"

#include "globdata.h"


/* killough 10/98:
 *
 * EV_LightTurnOnPartway()
 *
 * Turn sectors tagged to line lights on to specified or max neighbor level
 *
 * Passed the activating line, and a light level fraction between 0 and 1.
 * Sets the light to min on 0, max on 1, and interpolates in-between.
 * Used for doors with gradual lighting effects.
 *
 */

static void EV_LightTurnOnPartway(const line_t __far* line, fixed_t level)
{
  int16_t i;

  if (level < 0)          // clip at extremes
    level = 0;
  if (level > FRACUNIT)
    level = FRACUNIT;

  // search all sectors for ones with same tag as activating line
  for (i = -1; (i = P_FindSectorFromLineTag(line,i)) >= 0;)
    {
      sector_t __far* temp;
      sector_t __far* sector = _g_sectors+i;
      int16_t j, bright = 0, min = sector->lightlevel;

      for (j = 0; j < sector->linecount; j++)
  if ((temp = getNextSector(sector->lines[j],sector)))
    {
      if (temp->lightlevel > bright)
        bright = temp->lightlevel;
      if (temp->lightlevel < min)
        min = temp->lightlevel;
    }

      sector->lightlevel =   // Set level in-between extremes
  (level * bright + (FRACUNIT-level) * min) >> FRACBITS;
    }
}


///////////////////////////////////////////////////////////////
//
// Door action routines, called once per tick
//
///////////////////////////////////////////////////////////////

//
// T_VerticalDoor
//
// Passed a door structure containing all info about the door.
// See P_SPEC.H for fields.
// Returns nothing.
//

void T_VerticalDoor(vldoor_t __far* door)
{
  result_e  res;

  // Is the door waiting, going up, or going down?
  switch(door->direction)
  {
    case 0:
      // Door is waiting
      if (!--door->topcountdown)  // downcount and check
      {
        switch(door->type)
        {
          case normal:
            door->direction = -1; // time to go back down
            S_StartSound2(&door->sector->soundorg,sfx_dorcls);
            break;

          case close30ThenOpen:
            door->direction = 1;  // time to go back up
            S_StartSound2(&door->sector->soundorg,sfx_doropn);
            break;

          default:
            break;
        }
      }
      break;

    case -1:
      // Door is moving down
      res = T_MovePlaneCeiling(door->sector, door->speed, door->sector->floorheight, door->direction);

      /* killough 10/98: implement gradual lighting effects */
      // e6y: "Tagged doors don't trigger special lighting" handled wrong
      // http://sourceforge.net/tracker/index.php?func=detail&aid=1411400&group_id=148658&atid=772943
      // Old code: if (door->lighttag && door->topheight - door->sector->floorheight)
      if (door->lighttag && door->topheight - door->sector->floorheight)
        EV_LightTurnOnPartway(door->line,
                              FixedApproxDiv(door->sector->ceilingheight -
                                       door->sector->floorheight,
                                       door->topheight -
                                       door->sector->floorheight));

      // handle door reaching bottom
      if (res == pastdest)
      {
        switch(door->type)
        {
          // regular open and close doors are all done, remove them
          case normal:
            door->sector->ceilingdata = NULL; //jff 2/22/98
            P_RemoveThinker (&door->thinker);  // unlink and free
            break;

          // close then open doors start waiting
          case close30ThenOpen:
            door->direction = 0;
            door->topcountdown = TICRATE*30;
            break;

          default:
            break;
        }
      }
      /* jff 1/31/98 turn lighting off in tagged sectors of manual doors
       * killough 10/98: replaced with gradual lighting code
       */
      else if (res == crushed) // handle door meeting obstruction on way down
      {
        // other types bounce off the obstruction
        door->direction = 1;
        S_StartSound2(&door->sector->soundorg,sfx_doropn);
      }
      break;

    case 1:
      // Door is moving up
      res = T_MovePlaneCeiling(door->sector, door->speed, door->topheight, door->direction);

      /* killough 10/98: implement gradual lighting effects */
      // e6y: "Tagged doors don't trigger special lighting" handled wrong
      // http://sourceforge.net/tracker/index.php?func=detail&aid=1411400&group_id=148658&atid=772943
      // Old code: if (door->lighttag && door->topheight - door->sector->floorheight)
      if (door->lighttag && door->topheight - door->sector->floorheight)
        EV_LightTurnOnPartway(door->line,
                              FixedApproxDiv(door->sector->ceilingheight -
                                       door->sector->floorheight,
                                       door->topheight -
                                       door->sector->floorheight));

      // handle door reaching the top
      if (res == pastdest)
      {
        switch(door->type)
        {
          case normal:           // regular open/close doors start waiting
            door->direction = 0; // wait at top with delay
            door->topcountdown = door->topwait;
            break;

          case close30ThenOpen:  // close and close/open doors are done
          case dopen:
            door->sector->ceilingdata = NULL; //jff 2/22/98
            P_RemoveThinker (&door->thinker); // unlink and free
            break;

          default:
            break;
        }
      }
      break;
  }
}

///////////////////////////////////////////////////////////////
//
// Door linedef handlers
//
///////////////////////////////////////////////////////////////

//
// EV_DoDoor
//
// Handle opening a tagged door
//
// Passed the line activating the door and the type of door
// Returns true if a thinker created
//
boolean EV_DoDoor(const line_t __far* line, vldoor_e type)
{
  int16_t   secnum;
  boolean   rtn;
  sector_t __far* sec;
  vldoor_t __far* door;

  secnum = -1;
  rtn = false;

  // open all doors with the same tag as the activating line
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &_g_sectors[secnum];
    // if the ceiling already moving, don't start the door action
    if (sec->ceilingdata != NULL)
        continue;

    // new door thinker
    rtn = true;
    door = Z_CallocLevSpec(sizeof(*door));
    P_AddThinker (&door->thinker);
    sec->ceilingdata = door; //jff 2/22/98

    door->thinker.function = T_VerticalDoor;
    door->sector = sec;
    door->type = type;
    door->topwait = VDOORWAIT;
    door->speed = VDOORSPEED;
    door->line = line; // jff 1/31/98 remember line that triggered us
    door->lighttag = 0; /* killough 10/98: no light effects with tagged doors */

    // setup door parameters according to type of door
    switch(type)
    {
      case close30ThenOpen:
        door->topheight = sec->ceilingheight;
        door->direction = -1;
        S_StartSound2(&door->sector->soundorg,sfx_dorcls);
        break;

      case normal:
      case dopen:
        door->direction = 1;
        door->topheight = P_FindLowestCeilingSurrounding(sec);
        door->topheight -= 4*FRACUNIT;
        if (door->topheight != sec->ceilingheight)
          S_StartSound2(&door->sector->soundorg,sfx_doropn);
        break;

      default:
        break;
    }
  }
  return rtn;
}
