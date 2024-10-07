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
  }


//
// P_CheckSector
// jff 3/19/98 added to just check monsters on the periphery
// of a moving sector instead of all in bounding box of the
// sector. Both more accurate and faster.
//

static boolean P_CheckSector(sector_t __far* sector)
  {
  msecnode_t __far* n;

  nofit = false;

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
result_e T_MovePlaneFloor(sector_t __far* sector, fixed_t speed, fixed_t dest, int16_t direction)
{
  boolean       flag;
  fixed_t       lastpos;
  fixed_t       destheight; //jff 02/04/98 used to keep floors/ceilings
                            // from moving thru each other

      switch(direction)
      {
        case -1:
          // Moving a floor down
          if (sector->floorheight - speed < dest)
          {
            lastpos = sector->floorheight;
            sector->floorheight = dest;
            flag = P_CheckSector(sector);
            if (flag == true)
            {
              sector->floorheight =lastpos;
              P_CheckSector(sector);
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->floorheight;
            sector->floorheight -= speed;
            flag = P_CheckSector(sector);
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
            flag = P_CheckSector(sector);
            if (flag == true)
            {
              sector->floorheight = lastpos;
              P_CheckSector(sector);
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->floorheight;
            sector->floorheight += speed;
            flag = P_CheckSector(sector);
            if (flag == true)
            {

              sector->floorheight = lastpos;
              P_CheckSector(sector);
              return crushed;
            }
          }
          break;
      }

  return ok;
}


result_e T_MovePlaneCeiling(sector_t __far* sector, fixed_t speed, fixed_t dest, int16_t direction)
{
  boolean       flag;
  fixed_t       lastpos;
  fixed_t       destheight; //jff 02/04/98 used to keep floors/ceilings
                            // from moving thru each other

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
            flag = P_CheckSector(sector);

            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector);
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->ceilingheight;
            sector->ceilingheight -= speed;
            flag = P_CheckSector(sector);

            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector);
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
            flag = P_CheckSector(sector);
            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector);
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight += speed;
            flag = P_CheckSector(sector);
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
// move.
// No return.
//
// jff 02/08/98 all cases with labels beginning with gen added to support
// generalized line type behaviors.

typedef struct
{
  thinker_t thinker;
  floor_e type;
  sector_t __far* sector;
  int16_t direction;
  int16_t texture;
  fixed_t floordestheight;
  fixed_t speed;

} floormove_t;

static void T_MoveFloor(floormove_t __far* floor)
{
  result_e      res;

  // move the floor
  res = T_MovePlaneFloor(floor->sector, floor->speed, floor->floordestheight, floor->direction);

  if (!((int16_t)_g_leveltime&7))     // make the floormove sound
    S_StartSound2(&floor->sector->soundorg, sfx_stnmov);

  if (res == pastdest)    // if destination height is reached
  {
    if (floor->direction == 1)       // going up
    {
      switch(floor->type) // handle texture/type changes
      {
        case donutRaise:
          floor->sector->special  = 0;
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
  sector_t __far*     sec;
  floormove_t __far*  floor;

  secnum = -1;
  rtn = false;
  // move all floors with the same tag as the linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &_g_sectors[secnum];

    // Don't start a second thinker on the same floor
    if (sec->floordata != NULL)
      continue;

    // new floor thinker
    rtn = true;
    floor = Z_CallocLevSpec(sizeof(*floor));
    P_AddThinker (&floor->thinker);
    sec->floordata = floor; //jff 2/22/98
    floor->thinker.function = T_MoveFloor;
    floor->type = floortype;

    // setup the thinker according to the linedef type
    switch(floortype)
    {
      case lowerFloor:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindHighestFloorSurrounding(sec);
        break;

      case lowerFloorToLowest:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindLowestFloorSurrounding(sec);
        break;

      case turboLower:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED * 4;
        floor->floordestheight = P_FindHighestFloorSurrounding(sec);
        if (floor->floordestheight != sec->floorheight)
          floor->floordestheight += 8*FRACUNIT;
        break;

      case raiseFloor:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
        if (floor->floordestheight > sec->ceilingheight)
          floor->floordestheight = sec->ceilingheight;
        break;

      case raiseFloorToNearest:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindNextHighestFloor(sec);
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

boolean EV_BuildStairs(const line_t __far* line)
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
   if (sec->floordata == NULL) {
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

    // set up the speed and stepsize
    speed = FLOORSPEED/4;
    stairsize = 8*FRACUNIT;
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
        if (tsec->floordata != NULL)
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
    if (s1->floordata != NULL)
      continue;

    s2 = getNextSector(s1->lines[0],s1);  // s2 is pool's sector
    if (!s2) continue;                    // note lowest numbered line around
                                          // pillar must be two-sided

    /* do not start the donut if the pool is already moving
     * cph - DEMOSYNC - was !compatibility */
    if (s2->floordata != NULL)
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
      floor->direction = 1;
      floor->sector = s2;
      floor->speed = FLOORSPEED / 2;
      floor->texture = s3->floorpic;
      floor->floordestheight = s3->floorheight;

      //  Spawn lowering donut-hole pillar
      floor = Z_CallocLevSpec(sizeof(*floor));
      P_AddThinker (&floor->thinker);
      s1->floordata = floor; //jff 2/22/98
      floor->thinker.function = T_MoveFloor;
      floor->type = lowerFloor;
      floor->direction = -1;
      floor->sector = s1;
      floor->speed = FLOORSPEED / 2;
      floor->floordestheight = s3->floorheight;
      break;
    }
  }
  return rtn;
}
