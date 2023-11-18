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
 *  General plane mover and floor mover action routines
 *  Floor motion, pure changer types, raising stairs. donuts, elevators
 *
 *-----------------------------------------------------------------------------*/

#include "d_player.h"
#include "r_main.h"
#include "m_random.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_spec.h"
#include "p_tick.h"
#include "s_sound.h"
#include "sounds.h"

#include "globdata.h"


static boolean crushchange;
static boolean nofit;


//
// P_ThingHeightClip
// Takes a valid thing and adjusts the thing->floorz,
// thing->ceilingz, and possibly thing->z.
// This is called for all nearby monsters
// whenever a sector changes height.
// If the thing doesn't fit,
// the z will be set to the lowest value
// and false will be returned.
//

static boolean P_ThingHeightClip(mobj_t __far* thing)
{
  boolean   onfloor;

  onfloor = (thing->z == thing->floorz);

  P_CheckPosition (thing, thing->x, thing->y);

  /* what about stranding a monster partially off an edge?
   * killough 11/98: Answer: see below (upset balance if hanging off ledge)
   */

  thing->floorz = _g_tmfloorz;
  thing->ceilingz = _g_tmceilingz;
  thing->dropoffz = _g_tmdropoffz;    /* killough 11/98: remember dropoffs */

  if (onfloor)
    {

    // walking monsters rise and fall with the floor

    thing->z = thing->floorz;
    }
  else
    {

  // don't adjust a floating monster unless forced to

    if (thing->z+thing->height > thing->ceilingz)
      thing->z = thing->ceilingz - thing->height;
    }

  return thing->ceilingz - thing->floorz >= thing->height;
}


//
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
// If crunch is true, they will take damage
//  as they are being crushed.
// If Crunch is false, you should set the sector height back
//  the way it was and call P_ChangeSector again
//  to undo the changes.
//


//
// PIT_ChangeSector
//

static void PIT_ChangeSector(mobj_t __far* thing)
  {
  mobj_t __far* mo;

  if (P_ThingHeightClip (thing))
    return;

  // crunch bodies to giblets

  if (thing->health <= 0)
    {
    P_SetMobjState (thing, S_GIBS);

    thing->flags &= ~MF_SOLID;
    thing->height = 0;
    thing->radius = 0;
    return;
    }

  // crunch dropped items

  if (thing->flags & MF_DROPPED)
    {
    P_RemoveMobj (thing);

    return;
    }

  if (! (thing->flags & MF_SHOOTABLE) )
    {
    // assume it is bloody gibs or something
    return;
    }

  nofit = true;

  if (crushchange && !(_g_leveltime&3)) {
    int16_t t;
    P_DamageMobj(thing,NULL,NULL,10);

    // spray blood in a random direction
    mo = P_SpawnMobj (thing->x,
                      thing->y,
                      thing->z + thing->height/2, MT_BLOOD);

    /* killough 8/10/98: remove dependence on order of evaluation */
    t = P_Random();
    mo->momx = (t - P_Random ())<<12;
    t = P_Random();
    mo->momy = (t - P_Random ())<<12;
  }
  }


//
// P_CheckSector
// jff 3/19/98 added to just check monsters on the periphery
// of a moving sector instead of all in bounding box of the
// sector. Both more accurate and faster.
//

static boolean P_CheckSector(sector_t __far* sector, boolean crunch)
  {
  msecnode_t __far* n;

  nofit = false;
  crushchange = crunch;

  // killough 4/4/98: scan list front-to-back until empty or exhausted,
  // restarting from beginning after each thing is processed. Avoids
  // crashes, and is sure to examine all things in the sector, and only
  // the things which are in the sector, until a steady-state is reached.
  // Things can arbitrarily be inserted and removed and it won't mess up.
  //
  // killough 4/7/98: simplified to avoid using complicated counter

  // Mark all things invalid

  for (n=sector->touching_thinglist; n; n=n->m_snext)
    n->visited = false;

  do
    for (n=sector->touching_thinglist; n; n=n->m_snext)  // go through list
      if (!n->visited)               // unprocessed thing found
        {
        n->visited  = true;          // mark thing as processed
        if (!(n->m_thing->flags & MF_NOBLOCKMAP)) //jff 4/7/98 don't do these
          PIT_ChangeSector(n->m_thing);    // process it
        break;                 // exit and start over
        }
  while (n);  // repeat from scratch until all things left are marked valid

  return nofit;
  }


///////////////////////////////////////////////////////////////////////
//
// Plane (floor or ceiling), Floor motion and Elevator action routines
//
///////////////////////////////////////////////////////////////////////

//
// T_MovePlane()
//
// Move a plane (floor or ceiling) and check for crushing. Called
// every tick by all actions that move floors or ceilings.
//
// Passed the sector to move a plane in, the speed to move it at,
// the dest height it is to achieve, whether it crushes obstacles,
// whether it moves a floor or ceiling, and the direction up or down
// to move.
//
// Returns a result_e:
//  ok - plane moved normally, has not achieved destination yet
//  pastdest - plane moved normally and is now at destination height
//  crushed - plane encountered an obstacle, is holding until removed
//
result_e T_MovePlane(sector_t __far* sector, fixed_t speed, fixed_t dest, boolean crush, int16_t floorOrCeiling, int16_t direction)
{
  boolean       flag;
  fixed_t       lastpos;
  fixed_t       destheight; //jff 02/04/98 used to keep floors/ceilings
                            // from moving thru each other

  switch(floorOrCeiling)
  {
    case 0:
      // Moving a floor
      switch(direction)
      {
        case -1:
          // Moving a floor down
          if (sector->floorheight - speed < dest)
          {
            lastpos = sector->floorheight;
            sector->floorheight = dest;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
            if (flag == true)
            {
              sector->floorheight =lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->floorheight;
            sector->floorheight -= speed;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
      /* cph - make more compatible with original Doom, by
       *  reintroducing this code. This means floors can't lower
       *  if objects are stuck in the ceiling */
          }
          break;

        case 1:
          // Moving a floor up
          // jff 02/04/98 keep floor from moving thru ceilings
          // jff 2/22/98 weaken check to demo_compatibility
          destheight = (dest<sector->ceilingheight)?
                          dest : sector->ceilingheight;
          if (sector->floorheight + speed > destheight)
          {
            lastpos = sector->floorheight;
            sector->floorheight = destheight;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
            if (flag == true)
            {
              sector->floorheight = lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->floorheight;
            sector->floorheight += speed;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
            if (flag == true)
            {

              sector->floorheight = lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
              return crushed;
            }
          }
          break;
      }
      break;

    case 1:
      // moving a ceiling
      switch(direction)
      {
        case -1:
          // moving a ceiling down
          // jff 02/04/98 keep ceiling from moving thru floors
          // jff 2/22/98 weaken check to demo_compatibility
          destheight = (dest>sector->floorheight)?
                          dest : sector->floorheight;
          if (sector->ceilingheight - speed < destheight)
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight = destheight;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk

            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->ceilingheight;
            sector->ceilingheight -= speed;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk

            if (flag == true)
            {
              if (crush == true)
                return crushed;
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
              return crushed;
            }
          }
          break;

        case 1:
          // moving a ceiling up
          if (sector->ceilingheight + speed > dest)
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight = dest;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight += speed;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
          }
          break;
      }
      break;
  }
  return ok;
}

//
// T_MoveFloor()
//
// Move a floor to it's destination (up or down).
// Called once per tick for each moving floor.
//
// Passed a floormove_t structure that contains all pertinent info about the
// move. See P_SPEC.H for fields.
// No return.
//
// jff 02/08/98 all cases with labels beginning with gen added to support
// generalized line type behaviors.

void T_MoveFloor(floormove_t __far* floor)
{
  result_e      res;

  res = T_MovePlane       // move the floor
  (
    floor->sector,
    floor->speed,
    floor->floordestheight,
    floor->crush,
    0,
    floor->direction
  );

  if (!(_g_leveltime&7))     // make the floormove sound
    S_StartSound2(&floor->sector->soundorg, sfx_stnmov);

  if (res == pastdest)    // if destination height is reached
  {
    if (floor->direction == 1)       // going up
    {
      switch(floor->type) // handle texture/type changes
      {
        case donutRaise:
          floor->sector->special = floor->newspecial;
          floor->sector->floorpic = floor->texture;
          break;
        case genFloorChgT:
        case genFloorChg0:
          floor->sector->special = floor->newspecial;
          //jff add to fix bug in special transfers from changes
          floor->sector->oldspecial = floor->oldspecial;
          //fall thru
        case genFloorChg:
          floor->sector->floorpic = floor->texture;
          break;
        default:
          break;
      }
    }
    else if (floor->direction == -1) // going down
    {
      switch(floor->type) // handle texture/type changes
      {
        case lowerAndChange:
          floor->sector->special = floor->newspecial;
          //jff add to fix bug in special transfers from changes
          floor->sector->oldspecial = floor->oldspecial;
          floor->sector->floorpic = floor->texture;
          break;
        case genFloorChgT:
        case genFloorChg0:
          floor->sector->special = floor->newspecial;
          //jff add to fix bug in special transfers from changes
          floor->sector->oldspecial = floor->oldspecial;
          //fall thru
        case genFloorChg:
          floor->sector->floorpic = floor->texture;
          break;
        default:
          break;
      }
    }

    floor->sector->floordata = NULL; //jff 2/22/98
    P_RemoveThinker(&floor->thinker);//remove this floor from list of movers

    // make floor stop sound
    S_StartSound2(&floor->sector->soundorg, sfx_pstop);
  }
}

//
// T_MoveElevator()
//
// Move an elevator to it's destination (up or down)
// Called once per tick for each moving floor.
//
// Passed an elevator_t structure that contains all pertinent info about the
// move. See P_SPEC.H for fields.
// No return.
//
// jff 02/22/98 added to support parallel floor/ceiling motion
//

typedef struct
{
  thinker_t thinker;
  elevator_e type;
  sector_t __far* sector;
  int16_t direction;
  fixed_t floordestheight;
  fixed_t ceilingdestheight;
  fixed_t speed;
} elevator_t;

static void T_MoveElevator(elevator_t __far* elevator)
{
  result_e      res;

  if (elevator->direction<0)      // moving down
  {
    res = T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
    (
      elevator->sector,
      elevator->speed,
      elevator->ceilingdestheight,
      0,
      1,                          // move floor
      elevator->direction
    );
    if (res==ok || res==pastdest) // jff 4/7/98 don't move ceil if blocked
      T_MovePlane
      (
        elevator->sector,
        elevator->speed,
        elevator->floordestheight,
        0,
        0,                        // move ceiling
        elevator->direction
      );
  }
  else // up
  {
    res = T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
    (
      elevator->sector,
      elevator->speed,
      elevator->floordestheight,
      0,
      0,                          // move ceiling
      elevator->direction
    );
    if (res==ok || res==pastdest) // jff 4/7/98 don't move floor if blocked
      T_MovePlane
      (
        elevator->sector,
        elevator->speed,
        elevator->ceilingdestheight,
        0,
        1,                        // move floor
        elevator->direction
      );
  }

  // make floor move sound
  if (!(_g_leveltime&7))
    S_StartSound2(&elevator->sector->soundorg, sfx_stnmov);

  if (res == pastdest)            // if destination height acheived
  {
    elevator->sector->floordata = NULL;     //jff 2/22/98
    elevator->sector->ceilingdata = NULL;   //jff 2/22/98
    P_RemoveThinker(&elevator->thinker);    // remove elevator from actives

    // make floor stop sound
    S_StartSound2(&elevator->sector->soundorg, sfx_pstop);
  }
}


//
// P_FindNextHighestFloor()
//
// Passed a sector, returns the fixed point value
// of the smallest floor height in a surrounding sector larger than
// the floor height passed. If no such height exists the floorheight
// of the passed sector is returned.
//
// Rewritten by Lee Killough to avoid fixed array and to be faster
//
fixed_t P_FindNextHighestFloor(sector_t __far* sec)
{
  fixed_t currentheight = sec->floorheight;
  sector_t __far* other;
  int16_t i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->floorheight > currentheight)
    {
      fixed_t height = other->floorheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->floorheight < height &&
            other->floorheight > currentheight)
          height = other->floorheight;
      return height;
    }
  /* cph - my guess at doom v1.2 - 1.4beta compatibility here.
   * If there are no higher neighbouring sectors, Heretic just returned
   * heightlist[0] (local variable), i.e. noise off the stack. 0 is right for
   * RETURN01 E1M2, so let's take that. */
  return currentheight;
}


///////////////////////////////////////////////////////////////////////
//
// Floor motion linedef handlers
//
///////////////////////////////////////////////////////////////////////

//
// EV_DoFloor()
//
// Handle regular and extended floor types
//
// Passed the line that activated the floor and the type of floor motion
// Returns true if a thinker was created.
//
boolean EV_DoFloor(const line_t __far* line, floor_e floortype)
{
  int16_t           secnum;
  boolean           rtn;
  int16_t           i;
  sector_t __far*     sec;
  floormove_t __far*  floor;

  secnum = -1;
  rtn = false;
  // move all floors with the same tag as the linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &_g_sectors[secnum];

    // Don't start a second thinker on the same floor
    if (P_SectorActive(floor_special,sec)) //jff 2/23/98
      continue;

    // new floor thinker
    rtn = true;
    floor = Z_CallocLevSpec(sizeof(*floor));
    P_AddThinker (&floor->thinker);
    sec->floordata = floor; //jff 2/22/98
    floor->thinker.function = T_MoveFloor;
    floor->type = floortype;
    floor->crush = false;

    // setup the thinker according to the linedef type
    switch(floortype)
    {
      case lowerFloor:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindHighestFloorSurrounding(sec);
        break;

        //jff 02/03/30 support lowering floor by 24 absolute
      case lowerFloor24:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
        break;

        //jff 02/03/30 support lowering floor by 32 absolute (fast)
      case lowerFloor32Turbo:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED*4;
        floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;
        break;

      case lowerFloorToLowest:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindLowestFloorSurrounding(sec);
        break;

        //jff 02/03/30 support lowering floor to next lowest floor
      case lowerFloorToNearest:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight =
          P_FindNextLowestFloor(sec,floor->sector->floorheight);
        break;

      case turboLower:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED * 4;
        floor->floordestheight = P_FindHighestFloorSurrounding(sec);
        if (floor->floordestheight != sec->floorheight)
          floor->floordestheight += 8*FRACUNIT;
        break;

      case raiseFloorCrush:
        floor->crush = true;
      case raiseFloor:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
        if (floor->floordestheight > sec->ceilingheight)
          floor->floordestheight = sec->ceilingheight;
        floor->floordestheight -= (8*FRACUNIT)*(floortype == raiseFloorCrush);
        break;

      case raiseFloorTurbo:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED*4;
        floor->floordestheight = P_FindNextHighestFloor(sec);
        break;

      case raiseFloorToNearest:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindNextHighestFloor(sec);
        break;

      case raiseFloor24:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
        break;

        // jff 2/03/30 support straight raise by 32 (fast)
      case raiseFloor32Turbo:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED*4;
        floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;
        break;

      case raiseFloor512:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = floor->sector->floorheight + 512 * FRACUNIT;
        break;

      case raiseFloor24AndChange:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
        sec->floorpic = LN_FRONTSECTOR(line)->floorpic;
        sec->special = LN_FRONTSECTOR(line)->special;
        //jff 3/14/98 transfer both old and new special
        sec->oldspecial = LN_FRONTSECTOR(line)->oldspecial;
        break;

      case raiseToTexture:
        {
          int16_t minsize = 32000; /* jff 3/13/98 no ovf */
          side_t __far*     side;

          floor->direction = 1;
          floor->sector = sec;
          floor->speed = FLOORSPEED;
          for (i = 0; i < sec->linecount; i++)
          {
            if (twoSided (secnum, i) )
            {
              side = getSide(secnum,i,0);
              // jff 8/14/98 don't scan texture 0, its not real
              if (side->bottomtexture > 0)
                if (textureheight[side->bottomtexture] < minsize)
                  minsize = textureheight[side->bottomtexture];
              side = getSide(secnum,i,1);
              // jff 8/14/98 don't scan texture 0, its not real
              if (side->bottomtexture > 0)
                if (textureheight[side->bottomtexture] < minsize)
                  minsize = textureheight[side->bottomtexture];
            }
          }
          {
            floor->floordestheight =
              (floor->sector->floorheight>>FRACBITS) + minsize;
            if (floor->floordestheight>32000)
              floor->floordestheight = 32000;        //jff 3/13/98 do not
            floor->floordestheight<<=FRACBITS;       // allow height overflow
          }
        }
      break;

      case lowerAndChange:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindLowestFloorSurrounding(sec);
        floor->texture = sec->floorpic;

        // jff 1/24/98 make sure floor->newspecial gets initialized
        // in case no surrounding sector is at floordestheight
        // --> should not affect compatibility <--
        floor->newspecial = sec->special;
        //jff 3/14/98 transfer both old and new special
        floor->oldspecial = sec->oldspecial;

        //jff 5/23/98 use model subroutine to unify fixes and handling
        sec = P_FindModelFloorSector(floor->floordestheight,sec-_g_sectors);
        if (sec)
        {
          floor->texture = sec->floorpic;
          floor->newspecial = sec->special;
          //jff 3/14/98 transfer both old and new special
          floor->oldspecial = sec->oldspecial;
        }
        break;
      default:
        break;
    }
  }
  return rtn;
}

//
// EV_DoChange()
//
// Handle pure change types. These change floor texture and sector type
// by trigger or numeric model without moving the floor.
//
// The linedef causing the change and the type of change is passed
// Returns true if any sector changes
//
// jff 3/15/98 added to better support generalized sector types
//
boolean EV_DoChange(const line_t __far* line, change_e changetype)
{
  int16_t                   secnum;
  boolean                   rtn;
  sector_t __far*             sec;
  sector_t __far*             secm;

  secnum = -1;
  rtn = false;
  // change all sectors with the same tag as the linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &_g_sectors[secnum];

    rtn = true;

    // handle trigger or numeric change type
    switch(changetype)
    {
      case trigChangeOnly:
        sec->floorpic = LN_FRONTSECTOR(line)->floorpic;
        sec->special = LN_FRONTSECTOR(line)->special;
        sec->oldspecial = LN_FRONTSECTOR(line)->oldspecial;
        break;
      case numChangeOnly:
        secm = P_FindModelFloorSector(sec->floorheight,secnum);
        if (secm) // if no model, no change
        {
          sec->floorpic = secm->floorpic;
          sec->special = secm->special;
          sec->oldspecial = secm->oldspecial;
        }
        break;
      default:
        break;
    }
  }
  return rtn;
}

/*
 * EV_BuildStairs()
 *
 * Handles staircase building. A sequence of sectors chosen by algorithm
 * rise at a speed indicated to a height that increases by the stepsize
 * each step.
 *
 * Passed the linedef triggering the stairs and the type of stair rise
 * Returns true if any thinkers are created
 *
 * cph 2001/09/21 - compatibility nightmares again
 * There are three different ways this function has, during its history, stepped
 * through all the stairs to be triggered by the single switch
 * - original Doom used a linear P_FindSectorFromLineTag, but failed to preserve
 * the index of the previous sector found, so instead it would restart its
 * linear search from the last sector of the previous staircase
 * - MBF/PrBoom with comp_stairs fail to emulate this, because their
 * P_FindSectorFromLineTag is a chained hash table implementation. Instead they
 * start following the hash chain from the last sector of the previous
 * staircase, which will (probably) have the wrong tag, so they miss any further
 * stairs
 * - Boom fixed the bug, and MBF/PrBoom without comp_stairs work right
 */
static inline int16_t P_FindSectorFromLineTagWithLowerBound(const line_t __far* l, int16_t start, int16_t min)
{
  /* Emulate original Doom's linear lower-bounded P_FindSectorFromLineTag
   * as needed */
  do {
    start = P_FindSectorFromLineTag(l,start);
  } while (0 <= start && start <= min);
  return start;
}

boolean EV_BuildStairs(const line_t __far* line, stair_e type)
{
  /* cph 2001/09/22 - cleaned up this function to save my sanity. A separate
   * outer loop index makes the logic much cleared, and local variables moved
   * into the inner blocks helps too */
  int16_t                   ssec = -1;
  int16_t                   minssec = -1;
  boolean                   rtn = false;

  // start a stair at each sector tagged the same as the linedef
  while ((ssec = P_FindSectorFromLineTagWithLowerBound(line,ssec,minssec)) >= 0)
  {
   int16_t           secnum = ssec;
   sector_t __far*     sec = &_g_sectors[secnum];

    // don't start a stair if the first step's floor is already moving
   if (!P_SectorActive(floor_special,sec)) { //jff 2/22/98
    floormove_t __far*  floor;
    int16_t       texture;
    fixed_t       height;
    fixed_t       stairsize;
    fixed_t       speed;
    boolean       ok;

    // create new floor thinker for first step
    rtn = true;
    floor = Z_CallocLevSpec(sizeof(*floor));
    P_AddThinker (&floor->thinker);
    sec->floordata = floor;
    floor->thinker.function = T_MoveFloor;
    floor->direction = 1;
    floor->sector = sec;
    floor->type = buildStair;   //jff 3/31/98 do not leave uninited

    // set up the speed and stepsize according to the stairs type
    switch(type)
    {
      default: // killough -- prevent compiler warning
      case build8:
        speed = FLOORSPEED/4;
        stairsize = 8*FRACUNIT;
          floor->crush = false; //jff 2/27/98 fix uninitialized crush field
        break;
      case turbo16:
        speed = FLOORSPEED*4;
        stairsize = 16*FRACUNIT;
          floor->crush = true;  //jff 2/27/98 fix uninitialized crush field
        break;
    }
    floor->speed = speed;
    height = sec->floorheight + stairsize;
    floor->floordestheight = height;

    texture = sec->floorpic;

    // Find next sector to raise
    //   1. Find 2-sided line with same sector side[0] (lowest numbered)
    //   2. Other side is the next sector to raise
    //   3. Unless already moving, or different texture, then stop building
    do
    {
      int16_t i;
      ok = false;

      for (i = 0;i < sec->linecount;i++)
      {          
        sector_t __far* tsec = LN_FRONTSECTOR((sec->lines[i]));
        int16_t newsecnum;
        if ( !((sec->lines[i])->flags & ML_TWOSIDED) )
          continue;

        newsecnum = tsec-_g_sectors;

        if (secnum != newsecnum)
          continue;

        tsec = LN_BACKSECTOR((sec->lines[i]));
        if (!tsec) continue;     //jff 5/7/98 if no backside, continue
        newsecnum = tsec - _g_sectors;

        // if sector's floor is different texture, look for another
        if (tsec->floorpic != texture)
          continue;

        // if sector's floor already moving, look for another
        if (P_SectorActive(floor_special,tsec)) //jff 2/22/98
          continue;

        height += stairsize;

        sec = tsec;
        secnum = newsecnum;

        // create and initialize a thinker for the next step
        floor = Z_CallocLevSpec(sizeof(*floor));
        P_AddThinker (&floor->thinker);

        sec->floordata = floor; //jff 2/22/98
        floor->thinker.function = T_MoveFloor;
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = speed;
        floor->floordestheight = height;
        floor->type = buildStair; //jff 3/31/98 do not leave uninited
        //jff 2/27/98 fix uninitialized crush field
          floor->crush = type==build8? false : true;
        ok = true;
        break;
      }
    } while(ok);      // continue until no next step is found

   }
  }
  return rtn;
}

//
// EV_DoDonut()
//
// Handle donut function: lower pillar, raise surrounding pool, both to height,
// texture and type of the sector surrounding the pool.
//
// Passed the linedef that triggered the donut
// Returns whether a thinker was created
//
boolean EV_DoDonut(const line_t __far* line)
{
  sector_t __far* s1;
  sector_t __far* s2;
  sector_t __far* s3;
  int16_t       secnum;
  boolean       rtn;
  int16_t       i;
  floormove_t __far* floor;

  secnum = -1;
  rtn = false;
  // do function on all sectors with same tag as linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    s1 = &_g_sectors[secnum];                // s1 is pillar's sector

    // do not start the donut if the pillar is already moving
    if (P_SectorActive(floor_special,s1)) //jff 2/22/98
      continue;

    s2 = getNextSector(s1->lines[0],s1);  // s2 is pool's sector
    if (!s2) continue;                    // note lowest numbered line around
                                          // pillar must be two-sided

    /* do not start the donut if the pool is already moving
     * cph - DEMOSYNC - was !compatibility */
    if (P_SectorActive(floor_special,s2))
      continue;                           //jff 5/7/98

    // find a two sided line around the pool whose other side isn't the pillar
    for (i = 0;i < s2->linecount;i++)
    {


        if (!LN_BACKSECTOR((s2->lines[i])) || LN_BACKSECTOR((s2->lines[i])) == s1)
            continue;

      rtn = true; //jff 1/26/98 no donut action - no switch change on return

      s3 = LN_BACKSECTOR((s2->lines[i]));      // s3 is model sector for changes

      //  Spawn rising slime
      floor = Z_CallocLevSpec(sizeof(*floor));
      P_AddThinker (&floor->thinker);
      s2->floordata = floor; //jff 2/22/98
      floor->thinker.function = T_MoveFloor;
      floor->type = donutRaise;
      floor->crush = false;
      floor->direction = 1;
      floor->sector = s2;
      floor->speed = FLOORSPEED / 2;
      floor->texture = s3->floorpic;
      floor->newspecial = 0;
      floor->floordestheight = s3->floorheight;

      //  Spawn lowering donut-hole pillar
      floor = Z_CallocLevSpec(sizeof(*floor));
      P_AddThinker (&floor->thinker);
      s1->floordata = floor; //jff 2/22/98
      floor->thinker.function = T_MoveFloor;
      floor->type = lowerFloor;
      floor->crush = false;
      floor->direction = -1;
      floor->sector = s1;
      floor->speed = FLOORSPEED / 2;
      floor->floordestheight = s3->floorheight;
      break;
    }
  }
  return rtn;
}

//
// EV_DoElevator
//
// Handle elevator linedef types
//
// Passed the linedef that triggered the elevator and the elevator action
//
// jff 2/22/98 new type to move floor and ceiling in parallel
//
boolean EV_DoElevator(const line_t __far* line, elevator_e elevtype)
{
  int16_t                   secnum;
  boolean                   rtn;
  sector_t __far*             sec;
  elevator_t __far*           elevator;

  secnum = -1;
  rtn = false;
  // act on all sectors with the same tag as the triggering linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &_g_sectors[secnum];

    // If either floor or ceiling is already activated, skip it
    if (sec->floordata || sec->ceilingdata) //jff 2/22/98
      continue;

    // create and initialize new elevator thinker
    rtn = true;
    elevator = Z_CallocLevSpec(sizeof(*elevator));
    P_AddThinker (&elevator->thinker);
    sec->floordata = elevator; //jff 2/22/98
    sec->ceilingdata = elevator; //jff 2/22/98
    elevator->thinker.function = T_MoveElevator;
    elevator->type = elevtype;

    // set up the fields according to the type of elevator action
    switch(elevtype)
    {
        // elevator down to next floor
      case elevateDown:
        elevator->direction = -1;
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED;
        elevator->floordestheight =
          P_FindNextLowestFloor(sec,sec->floorheight);
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        break;

        // elevator up to next floor
      case elevateUp:
        elevator->direction = 1;
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED;
        elevator->floordestheight   = P_FindNextHighestFloor(sec);
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        break;

        // elevator to floor height of activating switch's front sector
      case elevateCurrent:
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED;
        elevator->floordestheight = LN_FRONTSECTOR(line)->floorheight;
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        elevator->direction =
          elevator->floordestheight>sec->floorheight?  1 : -1;
        break;

      default:
        break;
    }
  }
  return rtn;
}
